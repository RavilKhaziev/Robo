#include <freertos/FreeRTOS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <memory>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <string>
#include <iostream>
#include <math.h>
#include <esp_wifi.h>

#include "Motor.h"

#define WIFI_CONFIG_SSID "RoboId"
#define WEB_SOCKET_PORT 22
#define WEBHTTP_PORT 80

enum class RoboState {
    BOOT, // Включение робота
    WAIT_TO_CONFIG, // Ожидание настройки устройства указание SSID и пароля для передачи
    CONNECT_TO_WIFI, // Попытка соединения с сервером 
    REMOTE_SERVER, // Соединение прошло успешно. Слушаем команды сервера.
    ERROR_ROBO // Произошла внутреняя ошибка
};

Motor MotorL;
Motor MotorR;
AsyncWebServer WebServer(WEBHTTP_PORT);
AsyncWebSocket WebSocket("/ws");
RoboState globalState = RoboState::BOOT;
StaticJsonDocument<200> doc;
static uint MaxPower = 1024;

void initSPIFFS();
void ConnectToServerState(const std::string &ssid, const std::string &password);
void WaitToConfigState();
void ErrorState();
void RemoteControll();
void NextState(const RoboState &state);

void init(){
  esp_wifi_set_protocol( WIFI_IF_STA , WIFI_PROTOCOL_11B);
  pinMode(2, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(14, OUTPUT);
  MotorL.Pin1 = 12;
  MotorL.Pin2 = 14;
  MotorL.PinEnb = 13;
  MotorL.Power = 250;
  MotorL.Dir = Motor::Motor_dir::BACK;

  MotorR.Pin1 = 27;
  MotorR.Pin2 = 26;
  MotorR.PinEnb = 25;
  MotorR.Power = 250;
  MotorR.Dir = Motor::Motor_dir::BACK;
  
  MotorL.PowerCorrect = MaxPower;
  MotorR.PowerCorrect = MaxPower;

  MotorL.Revers = false;
  MotorR.Revers = true;

  initSPIFFS();
}


void setup() {
  NextState(RoboState::BOOT);
}

void loop() {
  WebSocket.cleanupClients();
}

void notifyClients() {
    WebSocket.textAll("a");
}

void NextState(const RoboState &state){
  switch(state){
    case RoboState::BOOT :{
      init();
      NextState(RoboState::CONNECT_TO_WIFI);
      break;
    }
    case RoboState::CONNECT_TO_WIFI:{
      File file = SPIFFS.open("/config_wifi.json", "r");
      if(!file){
        std::cout << "Запуск конфигурирования устройства" << std::endl;
        NextState(RoboState::WAIT_TO_CONFIG);
      }
      StaticJsonDocument<500> jsStr;
      deserializeJson(jsStr, file);
      
      std::string ssid = jsStr["ssid"];
      std::string password = jsStr["password"];
      std::cout <<"Название сети: " << ssid << "Пароль подключения: " << password << std::endl;
      file.close();
      
      ConnectToServerState(ssid, password);
    }
    case RoboState::REMOTE_SERVER:{
      RemoteControll();
    }
    default:{
      ErrorState();
    }
  }
}

void ErrorState(){

}
void WaitToConfigState(){

}
void SwitchLed(){
  static uint8_t ledStatus = LOW;
  analogWrite(2, ledStatus);
  ledStatus = ledStatus == LOW ? HIGH : LOW;
}
void SetLed(bool ledStatus){
  analogWrite(2, ledStatus);
}
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  MotorL.Power = 0;
  MotorR.Power = 0;
}

void ConnectToServerState(const std::string &ssid, const std::string &password){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    std::cout <<"Trying to connect " << WiFi.macAddress().c_str() << std::endl;
    while (WiFi.status() != wl_status_t::WL_CONNECTED)
    {
      std::cout << ".";
      SwitchLed();
      vTaskDelay(100);
    }
    WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  NextState(RoboState::REMOTE_SERVER);
}


void onEvent(AsyncWebSocket       *server,
             AsyncWebSocketClient *client,
             AwsEventType          type,
             void                 *arg,
             uint8_t              *data,
             size_t                len
             );
void onRootRequest(AsyncWebServerRequest *request);
void RemoteControll(){
    std::cout << "Настраиваем сервер" << std::endl;
    SetLed(true);
    WebServer.on("/", onRootRequest);
    WebServer.serveStatic("/", SPIFFS, "/");
    WebServer.addHandler(&WebSocket);
    WebServer.begin();
    WebSocket.onEvent(onEvent);
    std::cout << WiFi.localIP().toString().c_str() << std::endl;
    std::cout << "Настройка завершена" << std::endl;
    SetLed(false);
}
enum RoboCommand{
  MOVE = 1,
  ROTATE
};
void RoboMove(float dirX, float dirY, int power);
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  std::string_view sasd((char*)data, len);
  deserializeJson(doc, sasd);
  int type = doc["type"];
  switch (type)
  {
  case MOVE:{
    float dirX = doc["dirX"];
    float dirY = doc["dirY"];
    int power = doc["power"];
    RoboMove(dirX, dirY, power);
  }
    break;
  case ROTATE:{

  }
  default:
    break;
  }
}
void RoboMove(float dirX, float dirY, int power){
  dirX = std::clamp(dirX, -1.0f, 1.0f);
  dirY = std::clamp(dirY, -1.0f, 1.0f);
  float length = sqrtf(dirX * dirX + dirY * dirY);
  dirX = dirX / length;
  dirY = dirY / length;
  MotorL.Revers= true;
  MotorR.Revers = false;
  power = std::clamp((uint)power, (uint)0, MaxPower);
  if(dirY > 0){
    MotorL.Dir = Motor::Motor_dir::FORWARD;
    MotorR.Dir = Motor::Motor_dir::FORWARD;
  }else if(dirY < 0){
    MotorL.Dir = Motor::Motor_dir::BACK;
    MotorR.Dir = Motor::Motor_dir::BACK;
    MotorL.Revers= false;
    MotorR.Revers = true;
  }else{
    MotorL.Dir = Motor::Motor_dir::STOP;
    MotorR.Dir = Motor::Motor_dir::STOP;
  }

  if(dirX > 0){
    MotorL.Power = power;
    MotorR.Power = power * (1 - std::abs(dirX));
  }else if(dirX < 0){
    MotorL.Power = power * (1 - std::abs(dirX));
    MotorR.Power = power;
  }else{
    MotorL.Power = power;
    MotorR.Power = power;
  }

  if((dirY == 0) && (std::abs(dirX) == 1)){  
    if(dirX < 0){
      MotorL.Dir = Motor::Motor_dir::BACK;
      MotorR.Dir = Motor::Motor_dir::FORWARD;
      MotorL.Power = power;
      MotorR.Power = power;
    }else if(dirX > 0){
      MotorL.Dir = Motor::Motor_dir::FORWARD;
      MotorR.Dir = Motor::Motor_dir::BACK;
      MotorL.Power = power;
      MotorR.Power = power;
    }
  }
  SwitchLed();
  MotorL.Update();
  MotorR.Update();
  SwitchLed();
}
void onEvent(AsyncWebSocket       *server,
             AsyncWebSocketClient *client,
             AwsEventType          type,
             void                 *arg,
             uint8_t              *data,
             size_t                len
             )
{
  switch (type) {
    case WS_EVT_CONNECT:
      std::cout << "WebSocket client #%u connected from %s\n" << client->id() << client->remoteIP().toString().c_str() << std::endl;
      break;
    case WS_EVT_DISCONNECT:
        std::cout << "WebSocket client #%u disconnected\n" << client->id() << std::endl;
        MotorL.Dir = Motor::Motor_dir::STOP;
        MotorR.Dir = Motor::Motor_dir::STOP;
        break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      MotorL.Power = 0;
      MotorR.Power = 0;
        break;
    }
}
void initSPIFFS() {
  std::cout << "Начинаем проверять SPIFFS" << std::endl;
  if (!SPIFFS.begin()) {
    while (1)
    {
      std::cout << "Cannot mount SPIFFS volume..." << std::endl;
      SwitchLed();
    }
  }
}
void onRootRequest(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/controller.html", "text/html", false);
}