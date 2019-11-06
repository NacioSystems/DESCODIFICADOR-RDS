/*
 * Librería realizada por Ignacio Otero en 2018 a partir
 * del ejemplo de SparkFun Si4703 de 2011, de Nathan Seidie
 * 
 * Entre las modificaciones especiales está la utilizacion
 * de las interrupciones para la lectura del RDS, por ese
 * motivo se cambia la configuración de inicio del Si4703
 * 
 */

 #ifndef Si4703_h
 #define Si4703_h

 #include "Arduino.h"

 class Si4703 
 {
  public:
      Si4703(int resetPin, int sdioPin, int sclkPin, int intPin);
      uint16_t Registers[16]; 
      void Init();
      int seek(byte seekDirection);
      int readChannel();
      void gotoChannel(int newChannel);
      void setVolume(int volume, boolean nomute);

//Nombres de registros Si4703
      #define STATUSRSSI  0x0A
      #define RDSA        0x0C
      #define RDSB        0x0D
      #define RDSC        0x0E
      #define RDSD        0x0F      

//Register 0x06 - SYSCONFIG3
    #define VOLEXT 8  // el bit 0x00010000 a 1 pone el volumen a -28dBs

//Register 0x02 - POWERCFG
    #define DMUTE 14 // bit desactivación Mute
    
//Direction used for seeking. Default is down
    #define SEEK_DOWN 0 
    #define SEEK_UP   1
    
  private:
      int _resetPin;
      int _sdioPin;
      int _sclkPin;
      int _intPin;
      byte updateRegisters(void);
      void readRegisters(void);
      //void printRegisters(void)

      #define FAIL  0
      #define SUCCESS  1
      
//Nombres de registros Si4703
      #define DEVICEID 0x00
      #define CHIPID  0x01
      #define POWERCFG 0x02
      #define CHANNEL  0x03
      #define SYSCONFIG1  0x04
      #define SYSCONFIG2  0x05
      #define SYSCONFIG3  0x06
      #define READCHAN    0x0B
      
// Address de Si4703 en I2C      
      #define SI4703  0x10 //0b._001.0000 = I2C address of Si4703 - note that the Wire function assumes non-left-shifted I2C address, not 0b.0010.000W
      #define I2C_FAIL_MAX  10 //This is the number of attempts we will try to contact the device before erroring out

//Register 0x02 - POWERCFG
     #define SMUTE   15  // bit desactivación Soft Mute
     //static const uint8_t DMUTE =  14;  // bit desactivación Mute
     #define RDSM    11  // bit de modo RDS 1=Verbose
     #define SKMODE  10  // bit Seek Mode, continuo o parada 
     #define SEEKUP   9  // bit modo Seek, 1 Seek up
     #define SEEK     8  // bit activación Seek

     #define SEEK_DOWN 0 //Direction used for seeking. Default is down
     #define SEEK_UP   1 


//Register 0x03 - CHANNEL
     #define TUNE  15   // bit activación Tune

//Register 0x04 - SYSCONFIG1
    #define RDSIEN  15  // Activa si es 1 las interrupciones cuando hay RDS por GPIO2
    #define STCIEN  14  // Activa las interrupciones cuando completa Seek Tune GPIO2
    #define RDS     12  // bit activación RDS
    #define DE      11  // bit aconfiguración denfasis, EurOpa 1 50us
    #define GPIO2b1  2  // GPIO[1:0]=01

//Register 0x05 - SYSCONFIG2
    #define SPACE1  5  // bits [5:4] de ajusta banda
    #define SPACE0  4  // Europa 50KHz, 01

//Register 0x0A - STATUSRSSI // Por ahora se ven fuera
    #define RDSR  15  // bit RDS Ready
    #define STC   14  // bit Seek Tone Complete
    #define SFBL  13  // bit Seek Fail Band Limit
    #define AFCRL 12  // bit validadción canal
    #define RDSS  11  // bit RDS sincronizado
    #define ST     8  // bit indicador Stereo

 };

#endif
 
