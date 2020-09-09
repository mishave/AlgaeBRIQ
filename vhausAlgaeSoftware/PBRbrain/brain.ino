#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <VarSpeedServo.h>

//Software Serial Setup Requirements
SoftwareSerial brainESP(52, 53); // RX, TX

//Incoming Data doc from brainESP
const size_t capacity = JSON_OBJECT_SIZE(27) + 300;
DynamicJsonDocument brain(capacity);
const char* json[capacity];

//Servo Setup Requirements
VarSpeedServo harvestServo1;
VarSpeedServo dumpServo1;
VarSpeedServo dumpServo2;

//Serial Readings
int lp1_1, lp1_2, lp1_3, lp1_4,
    lp2_1, lp2_2, lp2_3, lp2_4,
    chillSS, chillOn, heatOn, airSS,
    topUp, phDown, nutMix, sPump, hAM, hSS,
    servoOn, psv, d1sv, d2sv,
    pOpen, pClose, d1Open, d1Close, d2Open, d2Close;

//Lighting On/Off
int lp[]  = {0, 0, 0, 0, 0, 0, 0, 0};
int lpL[] = {0, 0, 0, 0, 0, 0, 0, 0};
int lighPin[] = {26, 27, 28, 29, 30, 31, 32, 33};

//Servo Control
int servoOnPin = 12, psvPin = 9, d1svPin = 11, d2svPin = 10;
int d1svL = 0, d2svL = 0, psvL = 0;
byte servoOnCheck = 0;
unsigned long d1Millis, lastD1Delay, d1Delay = 300;

unsigned long servoMillis, lastServoDelay, servoDelay = 1000;


//pumps
int wpCultPin = 6, wpTankPin = 7, wpHarvestPin = 2, airPumpPin = 3;

//Cooling
int heatPin = 4, coolPin = 5;


//Add Output Pins here inorder to set low/Set as Ouput in Setup
int outPutPins[] = {servoOnPin,
                    wpCultPin, wpTankPin, wpHarvestPin, airPumpPin,
                    heatPin, coolPin
                   };

void setup() {
  Serial.begin(115200);
  brainESP.begin(9600);
  //Set Lights
  for (byte i = 0; i < (sizeof(lighPin) / sizeof(lighPin[0])); i++) {
    pinMode(lighPin[i], OUTPUT);
    digitalWrite(lighPin[i], LOW);
  }
  //Set Relays
  for (byte i = 0; i < (sizeof(outPutPins) / sizeof(outPutPins[0])); i++) {
    pinMode(outPutPins[i], OUTPUT);
    digitalWrite(outPutPins[i], LOW);
  }
  //Set Servos
  harvestServo1.attach(psvPin);
  harvestServo1.write(0, 127, false);
  dumpServo1.attach(d1svPin);
  dumpServo1.write(0, 127, false);
  dumpServo2.attach(d2svPin);
  dumpServo2.write(0, 127, true);

}

void loop() {
  readBrainESP();
  turnONOffLights();
  servoControl();
}

void readBrainESP() {
  if (brainESP.available() > 0) {
    deserializeJson(brain, brainESP);
    lp[0] = brain["lp1_1"];
    lp[1] = brain["lp1_2"];
    lp[2] = brain["lp1_3"];
    lp[3] = brain["lp1_4"];
    lp[4] = brain["lp2_1"];
    lp[5] = brain["lp2_2"];
    lp[6] = brain["lp2_3"];
    lp[7] = brain["lp2_4"];

    chillOn = brain["chillOn"];
    heatOn = brain["heatOn"];
    airSS = brain["airSS"];
    topUp = brain["topUp"];
    phDown = brain["phDown"];
    nutMix = brain["nutMix"];
    sPump = brain["sPump"];
    hSS = brain["hAM"];
    hSS = brain["hSS"];
    servoOn = brain["servoOn"];
    psv = brain["psv"];
    d1sv = brain["d1sv"];
    d2sv = brain["d2sv"];
    pOpen = brain["pOpen"];
    pClose = brain["pClose"];
    d1Open = brain["d1Open"];
    d1Close = brain["d1Close"];
    d2Open = brain["d2Open"];
    d2Close = brain["d2Close"];
    servoControl();
  }

}

void turnONOffLights()  {
  for (byte i = 0; i < (sizeof(lighPin) / sizeof(lighPin[0])); i++) {
    if (lpL[i] != lp[i]) {
      if (lp[i] == 1) digitalWrite(lighPin[i], HIGH);
      else digitalWrite(lighPin[i], LOW);
      lpL[i] = lp[i];
    }
  }
}

void servoControl() {
  if (d1svL != d1sv) {
    digitalWrite(servoOnPin, HIGH);
    int mapSV = d1sv;
    mapSV = map(mapSV, 0, 100, d1Open, d1Close);
    dumpServo1.write(mapSV, 255, false);
    d1svL = d1sv;
    servoOnCheck = 1;
    servoMillis = millis();
    lastServoDelay = servoMillis;
  }
  if (d2svL != d2sv) {
    digitalWrite(servoOnPin, HIGH);
    int mapSV = d2sv;
    mapSV = map(mapSV, 0, 100, d2Open, d2Close);
    dumpServo2.write(mapSV, 255, false);
    d2svL = d2sv;
    servoOnCheck = 1;
    servoMillis = millis();
    lastServoDelay = servoMillis;
  }
  if (psvL != psv) {
    digitalWrite(servoOnPin, HIGH);
    int mapSV = psv;
    mapSV = map(mapSV, 0, 100, pOpen, pClose);
    dumpServo1.write(mapSV, 255, false);
    psvL = psv;
    servoOnCheck = 1;
    servoMillis = millis();
    lastServoDelay = servoMillis;
  }


  if (servoOnCheck == 1)  {
    servoMillis = millis();
    if (servoMillis - lastServoDelay >= servoDelay) {
      // save the last time you blinked the LED
      Serial.println("OFF");
      lastServoDelay = servoMillis;
      digitalWrite(servoOnPin, LOW);
      servoOnCheck = 0;
    }
  }




}
