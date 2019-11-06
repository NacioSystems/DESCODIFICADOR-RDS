/*
  Modificado por Ignacio Otero para obtener datos de RDS en enero 2018
  Referencias:
  - Spark Fun
  1-6-2011
  Spark Fun Electronics 2011
  Nathan Seidle

  - CENELEC 50067 / 1998

  - J_RPM WWW.YOUTUBE.COM/
  https://www.youtube.com/watch?v=Hiti3FGuOgI&list=PLu7bTAqPV8_y0-5Ok_vPMW8Hp26_XmIxC

  This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  To use this code, connect the following 5 wires:
  Arduino : Si470x board
  3.3V : VCC
  GND : GND
  A5 : SCLK
  A4 : SDIO
  A3 : RST
  D3  SDIO2 (Int 1)

  Look for serial output at 57600bps.

  The Si4703 ACKs the first byte, and NACKs the 2nd byte of a read.

  1/18 - after much hacking, I suggest NEVER write to a register without first reading the contents of a chip.
  ie, don't updateRegisters without first readRegisters.

  If anyone manages to get this datasheet downloaded
  http://wenku.baidu.com/view/d6f0e6ee5ef7ba0d4a733b61.html
  Please let us know. It seem to be the latest version of the programming guide. It had a change on page 12 (write 0x8100 to 0x07)
  that allowed me to get the chip working..


 Si4703 CUIDADO! ESTE CHIP SÓLO ADMITE NIVELES DE 3,3V
--------------------------------------------------------------------

En los puertos están disponibles, A1, A2 y D0, D1 (RX / TX)
*/

#include "Configuracion.h" // Están todas las definiciones del programa
#include <Wire.h>
#include <EEPROM.h>       // Guardamos los datos de las memorias en la EEPROM

// Definimos un objeto RADIO de la clase Si4703 con las siguentes conexiones
// Si4703(int resetPin, int sdioPin, int sclkPin, int intPin);
#include "Si4703.h"
Si4703 RADIO(resetPin, SDIO, SCLK, GPIO2);

// Definimos un objeto lcd de la clse LiquidCrystal   
#include <LiquidCrystal.h>
LiquidCrystal lcd(RS, ENABLE, DB4, DB5, DB6, DB7);


// VARIABLES GENERALES
             int ModoPantalla = 1;  // Empieza el LCD con la pantalla principal
             boolean ModoConfiguracion = false;


             int Tecla = btnNONE; // Por defecto la tecla leida será 5

             char printBuffer[50]; // Utilizamos este string para formateo de impresión
             uint16_t si4703_registers[16]; //Son los 16 registros de Si4703, cada uno de 16 bist


             // Variables RDS en Grupo 0A
             uint16_t Pi = 0x00; // PI Program Station Identification Ah + Al
             uint8_t GP = 0; // Program Group Bh, b5-b8
             char Bcero = 'A'; // Es el tipo de grupo A=0 o B=1 en Bh bit 4
             boolean TP = 0, TA = 0; // Traffic Program b3 Bh y Traffic Anouncement b5 Bl
             uint8_t PTY = 0; // Program Type Bh + Bl b6-b8, 5 bits
             //String TextoPTY = "Indefinido"; // Esto lo hago para poner el texto de programa
             boolean MS = 0; // Modo de programa 0 Speech 1 Music
             String MSTexto = "_____"; // En vez de poner el bit pongo Speech o Music
             boolean DI = 0, PTYdinamico = 0, Comprimido = 0, Artificial = 0, Estereo = 0;
             uint8_t Control = 0; // 2 bits C1 y C0 para decodificar PS
             char PS[9] = "________\0"; // Program Station Name Ch + Cl en Grupo 0A
             uint8_t AF[26]; // Tabla de frecuencias alternativas en Grupo 0A
             uint8_t NAF = 0; // Número de AF en Grupo 0A
             int ContadorAF = 0; // Vamos contando AF hasta el máximo de 26 o código 0xCD

             // Variables del Grupo 2A 2B radio texto
             String RTA = "_______________________________________________________________"; // Radio texto en grupo A, 4 bytes en C y D
             String RTB = "_______________________________________________________________"; //Radio texto en grupo B, 2 bytes en D
             boolean TextAB = 0; // bit 5 de Bl para G2, para leer RT A o B
             boolean BanderaAB = 0; // Para comprobar que TextAB cambia

             // Varialbes del grupo 4A horas y minutos
             uint8_t Horas=0;
             uint8_t Minutos=0;
             uint8_t Segundos=0;
             int UTC=0;            // UTC puede ser negativo
             unsigned long MJD=0;  // MJD ocupa 17 bits, hacen falta 4 bytes
             uint8_t Dia=0;
             uint8_t Mes=0;
             uint16_t Ano=0;       // Es más de 256
             uint8_t ContDia=0;
             
             const char* TextoPTY[32] = {"None",    // 0
                                         "News",    // 1
                                         "Affairs", // 2
                                         "Info",    // 3
                                         "Sport",   // 4
                                         "Educate", // 5
                                         "Drama",   // 6
                                         "Culture", // 7
                                         "Science", // 8
                                         "Varied",  // 9
                                         "Pop M",   //10
                                         "Rock M",  //11
                                         "Easy M",  //12
                                         "Light M", //13
                                         "Classics",//14
                                         "Other M", //15
                                         "Weather", //16
                                         "Finance", //17
                                         "Children",//18
                                         "Social",  //19
                                         "Religion",//20
                                         "Phone In",//21
                                         "Travel",  //22
                                         "Leisure", //23
                                         "Jazz",    //24
                                         "Country", //25
                                         "Nation M",//26
                                         "Oldies",  //27
                                         "Folk M",  //28
                                         "Document",//29
                                         "TEST",    //30
                                         "Alamr!"   //31
                                         };
                                          
             const char* DiaSem[7]={"lunes","martes","miercoles","jueves","viernes","sabado","domingo"}; // Para que funcione tiene que ser puntero de Char

             // Grupos
             uint16_t GrupoA[16]; // Es para contar los grupos recibidos
             uint16_t GrupoB[16]; 
             uint16_t ContadorGrupo = 0; // Cuenta el total de grupos recibidos

             // Valores leidos de Si4703 en variables
             byte rssi = 0; // Nivel recibido en registro STATUSRSSI[0:7]
             // Son incrementos de 1dBuV, máximo 75 dBuV

             // Control del volumen extendido
             boolean VolumenExtendido = 0; // Por defecto=0;
             uint8_t Volumen = 10; // El máximo sería 31, 5 bits
             boolean Mute=true;
             // Si es menor que 17 sería Extendido
             uint8_t VolumenAnterior=Volumen;

             // Brillo pantalla lcd
             uint8_t Brillo=50;
             
             // Variables utilizadas en las interrupciones 
             volatile int Contador = 875; // La utilizaremos para la sintonia 875 - 1080
                                          // el volumen 0 - 31
                                          // Brillo del display 0 - 255
             volatile boolean RDSOK = false; // Nos dirá por interrupción si hay datos RDS
             boolean ESTEREOK = true; // El Si4703 nos da información de si la señal es estero                             

             // Memorias prefijadas 
             struct ObjetoMemoria {                      // Cada memoria ocuparía 9 + 2 + 2 = 13 bytes
                                    char     PS[9];      // Es el nombre de la emisora memorizada, poner null en el 9
                                    uint16_t Frecuencia; // Es la frecuencia en cientos de KHz, ejem. 875 es 87.5
                                    uint16_t Pi;         // Es la Identificación de Programa en hexadecimal
                                    };
             
             ObjetoMemoria Memorias;                     // En un array de 10 memorias metemos los datos de las memorias
                                                         // Ocuparían 13 x 10 = 130 bytes
                                                         // Los parámetros se llamarían por Memorias[i].Frecuencia...
                                                         
             int8_t ContadorMemorias = 5; // Cadena 100
             unsigned long tiempo = 0;
             unsigned long TSeg=0;
             boolean BuzzerON=true;

 
void setup() {
  // Inicializamos Interrupciones y Encoders
  pinMode(encoderA, INPUT_PULLUP); // Van conectados directamente
  pinMode(encoderB, INPUT_PULLUP); // a 5V, por eso pullup
  pinMode(GPIO2, INPUT_PULLUP);    // Aunque en la configuraión de GPIO ya esta a pull_up
  pinMode(resetPin, OUTPUT);       // Utilizamos A4 como digital esto ya lo hace init()
  pinMode(BuzzerPin, OUTPUT);      // La salida de D11 será la señal buzzer
  attachInterrupt(0, LeerEncoder, CHANGE); // Interrupción 0 para encoder
  attachInterrupt(1, DatosRDS, FALLING);   // Interrupcion 1 para RDS 
   
  
  // Inicializar el LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Ignacio Otero");
  lcd.setCursor(0, 1);
  lcd.print("Desco RDS 2018");
  delay(2000);
  lcd.noDisplay();
  delay(2000);
  lcd.display();

  pinMode(STATUS_LED, OUTPUT);
  pinMode(BOTONES, INPUT); // Leemos el teclado por A0
  pinMode(A1, INPUT); // Optional trimpot for analog station control
  pinMode(BLACK_TILT_CONTROL, OUTPUT);
  
  Serial.begin(57600);
  Serial.println();
  Serial.println(F("Ignacio Otero"));
  Serial.println(F("Desco RDS 2018"));
  for(int i=255; i>=0; i--) {
    analogWrite(BLACK_TILT_CONTROL, i);
    delay(10);
    }
   
/*
//**********************************************************************
// Todo esto lo enmarcaremos para comentar
// Volcamos datos en La EEPROM la primera vez
// Después sólo guardamos datos cuando se cambia una memoria
// Primero cargamos la informacion en la variable Memorias
//                            PS         Frec.       Pi
            Memorias[0] = {{'R','N','E',' ','1'},             1031,    0xE211}; // 1 RNE1
            Memorias[1] = {{'R','N','E',' ','2'},              981,    0xE212}; // 2 RNE2
            Memorias[2] = {{'R','N','E',' ','3'},              990,    0xE213}; // 3 RNE3
            Memorias[3] = {{'R','.','G','A','L','E','G','A'},  962,    0xE4E0}; // 4 RG
            Memorias[4] = {{'C','A','D','-','1','0','0'},      889,    0xE2CE}; // 5 C100
            Memorias[5] = {{'R','G','M','U','S','I','C','A'}, 1042,    0xE4E1}; // 6 RG MUSICA
            Memorias[6] = {{'L','O','S',' ','4','0'},          906,    0xEB19}; // 7 LOS 40
            Memorias[7] = {{'R','A','D','I','O','V','O','Z'}, 1061,    0xE2A2}; // 8 Radio Voz
            Memorias[8] = {{'K','I','S','S',' ','F','M'},      894,    0xE2EC}; // 9 KISS
            Memorias[9] = {{'C','O','P','E'},                 1021,    0xEBAB}; // 10 COPE
// Terminamos todos los PS con el carácter null
for(int i=0; i<10; i++) Memorias[i].PS[8]='\0';

// Despues pasamos estos datos a la EEPROM, sólo la primera vez
int EPA = 0; // EPA EEPROM ADDRESS = 0;
for(int i=0; i<10; i++) {
  EEPROM.put(EPA, Memorias[i]); // Leemos cada Memoria de su valor en EEMPROM
  EPA += sizeof(ObjetoMemoria); // Saltamos 12 bytes de cada vez
  }
                                     
//***********************************************************************
*/

// Cargamos las memorias de la EEPROM
// En cada reinicio
int EPA = 0; // EEPROM address
Serial.println(F("\n\nLeemos los valores de las memorias de la EEPROM"));
Serial.println(F("M      PS   \tFREC\tPI"));
for(int i=0; i<10; i++) {
  EEPROM.get(EPA, Memorias); // Leemos cada Memoria de su valor en EEMPROM
  EPA += sizeof(ObjetoMemoria); // Saltamos 12 bytes de cada vez
  sprintf(printBuffer, "%02d- %08s\t%04d\t%04X ",i+1, Memorias.PS, Memorias.Frecuencia, Memorias.Pi); //currentChannel / 10, currentChannel % 10);
  Serial.println(printBuffer);
  }
      
  RADIO.Init(); // Inicializaición Si4703 y protocolo 2 hilos Wire

  // Aviso musical, Do Re Mi
  tone(BuzzerPin,261,100);
  delay(100);
  tone(BuzzerPin,294,100);
  delay(100);
  tone(BuzzerPin,329,100);
  
  for(int i=0;i<Brillo;i++) {
    analogWrite(BLACK_TILT_CONTROL, i);
    digitalWrite(STATUS_LED, HIGH);
    delay(50);
    digitalWrite(STATUS_LED, LOW);
    i+=4;
    }

  TSeg=millis();

}

void loop() {
  char option;
  char vol = 10;
  Contador=vol;
  char VCH=' ';
  uint16_t currentChannel = 889; //Default the unit to a known good local radio station
  tiempo = millis();
 /* // borrar Scanner por plotter serie
   while (1) {
           for(int i=875;i<1080;i++) {
             if (Serial.available())
                   if (Serial.read() == 'x') break;
             RADIO.gotoChannel(i);
             si4703_readChannel();
             rssi = RADIO.Registers[STATUSRSSI] & 0x00FF; //Mask in RSSI
             Serial.println(rssi, DEC);
             delay(100);
             }
             delay(5000);
           }  

*/ // borrar

  RADIO.gotoChannel(currentChannel);

  // Leemos los datos de RDS antes de presentar en pantalla
  
  while (1) {
    if(RDSOK) LecturaRDS();  // Cada vez que pulsamos una tecla leemos los datos de RDS antes de 
    DatosBasicos(); // Poner datos en pantalla.

    Serial.println(F("Teclea opcion ?"));

    while (!Serial.available()) // Está en este blucle salvo que por Serial pasemos una tecla
    {
      if(RDSOK) LecturaRDS();// Solo leemos los datos si se hay interrupción
      Tecla=LeerBotonera();  // el encoder controla el volumen
      if(Contador>32) Contador=32;
      if(Contador<0) Contador=0;
      if(Contador!=vol) {     // Si movoemos el encoder
           ModoPantalla=14;   // Activamos la pantalla de volumen
           DysplayLCD();
           vol=Contador;
           ModoPantalla=1;    // Volvemos a la pantalla principal
          }
      if(Tecla!=btnNONE)
        {
           
           switch(Tecla)
           {
              case btnDOWN:  // Cambiamos display a siguiente
                 ModoPantalla++;
                 lcd.clear();
                 if((ModoPantalla>8)&&(ModoConfiguracion==0)) ModoPantalla=1;
                 else if((ModoPantalla>13)&&(ModoConfiguracion==1)) ModoPantalla=9;
                 break;

              case btnUP:   // Cambiamos display a anterior
                 ModoPantalla--;
                 lcd.clear();
                 if(ModoPantalla<1&&(ModoConfiguracion==0)) ModoPantalla=8;
                 else if((ModoPantalla<1)&&(ModoConfiguracion==1)) ModoPantalla=13;
                 break;

              case btnLEFT: // Escaneamos hacia abajo
                   if(ModoPantalla==2) {
                          Sintonia(-1);
                          break;
                   }
                   else if(ModoPantalla==5)
                      {
                        for(int i=0;i<16;i++) {
                           lcd.scrollDisplayRight();
                           delay(200);
                            }
                        break;
                      }
                   LimpiaRDS();
                   RADIO.seek(SEEK_DOWN);
                   break;
              

              case btnRIGHT: // Escaneamos hacia arriba
                   if(ModoPantalla==2) {
                          Sintonia(1);
                          break;
                       }
                   else if(ModoPantalla==5)
                      {
                        for(int i=0;i<16;i++) {
                            lcd.scrollDisplayLeft();
                            delay(200);
                            }
                        break;
                      }
                   LimpiaRDS();
                   RADIO.seek(SEEK_UP);
                   break;
                   
              case btnSELECT: // Modo configuracion
                   ModoPantalla=9;
                   ModoConfiguracion=!ModoConfiguracion;
                   break;


              default:
                  break; // Nada
           }
        delay(500); // Cuando terminamod de pulsar una tecla espera 500ms  
        }
      

      // Cada segundo actualizamos el tiempo
      // Aprovechamos para leer los datos de Si4703
      // ajustamos canal
      // ajustamos bit de estereo  
      if((millis()-TSeg)>1000)
        {
          // Leemos la frecuencia sintonizada
          currentChannel=RADIO.readChannel(); // Leemos el valor sintonizado

          // Activamos desactivamos el Led de estereo
          digitalWrite(STATUS_LED, ESTEREOK=RADIO.Registers[STATUSRSSI] & (1 << ST)); 

          // Actualizamos pantalla lcd
          DysplayLCD();
          tiempo=millis();

          // Actualizamos hora
          Segundos++;
          if(Segundos>59) {
              Segundos=0;
              Minutos++;
              if (Minutos>59){ 
                Minutos=0;
                Horas++;
                if(Horas>23) Horas=0;
              }
              
          }
          TSeg=millis();
        }
    }

    option = Serial.read();

    if (option == '0')  {
      String Opcion[4] = "";
      int i = 0;
      int mil=1000;
      uint16_t canal=0;
      LimpiaRDS();
      Serial.println(F("Teclea frecuencia sin puntos cuatro cifras: Ejem 0875"));
      while (1) {
        while (!Serial.available()) {
          //Nada;
          }
        option = Serial.read();
        if (option == char(13)) break;
        canal+=(option-48)*mil;
        mil/=10;
        Opcion[i] = option;
        i++;
        if (i > 3) break;
        }
        if(i==2) canal/=10;
      Serial.print(F("Cambiado a canal: "));
      Serial.println(canal);
      LimpiaRDS();
      RADIO.gotoChannel(canal);
      Serial.println(F(" "));
      if(canal>1080) canal = 1080;
      if(canal<870) canal = 870;
      Serial.println(canal);
    }
    else if (option == '1')  {
      LimpiaRDS();
      Serial.print(F("\nMemoria: "));
      Serial.println(ContadorMemorias + 1);
      int EPA = ContadorMemorias * sizeof(ObjetoMemoria); // Saltamos 12 bytes de cada vez
      EEPROM.get(EPA, Memorias); // Leemos cada Memoria de su valor en EEMPROM
      currentChannel = Memorias.Frecuencia;
      ContadorMemorias++;
      if (ContadorMemorias > 9) ContadorMemorias = 0;
      RADIO.gotoChannel(currentChannel);
    }
    else if (option == '2') {
      Serial.println(F("Mute toggle"));
      Mute=!Mute;
      RADIO.setVolume(vol,Mute); // Cuando es false no afecta el volumen
      }
    else if (option == '3') {
      Serial.println(F("\n\nContenido memorias:"));
      // Leemos las memorias de la EEPROM
      int EPA = 0; // EEPROM address
      Serial.println(F("Leemos los valores de las memorias de la EEPROM"));
      Serial.println(F("M      PS   \tFREC\tPI"));
      for(int i=0; i<10; i++) {
        EEPROM.get(EPA, Memorias); // Leemos cada Memoria de su valor en EEMPROM
        EPA += sizeof(ObjetoMemoria); // Saltamos 12 bytes de cada vez
        sprintf(printBuffer, "%02d- %08s\t%04d\t%04X ",i+1, Memorias.PS, Memorias.Frecuencia, Memorias.Pi); //currentChannel / 10, currentChannel % 10);
        Serial.println(printBuffer);
        }
        Serial.print(F("Memoria actual seleccionada: "));
        Serial.println(ContadorMemorias);
    }
    else if (option == '4') {
      LimpiaRDS();
      RADIO.seek(SEEK_UP);
    }
    else if (option == '5') {
      LimpiaRDS();
      RADIO.seek(SEEK_DOWN);
    }
    else if (option == '6') {
      Serial.println(F("\n\n\n\n********************************************************************************************"));
      Serial.println(F("DESCARGA INFORMACIÓN RDS:"));
      DatosBasicos();
      PantallaSerial();

    }
    else if (option == 'v') {
      Serial.println();
      Serial.print(F("Volumen:"));
      Serial.print(Volumen);
      Serial.print(F(" EXT="));
      Serial.println(VolumenExtendido, BIN);
      Serial.println(F("+) Up"));
      Serial.println(F("-) Down"));
      Serial.println(F("x) Exit"));
      while (1) {
        if (Serial.available()) {
          option = Serial.read();
          if ((option == '+')||(option=='-')) {
            Volumen=ActuacionVolumen(option, btnNONE);
            Serial.print(F("Volumen: "));
            Serial.print(Volumen);
            Serial.println(F(" "));
          }
          else if (option == 'x') break;
        }
      }
    }
    else if (option == 'w') {
      LimpiaRDS();
      Sintonia(1);
     }
    else if (option == 's') {
      LimpiaRDS();
      Sintonia(-1);
    }
    else if(option == '?') {      
      Serial.println(F("Configurado para Europa"));
      Serial.println(F("0) Introducir canal"));
      Serial.println(F("1) Siguiente Memoria"));
      Serial.println(F("2) Mute On/Off"));
      Serial.println(F("3) Lectura memorias"));
      Serial.println(F("4) Seek up"));
      Serial.println(F("5) Seek down"));
      Serial.println(F("6) Descarga datos RDS"));
      Serial.println(F("v) Volumen"));
      Serial.println(F("w) Sintonia up"));
      Serial.println(F("s) Sintonia down"));
      Serial.println(F("?) Esta pantalla"));
      Serial.print(F(": "));
    }
    else {
      Serial.print(F("Eleccion = "));
      Serial.println(option);
    }
  }
}


// FUNCION LECTURA DATOS RDS
// Esta función es recomendable llamarla a través de interrupciones
// Hay que activar RDSIEN, para que por GPIO2[1:0]=01 se active la interrupción
// Sale un pulso LOW de 5ms
void LecturaRDS(void)
{
  // Leemos los registros para obtener los datos RDS
  // Comprobamos que hay RDS y dividimos en octetos
  RADIO.readChannel(); // Al leer la frecuencia leemos todos los registros

  // Si hay señal de RDS el bit RDSR se pone a 1 en 40ms
  
  byte Ah, Al, Bh, Bl, Ch, Cl, Dh, Dl;
  Ah = (RADIO.Registers[RDSA] & 0xFF00) >> 8;
  Al = (RADIO.Registers[RDSA] & 0x00FF);

  Bh = (RADIO.Registers[RDSB] & 0xFF00) >> 8;
  Bl = (RADIO.Registers[RDSB] & 0x00FF);

  Ch = (RADIO.Registers[RDSC] & 0xFF00) >> 8;
  Cl = (RADIO.Registers[RDSC] & 0x00FF);

  Dh = (RADIO.Registers[RDSD] & 0xFF00) >> 8;
  Dl = (RADIO.Registers[RDSD] & 0x00FF);

  // Si está en modo RDS standar, la lectura de ERRORES da siempre cero.
  // Sólo funciona en modo Verbose
  byte ErrorRDS = (RADIO.Registers[STATUSRSSI] & 0x0600) >> 9; //Errores de RDS A STATUSRSSI
    
  // COMÚN A TODOS LOS GRUPOS 
  // El error sólo funciona en modo Verbose 
  if(ErrorRDS<3) // Si hay mas de 3 a 5 errores no es capaz de recuperarlos
    {
    // Si hay RDS se pone a 1 la variable RDSOK, que borramos una vez leidos los datos
    Pi = (Ah << 8) | Al; // Sacamos PI que es el registro RDSA
    GP = Bh >> 4; // Grupo son los bits [8:5] de RDSB
    if ((Bh & 0x08) == 0) Bcero = 'A'; // Tipo de Grupo bit 4 de RDSB alta
    else Bcero = 'B';
    if(Bcero=='A') GrupoA[GP] = GrupoA[GP] + 1;
    else GrupoB[GP] = GrupoB[GP] + 1;
    ContadorGrupo = ContadorGrupo + 1;
    TP = (Bh & 0b0100) >> 2; // En RDSB parte alta bit 3
    PTY = ((Bh & 0b11) << 3) | ((Bl & 0b11100000) >> 5); // Bits [8:6] baja y [1:2] alta de RDSB
    // Hasta aquí son datos comunes a todos los grupos
    
 
    // GRUPO 0
    // PS PROGRAM SERVICE
    // Leemos los datos del PS, dos a dos según Control
    if (GP == 0) // Solo miramos grupo cero
    {
       Control = Bl & 0b11; // Bits [1:2] baja de RDSB
       DI = (Bl & 0b100) > 2; // Bit 3 de RDSB baja
       TA = (Bl & 0x0F) >> 4; // En RDSB parte baja bit 5
       MS = (Bl & 0x08) >> 3; // En RDSB parte baja bit 4
       if (MS)  MSTexto = "Music";
       else MSTexto = "Speech";
      
      // Toma los 8 carácteres a partir de Control de 0 a 7
      PS[Control * 2] = char(Dh);
      PS[Control * 2 + 1] = char(Dl);
      
      // Valores relaccionados con d0, d1, d2 y d3
      switch(Control){
           case 3:
            if (DI) Estereo = true; // Miramos si es dinámico o estático
            else Estereo = false;
            break; 
            
           case 2:
            if (DI) Comprimido = true; // Miramos si es dinámico o estático
            else Comprimido = false;
            break;
            
           case 1:
            if (DI) Artificial = true; // Miramos si es dinámico o estático
            else Artificial = false;
            break;

           case 0:
            if (DI) PTYdinamico = true; // Miramos si es dinámico o estático
            else PTYdinamico = false;
            break;
      }

      // AF ALTERNATIVE FRECUENCIES
      // Ahora leemos Frecuencias alternativas
      // La envía de 2 en dos en el paquete C Ch+Cl
      if (Bcero == 'A') // Tiene que ser el grupo 0A
      {
        if ((Ch >= 0xE0) & (Ch < 0xFA)) // Si empieza el listado de AF
        {
          ContadorAF = 0;
          NAF = Ch & 0x1F; // Eliminamos los bits sobrantes
          if (NAF > 0) // Hay frecuencias alternativas
          {
            AF[ContadorAF] = Cl;         // Leemos la portadora principal
            ContadorAF = ContadorAF + 1; // e incrementamos uno
          }
        }
        if (ContadorAF < NAF) // Mientras tengamos portadoras que leer
        {
          if (Ch < 0xCD) // Leemos los datos de las frecuencias
          {
            AF[ContadorAF] = Ch; // Leemos primer byte
            if (Cl < 0xCD) AF[ContadorAF + 1] = Cl; // Leemos segundo byte
            ContadorAF = ContadorAF + 2; // Incrementamos el Contador 2 veces
          }
        }
      }

    }

    // RADIO TEXTO en Grupo 2A
    // Leemos los datos del Radio texto, cuatro a cuatro o dos a dos
    // El texto está en bytes en Ch, Cl, Dh, DL si es A, según Bl[1:4]
    // Tambien está en bytes en Dh, Dl si es B,
    // El bit 5 de Bl indica que se debe borrar el texto actual en A o B
    if ((GP == 2) & (Bcero == 'A')) // Solo miramos grupo 2A
    {
      TextAB = (Bl & 0x10) >> 4; // Bit 5 (0b00010000 = 0x10) de Bl o de RDSB. Se utiliza en G2A
      // los comandos 0x0D indican final de linea y 0x0A final de linea
      if(TextAB!=BanderaAB) { 
            Serial.println(RTA);         
            for(int i=0; i<64; i++) {
                RTB[i]=RTA[i];
                RTA[i]=" ";
                }
            BanderaAB=TextAB;
            }
      
      // Los 4 bits MSF de Bl indican la posición
      RTA[((Bl & 0x0F) << 2)] = Ch;
      RTA[((Bl & 0x0F) << 2) + 1] = Cl;
      RTA[((Bl & 0x0F) << 2) + 2] = Dh;
      RTA[((Bl & 0x0F) << 2) + 3] = Dl;
      
      if((Ch==0x0D)||(Cl==0x0D)||(Dh==0x0D)||(Dl==0x0D)||
         (Ch==0x0A)||(Cl==0x0A)||(Dh==0x0A)||(Dl==0x0A)) {
          // Borramos el resto de caracteres de RTA
          for(int i=(Bl&0x0F)<<2+4;i<63; i++) { 
            RTA[i]="_";
            }
            //RTA[63]=0x00;
          }
          
    }

    if ((GP == 2) & (Bcero == 'B')) // Solo miramos grupo B
    {
      // RTB
      // Tiene que ser el grupo 0B
      // if (TextAB) Serial.print(F("Text A/B Flag!"));
      Serial.println(F("Recibiendo texto G2B, TextAB=1, ==> RTB"));
      // Bl&0x0F nos da los 4 bits MSB
      // la posición Dh,Dl el otro 1 bits
      // Bl&0x0F<<2 -> Ch
      // Bl&0x0F<<2 + 1 -> Cl
      RTB[((Bl & 0x0F) << 2)] = Dh;
      RTB[((Bl & 0x0F) << 2) + 1] = Dl;
    }

// GRUPO 4A FECHA Y HORA
if ((GP == 4) & (Bcero == 'A')) { // Solo miramos grupo 4A
      // Horas Dh(4 bits mas sig) y Cl (bit menos sig)
      Horas = ((Cl & 0x01) << 4) | (((Dh & 0xF0) >> 4)&0x0F);
      Minutos = ((Dh &0x0F) << 2) | ((Dl & 0b11000000) >> 6);
      UTC = Dl&0x1F;
      UTC = UTC/2; // Las medias horas las convertimos en horas
      if((Dl&0b00100000)==1) UTC = -UTC;
      Segundos=0;
      MJD = ((RADIO.Registers[RDSC]>>1)&0x7FFF)|(32768*(Bl&0x01))|(65536*(Bl&0x02));
      float Yp = (float)MJD - 15078.2;
      Yp = int(Yp/365.25);
      float Mp = (float)MJD - 14956.1;
      Mp = Mp -(Yp*365.25);
      Mp = (Mp / 30.6001);
      Mp = (int)Mp;
      Dia=MJD - 14956-(int)(Yp*365.25)-(int)(Mp*30.6001);
      int K=0;
      if((Mp==14)||(Mp==15)) K=1;
      Mes=Mp-1-K*12;
      Ano=1900+Yp+K;
      ContDia=((MJD+2)%7)+1;
      }
  }
RDSOK=false; // Desactivamos la variable para una nueva interrupción 
}


// FUNCION LIMPIA DATOS RDS
void LimpiaRDS(void)
{
  // Limpiamos los datos de RDS para nueva sintonía
  Pi = 0x00; // PI Program Station Identification Ah + Al
  GP = 0; // Program Group Bh, b5-b8
  TP = 0, TA = 0; // Traffic Program b3 Bh y Traffic Anouncement b5 Bl
  PTY = 0; // Program Type Bh + Bl b6-b8, 5 bits
  //TextoPTY = "________"; // Esto lo hago para poner el texto de programa
  MS = 0; // Modo de programa 0 Speech 1 Music
  MSTexto = "_____"; // En vez de poner el bit pongo Speech o Music
  DI = 0, PTYdinamico = 0, Comprimido = 0, Artificial = 0, Estereo = 0;
  Control = 0; // 2 bits C1 y C0 para decodificar PS
  for(int i=0;i<8;i++) PS[i] = '_';
  NAF = 0; // Número de AF en Grupo 0A
  // Vamos contando AF hasta el máximo de 26
  for (ContadorAF = 0; ContadorAF < 26; ContadorAF++)
  { // Ponemos a cero las AF
    AF[ContadorAF] = 0;
  }
  ContadorAF = 0;
  // Variables del Grupo 2A 2B radio texto
  RTA = "____________________________________________________________"; // Radio texto en grupo A, 4 bytes en C y D
  RTB = "____________________________________________________________"; //Radio texto en grupo B, 2 bytes en D

  // Grupos
  for (ContadorGrupo = 0; ContadorGrupo < 64; ContadorGrupo++)
  {
    GrupoA[ContadorGrupo] = 0; // Es para contar los grupos recibidos
    GrupoB[ContadorGrupo] = 0;
  }
  ContadorGrupo = 0; // Cuenta el total de grupos recibidos

  // Valores leidos de Si4703 en variable
  rssi = 0; // Nivel recibido en registro STATUSRSSI[0:7]
  // Son incrementos de 1dBuV, máximo 75 dBuV

  RDSOK=false; // Ponemos a false el estado de RDS
}


// DYSPLAY LCD 16x2
void DysplayLCD(void)
{
  int currentChannel = RADIO.readChannel(); // CurrentChannel lee los registros
  rssi = RADIO.Registers[STATUSRSSI] & 0x00FF; //Mascara de RSSI STATUSRSSI
  byte blockerrors = (RADIO.Registers[STATUSRSSI] & 0x0600) >> 9; //Mask in BLERA STATUSRSSI
  char opcion=' ';
  switch(ModoPantalla)
  {
    case 1: // PANTALLA PRINCIPAL FRECUENCIA, PI, PS Y NIVEL
              lcd.setCursor(0, 0);
              lcd.print(F("F:            dB"));
              lcd.setCursor(2, 0);
              sprintf(printBuffer, "%02d.%01dMHz ", currentChannel / 10, currentChannel % 10);
              lcd.print(printBuffer);
              lcd.setCursor(12, 0);
              lcd.print(rssi, DEC);
              //lcd.setCursor(15, 0);
              //lcd.print(blockerrors, DEC);
              
              lcd.setCursor(0,1);
              lcd.print(F("PI:             "));
              lcd.setCursor(3,1);
              lcd.print(Pi,HEX);
              lcd.setCursor(8,1);
              lcd.print(PS);
              break;
              
   case 2: //PANTALLA SECUNDARIA RDS TP, TA Y M/S
              lcd.setCursor(0, 0);
              lcd.print(F("F:        dB TP"));
              lcd.setCursor(2, 0);
              sprintf(printBuffer, "%02d.%01d ", currentChannel / 10, currentChannel % 10);
              lcd.print(printBuffer);
              lcd.setCursor(8, 0);
              lcd.print(F(" TP"));
              lcd.print(TP, BIN);
              lcd.print(F(" TA"));
              lcd.print(TA, BIN);
              
              lcd.setCursor(0, 1);
              lcd.print(F("M/S:            "));
              lcd.setCursor(4, 1);
              lcd.print(MS, BIN);
              lcd.setCursor(6, 1);
              if(MS) lcd.print(F("Music"));
              else lcd.print(F("Speech"));
              //lcd.print(MSTexto);
              break;
   
   case 3: // PANTALLA 3 RDS TPY
              lcd.setCursor(0, 0);
              lcd.print(F("PTY:           "));
              lcd.setCursor(4, 0);
              lcd.print(PTY);
              lcd.setCursor(7, 0);
              lcd.print(TextoPTY[PTY]);

              lcd.setCursor(0, 1);
              if(PTYdinamico) lcd.print(F("PTY dinam.,    "));
              else lcd.print(F("PTY estat.,    "));
              lcd.setCursor(13, 1);        
              if(Estereo) lcd.print(F("Est"));
              else lcd.print(F("Mno"));
              
              break;   

   case 4: // FRECUENCIAS ALTERNATIVAS
              Volumen=Contador;
              lcd.setCursor(0, 0);
              lcd.print(F("NAF=             "));
              lcd.setCursor(4, 0);
              lcd.print(NAF);
              lcd.setCursor(14, 0);
              lcd.print("  ");
              lcd.setCursor(7, 0);
              if(NAF>0)  sprintf(printBuffer, "FP:%02d.%01d", ((AF[0]&0xFF)+875) / 10, ((AF[0]&0xFF)+875) % 10);
              else sprintf(printBuffer, "No hay AF");
              lcd.print(printBuffer);

              lcd.setCursor(0, 1);
              lcd.print(F("               "));
              delay(2000);
              Tecla=btnNONE;
              Contador=0;
              while(Contador<NAF) {  // Leemos los niveles de cada AF 
                    RADIO.gotoChannel(AF[Contador]+875);    // gotoChannel lee los regitros
                    lcd.setCursor(0, 0);
                    lcd.print(F("AF=             "));
                    lcd.setCursor(3, 0);
                    lcd.print(Contador+1);
                    lcd.print(F("/"));
                    lcd.print(NAF);
                    lcd.setCursor(12, 0);
                    lcd.print(RADIO.Registers[RDSA],HEX); // Ponemos el PI
                    lcd.setCursor(0, 1);
                    sprintf(printBuffer, "%02d.%01dMHz", ((AF[Contador]&0xFF)+875) / 10, ((AF[Contador]&0xFF)+875) % 10);
                    lcd.print(printBuffer);
                    lcd.setCursor(9, 1);
                    rssi = RADIO.Registers[STATUSRSSI] & 0x00FF; //Mask in RSSI STATUSRSSI
                    lcd.print(rssi, DEC);
                    lcd.print(F("dBuV  "));
                    RADIO.gotoChannel(currentChannel);
                    while(Tecla==btnNONE) { 
                      Tecla=LeerBotonera();
                      delay(200);
                      if(Tecla==btnSELECT) {
                          ModoPantalla=1;
                          currentChannel = AF[Contador]+875; // Si pulsamos select vamos a la AF
                          LimpiaRDS();
                          Contador=NAF; //Salimos del While
                          }
                      else if((Tecla==btnUP)||(Tecla==btnDOWN)) Contador=NAF; //Salimos del While
                      else if(Tecla==btnRIGHT) Contador++;
                      else if(Tecla==btnLEFT) Contador--;
                      }
                   Tecla=btnNONE;
                 }
              Contador=Volumen;
              RADIO.gotoChannel(currentChannel); // Volvemos a la principal
              break;               
             
   case 5: // RADIO TEXTO
              lcd.setCursor(0, 0);
              if(GrupoA[2]!=0) {
                    lcd.print(F("RT A: "));
                    for(int i=0;i<64;i++) {
                      /*
                       if(RTB[i]==0x80) RTB[i]=0x61; // a
                       if(RTB[i]==0x82) RTB[i]=0x65; // e
                       if(RTB[i]==0x84) RTB[i]=0x69; // i
                       if(RTB[i]==0x86) RTB[i]=0x6F; // o
                       if(RTB[i]==0x88) RTB[i]=0x75; // u
                       if(RTB[i]==0x9A) RTB[i]=0x6E; // n
                       if(RTB[i]==0x8A) RTB[i]=0x4E; // N
                       */
                       lcd.print(RTB[i]); // En el B están los textos completos de A
                       }
              }
              else lcd.print(F("No Radio Texto"));
              //lcd.print(RTA);
              //lcd.setCursor(0, 1);
              //lcd.print(F("RT B: "));
              //lcd.print(RTB);
              break;
              
    case 6: // GRUPOS
              Volumen=Contador;
              Tecla=btnNONE;
              Tecla=btnNONE;
              Contador=0;
              while(Contador<16) {
              //for(Contador=0;Contador<16;Contador++)
                //{
                   if(GrupoA[Contador]!=0) {  
                       lcd.setCursor(0, 0);
                       lcd.print(F("NGP Rec:         "));
                       lcd.setCursor(8, 0);
                       lcd.print(ContadorGrupo);
                       lcd.setCursor(0, 1);
                       lcd.print(F("                "));
                       lcd.setCursor(0, 1); 
                       lcd.print(F("GP "));
                       lcd.print(Contador);
                       lcd.print(F("A="));
                       lcd.print(GrupoA[Contador]);
                       lcd.print(F(" "));
                       lcd.setCursor(11, 1);
                       long int Porciento = ((long int)GrupoA[Contador]*100)/ContadorGrupo;
                       sprintf(printBuffer, " %03d", Porciento);
                       lcd.print(printBuffer);
                       lcd.print(F("%"));
                       while(Tecla==btnNONE) { 
                           Tecla=LeerBotonera();
                           delay(200);
                           if(Tecla==btnSELECT) {
                               ModoPantalla=1;
                               Contador=16; //Salimos del While
                           }
                           else if((Tecla==btnUP)||(Tecla==btnDOWN)) Contador=16; //Salimos del While
                           else if(Tecla==btnRIGHT) Contador++;
                           else if(Tecla==btnLEFT) Contador--;
                           }
                           Tecla=btnNONE;
                   }
                   else Contador++;
              }
              Contador=Volumen;        
              break;

      case 7: // HORA UTC
              lcd.setCursor(0, 0);
              lcd.print(F("UTC:           "));
              lcd.setCursor(5, 0);
              if(GrupoA[4]!=0)lcd.print(F("*")); // Al limpiar datos la hora sigue contando, pero
              lcd.setCursor(7, 0);               // si detectamos paquete de hora es que está actualizada
              sprintf(printBuffer, " %02d:%02d:%02d", Horas, Minutos, Segundos);
              lcd.print(printBuffer);
              lcd.setCursor(0, 1);
              lcd.print(F("Hora local:     "));
              lcd.setCursor(12,1);
              if(UTC>=0) lcd.print(F("+"));
              else lcd.print(F("-")); 
              lcd.setCursor(13, 1);
              lcd.print(UTC);
            break;
            
      case 8: // FECHA Y HORA
              lcd.setCursor(0, 0);
              lcd.print(F("Hora:           "));
              lcd.setCursor(6, 0);
              if(GrupoA[4]!=0)lcd.print(F("*"));
              lcd.setCursor(7, 0);
              sprintf(printBuffer, " %02d:%02d:%02d", Horas+UTC, Minutos, Segundos);
              lcd.print(printBuffer);
              lcd.setCursor(0, 1);
              //lcd.print(F("Dia:            "));
              //lcd.setCursor(5,1);
              sprintf(printBuffer, "Dia: %02d/%02d/%04d ", Dia, Mes, Ano);
              lcd.print(printBuffer);
            break;
                   

 case 9: // CONFIGURACION FRECUENCIA
             Volumen=Contador;
             Contador=currentChannel;
             ModoConfiguracion=1;
             while((Tecla!=btnDOWN)&&(Tecla!=btnUP)) {  
                 Tecla=LeerBotonera();
                 delay(200);
                 if(Tecla==btnRIGHT) Contador++;
                 if(Tecla==btnLEFT) Contador--;
                 if(Contador>1080) Contador=1080;
                 if(Contador<=875) Contador=875;
                 lcd.setCursor(0, 0);
                 lcd.print(F("Selec. frecuencia:  "));
                 lcd.setCursor(0,1);
                 sprintf(printBuffer, "Frec: %02d.%01dMHz   ", Contador / 10, Contador % 10);
                 lcd.print(printBuffer);
                 if(Tecla==btnSELECT) {
                   RADIO.gotoChannel(Contador);
                   LimpiaRDS();
                   ModoPantalla=1;
                   ModoConfiguracion=0;
                   Contador=Volumen;
                   break;
                   }
             }
            Contador=Volumen;
            break;   
             
  case 10: // MENU MEMORIAS
             Volumen=Contador;
             Contador=ContadorMemorias+1;
             while((Tecla!=btnDOWN)&&(Tecla!=btnUP)) { 
                 Tecla=LeerBotonera();
                 delay(200);
                 if(Tecla==btnRIGHT) Contador++;
                 if(Tecla==btnLEFT) Contador--;
                 if(Contador>10) Contador=10;
                 if(Contador<=1) Contador=1;
                 lcd.setCursor(0, 0);
                 lcd.print(F("Memoria:        "));
                 lcd.setCursor(9, 0);
                 lcd.print(Contador);

                 // Convertir Memorias[] en estructura {Numero,Pi,PS}
                 // Pasar Memorias a EEPROM
                 int EPA = (Contador-1) * sizeof(ObjetoMemoria); // Saltamos 12 bytes de cada vez
                 EEPROM.get(EPA, Memorias); // Leemos cada Memoria de su valor en EEMPROM
                 lcd.setCursor(0,1);
                 lcd.print(F("PI:             "));
                 lcd.setCursor(3,1);
               
                 lcd.print(Memorias.Pi, HEX);
                 lcd.setCursor(8,1);
                 lcd.print(Memorias.PS);
                 
                 if(Tecla==btnSELECT) {
                   ContadorMemorias=Contador-1;
                   RADIO.gotoChannel(Memorias.Frecuencia);
                   LimpiaRDS();
                   ModoPantalla=1;
                   ModoConfiguracion=0;
                   Contador=Volumen;
                   break;
                   }
             }
            Contador=Volumen;
            break;  

                     
  case 11: // GUARDAR EN MEMORIA
             Volumen=Contador;
             Contador=Brillo;
               while((Tecla!=btnDOWN)&&(Tecla!=btnUP)) { 
                 Tecla=LeerBotonera();
                 delay(200);
                 if(Tecla==btnRIGHT) Contador++;
                 if(Tecla==btnLEFT) Contador--;
                 if(Contador>10) Contador=10;
                 if(Contador<=1) Contador=1;
                 lcd.setCursor(0, 0);
                 lcd.print(F("Guarar M: <>   "));
                 lcd.setCursor(13, 0);
                 lcd.print(Contador);

                 // Convertir Memorias[] en estructura {Numero,Pi,PS}
                 // Pasar Memorias a EEPROM
                 lcd.setCursor(0,1);
                 lcd.print(F("PI:             "));
                 lcd.setCursor(3,1);
                 lcd.print(Pi, HEX);
                 lcd.setCursor(8,1);
                 lcd.print(PS);
                 
                 if(Tecla==btnSELECT) {
                   Memorias.Frecuencia=currentChannel;
                   Memorias.Pi=Pi;
                   for(int i=0;i<7;i++) Memorias.PS[i] = PS[i];
                   int EPA = (Contador-1) * sizeof(ObjetoMemoria); // Saltamos 12 bytes de cada vez
                   EEPROM.put(EPA, Memorias); // Guardamos el dato en Memoria EEMPROM
                   ModoPantalla=1;
                   ModoConfiguracion=0;
                   Contador=Volumen;
                   break;
                   }
             }
            Contador=Volumen;
             break;
                 
                     
  case 12: // CONFIGURACION BRILLO
             Volumen=Contador;
             Contador=Brillo/10;
             while((Tecla!=btnDOWN)&&(Tecla!=btnUP)) {
                 Tecla=LeerBotonera();
                 Brillo=Contador*10;
                 delay(200);
                 if(Brillo>250) Brillo=250;
                 if(Brillo<=1) Brillo=0;
                 lcd.setCursor(0, 0);
                 lcd.print(F("Selec. iluminac:"));
                 lcd.setCursor(0, 1);
                 lcd.print(F("Iluminacion:    "));
                 lcd.setCursor(12, 1);
                 lcd.print(Brillo);
                 //delay(100);
                 analogWrite(BLACK_TILT_CONTROL, Brillo);
                 if(Tecla==btnSELECT) {
                   ModoPantalla=1;
                   ModoConfiguracion=0;
                   Contador=Volumen;
                   break;
                 }
             }
            Brillo=Contador;
            Contador=Volumen;
            delay(100);
            break;  
                  
case 13: // CONFIGURACION BUZZER
             Volumen=Contador;
             while((Tecla!=btnDOWN)&&(Tecla!=btnUP)) {
                 Tecla=LeerBotonera();
                 delay(200);
                 //if(Contador>255) Contador=255;
                 //if(Contador<=0) Contador=0;
                 lcd.setCursor(0, 0);
                 lcd.print(F("Selec. bz: </> "));
                 lcd.setCursor(0, 1);
                 lcd.print(F("BUZZER:         "));
                 lcd.setCursor(8, 1);
                 if(BuzzerON) lcd.print(F("ON"));
                 else lcd.print(F("OFF"));
                 if((Tecla==btnRIGHT)||(Tecla==btnLEFT)||(Volumen!=Contador)) {
                  BuzzerON=!BuzzerON;
                  Contador=Volumen; // En caso de que cambiemos con el encoder
                  }
                 if(Tecla==btnSELECT) {
                   ModoPantalla=1;
                   ModoConfiguracion=0;
                   Contador=Volumen;
                   break;
                 }
                 
                 }
             Contador=Volumen;
             break;

case 14: // CONFIGURACION VOLUMEN
             Contador=ActuacionVolumen(' ', btnNONE);
             Volumen=Contador;
             tiempo=millis();
             while((Tecla!=btnDOWN)&&(Tecla!=btnUP)) { 
                 Tecla=LeerBotonera();
                 if(Tecla==btnRIGHT) Contador++;
                 if(Tecla==btnLEFT) Contador--;
                 if(Contador>32) Contador=31;
                 if(Contador<=0) Contador=0;
                 lcd.setCursor(0, 0);
                 lcd.print(F("Selec. volumen: "));
                 lcd.setCursor(0, 1);
                 lcd.print(F("Volumen:        "));
                 lcd.setCursor(10, 1);
                 lcd.print(Contador);
                 RADIO.setVolume(Contador, true);
                 if((millis()-tiempo)>5000) break; // a los 5 segundos rompe
                 if(Tecla==btnSELECT) {
                   ModoPantalla=1;
                   Tecla=btnDOWN; // Rompemos el while
                  }
                 delay(200);
             }
            Volumen=Contador;
            break;       
                                  
   default:
        ModoPantalla=1;
        break;           
                
  }

}                   

// Pantalla Serial
void PantallaSerial(void)
{
char option;
// char vol = 15;
uint16_t currentChannel = 889; //Default the unit to a known good local radio station
uint8_t MaxNivel=0;
uint16_t MejorCanal=0;

if(ContadorGrupo>0) {
      MaxNivel=rssi; // Canal actual
      currentChannel = RADIO.readChannel();
      MejorCanal=currentChannel-875;
      Serial.println(F("\n\nGRUPO 0A FRECUENCIAS ALTERNATIVAS:"));
      // FRECUENCIAS ALTERNATIVAS
      for(Contador=0;Contador<NAF; Contador++)
          {
             Serial.print(F("  "));
             Serial.print(Contador+1); // Leemos los niveles de cada AF
             sprintf(printBuffer, " -> %02d.%01dMHz", ((AF[Contador]&0xFF)+875) / 10, ((AF[Contador]&0xFF)+875) % 10);
             RADIO.gotoChannel(AF[Contador]+875); // gotoChannel lee los registros
             rssi = RADIO.Registers[STATUSRSSI] & 0x00FF; //Mask in RSSI STATUSRSSI
             Serial.print(printBuffer);
             Serial.print(F(" ")); // Nivel RX="));
             Serial.print(rssi, DEC);
             Serial.println(F("dBuV"));
             if (rssi > MaxNivel)
               {
                 MaxNivel = rssi;
                 MejorCanal = AF[Contador];
                }
         }
      RADIO.gotoChannel(currentChannel); // Volvemos a la principal
      Serial.print(F("Mejor frecuencia: "));
      sprintf(printBuffer, " -> %02d.%01dMHz", (MejorCanal + 875) / 10, (MejorCanal + 875) % 10);
      Serial.println(printBuffer);
      
      
      // RADIO TEXTO
      Serial.println(F("\nGRUPO 2A RADIO TEXTO:"));
      if(GrupoA[2]!=0) {
          Serial.print(F("RT A: "));
          Serial.println(RTA);
          Serial.print(F("RT B: "));
          Serial.println(RTB);
          }
      else Serial.println(F("No hay información de Radio Texto"));    
      
      // FECHA / HORA
      Serial.println(F("\nGRUPO 4A FECHA/HORA:"));
      if(GrupoA[4]!=0) {
          Serial.print(F("Hora: "));
          sprintf(printBuffer, "%02d:%02d:%02d", Horas+UTC, Minutos, Segundos);
          Serial.println(printBuffer);
          sprintf(printBuffer, "Dia: %02d/%02d/%04d ", Dia, Mes, Ano);
          Serial.println(printBuffer);
          Serial.print(F("Dia juliano: "));
          Serial.println(MJD);
          Serial.print(F("Dia semana: "));
          Serial.println(DiaSem[ContDia-1]);
          Serial.print(F("\n\n"));
          }
      else Serial.println(F("No hay informacion de fecha / hora"));
      
      
      // GRUPOS
      Serial.println(F("\nGRUPOS PORCENTAJE:"));
      Serial.println(F("---------------------------------------------------------------------------------------------------"));
      Serial.println(F("|%|  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  10 |  11 |  12 |  13 |  14 |  15 |"));
      Serial.println(F("---------------------------------------------------------------------------------------------------"));
      Serial.print(F("|A|"));
      for(int i=0;i<16;i++) {
           long int Porciento = ((long int)GrupoA[i]*100)/ContadorGrupo;
           sprintf(printBuffer, " %03d |", Porciento);
           Serial.print(printBuffer);
          }
      Serial.println(F("\n---------------------------------------------------------------------------------------------------"));
      Serial.print(F("|B|"));
      for(int i=0;i<16;i++) {
           long int Porciento = ((long int)GrupoB[i]*100)/ContadorGrupo;
           sprintf(printBuffer, " %03d |", Porciento);
           Serial.print(printBuffer);
          }
      Serial.println(F("\n---------------------------------------------------------------------------------------------------"));
      
      
      // Grupos del tipo A
      Serial.println(F("\nGRUPOS NUMERO:"));
      Serial.print(F("GP:"));
      for (Contador = 0; Contador < 16; Contador++)
      {
      if (GrupoA[Contador] != 0)
        {
          Serial.print(F(" "));
          Serial.print(Contador);
          Serial.print(F("A="));
          Serial.print(GrupoA[Contador]);
        }
      }
      Serial.println(F(" "));
      
      // Grupos del tipo B
      Serial.print(F("GP:"));
      for (Contador = 0; Contador < 16; Contador++)
      {
      if (GrupoB[Contador] != 0)
        {
          Serial.print(F(" "));
          Serial.print(Contador);
          Serial.print(F("B="));
          Serial.print(GrupoB[Contador]);
        }
      }
      Serial.println(F(" "));
      Serial.print(F("Ultimo grupo recibido GP="));
      Serial.print(GP);
      Serial.println(Bcero);
      Serial.print(F("Grupos totales recibidos: "));
      Serial.println(ContadorGrupo);
 }
 else Serial.println(F("\nNo hay señal ni datos RDS"));
}

// DATOS BASICOS
void DatosBasicos(void)
{
  char option;
  //char vol = 15;
  uint16_t currentChannel = 889; //Default the unit to a known good local radio station
  uint8_t MaxNivel = 0;
  uint16_t MejorCanal = 0;

  currentChannel = RADIO.readChannel();
  sprintf(printBuffer, "\nFrecuencia: %02d.%01dMHz", currentChannel / 10, currentChannel % 10);
  Serial.print(printBuffer);
  Serial.print(F(" PI:"));
  Serial.print(Pi, HEX);
  Serial.print(F(" TP="));
  Serial.print(TP, BIN);
  Serial.print(F(" TA="));
  Serial.print(TA, BIN);
  Serial.print(F(" M/S="));
  Serial.print(MS, BIN);
  Serial.print(F(" "));
  if(MS) Serial.print(F("Music"));
  else Serial.print(F("Speech"));
  //Serial.print(MSTexto);
  Serial.print(F(" PS:"));
  Serial.println(PS);
  Serial.print(F("PTY="));
  Serial.print(PTY);
  Serial.print(F(" "));
  Serial.print(TextoPTY[PTY]);
  if (PTYdinamico) Serial.print(F(", PTY dinamico, "));
  else Serial.print(F(", PTY estatico, "));
  if (Estereo) Serial.println(F("Estereo"));
  else Serial.println(F("Mono"));
  Serial.print(F("NAF="));
  Serial.print(NAF);
  if (NAF > 0)  sprintf(printBuffer, " Frec.Ppal.: %02d.%01dMHz", ((AF[0] & 0xFF) + 875) / 10, ((AF[0] & 0xFF) + 875) % 10);
  else sprintf(printBuffer, " No hay AF");
  Serial.println(printBuffer);
  rssi = RADIO.Registers[STATUSRSSI] & 0x00FF; //Mascara de RSSI STATUSRSSI
  Serial.print(F("Nivel="));
  Serial.print(rssi, DEC);
  Serial.println(F(" dBuV"));
}

// LEE LA BOTONERA DEL DISPLAY LCD
int LeerBotonera(void)  // Lee los botones y devuelve le valor pulsado
{
 int key = analogRead(BOTONES);  // Leemos la puerta A0 
 // Mis botones dan:  0, 145, 329,507,743 ; Comprueba que tu shield devuelve valores parecidos
 // Y ahora los comparamos con un margen comodo
 if ((key < 850)&&(BuzzerON)) tone(BuzzerPin,NotaTeclado,TiempoNota);
 if (key > 1000) return btnNONE; //Si no se pulsa nada salimos (el 99% de las veces) 
 if (key < 50)   return btnRIGHT;  
 if (key < 250)  return btnUP; 
 if (key < 450)  return btnDOWN; 
 if (key < 650)  return btnLEFT; 
 if (key < 850)  return btnSELECT;  
 return btnNONE;  // cuando nada lee devuelve esto...
}

// ACTUACION VOLUMEN
// Sube y baja el volumen con teclado y encoder
uint8_t ActuacionVolumen(char OPC_Tecla, uint8_t OPC_Numero)
{
      // Podemos pasar el volumen de tres maneras,
      // a través del serial con + y -
      // a través del teclado del display
      // a través del encoder
      // Esta apliccion devuelve el volumen actual con los parámetros (" ","btnNONE")
      RADIO.readChannel(); //Leemos los registros
      byte current_vol = RADIO.Registers[SYSCONFIG2] & 0x000F; //Leemos el volumen actual
      boolean VolExt = RADIO.Registers[SYSCONFIG3] & (1 << VOLEXT);
      uint8_t Vol = current_vol + 16 * boolean(!VolExt);
      if(((OPC_Tecla=='+')||(OPC_Numero==btnRIGHT))&&(Vol < 30)) {
        Vol++; //Máximo 30
        RADIO.setVolume(Vol, true);
      }
      if(((OPC_Tecla=='-')||(OPC_Numero==btnLEFT))&&(Vol >= 1)) {
        Vol--; //Limit min 0
        RADIO.setVolume(Vol, true);
      }
      return Vol;
}


// LEER ENCODER, BASADO EN INTERRUPCIONES
// Modifica dos variables volátiles CDAnterior y CDActual
void LeerEncoder(void){
  // Cada llamada a interrupción en cada cambio de A
  // Si el encoder A  y B son distintos va en una dirección,
  // si son iguales en otra, cuenta o descuenta.
  // Esta versión incluye un tiempo de debounce para
  // evitar rebotes.
  static long TiempoEncoder=0; // Esta variable es static y se 
                               // inicializa con 0 y se conserva cada vez
  if((millis()-TiempoEncoder)>IntevaloEncoder) {  // Evitamos rebotes
     int Dato = digitalRead(encoderB); //Leemos el dato B
     if (Dato==digitalRead(encoderA)) Contador++; // Positivo en sentido de las agujas del rejoj
     else Contador--;            // Negativo en sentido contarrio
     if(BuzzerON) tone(BuzzerPin,NotaEncoder,TiempoNota);
     TiempoEncoder=millis();     // Reiniciamos el tiempo del encoder
     }
}

// DATOS RDS NOS DA INFORMACION POR INTERUPCIONES
void DatosRDS(void){
// Cuando el Si4703 disponde de un paquete de datos RDS
// activa una interrupcion por el GPIO2 de 5ms Low
// Ponemos la variable RDSOK a 1
// Tendremos que desactivarla despúes de leer los registros
RDSOK=true;
}


// SINTONIA MAS O MENOS
void Sintonia(int MasMenos) {
      LimpiaRDS();
      int currentChannel = RADIO.readChannel();
      currentChannel += MasMenos; //Suma 1 o resta 1
      RADIO.gotoChannel(currentChannel);

}

