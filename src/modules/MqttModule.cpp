#include <rom/rtc.h>


#include <Adafruit_NeoPixel.h>
#include <Servo.h>
#include <CMMC_Legend.h>
#include "MqttModule.h"
#include "SharedPoolModule.h"
// #include "SensorModule.h"
// extern SensorModule* sensorModule;

extern SharedPoolModule pool;
extern String DB_MINI_APP_VERSION;
extern int DB_MINI_APP_VERSION_INT;

// char deviceName[20] = "";

extern int first_published;
extern bool doing_ota;
extern uint32_t seq;


#include <Preferences.h>
extern Preferences preferences;

extern Adafruit_NeoPixel strip;
extern Servo myservo;


void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}


uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void MqttModule::config(CMMC_System *os, AsyncWebServer *server) {
  strcpy(this->path, "/api/mqtt");
  this->_serverPtr = server;

  preferences.begin("db-conf-mqtt", false);

  MQTT_HOST = preferences.getString("MQTT_HOST");
  MQTT_USERNAME = preferences.getString("MQTT_USERNAME");
  MQTT_PASSWORD = preferences.getString("MQTT_PASSWORD");
  MQTT_PASSWORD.replace(" ", "+");
  MQTT_CLIENT_ID = preferences.getString("MQTT_CLIENT_ID");
  MQTT_PREFIX = preferences.getString("MQTT_PREFIX");
  MQTT_PORT  = preferences.getUInt("MQTT_PORT");
  PUBLISH_EVERY_S = preferences.getUInt("PUBLISH_EVERY_S");
  if (PUBLISH_EVERY_S < 1000) {
    PUBLISH_EVERY_S *= 1000;
  }
  DEVICE_NAME = preferences.getString("DEVICE_NAME");
  MQTT_LWT = false; // must be set false for netpie
  preferences.end();

  // MQTT_HOST     = String("mqtt.cmmc.io");
  // MQTT_USERNAME = String("");
  // MQTT_PASSWORD = String("");
  // DEVICE_NAME = "X";
  // MQTT_PREFIX = "PREFIX/";
  // MQTT_PORT  = 1883;
  // PUBLISH_EVERY_S = 50*1000;

  Serial.printf("[PREF][MQTT_USERNAME] = %s\r\n", MQTT_USERNAME.c_str());
  Serial.printf("[PREF][MQTT_HOST] = %s\r\n", MQTT_HOST.c_str());
  Serial.printf("[PREF][MQTT_PASSWORD] = %s\r\n", MQTT_PASSWORD.c_str());
  Serial.printf("[PREF][MQTT_CLIENT_ID] = %s\r\n", MQTT_CLIENT_ID.c_str());
  Serial.printf("[PREF][MQTT_PREFIX] = %s\r\n", MQTT_PREFIX.c_str());
  Serial.printf("[PREF][MQTT_PORT] = %d\r\n", MQTT_PORT);
  Serial.printf("[PREF][DEVICE_NAME] = %s\r\n", DEVICE_NAME.c_str());
  Serial.printf("[PREF][PUBLISH_EVERY_S] = %d\r\n", PUBLISH_EVERY_S);


  strcpy(pool.__device_name, DEVICE_NAME.c_str());
  Serial.println("XXXXXXXXXXX");
  Serial.print("MQTT_HOST: ");
  Serial.println(MQTT_HOST);
  Serial.print("MQTT_USERNAME: ");
  Serial.println(MQTT_USERNAME);
  // Serial.print("MQTT_PASSWORD: ");
  // Serial.println(MQTT_PASSWORD);
  Serial.print("MQTT_CLIENT_ID: ");
  Serial.println(MQTT_CLIENT_ID);
  Serial.print("MQTT_PREFIX: ");
  Serial.println(MQTT_PREFIX);
  Serial.print("MQTT_PORT: ");
  Serial.println(MQTT_PORT );
  Serial.print("PUBLISH_EVERY_S: ");
  Serial.println(PUBLISH_EVERY_S);
  Serial.print("DEVICE_NAME: ");
  Serial.println(DEVICE_NAME);
  Serial.println("XXXXXXXXXXX");
  this->configWebServer();
};

void MqttModule::configWebServer()
{
  static MqttModule *that = this;
  _serverPtr->on(this->path, HTTP_POST, [&](AsyncWebServerRequest * request) {
    Serial.println("saving config..");
    int params = request->params();
    Serial.print("PARAM: ");
    Serial.println(params);
    String output = "{";
    for (int i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      if (p->isPost()) {
        const char* key = p->name().c_str();
        const char* value = p->value().c_str();
        Serial.print("KEY:");
        Serial.println(key);
        Serial.print("VALUE:");
        Serial.println(value);
        String v;
        if (value == 0) {
          Serial.println("value is null..");
          v = String("");
        }
        else {
          v = String(value);
        }
        output += "\"" + String(key) + "\"";
        if (i == params - 1 ) {
          output += ":\"" + v + "\"";
        }
        else {
          output += ":\"" + v + "\",";
        }
      }
    }
    output += "}";

  // MQTT_HOST;
  // MQTT_PASSWORD;
  // MQTT_CLIENT_ID;
  // MQTT_PREFIX;
  // MQTT_PORT ;
  // PUBLISH_EVERY_S;
  // DEVICE_NAME;
  // MQTT_LWT;

    AsyncWebParameter* p = request->getParam("host", true);

    p = request->getParam("host", true);
    MQTT_HOST = p->value();

    p = request->getParam("username", true);
    MQTT_USERNAME = p->value();

    p = request->getParam("password", true);
    MQTT_PASSWORD = p->value();

    p = request->getParam("clientId", true);
    MQTT_CLIENT_ID = p->value();

    p = request->getParam("prefix", true);
    MQTT_PREFIX = p->value();
    //
    p = request->getParam("port", true);
    MQTT_PORT = p->value().toInt();
    //
    p = request->getParam("publishRateSecond", true);
    PUBLISH_EVERY_S = p->value().toInt();
    //
    p = request->getParam("deviceName", true);
    DEVICE_NAME = p->value();
    //
    p = request->getParam("lwt", true);
    MQTT_LWT = false;

    // MQTT_PASSWORD = MQTT_PASSWORD.replace(" ", "+");

  preferences.begin("db-conf-mqtt", false);
  preferences.putString("MQTT_HOST", MQTT_HOST);
  preferences.putString("MQTT_USERNAME", MQTT_USERNAME);
  preferences.putString("MQTT_PASSWORD", MQTT_PASSWORD);
  preferences.putString("MQTT_CLIENT_ID", MQTT_CLIENT_ID);
  preferences.putString("MQTT_PREFIX", MQTT_PREFIX);
  preferences.putUInt("MQTT_PORT", MQTT_PORT);
  preferences.putUInt("PUBLISH_EVERY_S", PUBLISH_EVERY_S);
  preferences.putString("DEVICE_NAME", DEVICE_NAME);
  MQTT_LWT = false; // must be set false for netpie
  preferences.end();

    Serial.println("/saving config..");
    request->send(200, "application/json", output);
  });
}

void MqttModule::configLoop() {
}

void MqttModule::set_force_publish_flag() {
  if (mqtt) {
    mqtt->set_force_publish_flag();
  }
}

void MqttModule::setup()
{
  Serial.println("MqttModule::setup");
  init_mqtt();
  static MqttModule *that;
  that = this;
  mqttMessageTicker.attach_ms(1000, []() { });
};

void MqttModule::loop()
{
  mqtt->loop();
};

// MQTT INITIALIZER

MqttConnector *MqttModule::init_mqtt()
{
  this->mqtt = new MqttConnector(this->MQTT_HOST.c_str(), this->MQTT_PORT);
  mqtt->on_connecting([&](int counter, bool * flag) {
    Serial.printf("[%lu] xMQTT CONNECTING.. \r\n", counter);
    if (counter >= MQTT_CONNECT_TIMEOUT)
    {
      this->mqttMessageTimeoutInSecond = 0;
      Serial.println("NO INTERNET...");
      // no_internet_link = 1;
    }
  });

  mqtt->on_prepare_configuration([&](MqttConnector::Config * config) -> void {
    MQTT_LWT = false; // must be set false for netpie
    Serial.printf("lwt = %lu\r\n", MQTT_LWT);
    config->clientId = MQTT_CLIENT_ID;
    config->channelPrefix = MQTT_PREFIX;
    config->enableLastWill = MQTT_LWT;
      config->retainPublishMessage = false;
    /*
        config->mode
        ===================
        | MODE_BOTH       |
        | MODE_PUB_ONLY   |
        | MODE_SUB_ONLY   |
        ===================
    */
    config->mode = MODE_BOTH;
    config->firstCapChannel = false;

    config->username = String(MQTT_USERNAME);
    config->password = String(MQTT_PASSWORD);

    // FORMAT
    // d:quickstart:<type-id>:<device-id>
    //config->clientId  = String("d:quickstart:esp8266meetup:") + macAddr;
    config->topicPub = MQTT_PREFIX + String(DEVICE_NAME) + String("/status");
  });

  mqtt->on_after_prepare_configuration([&](MqttConnector::Config config) -> void {
    String humanTopic = MQTT_PREFIX + DEVICE_NAME + String("/$/+");
    Serial.printf("[USER] HOST = %s\r\n", config.mqttHost.c_str());
    Serial.printf("[USER] PORT = %d\r\n", config.mqttPort);
    Serial.printf("[USER] PUB  = %s\r\n", config.topicPub.c_str());
    Serial.printf("[USER] SUB  = %s\r\n", config.topicSub.c_str());
    Serial.printf("[USER] SUB  = %s\r\n", humanTopic.c_str());
    // sub->add_topic(MQTT_PREFIX + String("/") + String(myName) + String("#"));
    // sub->add_topic(MQTT_PREFIX + "/" + MQTT_CLIENT_ID + "/$/+");
  });

  if (mqtt == NULL)
  {
    Serial.println("MQTT is undefined.");
    ESP.deepSleep(1);
  }

  register_publish_hooks(mqtt);
  register_receive_hooks(mqtt);

  Serial.println("connecting to mqtt..");
  mqtt->connect();
  return mqtt;
}

#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <Preferences.h>


extern Preferences preferences;

void MqttModule::register_receive_hooks(MqttConnector *mqtt)
{
  mqtt->on_subscribe([&](MQTT::Subscribe * sub) -> void {
    Serial.printf("onSubScribe myName = %s \r\n", DEVICE_NAME.c_str());
    sub->add_topic(MQTT_PREFIX + DEVICE_NAME + String("/$/+"));
    sub->add_topic(MQTT_PREFIX + DEVICE_NAME + String("/status"));
    Serial.println("done on_subscribe...");
    Serial.printf("publish every %lu s\r\n", PUBLISH_EVERY_S/1000);
  });

  mqtt->on_before_message_arrived_once([&](void) {});
  mqtt->on_message([&](const MQTT::Publish & pub) {});
 mqtt->on_after_message_arrived([&](String topic, String cmd, String payload) {
    Serial.printf("topic: %s\r\n", topic.c_str());
    Serial.printf("cmd: %s\r\n", cmd.c_str());
    Serial.printf("payload: %s\r\n", payload.c_str());
    if (cmd == "$/command") {
      // if (payload == "ON") {
      //   digitalWrite(LED_PIN, LOW);
      // }
      // else if (payload == "OFF") {
      //   digitalWrite(LED_PIN, HIGH);
      // }
      String inString = payload;
      if (inString.toFloat() == 0.0f) {
        Serial.println(inString);
        if (inString == "A")
        {
          colorWipe(strip.Color(0, 0, 255), 1);
//          Serial.println("Command completed LED turned ON");
        }
        if (inString == "B")
        {
          colorWipe(strip.Color(0, 255, 0), 1);
//          Serial.println("Command completed LED turned OFF");
        }
        if (inString == "E")
        {
          digitalWrite(16, LOW);
          colorWipe(strip.Color(255, 0, 0), 1);
//          Serial.println("Command completed LED turned OFF");
        }
        if (inString == "D")
        {
          colorWipe(strip.Color(255, 255, 255), 1);
//          Serial.println("Command completed LED turned OFF");
        }
        if (inString == "C")
        {
          rainbow(50);
//          Serial.println("Command completed LED turned OFF");
        }

      } else {
        Serial.println(inString.toFloat());
          myservo.write(inString.toFloat()*180);

      }
    }
    else if (cmd == "$/reboot") {
      ESP.restart();
    }
    else {
      // another message.
    }
  });
}


void MqttModule::register_publish_hooks(MqttConnector *mqtt)
{
  mqtt->on_prepare_data_once([&](void) {
    Serial.println("initializing sensor...");
  });

  mqtt->on_before_prepare_data([&](void) {

  });


  mqtt->on_prepare_data([&](JsonObject * root) {
    JsonObject &data = (*root)["d"];
    JsonObject &info = (*root)["info"];
    data["frameWorkVersion"] = LEGEND_APP_VERSION;
    data["appVersion"] = DB_MINI_APP_VERSION;
    data["appVersionInt"] = DB_MINI_APP_VERSION_INT;
    data["myName"] = DEVICE_NAME;
    data["millis"] = millis();

    data["first_published"] = first_published ? 1 : 0;

    data["raw_first_published"] = first_published ? 1 : 0;

    data["lwt"] = MQTT_LWT ? 1 : 0;
    data["seq"] = seq++;


    data["PUBLISH_EVERY_S"] = PUBLISH_EVERY_S / 1000;


    Serial.println("PUBLISHING...!");
  }, PUBLISH_EVERY_S);

  mqtt->on_after_prepare_data([&](JsonObject * root) {
    JsonObject &data = (*root)["d"];
    // if (!first_published) {
    //   data.remove("cpu0_reset_reason");
    //   data.remove("cpu1_reset_reason");
    //   data.remove("boot_count");
    //   data.remove("lwt");
    // }
    //   data.remove("pm10");
    //   data.remove("pm2_5");
    // }


    /**************
      JsonObject& data = (*root)["d"];
      data.remove("version");
      data.remove("subscription");
    **************/
  });
};
