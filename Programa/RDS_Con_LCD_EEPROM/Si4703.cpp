/*
 * Librería realizada por Ignacio Otero en 2018 a partir
 * del ejemplo de SparkFun Si4703 de 2011, de Nathan Seidie
 * 
 * Entre las modificaciones especiales está la utilizacion
 * de las interrupciones para la lectura del RDS, por ese
 * motivo se cambia la configuración de inicio del Si4703
 * 
 */


#include "Arduino.h"
#include "Si4703.h"
#include "Wire.h"

// Definimos la clase Si4703
Si4703::Si4703(int resetPin, int sdioPin, int sclkPin, int intPin) {
// En la definición de la clase introducimos los pines de control
     _resetPin = resetPin;
     _sdioPin = sdioPin;
     _sclkPin = sclkPin;
     _intPin = intPin;
}

//Given a channel, tune to it
//Channel is in MHz, so 973 will tune to 97.3MHz
//Note: gotoChannel will go to illegal channels (ie, greater than 110MHz)
//It's left to the user to limit these if necessary
//Actually, during testing the Si4703 seems to be internally limiting it at 87.5. Neat.
void Si4703::gotoChannel(int newChannel) {
  //Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
  //97.3 = 0.2 * Chan + 87.5
  //9.8 / 0.2 = 49
  newChannel *= 10; //973 * 10 = 9730
  newChannel -= 8750; //9730 - 8750 = 980
  newChannel /= 10; //980 / 10 = 98


  //These steps come from AN230 page 20 rev 0.5
  readRegisters();
  Registers[CHANNEL] &= 0xFE00; //Clear out the channel bits
  Registers[CHANNEL] |= newChannel; //Mask in the new channel
  Registers[CHANNEL] |= (1 << TUNE); //Set the TUNE bit to start
  updateRegisters();

  //Poll to see if STC is set
  while (1) {
    readRegisters();
    if ( (Registers[STATUSRSSI] & (1 << STC)) != 0) break; //Tuning complete!
    }

  readRegisters();
  Registers[CHANNEL] &= ~(1 << TUNE); //Clear the tune after a tune has completed
  updateRegisters();

  //Wait for the si4703 to clear the STC as well
  while (1) {
    readRegisters();
    if ( (Registers[STATUSRSSI] & (1 << STC)) == 0) break; //Tuning complete!
    }
}

//Reads the current channel from READCHAN
//Returns a number like 973 for 97.3MHz
int Si4703::readChannel(void) {
  readRegisters();
  int channel = Registers[READCHAN] & 0x03FF; //Mask out everything but the lower 10 bits
  //Freq(MHz) = 0.100(in Europe) * Channel + 87.5MHz
  //X = 0.1 * Chan + 87.5
  channel += 875; //98 + 875 = 973
  return (channel);
}

//Seeks out the next available station
//Returns the freq if it made it
//Returns zero if failed
int Si4703::seek(byte seekDirection) {
  readRegisters();

  //Set seek mode wrap bit
  //si4703_registers[POWERCFG] |= (1<<SKMODE); //Allow wrap
  Registers[POWERCFG] &= ~(1 << SKMODE); //Disallow wrap - if you disallow wrap, you may want to tune to 87.5 first

  if (seekDirection == SEEK_DOWN) Registers[POWERCFG] &= ~(1 << SEEKUP); //Seek down is the default upon reset
  else Registers[POWERCFG] |= 1 << SEEKUP; //Set the bit to seek up

  Registers[POWERCFG] |= (1 << SEEK); //Start seek

  updateRegisters(); //Seeking will now start

  //Poll to see if STC is set
  while (1) {
    readRegisters();
    if ((Registers[STATUSRSSI] & (1 << STC)) != 0) break; //Tuning complete!
    }

  readRegisters();
  int valueSFBL = Registers[STATUSRSSI] & (1 << SFBL); //Store the value of SFBL
  Registers[POWERCFG] &= ~(1 << SEEK); //Clear the seek bit after seek has completed
  updateRegisters();

  //Wait for the si4703 to clear the STC as well
  while (1) {
    readRegisters();
    if ( (Registers[STATUSRSSI] & (1 << STC)) == 0) break; //Tuning complete!
    }

  if (valueSFBL) { //The bit was set indicating we hit a band limit or failed to find a station
    return (FAIL);
  }

  //Serial.println("Seek complete"); //Tuning complete!
  return (SUCCESS); //readtChannel(); // integuer
}

//To get the Si4703 inito 2-wire mode, SEN needs to be high and SDIO needs to be low after a reset
//The breakout board has SEN pulled high, but also has SDIO pulled high. Therefore, after a normal power up
//The Si4703 will be in an unknown state. RST must be controlled
void Si4703::Init(void) {
  Serial.println(F("Inicializando I2C y Si4703"));

  // Configuramos para I2C
  pinMode(_resetPin, OUTPUT);
  pinMode(_sdioPin, OUTPUT); //SDIO is connected to A4 for I2C
  digitalWrite(_sdioPin, LOW); //A low SDIO indicates a 2-wire interface
  digitalWrite(_resetPin, LOW); //Put Si4703 into reset
  delay(1); //Some delays while we allow pins to settle
  digitalWrite(_resetPin, HIGH); //Bring Si4703 out of reset with SDIO set to low and SEN pulled high with on-board resistor
  delay(1); //Allow Si4703 to come out of reset

  Wire.begin(); //Now that the unit is reset and I2C inteface mode, we need to begin I2C

// Activamos el oscilador local
  readRegisters(); //Read the current register set
  Registers[0x07] = 0x8100; // 0x10010000 Enable the oscillator, from AN230 page 9, rev 0.61 (works)
  updateRegisters(); //Update
  delay(500); //Wait for clock to settle - from AN230 page 9

// Desactivamos mute y activamos chip
  readRegisters(); //Read the current register set
  Registers[POWERCFG] = 0x4001; // 0b01000001 Mute desactivado y Activa chip
  //  si4703_registers[POWERCFG] |= (1<<SMUTE) | (1<<DMUTE); //Disable Mute, disable softmute

//REGISTRO SYSCONGI1
  Registers[SYSCONFIG1] |= (1 << RDSIEN);   //Enable int RDS por GPIO2
  //Registers[SYSCONFIG1] |= (1 << STCIEN); //Enable int Seek / Tune
  Registers[SYSCONFIG1] |= (1 << RDS);      //Enable RDS
  Registers[SYSCONFIG1] |= (1 << GPIO2b1);  //Ponemos GPIO2 como salida de interrupciones
  //Registers[SYSCONFIG1] |= (1 << RDSM);   //Ponemos RDS en modo Verbose
  Registers[SYSCONFIG1] |= (1 << DE);       //50kHz Europe setup
  Registers[SYSCONFIG2] |= (1 << SPACE0);   //100kHz channel spacing for Europe

//REGISTRO SYSCONGI2 y SYSCONFIG3
  Registers[SYSCONFIG2] &= 0xFFF0; //Clear volume bits
  Registers[SYSCONFIG2] |= 0x000A; //Set volume a 10
  Registers[SYSCONFIG3] |= (1 << VOLEXT); //Set volume to Extended
  updateRegisters(); //Update todos los registros modificados

  delay(110); //Max powerup time, from datasheet page 13
}

//Write the current 9 control registers (0x02 to 0x07) to the Si4703
//It's a little weird, you don't write an I2C addres
//The Si4703 assumes you are writing to 0x02 first, then increments
byte Si4703::updateRegisters(void) {

  Wire.beginTransmission(SI4703);
  //A write command automatically begins with register 0x02 so no need to send a write-to address
  //Sólo se actualizan los regisros en los que se puede escribir, del 0x02 al 0x07
  //First we send the 0x02 to 0x07 control registers
  //In general, we should not write to registers 0x08 and 0x09
  for (int regSpot = 0x02 ; regSpot < 0x08 ; regSpot++) {
    byte high_byte = Registers[regSpot] >> 8;
    byte low_byte = Registers[regSpot] & 0x00FF;

    Wire.write(high_byte); //Upper 8 bits
    Wire.write(low_byte); //Lower 8 bits
    }

  //End this transmission
  byte ack = Wire.endTransmission();
  if (ack != 0) { //We have a problem!
    Serial.print(F("Write Fail:")); //No ACK!
    Serial.println(ack, DEC); //I2C error: 0 = success, 1 = data too long, 2 = rx NACK on address, 3 = rx NACK on data, 4 = other error
    return (FAIL);
  }

  return (SUCCESS);
}

//Read the entire register control set from 0x00 to 0x0F
void Si4703::readRegisters(void) {
  //Son 16 registros de 16 bits, a 400Kbps tardaría unos 640 microsegundos ¡MUCHO TIEMPO!
  //Hay que intetar leer y escribir lo mínimo.
  //Si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
  //La librería wire solo permite leer datos de byte, de 8 en 8 bits, y son 16 registros de 16 bits
  Wire.requestFrom(SI4703, 32); //We want to read the entire register set from 0x0A to 0x09 = 32 bytes.

  while (Wire.available() < 32) ; //Wait for 16 words/32 bytes to come back from slave I2C device
  //We may want some time-out error here

  //Remember, register 0x0A comes in first so we have to shuffle the array around a bit
  for (int x = 0x0A ; ; x++) { //Read in these 32 bytes
    if (x == 0x10) x = 0; //Loop back to zero
    Registers[x] = Wire.read() << 8;
    Registers[x] |= Wire.read();
    if (x == 0x09) break; //We're done!
  }
}

//Pone el volumen entre 0 y 31 teniendo en cuenta VOLEXT
void Si4703::setVolume(int volume, boolean nomute)
{
  readRegisters(); //Read the current register set
  if(nomute) {
      if(volume < 0) volume = 0;
      if (volume > 30) volume = 30;
      if (volume < 16) // Extendido es para volumen bajo
           {
              Registers[SYSCONFIG3] |= (1 << VOLEXT); //Activamos Extended
            }
          else // Si es más de 16 hay que anular el extendido
            {
              Registers[SYSCONFIG3] &= ~(1 << VOLEXT);
              volume = volume - 15;
            }
      Registers[SYSCONFIG2] &= 0xFFF0; //Clear volume bits
      // Cuando está VOLEXT desactivado, al poner 0 se hace un mute
      // para evitar el mute se pone a 1.
      if (volume==16) Registers[SYSCONFIG2] |= 1; //Set new volume
      else Registers[SYSCONFIG2] |= volume;
      Registers[POWERCFG] |= (1 << DMUTE); //Activa
      }
 else {
       Registers[POWERCFG] &= ~(1 << DMUTE); //Desactiva
       }
 updateRegisters();
 }


/*
// PRINT REGISTERS no tiene más valor que para desarrollo
void Si4703::printRegisters(void) {
  Read back the registers
  si4703_readRegisters();

  //Print the response array for debugging
  for (int x = 0 ; x < 16 ; x++) {
    sprintf(printBuffer, "Reg 0x%02X = 0x%04X", x, si4703_registers[x]);
    Serial.println(printBuffer);
  }
}
*/

