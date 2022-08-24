// I am not a developer, jsut an dangerous amateur who knows how to google. 
// I get code "working" there's no refinment here and no doubt this code is full of errors, but it works and doesn't crash.
// combine the ESP8266 with a decent super heterodyne 433 mhz receiver (look for ones with crystals on them)
// The ESP32 stuff foesn't work with the lightwave library, so this only works on ESP8266
// MQTT Publish topic is hard coded because.

#if defined (ESP8266)
#include <ESP8266WiFi.h>
#elif defined (ESP32)
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

#endif

#include <PubSubClient.h>
#include <LwRx.h>


char *SSID = "<SSID>"; // this compiles in vscode but throws and error. Compiles fine in Arduino IDE
char *PWD = "<PASSWORD>"; // this compiles in vscode but throws and error. Compiles fine in Arduino IDE
char *mqttServer = "IP Address"; 
int mqttPort = 1883;
#if defined(ESP8266)
  String newHostname = "lwrf433gw";
#elif defined(ESP32)
  const char* newHostname = "lwrf433gw";
#endif

bool rc = 0;
//MQTT client
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

//LED stuff
#define LED LED_BUILTIN //Define blinking LED pin
static const unsigned long REFRESH_INTERVAL = 1000; // ms
static unsigned long previousMillis = 0;
int ledState = LOW;


//lwrf constants
long last_time = 0;
char data[100];

#if defined (ESP8266)
  #define RX_PIN  D6 //27 gpio ESP32
#elif defined (ESP32)
  #define RX_PIN  27 //27 gpio ESP32
#endif

unsigned long millisOffset = millis();

char cDataBuffer[8];
//Msg data


//Repeats data
static byte repeats = 0;
static byte timeout = 20;

//pair data
static byte pairtimeout = 50;
static byte pairEnforce = 0;
static byte pairBaseOnly = 0;

//Serial message input
const byte maxvalues = 10;
byte indexQ;
boolean newvalue;
int invalues[maxvalues];
char *msgstr;
byte msglen = 10;
byte message[10] = { 0x80 , 0x15, 2 ,15 ,3 , 6, 0 ,4 ,13 };


void connectToWiFi() {
	Serial.println("");
	Serial.println("");
	Serial.println("");
	Serial.print("Connecting to ");

  WiFi.begin(SSID, PWD);
  Serial.println(SSID);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  

  //Set new hostname
  Serial.println("");


#if defined (ESP8266)
  WiFi.hostname(newHostname);
#elif defined (ESP32)
  WiFi.setHostname(newHostname);
#endif

  Serial.print("Default hostname: ");
//  Serial.println(WiFi.hostname());
  Serial.print("Connected to : ");
  Serial.println(SSID);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MQTT Publish : home/433toMQTT");

}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Callback - ");
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  rc = mqttClient.connect("lwrf433gw", "", "", "lwrf433gw/status", 0, 0, "online", 0);
//  set the callback function
//  mqttClient.setCallback(callback);
}


void setup() {
	pinMode(LED, OUTPUT); // Initialize the LED pin as an output
	lwrx_setup(RX_PIN);
  Serial.begin(9600);
   connectToWiFi();


  setupMQTT();

}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker..");
    String clientId = "lwrfgw";

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("Connected.");
      // subscribe to topic
      mqttClient.subscribe("/swa/commands");
    }

  }
}

void loop() {


	if (!mqttClient.connected())
		reconnect();

	mqttClient.loop();
	String vmsg;

	if (lwrx_message()) {
		//Serial.println("Got Something");
		lwrx_getmessage(message, msglen);

		char hexadecimalnum[10];
		for (byte i = 0; i < msglen; i++) {
			sprintf(hexadecimalnum, "%X", message[i]);
			vmsg = vmsg + hexadecimalnum;
			if (ledState == LOW) {
				ledState = HIGH;
			}
			else {
				ledState = LOW;
			}
			digitalWrite(LED, ledState);

		}
		Serial.println(vmsg);
		Serial.println(" ");
		int str_len = vmsg.length() + 1;
		char char_array[str_len];
		vmsg.toCharArray(char_array, str_len);
		char* token = strtok(char_array, " ");

		//msgstr = vmsg;
		mqttClient.publish("home/433toMQTT", token);
	}

	unsigned long currentMillis = millis();

	if (currentMillis - previousMillis >= REFRESH_INTERVAL) {
		// save the last time you blinked the LED
		previousMillis = currentMillis;

		// if the LED is off turn it on and vice-versa:
		if (ledState == LOW) {
			ledState = HIGH;
		}
		else {
			ledState = LOW;
		}
		digitalWrite(LED, ledState);


	}
}




void printMsg(byte *message, byte len) {
  //Serial.print(millis() - millisOffset);
  //Serial.print(" ");
	for (int i = 0; i < len; i++) {
		Serial.print(message[i]);
		Serial.print(", ");
		if (ledState == LOW) {
			ledState = HIGH;
		}
		else {
			ledState = LOW;
		}
		digitalWrite(LED, ledState);
	
	}
  Serial.println();
}
