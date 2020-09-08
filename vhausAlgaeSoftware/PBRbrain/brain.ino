#include <SoftwareSerial.h>
#include <ArduinoJson.h>
SoftwareSerial brainESP(52, 53); // RX, TX


const size_t capacity = JSON_OBJECT_SIZE(27) + 300;
DynamicJsonDocument brain(capacity);
const char* json[capacity];

int lp1_1, lp1_2, lp1_3, lp1_4,
    lp2_1, lp2_2, lp2_3, lp2_4,
    chillSS, chillOn, heatOn, airSS,
    topUp, phDown, nutMix, samplePump, harvestAM, harbestSS,
    servoOn, press_valve_sv, dump1_valve_sv, dump2_valve_sv,
    pbrPressOpen, pbrPressClose,
    pbrDump1Open, pbrDump1Close,
    pbrDump2Open, pbrDump2Close;

int lp1_1L, lp1_2L, lp1_3L, lp1_4L,
    lp2_1L, lp2_2L, lp2_3L, lp2_4L;

int lp1_1O = 26, lp1_2O = 27, lp1_3O = 28, lp1_4O = 29,
    lp2_1O = 30, lp2_2O = 31, lp2_3O = 32, lp2_4O = 33;

void setup() {
  Serial.begin(115200);
  brainESP.begin(9600);

  pinMode(lp1_1O, OUTPUT);
  pinMode(lp1_2O, OUTPUT);
  pinMode(lp1_3O, OUTPUT);
  pinMode(lp1_4O, OUTPUT);
  pinMode(lp2_1O, OUTPUT);
  pinMode(lp2_2O, OUTPUT);
  pinMode(lp2_3O, OUTPUT);
  pinMode(lp2_4O, OUTPUT);
  digitalWrite(lp1_1O, LOW);
  digitalWrite(lp1_2O, LOW);
  digitalWrite(lp1_3O, LOW);
  digitalWrite(lp1_4O, LOW);
  digitalWrite(lp2_1O, LOW);
  digitalWrite(lp2_2O, LOW);
  digitalWrite(lp2_3O, LOW);
  digitalWrite(lp2_4O, LOW);
}

void loop() {
  readBrainESP();
  turnONOffLights();
}

void readBrainESP() {
  if (brainESP.available() > 0) {
    deserializeJson(brain, brainESP);
    lp1_1 = brain["lp1_1"];
    lp1_2 = brain["lp1_2"];
    lp1_3 = brain["lp1_3"];
    lp1_4 = brain["lp1_4"];
    lp2_1 = brain["lp2_1"];
    lp2_2 = brain["lp2_2"];
    lp2_3 = brain["lp2_3"];
    lp2_4 = brain["lp2_4"];

    chillOn = brain["chillOn"];
    heatOn = brain["heatOn"];
    airSS = brain["airSS"];
    topUp = brain["topUp"];
    phDown = brain["phDown"];
    nutMix = brain["nutMix"];
    samplePump = brain["sPump"];
    harvestAM = brain["hAM"];
    harbestSS = brain["hSS"];
    servoOn = brain["servoOn"];
    press_valve_sv = brain["psv"];
    dump1_valve_sv = brain["d1sv"];
    dump2_valve_sv = brain["d2sv"];
    pbrPressOpen = brain["pOpen"];
    pbrPressClose = brain["pClose"];
    pbrDump1Open = brain["d1Open"];
    pbrDump1Close = brain["d1Close"];
    pbrDump2Open = brain["d2Open"];
    pbrDump2Close = brain["d2Close"];
  }

  Serial.println(lp1_1);
  Serial.println(lp1_2);
  Serial.println(lp1_3);
  Serial.println(lp1_4);
  Serial.println();
  
}

void turnONOffLights()  {
  if (lp1_1L != lp1_1) {
    if (lp1_1 == 1) digitalWrite(lp1_1O, HIGH);
    else digitalWrite(lp1_1O, LOW);
    lp1_1L = lp1_1;
  }
  if (lp1_2L != lp1_2) {
    if (lp1_2 == 1) digitalWrite(lp1_2O, HIGH);
    else digitalWrite(lp1_2O, LOW);
    lp1_2L = lp1_2;
  }
  if (lp1_3L != lp1_3) {
    if (lp1_3 == 1) digitalWrite(lp1_3O, HIGH);
    else digitalWrite(lp1_3O, LOW);
    lp1_3L = lp1_3;
  }
  if (lp1_4L != lp1_4) {
    if (lp1_4 == 1) digitalWrite(lp1_4O, HIGH);
    else digitalWrite(lp1_4O, LOW);
    lp1_4L = lp1_4;
  }

  if (lp2_1L != lp2_1) {
    if (lp2_1 == 1) digitalWrite(lp2_1O, HIGH);
    else digitalWrite(lp2_1O, LOW);
    lp2_1L = lp2_1;
  }
  if (lp2_2L != lp2_2) {
    if (lp2_2 == 1) digitalWrite(lp2_2O, HIGH);
    else digitalWrite(lp2_2O, LOW);
    lp2_2L = lp2_2;
  }
  if (lp2_3L != lp2_3) {
    if (lp2_3 == 1) digitalWrite(lp2_3O, HIGH);
    else digitalWrite(lp2_3O, LOW);
    lp2_3L = lp2_3;
  }
  if (lp2_4L != lp2_4) {
    if (lp2_4 == 1) digitalWrite(lp2_4O, HIGH);
    else digitalWrite(lp2_4O, LOW);
    lp2_4L = lp2_4;
  }

}


