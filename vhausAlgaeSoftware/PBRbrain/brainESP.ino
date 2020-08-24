
#include <WiFi.h>
#include <PubSubClient.h>
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

// MQTT Network
const char* mqtt_server = "192.168.0.200";
WiFiClient espClient;
PubSubClient client(espClient); // Setup MQTT client
const byte SWITCH_PIN = 0;           // Pin to control the light with
const char *ID = "brainESP32";  // Name of our device, must be unique


const char *phSens = "ph_pv";  // Topic to subcribe to
const char *rtdSen = "rtd_pv";  // Topic to subcribe to
const char *doSen = "do_pv";  // Topic to subcribe to







bool state = 0;
String switch1;
String strTopic;
String strPayload;
int SwitchedPin = 0;

// Home Assistant Credentials
const char *HA_USER = "vhausTech";
const char *HA_PASS = "vhaus";

// Moisture sensor
float soilMoistureValue = 0.0;
float soilmoisturepercent = 0.0;
int lastreading = 0;
int timervalue = 200;

void setup_wifi() {
  Serial.print("\nConnecting to ");

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  Serial.println("started");
  //reset saved settings

  //wifiManager.resetSettings();
  wifiManager.setCustomHeadElement("<style>html{filter: invert(40%); -webkit-filter: invert(40%);}</style>");
  WiFiManagerParameter custom_text("<p>This is just a text paragraph</p>");
  wifiManager.addParameter(&custom_text);
  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  //set custom ip for portal
  wifiManager.setAPStaticIPConfig(IPAddress(100, 100, 100, 100), IPAddress(100, 100, 100, 100), IPAddress(255, 255, 255, 0));
  //wifiManager.setSTAStaticIPConfig(IPAddress(192,168,0,99), IPAddress(192,168,0,1), IPAddress(255,255,255,0));

  wifiManager.autoConnect();
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  while (WiFi.status() != WL_CONNECTED) { // Wait for connection
    delay(500);
    Serial.print(".");
  }

  //if you get here you have connected to the WiFi
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Reconnect to client
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID, HA_USER, HA_PASS)) {
      Serial.println("connected");
      Serial.print("Publishing to: ");
      Serial.println(ID); //*********************************chaged from topic
      Serial.println('\n');
    }
    else {
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  /*StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);*/
  // use the JsonDocument as usual...
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setup_wifi(); // Connect to network
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  sendData();
}

void sendData() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  soilMoistureValue = 80;

  //Tclient.publish(TOPIC, "100");
  float val=random(10,35);
  bool MQTT_Reply = client.publish(TOPIC, String(val).c_str());
  if (MQTT_Reply = true) {
    Serial.println("worked");
  }
  else {
    Serial.println("fail");
    }
    delay(1000);

}