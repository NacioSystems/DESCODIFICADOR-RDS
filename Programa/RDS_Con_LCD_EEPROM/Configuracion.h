/* Este archivo de configuarción es para el programa Desco RDS

PINES UTILIZADOS POR ARDUINO UNO

  LCD 16x2 Utiliza los pines
---------------------------------  
D4  DB4
D5  DB5
D6  DB6
D7  DB7
D8  RS
D9  ENABLE
D10 BLACK TILT CONTROL
A0  BOTONES (Lecturas: Select = 720, Izq = 479, Arrb. = 131, Abaj.306, Der = 0;

  ENCODER ROTATIVO
----------------------------------
D2   encoderA (Int 0)
D12  encoderB

  Si4703 RDS DECODER
----------------------------------
D3  GPIO2 (Int 1)
A3  RESET PIN
A4  SDIO
A5  SCLK

  LED rojo
----------------------------------
D13 STATUS_LED


   BUZZER
---------------------------------- 
D11 BuzzerPin

En los puertos están disponibles, A1, A2 y D0, D1 (RX / TX)

*/
//Conexiones con Si4703 y Arduino
      #define GPIO2 3  // D3 Interrupcion 1 para deco RDS
      #define STATUS_LED 13
      #define resetPin A3
      #define SDIO A4 //SDA/A4 en Arduino 20 en Mega
      #define SCLK A5 //SCL/A5 en Arduino 21 en Mega

// Pines utilizados por LCD
      #define DB4 4
      #define DB5 5
      #define DB6 6
      #define DB7 7
      #define RS 8
      #define ENABLE 9
      #define BLACK_TILT_CONTROL 10
      #define BOTONES A0

// Valores para leer los botones y panel LCD
      #define btnRIGHT  0
      #define btnUP     1
      #define btnDOWN   2
      #define btnLEFT   3
      #define btnSELECT 4
      #define btnNONE   5

// Valores utilizados por el Encoder Rotativo
// Estos dos pines son interrupciones 0 y 1
      #define encoderA 2
      #define encoderB 12
// Tiempo para eliminar los rebotes en Encoder
      #define IntevaloEncoder 100

// Datos del buzzer
      #define BuzzerPin 11
      #define NotaTeclado 55
      #define NotaEncoder 84
      #define TiempoNota 25

