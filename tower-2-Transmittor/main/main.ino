//#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"

#define DOM_LOW 2
#define DOM_HIGH 3
#define DOM_DRY 8

#define FLU_LOW 5
#define FLU_HIGH 6
#define FLU_DRY 7

#define DEGENCOUNT 5
#define INTERVAL    2000 // 3 sec delay between publishing

#define CLIENT_ID   "TOWER_2_LEVEL" //client ID | Unique ID for SUMP & motor

#define DOM_LEVELTOPIC "/towertwo/dom/level"
#define FLU_LEVELTOPIC "/towertwo/flu/level"

#define ONLINESTATUS "/towertwo/online"

#define DOM_MOTORSTATUS "/towertwo/dom/motorstatus"
#define FLU_MOTORSTATUS "/towertwo/flu/motorstatus"

#define DOM_DASH_MOTORSTATUS "/towertwo/dom/dashStatus"
#define FLU_DASH_MOTORSTATUS "/towertwo/flu/dashStatus"

#define DRYRUNSTATUS "/towertwo/dom/dryrunstatus"
//#define FLU_DRYRUNSTATUS "/towertwo/flu/dryrunstatus"

#define DEGENSTATUS "/towertwo/dom/degen"
//#define FLU_DEGENSTATUS "/towerone/flu/degen"

#define MESSAGES "/towertwo/dom/messages"
//#define FLU_MESSAGES "/towerone/flu/messages"

//--------for Domestic Tank
bool dom_onTimStartFlag = false;
bool dom_firstDegenFlag = false;
bool dom_dryRunError = false;

unsigned long dom_onTime;//90000; //Dry run interval 1 Mnt
long dom_interval = 60000;

bool dom_motorOnFlag = false;
bool dom_motorWorking = false;
bool dom_degenTimer = false;
int dom_degenCount = 0;
unsigned long dom_degenStartTime; //Degeneration time 1Hr
long dom_degenInterval = 270000;

bool dom_motorConditionFlag = false;

//--------For Flush Tank
bool flu_onTimStartFlag = false;
bool flu_firstDegenFlag = false;
bool flu_dryRunError = false;

unsigned long flu_onTime;//90000; //Dry run interval 1 Mnt
bool flu_motorOnFlag = false;
bool flu_motorWorking = false;
bool flu_degenTimer = false;
int flu_degenCount = 0;
unsigned long flu_degenStartTime;//3600000; //Degeneration time 1Hr

bool flu_motorConditionFlag = false;

//--------ETherNet Config-----


short failCount = 0;
long previousMillis;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAD };

IPAddress ip(192, 168, 1, 82);
IPAddress myDns(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

EthernetClient client;
PubSubClient mqttClient;

//--------------resetFunction
void(* resetFunc) (void) = 0;

//--------------CallBack
void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print(F("Message arrived in topic: "));
  Serial.println(topic);

  Serial.print(F("Message:"));
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("-----------------------");
  if (strcmp(topic, "/online") == 0) {
    //    Serial.println("Online check status ");
    //    mqttClient.publish(ONLINESTATUS, "Online");
  }
}
void setup() {
  // put your setup code here, to run once:

  for (int i = 2; i <= 8; i++) { //setting all the sensor pins as input
    pinMode(i, INPUT);
  }

  Serial.begin(9600);
  delay(100);
  Serial.println(F("Initialize Ethernet with IP:"));
  
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println(F("Ethernet cable is not connected."));
  }

  Ethernet.begin(mac, ip, gateway, subnet);
  Serial.print(F("DHCP assigned IP "));
  Serial.println(Ethernet.localIP());
  // give the Ethernet shield a second to initialize:
  delay(1000);

  Serial.println(F("Tower 1 Level Testor"));
  
  mqttClient.setClient(client);
  mqttClient.setCallback(callback);
  
  mqttClient.setServer("192.168.1.85", 1883); //for using local broker
  
  Serial.println(F("MQTT client configured"));
  Serial.print(F(" \n Trying to connect as a client -> "));
  int connectionflag = mqttClient.connect(CLIENT_ID);
  Serial.println(connectionflag);
  
  mqttClient.subscribe(DOM_DASH_MOTORSTATUS);
  mqttClient.subscribe(FLU_DASH_MOTORSTATUS);
  Serial.println(F("subscribe Topics "));
}

int dom_checkLevel() {
  bool dom_lowLevel = !digitalRead(DOM_LOW);
  bool dom_highLevel = !digitalRead(DOM_HIGH);
  bool dom_dryRun = !digitalRead(DOM_DRY);

  Serial.print(F("DOM LOW : ")); Serial.print(dom_lowLevel);
  Serial.print(F("| DOM HIGH : ")); Serial.print(dom_highLevel);
  Serial.print(F("| DOM DRY : ")); Serial.print(dom_dryRun);
  Serial.println();

  if (dom_lowLevel == true && dom_highLevel == true) {
    Serial.println("100%");
    Serial.println("Motor OFF");//full
    dom_motorOnFlag = false;
    dom_motorWorking = false;
    dom_degenTimer = false;
    dom_degenCount = 0;
    dom_motorConditionFlag = false;
    dom_onTimStartFlag = false;
    dom_firstDegenFlag = false;
    dom_dryRunError = false;
    return 1;
  }
  else if (dom_lowLevel == true && dom_highLevel == false) {
    Serial.println("70%");//safe
    return 2;
  }
  else if (dom_lowLevel == false && dom_highLevel == false) {
    Serial.println("10%");
    Serial.println("Motor ON");//low level
    dom_motorConditionFlag = true;
    //motor on time
    //motorOnFlag = true;
    if (!dom_onTimStartFlag) {
      dom_onTime = millis();
      dom_onTimStartFlag = true;
    }
    return 3;
  }
  return 4;
}

int flu_checkLevel() {

  bool flu_lowLevel = !digitalRead(FLU_LOW);
  bool flu_highLevel = !digitalRead(FLU_HIGH);
  bool flu_dryRun = !digitalRead(FLU_DRY);

  Serial.print(F("FLU LOW : ")); Serial.print(flu_lowLevel);
  Serial.print(F("| FLU HIGH : ")); Serial.print(flu_highLevel);
  Serial.print(F("| FLU DRY : ")); Serial.print(flu_dryRun);
  Serial.println();

  if (flu_lowLevel == true && flu_highLevel == true) {
    Serial.println(F("100%"));
    Serial.println(F("Motor OFF"));//full
    flu_motorOnFlag = false;
    flu_motorWorking = false;
    flu_degenTimer = false;
    flu_degenCount = 0;
    flu_motorConditionFlag = false;
    flu_onTimStartFlag = false;
    flu_firstDegenFlag = false;
    flu_dryRunError = false;
    return 1;
  }
  else if (flu_lowLevel == true && flu_highLevel == false) {
    Serial.println("70%");//safe
    return 2;
  }
  else if (flu_lowLevel == false && flu_highLevel == false) {
    Serial.println("10%");
    Serial.println("Motor ON");//low level
    flu_motorConditionFlag = true;
    //motor on time
    //motorOnFlag = true;
    if (!flu_onTimStartFlag) {
      flu_onTime = millis();
      flu_onTimStartFlag = true;
    }
    return 3;
  }
  return 4;
}

void dom_dryRunCheck() {
  bool dom_dryRun = !digitalRead(DOM_DRY);

  if (dom_motorOnFlag == true && dom_dryRun == true ) {
    dom_motorOnFlag = false;
    dom_motorWorking = true;
    //no dry run
  }
  else if (dom_motorWorking == false && dom_dryRun == false ) {
    dom_motorOnFlag = true;
    dom_motorWorking = false;
    //not running
  }

  if (dom_motorOnFlag) {
    if (millis() - dom_onTime >= dom_interval) {
      Serial.println(F("==DOM Dry Run++++----+++---"));
      mqttClient.publish(DRYRUNSTATUS, "Dry Run: Tower 1 Domestic motor ");
      dom_firstDegenFlag = true;
      dom_motorConditionFlag = false;
      dom_onTimStartFlag = false;
      //stop the motor and start degeneration time.
      if (dom_firstDegenFlag) {
        dom_degenStartTime = millis();
        dom_firstDegenFlag = false;
      }
      dom_degenTimer = true;
      dom_motorOnFlag = false;
    }
  }
  if (dom_degenTimer) {
    Serial.println(F("Degen Counter running"));
    if (millis() - dom_degenStartTime >= dom_degenInterval) {
      mqttClient.publish(DEGENSTATUS, "Degen Timer: Tower 1 Domestic motor ");
      Serial.println(F("Staring again degen count ------"));
      if (dom_degenCount < DEGENCOUNT) {
        dom_degenCount ++;
        //try to turn on Motor and Start Dry RUN
        dom_motorConditionFlag = true;

      }
      else {
        dom_motorConditionFlag = false;
        Serial.println("Error");
        dom_dryRunError = true;
        mqttClient.publish(MESSAGES, "T1 - DOM DRY RUN");
        //Degen Count completed
        //Inform to check the overall system
      }
    }
  }
}

void flu_dryRunCheck() {
  bool flu_dryRun = !digitalRead(FLU_DRY);

  if (flu_motorOnFlag == true && flu_dryRun == true ) {
    flu_motorOnFlag = false;
    flu_motorWorking = true;
    //no dry run
  }
  else if (flu_motorWorking == false && flu_dryRun == false ) {
    flu_motorOnFlag = true;
    flu_motorWorking = false;
    //not running
  }

  if (flu_motorOnFlag) {
    if (millis() - flu_onTime >= dom_interval) {
      Serial.println(F("==++++=+Dry Run++++----+++---"));
      mqttClient.publish(DRYRUNSTATUS, "Dry Run: Tower 1 Flush motor ");
      flu_firstDegenFlag = true;
      flu_motorConditionFlag = false;
      flu_onTimStartFlag = false;
      //stop the motor and start degeneration time.
      if (flu_firstDegenFlag) {
        flu_degenStartTime = millis();
        flu_firstDegenFlag = false;
      }
      flu_degenTimer = true;
      flu_motorOnFlag = false;
    }
  }
  if (flu_degenTimer) {
    Serial.println("Degen Counter running");
    if (millis() - flu_degenStartTime >= dom_degenInterval) {
      Serial.println(F("Staring again degen count ------"));
      mqttClient.publish(DEGENSTATUS, "Degen Timer: Tower 1 Flush motor ");
      if (flu_degenCount < DEGENCOUNT) {
        flu_degenCount ++;
        //try to turn on Motor and Start Dry RUN
        flu_motorConditionFlag = true;
      }
      else {
        flu_motorConditionFlag = false;
        Serial.println("Error");
        flu_dryRunError = true;
        mqttClient.publish(MESSAGES, "T1 Flush tank DR");
        //Degen Count completed
        //Inform to check the overall system
      }
    }
  }
}

bool dom_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;
  int readLevel = dom_checkLevel();
  switch (readLevel) {
    case 1:
      msgBuffer = "100";
      break;
    case 2:
      msgBuffer = "70";
      break;
    case 3:
      msgBuffer = "10";
      break;
    default:
      msgBuffer = "error";
      break;
  }
  sendStatusFlag = mqttClient.publish(DOM_LEVELTOPIC, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  //-----------------
  if (dom_motorConditionFlag && !dom_degenTimer && !dom_dryRunError) {
    msgBuffer = "turnOnMotorOne";
    Serial.println(F("Turn ON Tower 1 DOM MOTOR"));
    sendStatusFlag = mqttClient.publish(DOM_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println("Error while publishsing ");
      return false;
    }
  }
  else if (!dom_motorConditionFlag && !dom_degenTimer) {
    msgBuffer = "turnOffMotorOne";
    Serial.println(F("Turn OFF Tower 1 DOM MOTOR"));
    sendStatusFlag = mqttClient.publish(DOM_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println("Error while publishsing ");
      return false;
    }
  }
  if (dom_onTimStartFlag) {
    Serial.println("Calling Dry Run");
    dom_dryRunCheck();
  }
  //-----------------
  Serial.println("Published Data ");
  return true;
}

bool flu_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;
  // getWaterLevel(); //fucntion to read water level
  int readLevel = flu_checkLevel();
  switch (readLevel) {
    case 1:
      msgBuffer = "100";
      break;
    case 2:
      msgBuffer = "70";
      break;
    case 3:
      msgBuffer = "10";
      break;
    default:
      msgBuffer = "error";
      break;
  }
  sendStatusFlag = mqttClient.publish(FLU_LEVELTOPIC, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  //-----------------
  if (flu_motorConditionFlag && !flu_degenTimer && !flu_dryRunError) {
    msgBuffer = "turnOnMotorOne";
    Serial.println(F("Turn ON Tower 1 FLU MOTOR"));
    sendStatusFlag = mqttClient.publish(FLU_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!flu_motorConditionFlag && !flu_degenTimer) {
    msgBuffer = "turnOffMotorOne";
    Serial.println(F("Turn OFF Tower 1 FLU MOTOR"));
    sendStatusFlag = mqttClient.publish(FLU_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println("Error while publishsing ");
      return false;
    }
  }
  if (flu_onTimStartFlag) {
    Serial.println(F("Calling Dry Run"));
    flu_dryRunCheck();
  }
  //-----------------
  Serial.println(F("Published Data "));
  return true;
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (mqttClient.connect(CLIENT_ID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish(ONLINESTATUS, "Online");
    }
    else {
      Serial.print("failed, rc=");
      int state = mqttClient.state();
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void loop() {
  // check interval
  bool sendFlagcheck = false;
  if (millis() - previousMillis > INTERVAL) {
    sendFlagcheck = dom_sendData();
    sendFlagcheck = flu_sendData();
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
    delay(100);
    resetFunc();
  }
  mqttClient.loop();
}
