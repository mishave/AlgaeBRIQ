#include <ArduinoJson.h>
#include <VarSpeedServo.h>

//Incoming Data doc from brainESP
const size_t capacity = JSON_OBJECT_SIZE(27) + 300;
DynamicJsonDocument brain(capacity);
const char* json[capacity];

const size_t capacity2 = JSON_OBJECT_SIZE(2);
DynamicJsonDocument docout(capacity2);


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

unsigned long readMillis,lastreadOff,readDelay =5000;
int co2in;
void setup() {
  Serial.begin(115200);
  Serial3.begin(9600);

  //Set Lights
  for (byte i = 0; i < (sizeof(lighPin) / sizeof(lighPin[0])); i++) {
    pinMode(lighPin[i], OUTPUT);
    digitalWrite(lighPin[i], LOW);
  }
  //Set Outputs - taken from outPutPins[] array
  for (byte i = 0; i < (sizeof(outPutPins) / sizeof(outPutPins[0])); i++) {
    pinMode(outPutPins[i], OUTPUT);
    digitalWrite(outPutPins[i], LOW);
  }

  pinMode(A0, INPUT);    // sets the digital pin 7 as input

  
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
  heatCool();
  airWaterPumps();
}

void readBrainESP() {
  if (Serial3.available() > 0) {
    deserializeJson(brain, Serial3);
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
    hAM = brain["hAM"];
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


    docout["sensor"] = co2in;
    docout["time"] = 1351824120;
    serializeJson(docout, Serial3);
    Serial3.println();
  }


  serializeJson(brain, Serial);
  Serial.println();
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
    harvestServo1.write(mapSV, 255, false);
    psvL = psv;
    servoOnCheck = 1;
    servoMillis = millis();
    lastServoDelay = servoMillis;
  }


  if (servoOnCheck == 1)  {
    servoMillis = millis();
    if (servoMillis - lastServoDelay >= servoDelay) {
      // save the last time you blinked the LED
      lastServoDelay = servoMillis;
      digitalWrite(servoOnPin, LOW);
      servoOnCheck = 0;
    }
  }
}

void heatCool() {
  if (chillOn == 1) {
    digitalWrite(coolPin, HIGH);
  }
  else if (chillOn == 0) {
    digitalWrite(coolPin, LOW);
  }
  if (heatOn == 1) {
    digitalWrite(heatPin, HIGH);
  }
  else if (heatOn == 0) {
    digitalWrite(heatPin, LOW);
  }

}

void airWaterPumps()  {
  if (airSS == 1) {
    digitalWrite(airPumpPin, HIGH);
  }
  else if (airSS == 0) {
    digitalWrite(airPumpPin, LOW);
  }
  if (topUp == 1) {
    digitalWrite(wpCultPin, HIGH);
  }
  else if (topUp == 0) {
    digitalWrite(wpCultPin, LOW);
  }
  if (hSS == 1) {
    digitalWrite(wpHarvestPin, HIGH);
  }
  else if (hSS == 0) {
    digitalWrite(wpHarvestPin, LOW);
  }
}

