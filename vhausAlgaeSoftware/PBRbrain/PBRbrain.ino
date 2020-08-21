#include <Wire.h>
#include <ArduinoJson.h>
#include <VarSpeedServo.h>

VarSpeedServo dumpServo1;
VarSpeedServo dumpServo2;
VarSpeedServo harvestServo;
int dumpServo1Pin = 11;  //input
int dumpServo2Pin = 10;  //input
int harvestServoPin = 9;  //input
byte servoPower = 12;
int dumpServo1LP = 0;
int dumpServo2LP = 0;
int harvestServoLP = 0;
int dumpServo1Closed = 0;
int dumpServo2Closed = 0;
int harvestServoClosed = 0;
int dumpServo1Open = 0;
int dumpServo2Open = 0;
int harvestServoOpen = 0;
byte servoMoved = 0;
unsigned long lastServoOff;
const long servoOffDelay = 4000;

unsigned long startMillis;
unsigned long lastStartDelay = 0;
const long StartDelay = 3000;

byte checkTime = 0;
unsigned long countDownFrom = 20160;
unsigned long remainingCycle = 0;
byte startHarvestFlag = 0;
unsigned long cycleCurrentMillis;
unsigned long lastCycleDelay = 0;
unsigned long cyleDelay = 60000;

int airPowerPin = 3;   //input

unsigned long pbrWaterLowMillis;
unsigned long pbrWaterFullMillis;
const int waterLevelPBR = A1;  //input
const int waterPumpPBR = 7;  //Output

const int waterPumpChiller = 6;

unsigned long lastPumpOn = 0;
unsigned long pumpOffDelay = 100;
unsigned long lastPumpOff = 0;
unsigned long pumpOnDelay = 100;

int phDownPump = 45;  //Output
int phUpPump = 46;  //Output
unsigned long lastpHDownDelay = 0;
unsigned long pHDownDelay = 20000;
unsigned long lastpHUpDelay = 0;
unsigned long pHUpDelay = 20000;
unsigned long lastpHOffDelay = 0;
unsigned long pHOffDelay = 0;
unsigned long pHDowncurrentMillis;
unsigned long pHUpcurrentMillis;
unsigned long pHOffcurrentMillis;
byte pumpOn = 0;

int lp1_1 = 26;  //input
int lp1_2 = 27;
int lp1_3 = 28;
int lp1_4 = 29;
int lp2_1 = 30;
int lp2_2 = 31;
int lp2_3 = 32;
int lp2_4 = 33;
unsigned long luxOncurrentMillis;
unsigned long lastLuxOnDelay = 0;
unsigned long luxOnDelay = 15000;
unsigned long luxOffcurrentMillis;
unsigned long lastLuxOffDelay = 0;
unsigned long luxOffDelay = 15000;
byte lightingON = 0;

int chillOn = 48;  //input
int chillErr = 46;
int chillEmpty = 44;
byte chillState = 0;
int heatOn = 4;  //input
byte heatState = 0;

//I2C Slave Address
const int SensorBoardAdd = 8;
const int DOmeterAdd = 97;
const int RTDmeterAdd = 102;
const int PHmeterAdd = 99;
float b3;

char input[1200];
// I2C Commands
char command = 0;
enum {
  CMD_Pi01 = 1,
  CMD_Pi02 = 2,
  CMD_Pi03 = 3,
  CMD_Pi04 = 4,
  CMD_Pi05 = 5,
  CMD_Pi06 = 6,
  CMD_Pi07 = 7,
  CMD_Pi08 = 8,
  CMD_Pi09 = 9,
  CMD_Pi010 = 10,

  CMD_SB01 = 11,
  CMD_SB02 = 12,
  CMD_SB03 = 13
};

char Atlasdata[20];
float ph_data, do_data, rtd_data;
const int readingDelay = 800;

const size_t capacityIn = JSON_OBJECT_SIZE(58) + 1080;
DynamicJsonDocument docIn(capacityIn);
char json[capacityIn];
DynamicJsonDocument docOut(capacityIn);
DynamicJsonDocument docSen(capacityIn);
DynamicJsonDocument doc1(capacityIn);
DynamicJsonDocument doc2(capacityIn);
DynamicJsonDocument doc3(capacityIn);

byte pbr_auto_man_in, pbr_start_stop_in, light_auto_man_in, light_panel_1_in, lp1_1_in, lp1_2_in,
     lp1_3_in, lp1_4_in, light_panel_2_in, lp2_1_in, lp2_2_in, lp2_3_in, lp2_4_in, temp_auto_man_in,
     temp_start_stop_in, air_auto_man_in, air_start_stop_in, dose_auto_man_in, dose_start_stop_in,
     dose_ph_up_in, dose_ph_down_in, dose_nut_in, dose_sample_in, dose_top_up_in, harvset_auto_mann_in,
     harvest_start_stop_in;
char pbr_status_in, pbr_alarm_in;

float lux_sv_in, lux_pv_in, ph_sv_in, ph_pv_in, do_sv_in, do_pv_in, temp_sv_in, temp_pv_in, turbity_sv_in, turbity_pv_in,
      coil1_sv_in, coil1_pv_in, coil2_sv_in, coil2_pv_in, chiller_temp_sv_in, chiller_temp_pv_in,
      co2_in_sv_in, co2_in_pv_in, co2_out_sv_in, co2_out_pv_in, pressure_sv_in, pressure_pv_in,
      press_valve_sv_in, press_valve_pv_in, dump1_valve_sv_in, dump1_valve_pv_in,
      dump2_valve_sv_in, dump2_valve_pv_in;
long pbr_cycle_length_in, pbr_cycle_remaining_in;

unsigned long EzoMillis;
unsigned long previousEzoMillis;
int ezoDelay = 3000;

int sb1 = 0;
int sb2 = 0;
int sb3 = 0;

int sb4 = 0;
int sb5 = 0;
int sb6 = 0;

int sb7 = 0;
int sb8 = 0;
int sb9 = 0;

int startCycleFlag = 0;

void setup() {
  // put your setup code here, to run once:

  Wire.begin(SensorBoardAdd);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(9600);

  Serial.begin(57600);

  dumpServo1.attach(dumpServo1Pin);  // attaches the servo on pin 9 to the servo object
  dumpServo1.write(0, 125, false); // (Intial pos,Speed(0-255),wait for servo or keep going(true/false))
  dumpServo2.attach(dumpServo2Pin);  // attaches the servo on pin 9 to the servo object
  dumpServo2.write(0, 125, true);
  harvestServo.attach(harvestServoPin);  // attaches the servo on pin 9 to the servo object
  harvestServo.write(0, 125, true);
  pinMode(servoPower, OUTPUT);
  digitalWrite(servoPower, LOW);

  pinMode(airPowerPin, OUTPUT);
  digitalWrite(airPowerPin, LOW);

  pinMode(waterLevelPBR, INPUT);
  pinMode(waterPumpPBR, OUTPUT);
  digitalWrite(waterPumpPBR, LOW);

  pinMode(phDownPump, INPUT);
  pinMode(phUpPump, OUTPUT);
  digitalWrite(phDownPump, LOW);
  digitalWrite(phUpPump, LOW);

  pinMode(chillOn, OUTPUT);
  pinMode(heatOn, OUTPUT);
  digitalWrite(chillOn, LOW);
  digitalWrite(heatOn, LOW);

  pinMode(lp1_1, OUTPUT);
  pinMode(lp1_2, OUTPUT);
  pinMode(lp1_3, OUTPUT);
  pinMode(lp1_4, OUTPUT);
  pinMode(lp2_1, OUTPUT);
  pinMode(lp2_2, OUTPUT);
  pinMode(lp2_3, OUTPUT);
  pinMode(lp2_4, OUTPUT);
  digitalWrite(lp1_1, LOW);
  digitalWrite(lp1_2, LOW);
  digitalWrite(lp1_3, LOW);
  digitalWrite(lp1_4, LOW);
  digitalWrite(lp2_1, LOW);
  digitalWrite(lp2_2, LOW);
  digitalWrite(lp2_3, LOW);
  digitalWrite(lp2_4, LOW);
}

//////////////////////////////////////////////////////////
//              Handles incoming data                   //
//////////////////////////////////////////////////////////
void receiveEvent (int howMany) {
  command = Wire.read ();  // remember command for when we get request
}

//////////////////////////////////////////////////////////
//              Handels outgoing data                   //
//////////////////////////////////////////////////////////
void requestEvent ()  {
  switch (command)  {
      //case CMD_SB01:      dumpSB01(); break;  // send 1st Packet
      //case CMD_SB02:      dumpSB02(); break;  // send 2nd Packet
      //case CMD_SB03:      dumpSB03(); break;  // send 3rd Packet
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  RequestSensorBoard();
  RequestMeterData();
  //mergeDocJSON();
  if (Serial.available() > 0)  {
    RecievePiUpdate();
    Cycle();
    sendBack();
  }
  if (servoMoved == 1)  {
    ServoPowerOffDelay();
  }
}

void Cycle()  {
  autoCycle();                  //Run Auto Cycle
  //manualControl();              //Run Manual Commands
}

void autoCycle()  {
  if (pbr_auto_man_in == 1 && pbr_start_stop_in == 1) {
    startMillis = millis();
    if (startMillis - lastStartDelay >= StartDelay) {
      // save the last time you blinked the LED
      lastStartDelay = startMillis;
      startCycleFlag = 1;
    }
  }
  else  {
    startCycleFlag = 0;
  }

  if (startCycleFlag == 1)  {
    checkTimeRemaining(); //Check Time Remaining and Update
    setServosClosed();    //Set Dump Valves Are Closed
    checkAir();           //Check That Air Is On
    checkWaterLevel();    //Check Water Level and Top Up
    checkPh();            //Check pH Level
    //checkTemp();          //Check Tempreture
    //checkLighting();      //Check Light Status
    reportStatus();

  }
}
void checkTimeRemaining()   {
  //check time at start of cycle

  if (startCycleFlag == 1) {
    if (checkTime == 0) {
      countDownFrom = pbr_cycle_length_in;
      remainingCycle = countDownFrom;
      pbr_cycle_remaining_in = remainingCycle;
      checkTime = 1;
    }
    cycleCurrentMillis = millis();
    if (cycleCurrentMillis - lastCycleDelay >= cyleDelay && startCycleFlag == 1) {
      // save the last time you blinked the LED
      lastCycleDelay = cycleCurrentMillis;
      remainingCycle = --remainingCycle;
      pbr_cycle_remaining_in = remainingCycle;
    }

    if (startCycleFlag == 1 && pbr_cycle_remaining_in == 0) {
      startHarvestFlag = 1;
    }
  }
}

void setServosClosed()  {

  if (startCycleFlag == 1 && harvset_auto_mann_in == 1) {

    if (dumpServo1LP != dumpServo1Closed || dumpServo2LP != dumpServo2Closed )  {
      digitalWrite(servoPower, HIGH);
      dumpServo1.write(dumpServo1Closed, 125, false); // (Intial pos,Speed(0-255),wait for servo or keep going(true/false))
      dumpServo1LP = dumpServo1Closed;
      dump1_valve_sv_in = dumpServo1Closed;
      dump1_valve_pv_in = dumpServo1Closed;
      dumpServo2.write(dumpServo2Closed, 125, false);
      dumpServo2LP = dumpServo2Closed;
      dump2_valve_sv_in = dumpServo2Closed;
      dump2_valve_pv_in = dumpServo2Closed;
      //harvestServo.write(harvestServoClosed,125,false);
      //harvestServoLP = harvestServoClosed;
      //press_valve_sv_in = harvestServoClosed;
      //press_valve_pv_in = harvestServoClosed;
      servoMoved = 1;
    }
  }
}

void ServoPowerOffDelay() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastServoOff >= servoOffDelay) {
    // save the last time you blinked the LED
    lastServoOff = currentMillis;
    digitalWrite(servoPower, LOW);
    servoMoved = 0;
  }
}

void checkAir() {
  if (startCycleFlag == 1 && air_auto_man_in == 1) {
    digitalWrite(airPowerPin, HIGH);
    air_start_stop_in = 1;
  }
}

void checkWaterLevel()  {
  if (startCycleFlag == 1 && dose_auto_man_in == 1) {
    int reading = digitalRead(waterLevelPBR);
    if (reading == 0) {
      pbrWaterLowMillis = millis();
      if (pbrWaterLowMillis - lastPumpOn >= pumpOnDelay) {
        // save the last time you blinked the LED
        lastPumpOn = pbrWaterLowMillis;
        digitalWrite(waterPumpPBR, HIGH);
        dose_top_up_in = 1;
      }
    }
    if (reading == 1) {
      pbrWaterFullMillis = millis();
      if (pbrWaterFullMillis - lastPumpOff >= pumpOffDelay) {
        // save the last time you blinked the LED
        lastPumpOff = pbrWaterFullMillis;
        digitalWrite(waterPumpPBR, LOW);
        dose_top_up_in = 0;
      }
    }
  }
}

void checkPh()  {
  if (startCycleFlag == 1 && dose_auto_man_in == 1) {
    //int reading = ph_pv_in;
    if (ph_pv_in >= (ph_sv_in + 1)) {
      pHDowncurrentMillis = millis();
      if (pHDowncurrentMillis - lastpHDownDelay >= pHDownDelay) {
        // save the last time you blinked the LED
        lastpHDownDelay = pHDowncurrentMillis;
        digitalWrite(phDownPump, HIGH);
        dose_ph_down_in = 1;
        pumpOn = 1;
      }
    }

    if (ph_pv_in <= (ph_sv_in - 1)) {
      pHUpcurrentMillis = millis();
      if (pHUpcurrentMillis - lastpHUpDelay >= pHUpDelay) {
        // save the last time you blinked the LED
        lastPumpOff = pHUpcurrentMillis;
        digitalWrite(phUpPump, HIGH);
        dose_ph_up_in = 1;
        pumpOn = 1;
      }
    }
  }

  if (pumpOn == 1) {
    pHOffcurrentMillis = millis();
    if (pHOffcurrentMillis - lastpHOffDelay >= pHOffDelay) {
      // save the last time you blinked the LED
      lastpHOffDelay = pHOffcurrentMillis;
      digitalWrite(phUpPump, LOW);
      digitalWrite(phDownPump, LOW);
      dose_ph_up_in = 0;
      dose_ph_down_in = 0;
      pumpOn = 0;
    }
  }
}

void checkTemp()  {
  if (startCycleFlag == 1 && temp_auto_man_in == 1) {
    //int reading = ph_pv_in;
    if (temp_pv_in >= (temp_sv_in + 3) && chillState == 0) {
      digitalWrite(chillOn, HIGH);
      chillState = 1;
      temp_start_stop_in = 1;
    }
    if (temp_pv_in == (temp_sv_in) && chillState == 1) {
      digitalWrite(chillOn, LOW);
      chillState = 0;
      temp_start_stop_in = 0;
    }
    if (temp_pv_in <= (temp_sv_in - 3) && heatState == 0) {
      digitalWrite(heatOn, HIGH);
      heatState = 1;
      temp_start_stop_in = 1;
    }
    if (temp_pv_in == (temp_sv_in) && heatState == 1) {
      digitalWrite(heatOn, LOW);
      heatState = 0;
      temp_start_stop_in = 0;
    }
  }
}

void checkLighting()  {
  if (startCycleFlag == 1 && light_auto_man_in == 1) {

    //light cycle time length
    //light cycle start time
    //Light cycle Off Time
    if (lux_pv_in >= lux_sv_in) {
      luxOncurrentMillis = millis();
      if (luxOncurrentMillis - lastLuxOnDelay >= luxOnDelay) {
        // save the last time you blinked the LED
        lastLuxOnDelay = luxOncurrentMillis;
        digitalWrite(lp1_1, HIGH);
        digitalWrite(lp1_2, HIGH);
        digitalWrite(lp1_3, HIGH);
        digitalWrite(lp1_4, HIGH);
        digitalWrite(lp2_1, HIGH);
        digitalWrite(lp2_2, HIGH);
        digitalWrite(lp2_3, HIGH);
        digitalWrite(lp2_4, HIGH);
        light_panel_1_in = 1;
        lp1_1_in = 1;
        lp1_2_in = 1;
        lp1_3_in = 1;
        lp1_4_in = 1;
        light_panel_2_in = 1;
        lp2_1_in = 1;
        lp2_2_in = 1;
        lp2_3_in = 1;
        lp2_4_in = 1;
        lightingON = 1;
      }
    }

    if (lux_pv_in <= (lux_sv_in - 10)) {
      luxOffcurrentMillis = millis();
      if (luxOffcurrentMillis - lastLuxOffDelay >= luxOffDelay) {
        // save the last time you blinked the LED
        lastLuxOffDelay = luxOffcurrentMillis;
        digitalWrite(lp1_1, LOW);
        digitalWrite(lp1_2, LOW);
        digitalWrite(lp1_3, LOW);
        digitalWrite(lp1_4, LOW);
        digitalWrite(lp2_1, LOW);
        digitalWrite(lp2_2, LOW);
        digitalWrite(lp2_3, LOW);
        digitalWrite(lp2_4, LOW);
        light_panel_1_in = 0;
        lp1_1_in = 0;
        lp1_2_in = 0;
        lp1_3_in = 0;
        lp1_4_in = 0;
        light_panel_2_in = 0;
        lp2_1_in = 0;
        lp2_2_in = 0;
        lp2_3_in = 0;
        lp2_4_in = 0;
        lightingON = 0;
      }
    }
  }
}

void reportStatus() {
  if (startCycleFlag == 1)  {
    pbr_status_in = 1;
    pbr_alarm_in = 2;

  }
}

void manualControl() {

  /*unsigned long currentMillis = millis();
    dump1_valve_pv_in = map(dump1_valve_sv_in, 0, 100, 0, 255);
    dumpServo1LP = dump1_valve_sv_in;
    dump2_valve_pv_in = map(dump2_valve_sv_in, 0, 100, 0, 255);
    dumpServo2LP = dump2_valve_sv_in;
    press_valve_pv_in = map(press_valve_sv_in, 0, 100, 0, 255);
    harvestServoLP = press_valve_sv_in;
  */
  if (pbr_auto_man_in == 0 || pbr_start_stop_in == 0) {
    pbr_start_stop_in = 0;
    startCycleFlag = 0;
  }

  if (light_auto_man_in == 0) {
    if (light_panel_1_in == 1) {
      digitalWrite(lp1_1, HIGH);
      digitalWrite(lp1_2, HIGH);
      digitalWrite(lp1_3, HIGH);
      digitalWrite(lp1_4, HIGH);
      lp1_1_in = 1;
      lp1_2_in = 1;
      lp1_3_in = 1;
      lp1_4_in = 1;
    }
    if (light_panel_2_in == 1) {
      digitalWrite(lp2_1, HIGH);
      digitalWrite(lp2_2, HIGH);
      digitalWrite(lp2_3, HIGH);
      digitalWrite(lp2_4, HIGH);
      lp2_1_in = 1;
      lp2_2_in = 1;
      lp2_3_in = 1;
      lp2_4_in = 1;
    }
    if (light_panel_1_in == 0) {
      digitalWrite(lp1_1, LOW);
      digitalWrite(lp1_2, LOW);
      digitalWrite(lp1_3, LOW);
      digitalWrite(lp1_4, LOW);
      lp1_1_in = 0;
      lp1_2_in = 0;
      lp1_3_in = 0;
      lp1_4_in = 0;
    }
    if (light_panel_2_in == 0) {
      digitalWrite(lp2_1, LOW);
      digitalWrite(lp2_2, LOW);
      digitalWrite(lp2_3, LOW);
      digitalWrite(lp2_4, LOW);
      lp2_1_in = 0;
      lp2_2_in = 0;
      lp2_3_in = 0;
      lp2_4_in = 0;
    }
  }

  if (dose_auto_man_in == 1)  {
    dose_ph_up_in = 0;
    dose_ph_down_in = 0;
    dose_nut_in = 0;
    dose_sample_in = 0;
    dose_top_up_in = 0;
  }

}

void RecievePiUpdate() {
  deserializeJson(docIn, Serial);
  pbr_auto_man_in = docIn["pbr_auto_man"];
  pbr_start_stop_in = docIn["pbr_start_stop"];
  light_auto_man_in = docIn["light_auto_man"];
  light_panel_1_in = docIn["lp_1"];
  lp1_1_in = docIn["lp1_1"];
  lp1_2_in = docIn["lp1_2"];
  lp1_3_in = docIn["lp1_3"];
  lp1_4_in = docIn["lp1_4"];
  light_panel_2_in = docIn["lp_2"];
  lp2_1_in = docIn["lp2_1"];
  lp2_2_in = docIn["lp2_2"];
  lp2_3_in = docIn["lp2_3"];
  lp2_4_in = docIn["lp2_4"];
  temp_auto_man_in = docIn["temp_auto_man"];
  temp_start_stop_in = docIn["temp_start_stop:"];
  air_auto_man_in = docIn["air_auto_man"];
  air_start_stop_in = docIn["air_start_stop"];
  dose_auto_man_in = docIn["dose_auto_man"];
  dose_start_stop_in = docIn["dose_start_stop"];
  dose_ph_up_in = docIn["dose_ph_up"];
  dose_ph_down_in = docIn["dose_ph_down"];
  dose_nut_in = docIn["dose_nut"];
  dose_sample_in = docIn["dose_sample"];
  dose_top_up_in = docIn["dose_top_up"];
  harvset_auto_mann_in = docIn["harvset_auto_mann"];
  harvest_start_stop_in = docIn["harvest_start_stop"];

  pbr_cycle_length_in = docIn["pbr_cycle_length"];
  pbr_cycle_remaining_in = docIn["pbr_cycle_remaining"];

  lux_sv_in = docIn["lux_sv"];
  lux_pv_in = docIn["lux_pv"];
  ph_sv_in = docIn["ph_sv"];
  ph_pv_in = docIn["ph_pv"];
  do_sv_in = docIn["do_sv"];
  do_pv_in = docIn["do_pv"];
  temp_sv_in = docIn["temp_sv"];
  temp_pv_in = docIn["temp_pv"];
  turbity_sv_in = docIn["turbity_sv"];
  turbity_pv_in = docIn["turbity_pv"];
  coil1_sv_in = docIn["coil1_sv"];
  coil1_pv_in = docIn["coil1_pv"];
  coil2_sv_in = docIn["coil2_sv"];
  coil2_pv_in = docIn["coil2_pv"];
  chiller_temp_sv_in = docIn["chiller_temp_sv"];
  chiller_temp_pv_in = docIn["chiller_temp_pv"];
  co2_in_sv_in = docIn["co2_in_sv"];
  co2_in_pv_in = docIn["co2_in_pv"];
  co2_out_sv_in = docIn["co2_out_sv"];
  co2_out_pv_in = docIn["co2_out_pv"];
  pressure_sv_in = docIn["pressure_sv"];
  pressure_pv_in = docIn["pressure_pv"];
  press_valve_sv_in = docIn["press_valve_sv"];
  press_valve_pv_in = docIn["press_valve_pv"];
  dump1_valve_sv_in = docIn["dump1_valve_sv"];
  dump1_valve_pv_in = docIn["dump1_valve_pv"];
  dump2_valve_sv_in = docIn["dump2_valve_sv"];
  dump2_valve_pv_in = docIn["dump2_valve_pv"];
  pbr_status_in = docIn["pbr_status"];
  pbr_alarm_in = docIn["pbr_alarm"];
}

void sendBack() {
  docOut["pbr_auto_man"] = pbr_auto_man_in;
  docOut["pbr_start_stop"] = pbr_start_stop_in;
  docOut["light_auto_man"] = light_auto_man_in;
  docOut["lp_1"] = light_panel_1_in;
  docOut["lp1_1"] = lp1_1_in;
  docOut["lp1_2"] = lp1_2_in;
  docOut["lp1_3"] = lp1_3_in;
  docOut["lp1_4"] = lp1_4_in;
  docOut["lp_2"] = light_panel_2_in;
  docOut["lp2_1"] = lp2_1_in;
  docOut["lp2_2"] = lp2_2_in;
  docOut["lp2_3"] = lp2_3_in;
  docOut["lp2_4"] = lp2_4_in;
  docOut["temp_auto_man"] = temp_auto_man_in;
  docOut["temp_start_stop"] = temp_start_stop_in;
  docOut["air_auto_man"] = air_auto_man_in;
  docOut["air_start_stop"] = air_start_stop_in;
  docOut["dose_auto_man"] = dose_auto_man_in;
  docOut["dose_start_stop"] = dose_start_stop_in;
  docOut["dose_ph_up"] = dose_ph_up_in;
  docOut["dose_ph_down"] = dose_ph_down_in;
  docOut["dose_nut"] = dose_nut_in;
  docOut["dose_sample"] = dose_sample_in;
  docOut["dose_top_up"] = dose_top_up_in;
  docOut["harvset_auto_mann"] = harvset_auto_mann_in;
  docOut["harvest_start_stop"] = harvest_start_stop_in;
  docOut["pbr_cycle_length"] = pbr_cycle_length_in;
  docOut["pbr_cycle_remaining"] = pbr_cycle_remaining_in;
  docOut["lux_sv"] = lux_sv_in;
  docOut["lux_pv"] = sb4;
  docOut["ph_sv"] = ph_sv_in;
  docOut["ph_pv"] = sb1;
  docOut["do_sv"] = do_sv_in;
  docOut["do_pv"] = sb3;
  docOut["temp_sv"] = temp_sv_in;
  docOut["temp_pv"] = sb2;
  docOut["turbity_sv"] = turbity_sv_in;
  docOut["turbity_pv"] = turbity_pv_in;
  docOut["coil1_sv"] = coil1_sv_in;
  docOut["coil1_pv"] = coil1_pv_in;
  docOut["coil2_sv"] = coil2_sv_in;
  docOut["coil2_pv"] = coil2_pv_in;
  docOut["chiller_temp_sv"] = chiller_temp_sv_in;
  docOut["chiller_temp_pv"] = chiller_temp_pv_in;
  docOut["co2_in_sv"] = co2_in_sv_in;
  docOut["co2_in_pv"] = co2_in_pv_in;
  docOut["co2_out_sv"] = co2_out_sv_in;
  docOut["co2_out_pv"] = sb7;
  docOut["pressure_sv"] = pressure_sv_in;
  docOut["pressure_pv"] = pressure_pv_in;
  docOut["press_valve_sv"] = press_valve_sv_in;
  docOut["press_valve_pv"] = press_valve_pv_in;
  docOut["dump1_valve_sv"] = dump1_valve_sv_in;
  docOut["dump1_valve_pv"] = dump1_valve_pv_in;
  docOut["dump2_valve_sv"] = dump2_valve_sv_in;
  docOut["dump2_valve_pv"] = dump2_valve_pv_in;
  docOut["pbr_status"] = pbr_status_in;
  docOut["pbr_alarm"] = pbr_alarm_in;
  serializeJson(docOut, Serial); //Return Current Readings
  Serial.println();
}

//////////////////////////////////////////////////////////
//              Read Sensor Board data and              //
//                   combined for use                   //
//////////////////////////////////////////////////////////
void RequestSensorBoard() {
  const size_t capacity = JSON_OBJECT_SIZE(3) + 20;
  DynamicJsonDocument doc(300);
  if (Wire.requestFrom (SensorBoardAdd, capacity) == 0) {
    //Use to throw an error need to think about this ;/
    //Serial.println("Sensor Board Error::No reply");
  }
  else  {
    sendCommand (SensorBoardAdd, CMD_SB01, capacity);
    char json[capacity];
    DynamicJsonDocument doc1(capacity);
    deserializeJson(doc1, Wire);
    sb1 = doc1["s1"]; // 10
    sb2 = doc1["s2"]; // 11
    sb3 = doc1["s3"]; // 23

    sendCommand (SensorBoardAdd, CMD_SB02, capacity);
    DynamicJsonDocument doc2(capacity);
    deserializeJson(doc2, Wire);
    sb4 = doc2["s4"]; // 10
    sb5 = doc2["s5"]; // 11
    sb6 = doc2["s6"]; // 23

    sendCommand (SensorBoardAdd, CMD_SB03, capacity);
    DynamicJsonDocument doc3(capacity);
    deserializeJson(doc3, Wire);
    sb7 = doc3["s7"]; // 10
    sb8 = doc3["s8"]; // 11
    sb9 = doc3["s9"]; // 23

    //Combine messages for use
    merge(docSen.as<JsonVariant>(), doc1.as<JsonVariant>());
    merge(docSen.as<JsonVariant>(), doc2.as<JsonVariant>());
    merge(docSen.as<JsonVariant>(), doc3.as<JsonVariant>());
    //serializeJsonPretty(doc, Serial);
  }

}

//////////////////////////////////////////////////////////
//             Request EZO meter readings               //
//////////////////////////////////////////////////////////
void RequestMeterData() {
  EzoMillis = millis();
  if (EzoMillis - previousEzoMillis > ezoDelay) {
    previousEzoMillis = EzoMillis;
    sendCommand(PHmeterAdd, 'r', 20);
    delay(readingDelay);                                  //if it is the sleep command, we do nothing. Issuing a sleep command and then requesting data will wake the RTD circuit.
    ph_data = readMeter(PHmeterAdd, 20, 1);

    sendCommand(RTDmeterAdd, 'r', 20);
    delay(readingDelay);                                  //if it is the sleep command, we do nothing. Issuing a sleep command and then requesting data will wake the RTD circuit.
    rtd_data = readMeter(RTDmeterAdd, 20, 1);

    sendCommand(DOmeterAdd, 'r', 20);
    delay(readingDelay);                                  //if it is the sleep command, we do nothing. Issuing a sleep command and then requesting data will wake the RTD circuit.
    do_data = readMeter(DOmeterAdd, 20, 1);
    if (ph_data != 0) {
      //Serial.println(ph_data);              //print the data.
    }

  }

}

//////////////////////////////////////////////////////////
//              Read EZO meter reading                  //
//////////////////////////////////////////////////////////
float readMeter(const int WireAdd, const int cmd, const int responseSize)  {
  char dataStorage[20];
  byte i = 0;
  Wire.requestFrom(WireAdd, cmd, responseSize);  //call the circuit and request 20 bytes (this may be more than we need)
  int code = Wire.read();

  while (Wire.available()) {
    byte in_char = Wire.read();
    dataStorage[i] = in_char;           //load byte into our array.
    i += 1;                             //next bit.
    if (in_char == 0) {                 //throw null error.
      i = 0;                            //reset the counter i to 0.
      break;                            //exit
    }
  }
  float datafloat = atof(dataStorage);
  return datafloat;
}

//////////////////////////////////////////////////////////
//              Send Request for data                   //
//////////////////////////////////////////////////////////
void sendCommand (const int WireAdd, const byte cmd, const int responseSize)  {
  Wire.beginTransmission (WireAdd);
  Wire.write (cmd);
  Wire.endTransmission ();

  if (WireAdd != PHmeterAdd) {
    Wire.requestFrom (WireAdd, responseSize);
  }
}

//////////////////////////////////////////////////////////
//              Merge JSON messages                     //
/////////////////////////////////////////////////////////
void merge(JsonVariant dst, JsonVariantConst src) {
  if (src.is<JsonObject>()) {
    for (auto kvp : src.as<JsonObject>()) {
      merge(dst.getOrAddMember(kvp.key()), kvp.value());
    }
  }
  else {
    dst.set(src);
  }
}
