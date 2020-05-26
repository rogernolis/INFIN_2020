#include <avr/interrupt.h>
#include <avr/io.h>
//AL UTILITZAR EL MONITOR SERIE SELECCIONAR "SIN AJUSTE DE LÍNEA" EN LA PART POSTERIOR PER EVITAR /n AL INTRODUIR CARACTERS
//DEFINICIONS
#define SERIE_DELAY 25
#include <stdlib.h>
#include <string.h>
//VARIABLES GLOBALS
const int analogPin = A3;                       // Pin sensor
String comanda, convertidor;
int tempsdemostreig;
int Marcha = 0;
int c = 0;
int valormostra = "";
int valorSortida;
int NumeroSortida, NumeroEntrada;

//FUNCIONS
String EmpaquetamentMostra();                   // Compacta la mostra a treure per l'ordre Concvertidor. L'omple fins a tenir 4 dígits.
String getLine();                               // Extreu la comanda final i neta des del Serial Port.
void Enviar(String, String, int);                // Concatena i envia el missatge de resposta en cada cas.

void setup() {
  Serial.begin(9600);
  noInterrupts();                               // Desabilitar interrupcions
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 62499;                                // Valor de comparació equivalent a 1 segon.
  TCCR1B |= (1 << WGM12);                       // Mode CTC.
  TCCR1B |= (1 << CS12);                        // 256 prescalat
  TIMSK1 |= (1 << OCIE1A);                      // Enable timer comparador
  interrupts();                                 // Habilitar totes les interrupcions
  analogReference(DEFAULT);                     // Configuració del rang de l'entrada en volts
}
void loop() {
  while (Serial.available() == 0) {                                        // Esperar a tenir algun valor a lentrada
    int codiRetorn = 0;                                                       // Inicialització del codi de retorn
    //LLEGIR TOT
    comanda = getLine();                                                      // Posem a comanda el missatge llegit a la funció getLine
    if (comanda != "\n") {                                                    // Comprovar que el missatge no és buit
      switch (comanda[1]) {                                                   // Analitzar byte comanda
        case 'M':                                                             // Entrem a Marxa/Parada
          if (comanda[5] == 'Z') {                                            // Comprobació de protocol
            switch (comanda [2]) {
              case '1':                                                       // Entrem a Marxa
                if (Marcha == 0) {                                            // Comprovem si l'adquisició està en parada
                  if (isDigit(comanda[3]) && isDigit(comanda[4])) {           // Comprovació que siguin números les xifres del temps
                    convertidor = comanda.substring(3, 5);                    // Extreure valors en simbologia ASCII i passarlos a numero ASCII
                    tempsdemostreig = convertidor.toInt();                    // Fer equivalència a integer i obtenir el valor numèric real.
                    if (tempsdemostreig <= 20 && tempsdemostreig > 0) {       // Comprobació dels paràmetres
                      Marcha = 1;                                             // Activar senyal per l'adquisició mostres
                      codiRetorn = 0;                                       // Sense errors, Codi Retorn: OK

                    }
                    else {
                      codiRetorn = 2;                                       // Error de paràmetres
                    }
                  }
                  else {
                    codiRetorn = 2;                                         // Error de paràmetres
                  }
                }
                else {
                  codiRetorn = 2;                                           // Error de paràmetres
                }
                break;
              case '0':
                if (Marcha == 1) {                                            // Comprovem si l'adquisició està en marxa
                  Marcha = 0;                                                 // Posem la adquisició en parada
                  codiRetorn = 0;                                           // Sense errors, Codi Retorn: OK
                }
                else {
                  codiRetorn = 2;                                           // Error de paràmetres
                }
                break;
              default:
                codiRetorn = 2;                                             // Error de paràmetres
                break;
            }
          }
          else {
            codiRetorn = 1;                                                 // Error de protocol
          }
          Enviar("M", "", codiRetorn);                                        // Enviar missatge amb el cas que s'ha entrat
          break;
        case 'S':
          if (comanda[5] == 'Z') {
            if (isDigit(comanda[2]) && isDigit(comanda[3]) && isDigit(comanda[4])) {                          // Comprvació de paràmetres.
              valorSortida = comanda[4] - 48;                                                                 // Transformació de simbologia ASCII a decimal ASCII.
              convertidor = comanda.substring(2, 4);                                                          // Aillem la part del missatge que ens interessa, el número de la sortida.
              NumeroSortida = convertidor.toInt();
              if ((0 == valorSortida || valorSortida == 1) && (NumeroSortida >= 1 && NumeroSortida < 14)) {   // Comprvació de paràmetres.
                codiRetorn = 0;                                                                             // Sense errors, Codi Retorn: OK
                pinMode(NumeroSortida, OUTPUT);
                digitalWrite(NumeroSortida, valorSortida);                                                    // Posem el valor escollit a la sortida digital triada.
              }
              else {
                codiRetorn = 2;                                             // Error de paràmetres
              }
            }
            else {
              codiRetorn = 2;                                               // Error de paràmetres
            }
          }
          else {
            codiRetorn = 1;                                                 // Error de protocol
          }
          Enviar("S", "", codiRetorn);                                        //Contestació de control sensor a servidor
          break;
        case 'E':
          if (comanda[4] == 'Z') {
            if (isDigit(comanda[2]) && isDigit(comanda[3])) {                 // Comprvació de paràmetres.
              convertidor = (comanda.substring(2, 4));
              NumeroEntrada = convertidor.toInt();
              if (NumeroEntrada >= 1 && NumeroEntrada < 14) {
                if (digitalRead(NumeroEntrada)) {                             // Retornar el valor de la entrada seleccionada pel Servidor
                  Enviar("E", "1", 0);                                      // Enviar sortida HIGH i sense errors
                }
                else {
                  Enviar("E", "0", 0);                                      // Enviar sortida LOW i sense errors
                }
              }
              else {
                Enviar("E", "0", 2);                                       // Enviar error paràmetres
              }
            }
            else {
              Enviar("E", "0", 2);                                         // Enviar error paràmetres
            }
          }
          else {
            Enviar ("E", "0", 1);                                           // Enviar error paràmetres
          }
          break;
        case 'C':
          if (comanda[2] == 'Z') {                                            // Comprvació de paràmetres.
            if (Marcha == 1) {                                                // Comprvació de paràmetres.
              convertidor = EmpaquetamentMostra();
              Enviar("C", convertidor, 0);                                  // Enviar resposta tot OK
            }
            else {                                                            //Quan està en mode parada, enviar última mostra guardada o, si ja s'ha fet, retornar error. Comprovació d'errors
              if (valormostra != "") {                                        // Comprvació de paràmetres.
                convertidor = EmpaquetamentMostra();                          // Assignació valor de la funció
                Enviar("C", convertidor, 0);                                // Enviar valor convertit i que tot OK
                valormostra = "";                                             // Esborrar la última mostra ja enviada.
              }
              else {
                Enviar("C", "0000", 2);                                     // Comprvació de paràmetres.
              }
            }
          }
          else {
            Enviar("C", "0000", 1);                                         // Error de protocol
          }
          break;
        default :
          // Al no tenir un missatge que contingui res de una comanda, el programa no envia cap tipus d'error.
          break;
      }
    }
  }
}
String getLine() {                                                          // Funció de agafar només la comanda, neta de soroll i errors.
  int ultimaA;                                                              // Variable per tenir la posició de l'última A
  while (Serial.available() == 0) {                                         // Esperar que arribi algun byte al port serie.
    String S = "" ;                                                           // Inicialització del string per la comanda
    char c = "";                                                              // Inicialització del char per cada caràcter de la cadena
    while ( c != 'Z') {                                                      // Llegeix caracters fins indicador final de carrera
      c = Serial.read();
      S = S + c;                                                              // Acumula els caràcters a l'String
      delay(SERIE_DELAY);
    }
    ultimaA = S.lastIndexOf('A');                                             // Comprovar si existeix l'inici indicador de comanda.
    if (ultimaA == -1) {                                                      // En cas de no existir no enviar res al servidor.
      S = "Z";
    }
    else {                                                                    // En cas de existir, agafar només la darrera comanda.
      S = S.substring(ultimaA);
    }

    return (S) ;                                                              // Retornar l'String
  }
}

void Enviar(String cas, String cas_especial, int codi_retorn) {              // Empaquetament dels missatges per enviar-los al servidor. Es crea una cadena amb la informació necessaria per cada cas
  String Envio = "";
  String codi_retorn_string = (String)codi_retorn;
  char Dada[10];
  for ( int k = 0; k < sizeof(Dada);  ++k ) {
    Dada[k] = (char)0;
  }
  Envio = "A" + cas + codi_retorn_string + cas_especial + "Z";
  for ( int k = 0; k <= sizeof(Envio) + 1;  ++k ) {
    Dada[k] = Envio[k];
  }
  Serial.write(Dada);

}

String EmpaquetamentMostra() {                                              // Funció per acabar d'omplir els 4 bytes requerits per la operació Convertidor
  int d;
  String x;
  convertidor = valormostra;                                                // Passar de integer a simbologia ASCII
  if (convertidor.length() < 4) {                                           // Comprobació longitud de la comanda a enviar
    for (d = 0; d < (4 - convertidor.length()); d++) {                      // Afegir tants '0' com xifres faltin per omplir les 4
      x.concat("0");
    }
    x.concat(convertidor);
  }
  else {
    x = convertidor;
  }
  return (x);
}

ISR(TIMER1_COMPA_vect)                                                      // Execució de la rutina d'interrupció
{
  if  (Marcha == 1) {                                                       // Comprova el senyal de marxa per poder adquirir dades
    c++;                                                                    // Incrementar el contador
    if (c == tempsdemostreig ) {                                            // Comparar si el comptador ha arribat al temps especificat
      c = 0;                                                                // Reset comptador
      valormostra = analogRead(analogPin);                                  // Lectura del sensor.
    }
  }
}
