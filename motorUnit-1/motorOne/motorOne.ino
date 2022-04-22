#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"

#define INTERVAL    2000 // 2 sec delay between publishing

#define CLIENT_ID   "Tower_ONE_SWITCH" //client ID | Unique ID for SUMP & motor

#define T1_DOM_ON 2
#define T1_DOM_OFF 3

#define T1_FLU_ON 4
#define T1_FLU_OFF 5

#define T2_DOM_ON 6
#define T2_DOM_OFF 7

#define T2_FLU_ON 8
#define T2_FLU_OFF 9

#define T3_DOM_ON 7
#define T3_DOM_OFF 7

#define T3_FLU_ON 14
#define T3_FLU_OFF 15

//-----------------------------
#define T1_DOM_LEVELTOPIC "/towerone/dom/level"
#define T2_DOM_LEVELTOPIC "/towertwo/dom/level""
#define T3_DOM_LEVELTOPIC "/towerthree/dom/level"

#define T1_FLU_LEVELTOPIC  "/towerone/flu/level"
#define T2_FLU_LEVELTOPIC  "/towertwo/flu/level"
#define T3_FLU_LEVELTOPIC  "/towerthree/flu/level"
//-----------------------------
///towertwo/dom/switch

#define UNIT_1_ONLINESTATUS "/unitone/online" //Unit one includes T1, T2, and T3. To reduce unwanted memory coneception this will be better 

#define T1_DOM_MOTORSTATUS "/towerone/dom/motorstatus"
#define T2_DOM_MOTORSTATUS "/towertwo/dom/motorstatus"
#define T3_DOM_MOTORSTATUS "/towerthree/dom/motorstatus"

#define T1_FLU_MOTORSTATUS "/towerone/flu/motorstatus"
#define T2_FLU_MOTORSTATUS "/towertwo/flu/motorstatus"
#define T3_FLU_MOTORSTATUS "/towerthree/flu/motorstatus"

#define T1_DOM_DASH_MOTORSTATUS "/towerone/dom/dashStatus"
#define T2_DOM_DASH_MOTORSTATUS "/towertwo/dom/dashStatus"
#define T3_DOM_DASH_MOTORSTATUS "/towerthree/dom/dashStatus"

#define T1_FLU_DASH_MOTORSTATUS "/towerone/flu/dashStatus"
#define T2_FLU_DASH_MOTORSTATUS "/towertwo/flu/dashStatus"
#define T3_FLU_DASH_MOTORSTATUS "/towerthree/flu/dashStatus"

// #define DRYRUNSTATUS "/sumpone/dryrunstatus"
// #define DRYRUNSTATUS "/sumpone/dryrunstatus"
// #define DRYRUNSTATUS "/sumpone/dryrunstatus"

// #define DEGENSTATUS "/sumpone/degen"
// #define DEGENSTATUS "/towertwo/degen"
// #define DEGENSTATUS "/towerthree/degen"

#define T1_MESSAGES "/towerone/messages"
#define T2_MESSAGES "/towertwo/messages"
#define T3_MESSAGES "/towerthree/messages"


//-----Motor Condition
bool t1_dom_motorConditionFlag = false;
bool t1_flu_motorConditionFlag = false;

bool t2_dom_motorConditionFlag = false;
bool t2_flu_motorConditionFlag = false;

bool t3_dom_motorConditionFlag = false;
bool t3_flu_motorConditionFlag = false;

//--------Motor Func
bool t1_dom_motorFunctionFlag  = false;
bool t1_flu_motorFunctionFlag  = false;

bool t2_dom_motorFunctionFlag  = false;
bool t2_flu_motorFunctionFlag  = false;

bool t3_dom_motorFunctionFlag  = false;
bool t3_flu_motorFunctionFlag  = false;
//--------ETherNet Config-----


int failCount = 0;
long previousMillis;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xEE };

IPAddress ip(192, 168, 1, 91);
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
  //-----T1 DOM
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T1_DOM_MOTORSTATUS) == 0) { //for T1 DOM
    Serial.println("T1-DOM: TurnOn Motor ");
    mqttClient.publish(T1_MESSAGES, "T1:DOM Motor ON");
    t1_dom_motorConditionFlag = true;
    if (!t1_dom_motorFunctionFlag) {
      t1_dom_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T1_DOM_MOTORSTATUS) == 0) {
    Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(T1_MESSAGES, "Motor ONe OFFF");
    t1_dom_motorConditionFlag = true;
    if (t1_dom_motorFunctionFlag) {
      t1_dom_turnOff();
    }
  }
  //-----T2 DOM
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T2_DOM_MOTORSTATUS) == 0) { //for T2 DOM
    Serial.println("T1-DOM: TurnOn Motor ");
    mqttClient.publish(T2_MESSAGES, "T1:DOM Motor ON");
    t2_dom_motorConditionFlag = true;
    if (!t2_dom_motorFunctionFlag) {
      t2_dom_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T2_DOM_MOTORSTATUS) == 0) {
    Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(T2_MESSAGES, "Motor ONe OFFF");
    t2_dom_motorConditionFlag = true;
    if (t2_dom_motorFunctionFlag) {
      t2_dom_turnOff();
    }
  }
  //-----T3 DOM
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T3_DOM_MOTORSTATUS) == 0) { //for T3 DOM
    Serial.println("T1-DOM: TurnOn Motor ");
    mqttClient.publish(T3_MESSAGES, "T1:DOM Motor ON");
    t3_dom_motorConditionFlag = true;
    if (!t3_dom_motorFunctionFlag) {
      t3_dom_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T3_DOM_MOTORSTATUS) == 0) {
    Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(T3_MESSAGES, "Motor ONe OFFF");
    t3_dom_motorConditionFlag = true;
    if (t3_dom_motorFunctionFlag) {
      t3_dom_turnOff();
    }
  }
  //----Flush
  //-----T1 FLU
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T1_FLU_MOTORSTATUS) == 0) { //for T3 DOM
    Serial.println(F("T1-FLU: TurnOn Motor "));
    mqttClient.publish(T1_MESSAGES, "T1:FLU Motor ON");
    t1_flu_motorConditionFlag = true;
    if (!t1_flu_motorFunctionFlag) {
      t1_flu_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T1_FLU_MOTORSTATUS) == 0) {
    Serial.println(F("T1-FLU: TurnOff Motor "));
    mqttClient.publish(T1_MESSAGES, "Motor ONe OFFF");
    t1_flu_motorConditionFlag = true;
    if (t1_flu_motorFunctionFlag) {
      t1_flu_turnOff();
    }
  }
  //-----T2 FLU
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T2_FLU_MOTORSTATUS) == 0) { //for T3 DOM
    Serial.println(F("T2-FLU: TurnOn Motor "));
    mqttClient.publish(T2_MESSAGES, "T2:FLU Motor ON");
    t2_flu_motorConditionFlag = true;
    if (!t2_flu_motorFunctionFlag) {
      t2_flu_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T2_FLU_MOTORSTATUS) == 0) {
    Serial.println(F("T2-FLU: TurnOff Motor "));
    mqttClient.publish(T2_MESSAGES, "Motor ONe OFFF");
    t2_flu_motorConditionFlag = true;
    if (t2_flu_motorFunctionFlag) {
      t2_flu_turnOff();
    }
  }
  //-----T3 FLU
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T3_FLU_MOTORSTATUS) == 0) { //for T3 DOM
    Serial.println(F("T3-FLU: TurnOn Motor "));
    mqttClient.publish(T3_MESSAGES, "T3:FLU Motor ON");
    t3_flu_motorConditionFlag = true;
    if (!t3_flu_motorFunctionFlag) {
      t3_flu_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T3_FLU_MOTORSTATUS) == 0) {
    Serial.println(F("T3-FLU: TurnOff Motor "));
    mqttClient.publish(T3_MESSAGES, "Motor ONe OFFF");
    t3_flu_motorConditionFlag = true;
    if (t3_flu_motorFunctionFlag) {
      t3_flu_turnOff();
    }
  }
}

void setup() {
  Serial.begin(9600);
  for (int i = 2; i <= 9; i++) { //pin config of relay
    pinMode(i, OUTPUT);
    digitalWrite(i, !LOW);
  }
  for (int i = 14; i <= 19; i++) { //pin config of relay
    pinMode(i, OUTPUT);
    digitalWrite(i, !LOW);
  }
  //-----ethernet setup
  // start the Ethernet connection:
  Serial.println(F("Initialize Ethernet with DHCP:"));
  Ethernet.begin(mac, ip, myDns, gateway, subnet);
  Serial.print("  DHCP assigned IP ");
  Serial.println(Ethernet.localIP());
  // give the Ethernet shield a second to initialize:
  delay(1000);

  Serial.println(F("Motor Unit 1 - T1/T2/T3"));

  //setup MQTT client
  mqttClient.setClient(client);
  mqttClient.setCallback(callback);

  mqttClient.setServer("192.168.1.85", 1883); //for using local broker

  Serial.println(F("MQTT client configured"));

  Serial.print(F(" \n Trying to connect as a client -> "));
  int connectionflag = mqttClient.connect(CLIENT_ID);
  Serial.println(connectionflag);

  boolean r;
  r = mqttClient.subscribe(T1_DOM_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);

  //  r = mqttClient.subscribe(LEVELTOPIC);
  //  Serial.println("subscribed status LEVELTOPIC");
  //  Serial.println(r);
}


bool t1_dom_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t1_dom_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T1_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t1_dom_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T1_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}

bool t2_dom_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t2_dom_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T2_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t2_dom_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T2_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}

bool t3_dom_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t1_dom_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T3_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t3_dom_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T3_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}
//------FLU 
bool t1_flu_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t1_flu_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T1_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t1_flu_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T1_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}

bool t2_flu_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t2_flu_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T2_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t2_flu_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T2_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}

bool t3_flu_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t3_flu_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T3_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t3_flu_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T3_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}
void listenData() {
  //Listen TOPIC for necessory conditions
}
//---------------Tower One ------
void t1_dom_turnOn() {
  //Turn Off by 2Second control of starter box
  t1_dom_motorFunctionFlag = true;
  Serial.println(F("T1 -DOM motorON"));
  //M1_ON_RELAY
  digitalWrite(T1_DOM_ON, !HIGH);
  delay(2000);
  digitalWrite(T1_DOM_ON, !LOW);
}
void t1_dom_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T1 -DOM motorOFF"));
  t1_dom_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T1_DOM_OFF, !HIGH);
  delay(2000);
  digitalWrite(T1_DOM_OFF, !LOW);
}

void t1_flu_turnOn() {
  //Turn Off by 2Second control of starter box
  t1_flu_motorFunctionFlag = true;
  Serial.println(F("T1 -FLU motorON"));
  //M1_ON_RELAY
  digitalWrite(T1_FLU_ON, !HIGH);
  delay(2000);
  digitalWrite(T1_FLU_ON, !LOW);
}
void t1_flu_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T1 -FLU motorOFF"));
  t1_flu_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T1_FLU_OFF, !HIGH);
  delay(2000);
  digitalWrite(T1_FLU_OFF, !LOW);
}
//
//---------------Tower Two ------
void t2_dom_turnOn() {
  //Turn Off by 2Second control of starter box
  t2_dom_motorFunctionFlag = true;
  Serial.println(F("T2 -DOM motorON"));
  //M1_ON_RELAY
  digitalWrite(T2_DOM_ON, !HIGH);
  delay(2000);
  digitalWrite(T2_DOM_ON, !LOW);
}
void t2_dom_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T2 -DOM motorOFF"));
  t2_dom_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T2_DOM_OFF, !HIGH);
  delay(2000);
  digitalWrite(T2_DOM_OFF, !LOW);
}

void t2_flu_turnOn() {
  //Turn Off by 2Second control of starter box
  t2_flu_motorFunctionFlag = true;
  Serial.println(F("T2 -FLU motorON"));
  //M1_ON_RELAY
  digitalWrite(T2_FLU_ON, !HIGH);
  delay(2000);
  digitalWrite(T2_FLU_ON, !LOW);
}
void t2_flu_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T2 -FLU motorOFF"));
  t2_flu_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T2_FLU_OFF, !HIGH);
  delay(2000);
  digitalWrite(T2_FLU_OFF, !LOW);
}
//---------------Tower Three ------
void t3_dom_turnOn() {
  //Turn Off by 2Second control of starter box
  t3_dom_motorFunctionFlag = true;
  Serial.println(F("T3 -DOM motorON"));
  //M1_ON_RELAY
  digitalWrite(T3_DOM_ON, !HIGH);
  delay(2000);
  digitalWrite(T3_DOM_ON, !LOW);
}
void t3_dom_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T3 -DOM motorOFF"));
  t3_dom_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T3_DOM_OFF, !HIGH);
  delay(2000);
  digitalWrite(T3_DOM_OFF, !LOW);
}

void t3_flu_turnOn() {
  //Turn Off by 2Second control of starter box
  t3_flu_motorFunctionFlag = true;
  Serial.println(F("T3 -FLU motorON"));
  //M1_ON_RELAY
  digitalWrite(T3_FLU_ON, !HIGH);
  delay(2000);
  digitalWrite(T3_FLU_ON, !LOW);
}
void t3_flu_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T3 -FLU motorOFF"));
  t3_flu_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T3_FLU_OFF, !HIGH);
  delay(2000);
  digitalWrite(T3_FLU_OFF, !LOW);
}

void loop() {
  bool sendFlagcheck = false;
  if (millis() - previousMillis > INTERVAL) {
    sendFlagcheck = t1_dom_sendData();
    sendFlagcheck = t2_dom_sendData();
    sendFlagcheck = t3_dom_sendData();

    sendFlagcheck = t1_flu_sendData();
    sendFlagcheck = t2_flu_sendData();
    sendFlagcheck = t3_flu_sendData();
    if (sendFlagcheck == false) {
      failCount ++;
    }
    else if (sendFlagcheck == true) {
      failCount = 0;
    }
    previousMillis = millis();
  }
  if (failCount > 5) {
    Serial.println(F("Resetting conection"));
    resetFunc();
  }
  mqttClient.loop();
}
