#include <ESP8266WiFi.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

WiFiClient client;

#define RECV_PIN D5
#define BAUD_RATE 115200

IRrecv irrecv(RECV_PIN); // 

void doConnect() {
  client.stop();
  client.connect("192.168.121.72", 6600);
  while (!client.available()) {
    delay(50);
  }

  Serial.println("Connected!");

  while (client.connected() && client.available()) {
    char c = client.read();
    Serial.print(c);
    if (c == '\n') {
      break;
    }
  }

}

void setup() {
  // pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D7, OUTPUT);
  Serial.begin(115200);

  WiFi.begin("rabbithole_asus", "e0953676523");
  WiFi.waitForConnectResult();

  // irrecv.setUnknownThreshold(12); 
  irrecv.enableIRIn();  // Start the receiver

  // Serial.println("Connecting");

/*
  client.print("currentsong\n");

  Serial.println("Request sent!");

  while (!client.available()) {
    delay(50);
  }
  
  while (client.connected() && client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  Serial.println("Here you go!");
*/
}
int lstInputVal = - 100;
int lastSent = millis() - 1000;

int lastDigit = 0;
int lastDigitPressed = -1; 

decode_results results;

// char* startBits = "101000000000101010";


struct Key {
  char* bin;
  char* value;
};

Key keys[] = {
  { "101000000000101010001000101000001000000000000010001010101", "n0" },
  { "1010000000001010100010001010001000000000000000001010101010", "n1" },
  { "1010000000001010100010001010001010001000000000000010001010101", "n2" },
  { "10100000000010101000100010100010100010100000000000100000101010", "n3" },
  { "10100000000010101000100010100010001000000000000010001010101010", "n4" },
  { "1010000000001010100010001010001000001000000000001010001010", "n5" },
  { "1010000000001010100010001010001000100010000000001000100010101", "n6" },
  { "10100000000010101000100010100000101000000000001000001010101", "n7" },
  { "1010000000001010100010001010000010001000000000100010001010", "n8" },
  { "1010000000001010100010001010000010000010000000100010100010101", "n9" },

  { "1010000000001010100010001010001010000000000000000010101", "tvfm" },
  { "10100000000010101000100010100010101000000000000000001010101", "source" },
  { "10100000000010101000100010100000001010100000001010000000101010", "scan" },
  { "10100000000010101000100010100000101010100000001000000000101010", "power" },
  { "10100000000010101000100010100010100000100000000000101000101", "recall" },
  { "1010000000001010100010001010000000000010000000101010100010101", "plus_100" },
  { "1010000000001010100010001010001010101010000000000000000010101", "channel_up" },
  { "1010000000001010100010001010001010100010000000000000100010101", "channel_down" },
  { "1010000000001010100010001010000010100010000000100000100010101", "volume_up" },
  { "1010000000001010100010001010000000100010000000101000100010101", "volume_down" },
  { "1010000000001010100010001010000000001010000000101010000010101", "mute" },
  { "1010000000001010100010001010001000000010000000001010100010101", "play" },
  { "1010000000001010100010001010000000001000000000101010001010101", "stop" },
  { "1010000000001010100010001010000000000000000000101010101010101", "record" },
  { "1010000000001010100010001010000010001010000000100010000010101", "freeze" },
  { "1010000000001010100010001010001000001010000000001010000010101", "zoom" },
  { "1010000000001010100010001010000000100000000000101000101010101", "rewind" },
  { "1010000000001010100010001010000010101000000000100000001010101", "function" },
  { "1010000000001010100010001010000000101000000000101000001010101", "wind" },
  { "1010000000001010100010001010001000101000000000001000001010101", "mts" },
  { "10100000000010101000100010100010001010100000000010000000101010", "reset" },
  { "10100000000010101000100010100010101010000000000000000010101010", "min" },
};

void sendToMpc(const String& val) {
    if (millis() - lastSent > 3000) {
      doConnect();
    }
    client.print(val + String("\n"));
}

void loop() {
  digitalWrite(D7, HIGH);   // turn the D7 on (HIGH is the voltage level)
  delay(10);                       // wait
  int inputVal = analogRead (A0); // Analog Values 0 to 1024
  //     // turn the LED off by making the voltage LOW
  // delay(inputVal);                       // wait for a second

  if (abs(inputVal - lstInputVal) > 2) {
    String vol(String("setvol ") + String(inputVal/5));
    sendToMpc(vol); 
    
    lastSent = millis();
    lstInputVal = inputVal;
  }
  digitalWrite(D7, LOW);
  delay(20);

  if (irrecv.decode(&results)) {
    if (results.rawlen > 20) {
      int decodedLen = 0;
      char decoded[300] = {0};
      for (int i = 0; i < results.rawlen && i < sizeof(decoded); ++i) {
        char c = -1;
        int val = results.rawbuf[i];
        if (val > 1000) {
          continue;
        } else if (val < 100) {
          // Serial.print(".");
          continue; // skip!
        } else if (val < 500) {
          c = '0';
        } else {
          c = '1';
        }
        decoded[decodedLen++] = c;
      }

      String decodedStr(decoded);
      Key* recognized = NULL;
      int kk = 0;
      for (int k = 0; k < (sizeof(keys)/sizeof(keys[0])); ++k) {
        if (decodedStr.indexOf(keys[k].bin) != -1) {
          // Key pressed!
          recognized = &(keys[k]);
          kk = k;

          Serial.println(keys[k].value);
  
          break;
        }
      }

      if (recognized == NULL) {
        Serial.println(decoded);
      } else {
        if (recognized->value == "volume_up") {
          sendToMpc("volume 1");
        } else if (recognized->value == "volume_down") {
          sendToMpc("volume -1");
        } else if (recognized->value == "channel_down") {
          sendToMpc("next");
        } else if (recognized->value == "channel_up") {
          sendToMpc("previous");
        } else if (recognized->value == "mute") {
          sendToMpc("setvol 0");
        } else if (recognized->value == "stop") {
          sendToMpc("stop");
        } else if (recognized->value == "play") {
          sendToMpc("play");
        } else if (kk <= 9) {
          if (millis() - lastDigitPressed < 500) {
            // Add this digit
            lastDigit = lastDigit * 10 + kk;
          } 
          lastDigitPressed = millis();
        } 
      }
    }

    if (lastDigitPressed != -1 && (millis() - lastDigitPressed > 500)) {
      sendToMpc("play " + String(lastDigit, DEC));
      lastDigit = 0;
      lastDigitPressed = -1;
    }
    
    // serialPrintUint64(results.value, BIN);
    // Serial.print(" ");
    // Serial.print(results.decode_type);

    irrecv.resume();  // Receive the next value
  }
}

