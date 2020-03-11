
#ifndef CMMC_MQTT_MODULE_H
#define CMMC_MQTT_MODULE_H

#include <MqttConnector.h>
#include <CMMC_Module.h>
#include <Ticker.h>

class MqttModule : public CMMC_Module
{
public:
  unsigned int mqttMessageTimeoutInSecond = 0;
  unsigned int afterMotionDetectedInSecond = 0;
  void config(CMMC_System *os, AsyncWebServer *server);
  void configLoop();
  void setup();
  void loop();
  void set_force_publish_flag();
  const char* name() {
    return "MqttModule";
  }

protected:
  void configWebServer();
  MqttConnector *init_mqtt();
  void register_receive_hooks(MqttConnector *mqtt);
  void register_publish_hooks(MqttConnector *mqtt);

private:
  MqttConnector *mqtt;
  Ticker mqttMessageTicker;
  String MQTT_HOST;
  String MQTT_USERNAME;
  String MQTT_PASSWORD;
  String MQTT_CLIENT_ID;
  String MQTT_PREFIX;
  String DEVICE_NAME;
  int MQTT_PORT;
  int PUBLISH_EVERY_S = 50*1000;
  int MQTT_CONNECT_TIMEOUT = 10;
  bool MQTT_LWT;
  unsigned long previousMillis = 0;
};

#endif
