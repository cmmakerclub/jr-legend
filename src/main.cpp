
/*** initial button ***/

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <CMMC_Legend.h>
#include "SPI.h"

byte BT1 = 38;
byte BT2_MIDDLE = 37;
byte BT3 = 39;

uint32_t seq = 0;

extern String DB_MINI_APP_VERSION = "v6.0.5";
extern int DB_MINI_APP_VERSION_INT = 605;
extern bool FLAG_DISPLAY_OFF;


#include <Preferences.h>
Preferences preferences;

bool sponsoring = true;
char ap_name[40];
char full_mac[40];
uint8_t SCREEN = 0;

/*** Global Variable ***/
uint32_t pevTime = 0;
const long interval = 1 * 1000;

#define RUN 1
#define CONFIG 2

int mode = RUN;

/*** Serial1 for dust sensor ***/
#define RXD2 0
#define TXD2 4


#include "modules/WiFiModule.h"
#include "modules/MqttModule.h"
#include "modules/SharedPoolModule.h"

int first_published = true;


WiFiModule *wifiModule;
MqttModule *mqttModule;
SharedPoolModule pool;

CMMC_Legend *os;

bool no_internet_link = 0;
String pressed_text = "";
unsigned int boot_count = 0;

// void sensorTask(void *parameter);
// void motionTask(void *parameter);

int LOCKING = false;

void hook_before_init_ap()
{
  Serial.println("HOOK BEFORE_INIT");
  Serial.println("HOOK BEFORE_INIT");
  Serial.println("HOOK BEFORE_INIT");
  Serial.println("HOOK BEFORE_INIT");
  SCREEN = 3;
  mode = CONFIG;
}

void hook_init_ap(char *name, char* fullmac, IPAddress ip)
{
  strcpy(ap_name, name);
  strcpy(full_mac, fullmac);
  Serial.println("----------- hook_init_ap -----------");
  Serial.println(name);
  Serial.println(ip);
  Serial.println("/----------- hook_init_ap -----------");
  SCREEN = 0;
  mode = CONFIG;
}

void hook_button_pressing()
{
      Serial.println("pressing..");
      delay(50);

}
void hook_button_pressed()
{
  Serial.println("[user] hook_button_pressed");
  pressed_text = "Waiting...";
  Serial.println("[user] set SCREEN = SCREEN_PRESSED.");
}

void hook_button_released()
{
  // pressed_text = "xxxxxxxx";
  Serial.println("[user] hook_button_released");
}

void hook_button_long_pressed()
{
  Serial.println("[user] hook_button_long_pressed");
  pressed_text = "Release the button...";
}

void hook_config_loaded() {

}

void hook_ready() {
  Serial.println("[user] hook_button_released");

}

void increaseBootCount() {
  preferences.begin("dustboy-mini", false);
  boot_count = preferences.getUInt("boot_count", 0);
  boot_count++;
  preferences.putUInt("boot_count", boot_count);
  preferences.end();
}

void setup()
{
  esp_log_level_set("*", ESP_LOG_VERBOSE);        // set all components to ERROR level
  Serial.begin(115200);
  increaseBootCount();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  // syncSemaphore = xSemaphoreCreateBinary();
  // attachInterrupt(digitalPinToInterrupt(motionSensorPin), handleInterrupt, CHANGE);
  static os_config_t config = {
      .BLINKER_PIN = 33,
      .BUTTON_MODE_PIN = 37,
      .SWITCH_PIN_MODE = INPUT_PULLUP,
      .SWITCH_PRESSED_LOGIC = LOW,
      .delay_after_init_ms = 200,
      .hook_init_ap = hook_init_ap,
      .hook_before_init_ap = hook_before_init_ap,
      .hook_button_pressed = hook_button_pressed,
      .hook_button_pressing = hook_button_pressing,
      .hook_button_long_pressed = hook_button_long_pressed,
      .hook_button_released = hook_button_released,
      .hook_ready = hook_ready,
      .hook_config_loaded = hook_config_loaded,
  };

  wifiModule = new WiFiModule();
  mqttModule = new MqttModule();

  Serial.println();

  // xTaskCreate(lcd_task, "lcd_task", 4096, NULL, 2, NULL);
  // xTaskCreate(sensorTask, "sensorTask", 4096, NULL, 1, NULL);
  // // xTaskCreate(motionTask, "motionTask", 512, NU LL, 1, NULL);
  //
  Serial.println("Starting Legend.");

  os = new CMMC_Legend(&Serial);


  os->addModule(wifiModule);
  os->addModule(mqttModule);

  Serial.println("add wifi");
  os->setup(&config);

      pinMode(37, INPUT_PULLUP);
      if(digitalRead(37) == LOW) {
        os->getBlinker()->blink(20);
        while(digitalRead(37) == LOW) {
          Serial.println("digitalRead(37) is low");
        }
        delay(100);
        os->enable_config_mode();
        ESP.restart();
      }

  os->start();
}

void loop()
{
  yield();
  os->run();
}
