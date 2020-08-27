#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

#define RXD2 16
#define TXD2 17

unsigned long updateCurrentMillis;
unsigned long lastUpdateDelay;
unsigned long updateDelay = 1000;

// MQTT Network
const char* mqtt_server = "192.168.0.200";

// Home Assistant Credentials
const char *HA_USER = "vhausTech";
const char *HA_PASS = "vhaus";

//Node ID
const char *ID = "sensorESP32";  // Name of our device, must be unique

int pbrAM, pbrSS, lightAM,
    lp1, lp1_1, lp1_2, lp1_3, lp1_4,
    lp2, lp2_1, lp2_2, lp2_3, lp2_4,
    chillAM, chillSS, airAM, airSS,
    doseAM, phUp, phDown, nutMix, samplePump, topUp,
    harvestAM, harbestSS;

float luxPV, phPV, doPV, tempPV, TurbPV, co2InPV, co2OutPV, presPV;
float luxSV, phSV, doSV, tempSV, TurbSV, co2InSV, co2OutSV, presSV;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = topic;
  Serial.print("Topic: ");
  Serial.println(topicStr);
  payload[length] = '\0';
  String payloadStr = String((char*)payload);
  Serial.println(payloadStr);

  if (topicStr == "pbr/pbrAM/switch") {
    if (payloadStr == "ON") client.publish("pbr/pbrAM/status", "ON"), pbrAM = 1;
    else if (payloadStr == "OFF") client.publish("pbr/pbrAM/status", "OFF"), pbrAM = 0;
  }

}

// Reconnect to client
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID, HA_USER, HA_PASS)) {
      Serial.println("connected");
      //publish test connection
      client.publish("outTopic", "hello world");
      //subscribe to topics
      //Switches
      //client.subscribe("brainOutSV1");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial Txd is on pin: " + String(TX));
  Serial.println("Serial Rxd is on pin: " + String(RX));

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  setup_wifi(); // Connect to network
  delay(1500);
}

void setup_wifi() {
  Serial.print("\nConnecting to ");
  //WiFiManager
  WiFiManager wifiManager;
  Serial.println("started");
  //wifiManager.resetSettings(); // turn off after intital settingfs are done
  wifiManager.setCustomHeadElement("<style>html{filter: invert(40%); -webkit-filter: invert(40%);}</style>");
  WiFiManagerParameter custom_text("<p>This is just a text paragraph</p>");
  wifiManager.addParameter(&custom_text);
  wifiManager.setAPStaticIPConfig(IPAddress(100, 100, 100, 100), IPAddress(100, 100, 100, 100), IPAddress(255, 255, 255, 0));
  wifiManager.autoConnect();

  while (WiFi.status() != WL_CONNECTED) { // Wait for connection
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  updateCurrentMillis = millis(); //send update every x seconds
  if (updateCurrentMillis - lastUpdateDelay >= updateDelay) {
    lastUpdateDelay = updateCurrentMillis;
    packetUpDate();  //dump data
  }
  while (Serial2.available()) {
    const size_t capacity = JSON_OBJECT_SIZE(8) + 90;
    DynamicJsonDocument doc(capacity);
    const char* json[capacity];
    deserializeJson(doc, Serial2);
    luxPV = doc["luxPV"]; // 21.3
    phPV = doc["phPV"]; // 14.02
    doPV = doc["doPV"]; // 0
    tempPV = doc["tempPV"]; // 0
    TurbPV = doc["TurbPV"]; // 0
    co2InPV = doc["co2InPV"]; // 0
    co2OutPV = doc["co2OutPV"]; // 0
    presPV = doc["presPV"]; // 0
    serializeJson(doc, Serial);
    Serial.println();
  }
}

void packetUpDate() {
  const size_t capacity = JSON_OBJECT_SIZE(26);
  DynamicJsonDocument doc1(capacity);
  DynamicJsonDocument doc2(capacity);
  doc1["luxPV"] = luxPV;
  doc1["phPV"] = phPV;
  doc1["doPV"] = doPV;
  doc1["tempPV"] = tempPV;
  doc1["TurbPV"] = TurbPV;
  doc1["co2InPV"] = co2InPV;
  doc1["co2OutPV"] = co2OutPV;
  doc1["presPV"] = presPV;
  char buffer[256];
  size_t n = serializeJson(doc1, buffer);
  client.publish("sensorOutPV1", buffer, n);
  doc2["luxSV"] = luxSV;
  doc2["phSV"] = phSV;
  doc2["doSV"] = doSV;
  doc2["tempSV"] = tempSV;
  doc2["TurbSV"] = TurbSV;
  doc2["co2InSV"] = co2InSV;
  doc2["co2OutSV"] = co2OutSV;
  doc2["presSV"] = presSV;
  n = serializeJson(doc2, buffer);
  client.publish("sensorOutSV1", buffer, n);
}