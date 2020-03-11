#include "WiFiModule.h"
extern CMMC_Legend *os;

#include <Preferences.h>
extern Preferences preferences;

static const char* DB_PREF_NAME = "db-conf-wifi";

void WiFiModule::openSharedPref() {
  preferences.begin(DB_PREF_NAME, false);
}

void WiFiModule::closeSharedPref() {
  preferences.end();
}

void WiFiModule::isLongPressed() {
  // Serial.println("isLongPresssed");
  delay(10);
  // if (digitalRead(37) == LOW) {
  //   uint32_t pressed_start = millis();
  //   while (digitalRead(37) == LOW) {
  //     if (millis() - pressed_start > 2000) {
  //       while (digitalRead(37) == LOW) {
  //         delay(10);
  //       }
  //       os->enable_config_mode();
  //       delay(1000);
  //       ESP.deepSleep(10e5);
  //     }
  //   }
  // }
}

  void WiFiModule::configLoop() {
    // if (digitalRead(15) == HIGH) {
    //   // lcdModule->displayLogoWaitReboot();
    //   while(digitalRead(15) == HIGH) {
    //     delay(10);
    //   }
    //   digitalWrite(0, HIGH);
    //   delay(100);
    //   ESP.restart();
    // }
  }

  void WiFiModule::configSetup() {
    // lcdModule->displayConfigWiFi();
  }

  void WiFiModule::config(CMMC_System *os, AsyncWebServer *server)
  {
    CMMC_Legend *x = (CMMC_Legend*) os;
    // tft.fillRect(4, 190, 170, 200, ILI9341_PURPLE);
    strcpy(this->path, "/api/wifi/sta");
    static WiFiModule *that = this;
    that->_os = x;
    this->_serverPtr = server;
    this->openSharedPref();
    String a = preferences.getString("WIFI_SSID");
    String b = preferences.getString("WIFI_PASS");
    String firstsaved = preferences.getString("WIFI_FIRST");
    Serial.print("WIFI_FIRST=");
    Serial.println(firstsaved);

  if (firstsaved=="1") {
      preferences.putString("WIFI_FIRST", "0");
      this->closeSharedPref();
      delay(10);

      ESP.deepSleep(0.5*10e5);
  }
      // preferences.putBool("WIFI_FIRST", true);
  this->closeSharedPref();
  strcpy(that->sta_ssid, a.c_str());
  strcpy(that->sta_pwd, b.c_str());
  Serial.print("WIFI SSID = ");
  Serial.println(a.c_str());
  Serial.println("WIFI PASS = ");
  Serial.print(b.c_str());
  this->configWebServer();
}

void WiFiModule::configWebServer()
{
  static WiFiModule *that = this;
  _serverPtr->on(this->path, HTTP_POST, [&](AsyncWebServerRequest * request) {
    // String output = that->saveConfig(request, this->_managerPtr);
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
  Serial.println(output);

    AsyncWebParameter* p = request->getParam("sta_ssid", true);
    String ssid = p->value();

    p = request->getParam("sta_password", true);
    String password = p->value();

    if (password == "") {
      WiFi.begin(ssid.c_str());
    }
    else {
      WiFi.begin(ssid.c_str(), password.c_str());
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial1.print(ssid);
      Serial1.print(",,,");
      Serial1.println(password) ;
      that->openSharedPref();
      preferences.putString("WIFI_SSID", ssid);
      preferences.putString("WIFI_PASS", password);
      preferences.putString("WIFI_FIRST", "1");
      that->closeSharedPref();
      request->send(200, "application/json", "{\"status\": \"WiFi Connected.\"}");
      // WiFi.disconnect(bool wifioff = false, optional bool eraseap = false)
      delay(400);
      that->_os->enable_run_mode();
      WiFi.disconnect(true, true);
      ESP.deepSleep(0.5*10e5);
    }
    else {
      that->openSharedPref();
      preferences.putString("WIFI_SSID", ssid);
      preferences.putString("WIFI_PASS", password);
      preferences.putString("WIFI_FIRST", "1");
      that->closeSharedPref();
      that->_os->enable_run_mode();
      WiFi.disconnect(true, true);
      delay(400);
      ESP.deepSleep(0.5*10e5);
      // tft.println("Failed. try again!");
      request->send(200, "application/json", "{\"status\": \"WiFi connect failed.\"}");
    }
  });
}

void WiFiModule::setup()
{
  _init_sta();
}

void WiFiModule::loop() {

}

void WiFiModule::_init_sta()
{
  os->getBlinker()->blink(500);
  WiFi.persistent(false);
  WiFi.softAPdisconnect();
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);
  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);

  if (String(sta_pwd) == "") {
    WiFi.begin(sta_ssid);
  }
  else {
    WiFi.begin(sta_ssid, sta_pwd);
  }

  Serial.print(sta_ssid);
  Serial.print(":");
  Serial.println(sta_pwd);

  // uint32_t counter = 0;
  uint32_t prev_ms = millis();

  prev_ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
      Serial.print(sta_ssid);
      Serial.print(":");
      Serial.println(sta_pwd);
      // if (digitalRead(38) == LOW) {
      //   ESP.deepSleep(4*10e8);
      // }
    // Serial.println("Connecting WiFi in WiFiModule...");
    isLongPressed();
    // if (millis() - prev_ms > 240L*1000) {
    //   ESP.deepSleep(10e5);
    // }
    // if (millis() - prev_ms >20L*1000L) {
    //   // ESP.deepSleep(1);
    // }
  }
  Serial.println("WiFi Connected.");
  __wifi_connected = true;
}


void WiFiModule::on_event(system_event_id_t event) {

}
