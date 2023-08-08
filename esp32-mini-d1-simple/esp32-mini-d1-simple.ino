#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_err.h"

#include <Bounce2.h>
#include <BleKeyboard.h>

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

// DeviceName, DeviceManufacturer, BatteryLevel
BleKeyboard bleKeyboard("Mando Sadrollas", "Sadrollas Corp, 100");

int zoom_out = 4; //Pin GPIO al que se conecta el botón
int zoom_in = 0;
int map_down = 25;
int map_up = 27;
int map_right = 32;
int map_left = 17;
int go_location = 2;
int pulse = 150; //Tiempo de espera para repetir al mantener pulsado

// Set to 0 to view all bonded devices addresses, set to 1 to remove
#define REMOVE_BONDED_DEVICES 0
//The NimBLE mode enables a significant saving of RAM and FLASH memory
#define USE_NIMBLE

#define PAIR_MAX_DEVICES 20
uint8_t pairedDeviceBtAddr[PAIR_MAX_DEVICES][6];
char bda_str[18];

#define NUM_BUTTONS 7
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {
  go_location,
  map_up,
  map_down,
  map_right,
  map_left,
  zoom_in,
  zoom_out,
};

Bounce * buttons = new Bounce[NUM_BUTTONS];


void setup() {
  Serial.begin(115200);
  Serial.println("Starting BT Device!!!");
  //initBluetooth();
  //checkReset();
  bleKeyboard.begin();
  for (int i = 0; i < NUM_BUTTONS; i++) {
    //setup the bounce instance for the current button
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP);
    //interval in ms
    buttons[i].interval(100);
  }
}

void loop() {
  if(bleKeyboard.isConnected()) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
      //Update the Bounce instance
      buttons[i].update();
    }

    // fell() => Returns true if pin signal transitions from low to high
    if (buttons[0].fell()) {
      Serial.println("go location");
      if (bleKeyboard.isConnected()) {
        bleKeyboard.write(67); // El 67 es la C del teclado decimal
      }
    }

    if (buttons[1].fell()) {
      Serial.println("map top");
      if (bleKeyboard.isConnected()) {
        bleKeyboard.write(KEY_UP_ARROW);
      }
    }

    if (buttons[2].fell()) {
      Serial.println("map down");
      if (bleKeyboard.isConnected()) {
        bleKeyboard.write(KEY_DOWN_ARROW);
      }
    }

    if (digitalRead(zoom_in) == LOW) {
      Serial.println("zoom in");
      bleKeyboard.write(43); // el 43 es la tecla + del teclado para el zoom out
      delay(pulse);
    }
    else if (digitalRead(zoom_out) == LOW) {
      Serial.println("zoom out");
      bleKeyboard.write(45); //el  45 es la tecla - para zoom in
      delay(pulse);
    }
    else if (digitalRead(map_right) == LOW) {
      Serial.println("map right");
      bleKeyboard.write(KEY_RIGHT_ARROW);
      delay(pulse);
    }
    else if (digitalRead(map_left) == LOW) {
      Serial.println("map left");
      bleKeyboard.write(KEY_LEFT_ARROW);
      delay(pulse);
    }
  }
}


bool initBluetooth() {
  if (!btStart()) {
    Serial.println("Failed to initialize controller");
    return false;
  }

  if (esp_bluedroid_init() != ESP_OK) {
    Serial.println("Failed to initialize bluedroid");
    return false;
  }

  if (esp_bluedroid_enable() != ESP_OK) {
    Serial.println("Failed to enable bluedroid");
    return false;
  }
  
  return true;
}

char * bda2str(const uint8_t * bda, char * str, size_t size) {
  if (bda == NULL || str == NULL || size < 18) {
    return NULL;
  }
  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
    bda[0], bda[1], bda[2], bda[3], bda[4], bda[5], bda[6], bda[7]);
  return str;
}

void checkReset() {
  Serial.print("ESP32 bluetooth address: ");
  Serial.println(bda2str(esp_bt_dev_get_address(), bda_str, 18));
  // Get the numbers of bonded/paired devices in the BT module
  int count = esp_bt_gap_get_bond_device_num();
  if (!count) {
    Serial.println("No bonded BT device found.");
  }
  else {
    Serial.print("Bonded device count: ");
    Serial.println(count);
    if (abs(count) > PAIR_MAX_DEVICES) {
      count = PAIR_MAX_DEVICES;
      Serial.print("Max pair devices exceed... resetting bonded devices");
      Serial.println(count);
    }
    esp_err_t tError = esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
    if (ESP_OK == tError) {
      for (int i = 0; i < count; i++) {
        Serial.print("Found bonded device # ");
        Serial.print(i);
        Serial.print(" -> ");
        Serial.println(bda2str(pairedDeviceBtAddr[i], bda_str, 18));
        if (REMOVE_BONDED_DEVICES) {
          esp_err_t tError = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
          if (ESP_OK == tError) {
            Serial.print("Removed bonded device # ");
          }
          else {
            Serial.print("Failed to remove bonded BT device # ");
          }
          Serial.println(i);
        }
      }
    }
  }
}
