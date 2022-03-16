#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"

#define INTERVAL    2000 // 2 sec delay between publishing

#define CLIENT_ID   "MOTOR_ONE_SWITCH" //client ID | Unique ID for SUMP & motor
#define M1_ON_RELAY 2//SUMP One Motor One Relay One 
#define M1_OFF_RELAY 3//SUMP One Motor One Relay Two

#define M2_ON_RELAY 4//SUMP One Motor OFF Relay One 
#define M2_OFF_RELAY 5//SUMP One Motor OFF Relay Two

#define M3_ON_RELAY 6//SUMP One Motor OFF Relay One 
#define M3_OFF_RELAY 7//SUMP One Motor OFF Relay Two

#define LEVELTOPIC "/sumpone/level"
#define ONLINESTATUS "/motorone/online"
#define MOTORSTATUS "/motorone/motorstatus"
#define DASH_MOTORSTATUS "/motorone/dashStatus"
#define DRYRUNSTATUS "/sumpone/dryrunstatus"
#define DEGENSTATUS "/sumpone/degen"
#define MESSAGES "/messages"


bool motorConditionFlag = false;
bool motorFunctionFlag  = false;
//--------ETherNet Config-----


int failCount = 0;
long previousMillis;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };

IPAddress ip(192, 168, 1, 82);
IPAddress myDns(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

EthernetClient client;
PubSubClient mqttClient;

//--------------resetFunction
void(* resetFunc) (void) = 0;

//--------------CallBack
void callback(char* topic, byte* payload, unsigned int length) {
  char messageBuffer[30];  //somewhere to put the message
  int i = 0;
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("-----Message-----------\n");
  Serial.print("Message:");


  memcpy(messageBuffer, payload, length);  //copy in the payload
  messageBuffer[length] = '\0';  //convert copied payload to a C style string
  Serial.println(messageBuffer);  //print the buffer if you want to

  Serial.println("-----");

  if (strcmp(messageBuffer, "turnOnMotorOne") == 0) {
    Serial.println("Turn On Motor Command Received ");
    mqttClient.publish(MESSAGES, "Motor ONe ON");
    motorConditionFlag = true;
    if(!motorFunctionFlag){
      turnOnMotorOne();
    }
    
  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0) {
    Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(MESSAGES, "Motor ONe OFFF");
    motorConditionFlag = true;
    if(motorFunctionFlag){
      turnOffMotorOne(); 
    }
  }
}

void setup() {
  Serial.begin(9600);
  for (int i = 2; i <= 3; i++) { //pin config of relay
    pinMode(i, OUTPUT);
    digitalWrite(i, !LOW);
  }

  //-----ethernet setup
  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  //  if (Ethernet.begin(mac) == 0) {
  //    Serial.println("Failed to configure Ethernet using DHCP");
  //    // Check for Ethernet hardware present
  //    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
  //      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
  //      while (true) {
  //        delay(1); // do nothing, no point running without Ethernet hardware
  //      }
  //    }
  //    if (Ethernet.linkStatus() == LinkOFF) {
  //      Serial.println("Ethernet cable is not connected.");
  //    }
  //    // try to congifure using IP address instead of DHCP:
  //    Ethernet.begin(mac, ip, myDns);
  //  } else {
  Ethernet.begin(mac, ip, myDns,gateway,subnet);
  Serial.print("  DHCP assigned IP ");
  Serial.println(Ethernet.localIP());
  // give the Ethernet shield a second to initialize:
  delay(1000);

  Serial.println(F("SUMP 1 Level Testor"));

  //setup MQTT client
  mqttClient.setClient(client);
  mqttClient.setCallback(callback);

  //mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setServer("192.168.1.85", 1883); //for using local broker
  //mqttClient.setServer("broker.hivemq.com",1883);

  Serial.println(F("MQTT client configured"));

  Serial.print(" \n Trying to connect as a client -> ");
  int connectionflag = mqttClient.connect(CLIENT_ID);
  Serial.println(connectionflag);

  boolean r;
  r = mqttClient.subscribe(MOTORSTATUS);
  Serial.println("subscribed status MOTORSTATUS");
  Serial.println(r);

  //  r = mqttClient.subscribe(LEVELTOPIC);
  //  Serial.println("subscribed status LEVELTOPIC");
  //  Serial.println(r);
}


bool sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println("Error while publishsing ");
    return false;
  }
  if (motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println("Error while publishsing ");
      return false;
    }
  }
  else if (!motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println("Error while publishsing ");
      return false;
    }
  }
  Serial.println("Published Data ");
  return true;
}
void listenData() {
  //Listen TOPIC for necessory conditions
}

void turnOnMotorOne() {
  //Turn Off by 2Second control of starter box
  motorFunctionFlag = true;
  //M1_ON_RELAY
  digitalWrite(M1_ON_RELAY, !HIGH);
  delay(2000);
  digitalWrite(M1_ON_RELAY, !LOW);
}
void turnOffMotorOne() {
  //Turn Off by 2Second control of starter box
  motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(M1_OFF_RELAY, !HIGH);
  delay(2000);
  digitalWrite(M1_OFF_RELAY, !LOW);
}

void loop() {
  bool sendFlagcheck = false;
  if (millis() - previousMillis > INTERVAL) {
    sendFlagcheck = sendData();
    if (sendFlagcheck == false) {
      failCount ++;
    }
    else if (sendFlagcheck == true) {
      failCount = 0;
    }
    previousMillis = millis();
  }
  if (failCount > 5) {
    Serial.println("Resetting conection");
    resetFunc();
  }
  mqttClient.loop();
}
