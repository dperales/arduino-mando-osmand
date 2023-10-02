#define USE_NIMBLE
#include <BleKeyboard.h>
#include <Bounce2.h>

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

// DeviceName, DeviceManufacturer, BatteryLevel
BleKeyboard bleKeyboard("Mando xxxx", "XXXXX Corp, 100");

int zoom_out = 4; //Pin GPIO al que se conecta el botón
int zoom_in = 0;
int map_down = 25;
int map_up = 27;
int map_right = 32;
int map_left = 17;
int go_location = 2;
int pulse = 150; //Tiempo de espera para repetir al mantener pulsado

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
