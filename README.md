# DESCODIFICADOR RDS

## de NacioSystems:

Versión 1.0
Fecha: enero 2018

![DESCO RDS](https://github.com/NacioSystems/DESCODIFICADOR-RDS/blob/master/Fotos/Montaje.jpg "Desco RDS")

### Indroducción:

A través de un Arduino UNO se controla el receptor de FM con RDS Si-4703. El conjunto se complementa con una bononera y display de la placa LCD KeyPad, además de un encoder rotativo. El Arduino UNO funciona con lógica a 5V, mientras que el Si-4703 funciona a 3.3V, por ello es necesaria una placa adaptadora de niveles. Todo está montado sobre una caja impresa en 3D, incluyendo cuatro pilas AA de 1,5V. Es necesario disponer de unos auriculares con jack de 3,5, para las funciones de antena y poder escuchar los contenidos estereofónicos.

Tambien funciona sin baterías, conectando el puerto USB a una fuente de alimentación.

A través de la botonera se pude pasar de una frecuencia a otra, mediante escaneo o por pasos de 100KHz. En el display nos indicará la frecuelncia de recepción y en caso de disponer de servicios RDS nos indicará el PI de la emisora y su nombre. También se indica el nivel de recepción de la señal radio en bornas del conector de antena/auriculares, en dBuV. Dependiendo del modo, el encoder rotativo servirá par subir bajar el volumen, que es la función principal, pero puede funcionar para subir y bajar frecuencias. Cuando una emisora está bien sintonizada se activará el LED indicando la recepcón estereofónica. Las pulsaciones de la botonera y el encoder generan un pequeño bip en el buzzer, confirmando que la tecla se ha pulsado correctamente.

No sólo es una radio convencional, es un descodificador de señal RDS, aportando la información en diferentes pantallas y módos, seleccionables con los bonones "up" y "down". Entre otras cosas aporta la siguiente información, tanto por el dsplay LCD como a través del puerto serie, este último más completo:

* PI código hexadecimal de identificación de programa
* PS nombre de la emisora de radio o programa musical
* TP si el programa da información de tráfico
* TA si se están dando contenidos de anuncio de tráfico
* M/S si es música o palabra lo emitido
* PTY la identificación de tipo de programa, noticias, música,...
* RTX el radiotexto que pueda estar emitiendo la emisora A/B
* N, número de frecuencias alternativas
* AF frecuencias alternativas de la emisión y nivel de recepción de cada frecuencia
* G A/B informacíon estadística sobre los paquetes y grupos emitidos en RDS
* Hora UTC, Información de la fecha y hora transmitida, en caso de emitirla.

Otra funcionalidad que contempla es la grabación en la memoria EEPROM del Arduiono UNO de 10 memorias con las emisoras favoritas. Por defecto incluye 10 memorias ya cargadas del entorno de Santiago de Compostela, pero a través del teclado permite guardar en cada posición de memoria datos de nuevas emisoras.

### Materiales electrónicos:

* [Arduino UNO R3][2]
* [LCD Keypad for Arduino UNO][3]
* [Receptor FM SI-4703][4]
* [Adaptador de niveles lógicos 3,3V a 5V][5]
* [Encoder rotativo eje largo][11]
* [Porta pilas 4xAA][6]
* [Buzzer][7]
* Otro pequeño material como diodo LED, interruptor endendido, cables...

![Prototipado](https://github.com/NacioSystems/DESCODIFICADOR-RDS/blob/master/Fotos/Baja%20resolucion/Prototipo2.jpg "Prototipado")

### Piezas impresas:

La caja soporte está realizada a través de impresión en 3D, con PLA. Los modelos STL se pueden descargar del repositorio:

* [Caja principal][8]
* [Botonera][9]
* [Tapa posterior][10]

[8]: https://github.com/NacioSystems/DESCODIFICADOR-RDS/blob/master/Modelos%203D/DescoRDS.stl
[9]: https://github.com/NacioSystems/DESCODIFICADOR-RDS/blob/master/Modelos%203D/botones.stl
[10]: https://github.com/NacioSystems/DESCODIFICADOR-RDS/blob/master/Modelos%203D/tapa.stl

![Caja 3D](https://github.com/NacioSystems/DESCODIFICADOR-RDS/blob/master/Fotos/Diseno3D.JPG "Diseño caja 3D")

### Mejoras pendientes:

Está pendiente incluir una antena telescópica fija, para no depender de los auriculares. También un pequeño amplificador con altavoz para escuchar la señal emitida sin auriculares. Implementar el cambio automático de emisora a través de las frecuencias alternativas y el código PI, cuando la calidad de recepción se reduce.

### Autor:

**Ignacio Otero**

### Agradecimientos:

Muchas gracias [**RPM**][1] por tus tutoriales sobre RDS.

Gracias también a [**Spark Fun Electronics**][4], de quien aprovecho las librerías Si4703.

### Licencia:

Todos estos productos están liberados mediante Creative Commons Attribution-ShareAlike 4.0 International License.

[1]: http://j-rpm.com/
[2]: https://store.arduino.cc/arduino-uno-rev3
[3]: https://www.amazon.es/Aptotec-Keypad-Arduino-Mega2560-Duemilanove/dp/B01BI6UKHW/ref=sr_1_fkmr1_1?adgrpid=55044401886&gclid=CjwKCAiAlO7uBRANEiwA_vXQ-8poL89WK_BCXPjUbC35tp92i_Hjc4qCP3N4zbFsbQ-p1g6ebLKiCRoChUAQAvD_BwE&hvadid=275347219637&hvdev=c&hvlocphy=1005483&hvnetw=g&hvpos=1o1&hvqmt=e&hvrand=16829546787644630856&hvtargid=kwd-329815121004&hydadcr=28887_1774500&keywords=lcd+keypad+arduino&qid=1574682044&sr=8-1-fkmr1
[4]: https://learn.sparkfun.com/tutorials/si4703-fm-radio-receiver-hookup-guide/all
[5]: https://tienda.bricogeek.com/herramientas-de-prototipado/82-conversor-de-niveles-logicos-33-5v.html
[6]: https://tienda.bricogeek.com/componentes/943-base-para-baterias-4xaa-cilindrica.html
[7]: https://tienda.bricogeek.com/componentes/299-zumbador-piezo-12mm.html
[11]: https://www.amazon.es/Ociodual-CODIFICADOR-ROTATORIO-PULSADOR-Encoder/dp/B075NJSMWM/ref=sr_1_10?adgrpid=65969624113&gclid=CjwKCAiAlO7uBRANEiwA_vXQ-3gBKTK8yzZNJGjGjNc08mlUVqVoJ-zUFTFujZLM8sPl8-u1ELTtbRoCypcQAvD_BwE&hvadid=322329185658&hvdev=c&hvlocphy=1005483&hvnetw=g&hvpos=1o1&hvqmt=b&hvrand=10448089530400385996&hvtargid=kwd-566645944781&hydadcr=14558_1815328&keywords=arduino+codificador&qid=1574683829&sr=8-10
