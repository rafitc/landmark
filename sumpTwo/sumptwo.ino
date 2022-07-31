#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"

#define LOWLEVEL 8 //yellow
#define HIGHLEVEL 9 //green
#define DRYRUN 6 //red

#define DEGENCOUNT 2
#define INTERVAL    2000 // 3 sec delay between publishing

#define CLIENT_ID       "SUMP_2_LEVEL" //client ID | Unique ID for SUMP & motor

#define LEVELTOPIC "/sumptwo/level"
#define ONLINESTATUS "/sumptwo/online"
#define MOTORSTATUS "/sumptwo/motor/motorstatus"
#define DASH_MOTORSTATUS "/sumptwo/motor/dashStatus"
#define DRYRUNSTATUS "/sumptwo/dryrunstatus"
#define DEGENSTATUS "/sumptwo/degen"
#define MESSAGES "/messages"

bool onTimStartFlag = false;
bool firstDegenFlag = false;
bool dryRunError = false;

unsigned long onTime, interval = 5000;//90000; //Dry run interval 1 Mnt
bool motorOnFlag = false;
bool motorWorking = false;
bool degenTimer = false;
int degenCount = 0;
unsigned long degenStartTime, degenInterval = 5000;//3600000; //Degeneration time 1Hr

bool motorConditionFlag = false;

//--------ETherNet Config-----


int failCount = 0;
long previousMillis;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEE, 0xAE, 0xED };

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

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
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
  pinMode(LOWLEVEL, INPUT);
  pinMode(HIGHLEVEL, INPUT);
  pinMode(DRYRUN, INPUT);

  Serial.begin(9600);
  delay(100);

  //-----ethernet setup
  //Removed DHCP setting, coz. this is a local network with no internet = so, used static IP.
  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with IP:");
  // Check for Ethernet hardware present
//  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
//    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
//    while (true) {
//      delay(1); // do nothing, no point running without Ethernet hardware
//    }
//  }
//  if (Ethernet.linkStatus() == LinkOFF) {
//    Serial.println("Ethernet cable is not connected.");
//  }
  // try to congifure using IP address instead of DHCP:
  Ethernet.begin(mac, ip, gateway, subnet);
  Serial.print("  DHCP assigned IP ");
  Serial.println(Ethernet.localIP());
// give the Ethernet shield a second to initialize:
delay(1000);

Serial.println(F("SUMP 2 Level Testor"));

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
r = mqttClient.subscribe(DASH_MOTORSTATUS);
Serial.println("subscribe status MOTORSWITCH and online ");
Serial.println(r);
}


int checkLevel() {
  bool lowLevel = digitalRead(LOWLEVEL);
  bool highLevel = digitalRead(HIGHLEVEL);
  bool dryRun = digitalRead(DRYRUN);

  Serial.print("LOW : "); Serial.print(lowLevel);
  Serial.print("| HIGH : "); Serial.print(highLevel);
  Serial.print("| DRY : "); Serial.print(dryRun);
  Serial.println();

  if (lowLevel == true && highLevel == true) {
    Serial.println("100%");
    Serial.println("Motor OFF");//full
    motorOnFlag = false;
    motorWorking = false;
    degenTimer = false;
    degenCount = 0;
    motorConditionFlag = false;
    onTimStartFlag = false;
    firstDegenFlag = false;
    dryRunError = false;
    return 1;
  }
  else if (lowLevel == true && highLevel == false) {
    Serial.println("80%");//safe
    return 2;
    //
  }
  else if (lowLevel == false && highLevel == false) {
    Serial.println("10%");
    Serial.println("Motor ON");//low level
    motorConditionFlag = true;
    //motor on time
    //motorOnFlag = true;
    if (!onTimStartFlag) {
      onTime = millis();
      onTimStartFlag = true;
    }

    return 3;
  }
  return 4;
}

void dryRunCheck() {
  bool dryRun = digitalRead(DRYRUN);

  if (motorOnFlag == true && dryRun == true ) {
    motorOnFlag = false;
    motorWorking = true;
    //no dry run
  }
  else if (motorWorking == false && dryRun == false ) {
    motorOnFlag = true;
    motorWorking = false;
    //not running
  }

  if (motorOnFlag) {
    if (millis() - onTime >= interval) {
      Serial.println("==++++=+Dry Run++++----+++---");
      firstDegenFlag = true;
      motorConditionFlag = false;
      onTimStartFlag = false;
      //stop the motor and start degeneration time.
      if (firstDegenFlag) {
        degenStartTime = millis();
        firstDegenFlag = false;
      }
      degenTimer = true;
      motorOnFlag = false;
    }
  }
  if (degenTimer) {
    Serial.println("Degen Counter running");
    if (millis() - degenStartTime >= degenInterval) {
      Serial.println("Staring again degen count ------");
      if (degenCount < DEGENCOUNT) {
        degenCount ++;
        //try to turn on Motor and Start Dry RUN
        motorConditionFlag = true;

      }
      else {
        motorConditionFlag = false;
        Serial.println("Error");
        dryRunError = true;
        mqttClient.publish(MESSAGES, "DRY RUN, Check your tank water");
        //Degen Count completed
        //Inform to check the overall system
      }
    }
  }
}
bool sendData() {
  bool sendStatusFlag;
  char* msgBuffer;
  // getWaterLevel(); //fucntion to read water level
  int readLevel = checkLevel();
  switch (readLevel) {
    case 1:
      msgBuffer = "100";
      break;
    case 2:
      msgBuffer = "80";
      break;
    case 3:
      msgBuffer = "10";
      break;
    default:
      msgBuffer = "error";
      break;
  }
  sendStatusFlag = mqttClient.publish(LEVELTOPIC, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println("Error while publishsing ");
    return false;
  }
  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println("Error while publishsing ");
    return false;
  }
  //-----------------
  if (motorConditionFlag && !degenTimer && !dryRunError) {
    msgBuffer = "turnOnMotorOne";
    Serial.println("Turn ON MOTOR ONE");
    sendStatusFlag = mqttClient.publish(MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println("Error while publishsing ");
      return false;
    }
  }
  else if (!motorConditionFlag && !degenTimer) {
    msgBuffer = "turnOffMotorOne";
    Serial.println("Turn OFF MOTOR ONE");
    sendStatusFlag = mqttClient.publish(MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println("Error while publishsing ");
      return false;
    }
  }
  if (onTimStartFlag) {
    Serial.println("Calling Dry Run");
    dryRunCheck();
  }
  //-----------------
  Serial.println("Published Data ");
  return true;
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(CLIENT_ID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish(ONLINESTATUS, "Online");

    }
    else {
      Serial.print("failed, rc=");
      int state = mqttClient.state();
      //mqttErrorCode(state);
      Serial.println(" try again in 2 seconds");
      // Wait 1 seconds before retrying
      delay(1000);
    }
  }
}

void loop() {
  // check interval
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
    reconnect();
  }
  mqttClient.loop();
}
