//DISPLAY
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4 // not used 
Adafruit_SSD1306 display(OLED_RESET);

// Speed über Poti
#define SpeedPin A3
int istSpeed = 300; // Startgeschwindigkeit vom Motor

// Taster 1-3
#define TasterPin1 8
unsigned int WegTaste1 = 0;
#define TasterPin2 9
unsigned int WegTaste2 = 0;
#define TasterPin3 10
unsigned int WegTaste3 = 0;

// Endschalter
#define ESchalter1 11
#define ESchalter2 12
unsigned int WegStrecke = 0;
unsigned int IstStrecke = 0;

//Drehschalter

#define encoderPinA 2
#define encoderPinB 3

volatile unsigned int encoderPos = 0;  // a counter for the dial
unsigned int lastReportedPos = 0;   // change management
static boolean rotating = false;    // debounce management

// Interrupts für den Drehschalter
boolean A_set = false;
boolean B_set = false;

//Taster vom Drehschalter
int pin_drehschalter = 4;

//Schrittmotor
int schritte = 3200;
// Schrittweite pro Inkrement
int SchrittProInkrement = 10;

int PUL1 = 5; //define PUL1se pin
int DIR1 = 6; //define DIR1ection pin
int ENA1 = 7; //define ENA1ble Pin

//Variablen
String text1 = "";
int i, k = 0;
bool taster = false;
void goTo(int soll) {// So lange bewegen bis ist == soll
  while (IstStrecke != soll) {
    if (soll > IstStrecke) {
      digitalWrite (DIR1, HIGH);
      MSteppMotor();
      IstStrecke++;
    }
    if (soll < IstStrecke) {
      digitalWrite (DIR1, LOW);
      MSteppMotor();
      IstStrecke--;
    }
  }
}
void MSteppMotor() {
  digitalWrite(PUL1, LOW);
  delayMicroseconds(istSpeed);
  digitalWrite(PUL1, HIGH);
  delayMicroseconds(istSpeed);
}
void setup() {
  Serial.begin(9600);  // output
  // Speed Poti
  istSpeed = analogRead(SpeedPin) * 10 ; //Potigenauigkeit
  // DISPLAY
  // I2C-Adresse 0x3c initialisieren
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // zufälligen Startwert für Random-Funtionen initialisieren
  randomSeed(analogRead(0));
  display.clearDisplay();

  //Inkrementgeber
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  digitalWrite(encoderPinA, HIGH);  // turn on pullup resistors
  digitalWrite(encoderPinB, HIGH);  // turn on pullup resistors

  attachInterrupt(0, doEncoderA, CHANGE); // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(1, doEncoderB, CHANGE); // encoder pin on interrupt 1 (pin 3)

  //Taster vom Inkrementgeber
  pinMode(pin_drehschalter, INPUT_PULLUP);

  //Schrittmotor
  pinMode (PUL1, OUTPUT);
  pinMode (DIR1, OUTPUT);
  pinMode (ENA1, OUTPUT);
  digitalWrite(ENA1, HIGH);//Motor ist noch abgeschaltet

  //Taster 1-3
  pinMode(TasterPin1, INPUT_PULLUP);
  pinMode(TasterPin2, INPUT_PULLUP);
  pinMode(TasterPin3, INPUT_PULLUP);

  //Endschalter 1-2
  pinMode(ESchalter1, INPUT_PULLUP);
  pinMode(ESchalter2, INPUT_PULLUP);
  //DISPLAY
  display.clearDisplay();
  // set text color / Textfarbe setzen
  display.setTextColor(WHITE);
  display.setTextSize(2);
  text1 = String(istSpeed) + " " + String(IstStrecke); // Gibt die akutelle Geschwindigkeit und die Position in Umdrehungen an
  display.setCursor(1, 0);
  display.println(text1);
  display.display();

  //Endschalter anlernen
  digitalWrite (DIR1, LOW);
  while (!digitalRead(ESchalter1)) {// Anfangswert anfahren !!
    //Serial.println("Anfangswert anfahren !!");
    digitalWrite(PUL1, LOW);
    delayMicroseconds(istSpeed);
    digitalWrite(PUL1, HIGH);
    delayMicroseconds(istSpeed);
  }
  digitalWrite (DIR1, HIGH);
  while (!digitalRead(ESchalter2)) {// Zum zweiten Endstop fahren und sich die Wegstrecke merken.
    //Serial.println("zweiten Endstop anfahren !!");
    digitalWrite(PUL1, LOW);
    delayMicroseconds(istSpeed);
    digitalWrite(PUL1, HIGH);
    delayMicroseconds(istSpeed);
    WegStrecke++;
  }
  IstStrecke = WegStrecke - 1; // Ab jetzt muss jede Bewegung in IstStrecke gespeichert werden
  encoderPos = IstStrecke;

  // Taster 1 anlernen
  //Drehschatler drehen bin zum Anlernpunkt und dann den Taster 1 drücken
  //Serial.println("TST " + String(digitalRead(TasterPin1)));
  while (digitalRead(TasterPin1)) {
    //Serial.println("Pin1");
    rotating = true;  // Interrupt starten
    goTo(encoderPos);
  }
  WegTaste1 = IstStrecke;

  // Taster 2 anlernen
  //Drehschatler drehen bin zum Anlernpunkt und dann den Taster 2 drücken
  while (digitalRead(TasterPin2)) {
    //Serial.println("Pin2");
    rotating = true;  // Interrupt starten
    goTo(encoderPos);
  }
  WegTaste2 = IstStrecke;

  // Taster 3 anlernen
  //Drehschatler drehen bin zum Anlernpunkt und dann den Taster 3 drücken
  while (digitalRead(TasterPin3)) {
    //Serial.println("Pin3");
    rotating = true;  // Interrupt starten
    goTo(encoderPos);
  }
  WegTaste3 = IstStrecke;

}

//DISPLAY
#define DRAW_DELAY 118
#define D_NUM 47

void loop() {
  //Poti
  istSpeed = analogRead(SpeedPin) ; //Potigenauigkeit wird in 1024/50 Teile zerteilt
  //Drehschalter
  rotating = true;  // Interrupt starten
  if (lastReportedPos != encoderPos) {
    lastReportedPos = encoderPos;
  }
  //DISPLAY
  display.clearDisplay();
  // set text color / Textfarbe setzen
  display.setTextColor(WHITE);
  display.setTextSize(2);
  text1 = String(istSpeed) + " " + String(IstStrecke); // Gibt die akutelle Geschwindigkeit und die Position in Umdrehungen an
  display.setCursor(1, 0);
  display.println(text1);
  display.display();

  //Taster vom Drehschalter
  if (!digitalRead(TasterPin1)) {
    encoderPos = WegTaste1;
  }
  if (!digitalRead(TasterPin2)) {
    encoderPos = WegTaste2;
  }
  if (!digitalRead(TasterPin3)) {
    encoderPos = WegTaste3;
  }
  goTo(encoderPos);
}

//Drehschalter
// Interrupt on A changing state
void doEncoderA()
{
  if ( rotating ) delay (1);  // wait a little until the bouncing is done
  if ( digitalRead(encoderPinA) != A_set ) { // debounce once more
    A_set = !A_set;
    // adjust counter + if A leads B
    if ( A_set && !B_set ) {
      if (encoderPos < WegStrecke) {
        encoderPos += SchrittProInkrement;
      }
    }
    rotating = false;  // no more debouncing until loop() hits again
  }
}

// Interrupt on B changing state, same as A above
void doEncoderB() {
  if ( rotating ) delay (1);
  if ( digitalRead(encoderPinB) != B_set ) {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if ( B_set && !A_set ) {
      if (encoderPos > SchrittProInkrement) {
        encoderPos -= SchrittProInkrement;
      }
    }
    rotating = false;
  }
}
