#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>

#define RX_PIN 5 // GPIO5, can be changed to any available pin
#define TX_PIN 4 // GPIO4, can be changed to any available pin
#define LED LED_BUILTIN

#define ID 3
#define HOST_CLIENT 4
#define CHECK 8
#define X_DIR 16
#define Y_DIR 32
#define RUNNING_GAME 64
#define PLAYER_CHECK 128

WiFiClient espClient;
PubSubClient client(espClient);

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Serial 1
String ssid = "GameNetwork"; // Default SSID
String password = "12345678"; // Default password
const char* broker = "192.168.4.1";

uint8_t data_game = 0;
uint8_t last_data_game[4] = {0};
uint8_t consolidatedData[4] = {0};

void callback(char* topic, byte* payload, unsigned int length) {
  consolidatedData[payload[0]&3] = payload[0];
}

void updateConsolidated(){
  consolidatedData[data_game&ID] = data_game;
}

void readData(){
  Serial.println("Awaiting data...");
  while (!mySerial.available()) {
    // Wait until data is available
    delay(1);
  }
  data_game = mySerial.read();
  updateConsolidated();
  for (int i = 8; i > 0; i--) {
    Serial.print(bitRead(data_game, i));
  }
}

void sendData(uint8_t data){
  mySerial.write(data);
}

void sendDataOfEveryOther(){
  for(int i = 0; i < 4; i++){
    if (i != (data_game & ID) && consolidatedData[i]&PLAYER_CHECK == PLAYER_CHECK){
      sendData(consolidatedData[i]);
    }
  }
}

bool checkForEvery(uint8_t bit_condition){
  uint8_t control = 0;
  uint8_t control_users = 0;
  for(int i = 0; i < 4; i++){
    if(consolidatedData[i]& PLAYER_CHECK == PLAYER_CHECK){
      control_users++;
      if(consolidatedData[i]& bit_condition == bit_condition) control++;
    }
  }
  if(control == control_users) return true;
  return false;
}

void copyArray(){
  for(int i = 0; i < 4; i++){
    last_data_game[i] = consolidatedData[i];
  }
}

bool checkForEveryLastEqual(uint8_t bit_condition){
  uint8_t control = 0;
  uint8_t control_users = 0;
  for(int i = 0; i < 4; i++){
    if(consolidatedData[i]& PLAYER_CHECK == PLAYER_CHECK){
      if(consolidatedData[i]& bit_condition == last_data_game[i] & bit_condition) control++;
    }
    control_users++;
    }
    if(control == control_users) return true;
    return false;
}

void setup_host(){
    Serial.println("Waiting for data...");
    String data = mySerial.readStringUntil('\0');
    int separatorIndex = data.indexOf(':');
    if (separatorIndex != -1) {
      ssid = data.substring(0, separatorIndex);
      password = data.substring(separatorIndex + 1);
      WiFi.softAP(ssid.c_str(), password.c_str());
      client.setServer(broker, 1883);

      client.connect(("client" + String((char)(data_game & 3))).c_str());
      client.publish(("game/" + String((char)(data_game & 3))).c_str(), String((char)data_game).c_str(), true);
      client.setCallback(callback);

      for(int i = 0; i < 4; i++){
        if (i != (data_game & 3)){
          client.subscribe(("game/" + String((char)i)).c_str());
        }
      }
      digitalWrite(LED, LOW);
    }
}

void publishForEveryOther(){
  client.publish(("game/" + String((char)(data_game & ID))).c_str(), String((char)data_game).c_str());
}

void setup_client() {
  Serial.println("Waiting for network credentials...");
  String data = mySerial.readStringUntil('\0');
  int separatorIndex = data.indexOf(':');
  if (separatorIndex != -1) {
    ssid = data.substring(0, separatorIndex);
    password = data.substring(separatorIndex + 1);
    WiFi.begin(ssid.c_str(), password.c_str());

    client.setCallback(callback);

    for(int i = 0; i < 4; i++){
      if (consolidatedData[i] == 0 && i == (data_game & ID)) {
        client.publish(("game/" + String((char)(data_game & ID))).c_str(), String((char)data_game).c_str());
        consolidatedData[i] = data_game;
        sendData(1);
        digitalWrite(LED, LOW);

        for(int j = 0; j < 4; j++){
          if (j != (data_game & ID)){
            client.subscribe(("game/" + String((char)j)).c_str());
          }
        }
        
        break;
      }
      else if (i == 3){
        sendData(0);
        digitalWrite(LED, HIGH);
      }
    }
  }
}

void game_run(){
  do{
    readData();
    publishForEveryOther();
  }while(!checkForEvery(CHECK));

  data_game |= RUNNING_GAME;
  updateConsolidated();

  while(checkForEvery(RUNNING_GAME)){
    if(!checkForEveryLastEqual(CHECK)){
      readData();
      publishForEveryOther();
      sendDataOfEveryOther();
      copyArray();
    }
  }
}

void setup()
{
  pinMode(LED, OUTPUT);
  mySerial.begin(9600); // Serial 1 initialization
  digitalWrite(LED, HIGH); // Turn off the indicator LED

  Serial.begin(9600); //Debug

}

void loop()
{
  readData();
  //if((data_game & 64) == 64){
    if((data_game & 4) == 4){
      setup_host();
      game_run();
    }
    else{
      setup_client();
      game_run();
    }
  //}
}