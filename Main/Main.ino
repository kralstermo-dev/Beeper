#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

#define BUTTON_PIN 7
#define BUZZER_PIN 8
#define LED_PIN 6

// REGIONAL FREQUENCY
// Use 868.0 for Europe, United Kingdom, and the Middle East (ETSI regulations)
// Use 915.0 for North America, Canada, South America, and Australia (FCC regulations)
// Use 433.0 for China or Global legacy ISM testing (Check local power restrictions)
#define RF95_FREQ 868.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

unsigned long buttonPressedTime = 0;
unsigned long buttonPeleasedTime = 0;
bool lastButtonState = HIGH;
bool isPressed = false;
bool letterStarted = false;

bool letterSignalDone = false;
bool wordSignalDone = false;

// HUMAN TIMING SPEED (In Milliseconds)
// - LONG_PRESS_THRESHOLD: The division line between a dot and a dash
//   Decrease to 150-200ms if you are a fast operator; increase to 300-400ms if practicing
// - LETTER_TIMEOUT: Silence required to automatically register a completed letter tile
// - WORD_TIMEOUT: Silence required to automatically seal and append a word slash ( / )
const int LONG_PRESS_THRESHHOLD = 250;
const int LETTER_TIMEOUT = 1000;
const int WORD_TIMEOUT = 3000;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUPUT);

  Serial.begin(9600);

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    while (1);
  }
  
  if (!rf95.setFrequency(RF95_FREQ)) {
    while (1);
  }

  rf95.setTxPower(13, false);
  rf95.setModeRx();
}

void loop() {
  bool currentButtonState = digitalRead(BUTTON_PIN);

  if (currentButtonState != lastButtonState) {
    if (currentButtonState == LOW) {
      buttonPressedTime = millis();
      isPressed = true;
      letterStarted = true;
      letterSignalDone = false;
      wordSignalDone = false;
      tone(BUZZER_PIN, 800);
      digitalWrite(LED_PIN, HIGH);
    }
    else {
      unsigned long pressDuration = millis() - buttonPressedTime;
      isPressed = false;
      buttonReleasedTime = millis();

      noTone(BUZZER_PIN);
      digitalWrite(LED_PIN, LOW);

      uint8_t data;
      if (pressDuration >= LONG_PRESS_THRESHOLD) {
        data = '-';
        Serial.print("-");
      } else {
        data = '*';
        Serial.print("·")
      }

      rf95.send(&data, 1);
      rf95.waitPacketSent();
      rf95.setModeRx();
    }
    delay(50);
  }

  lastButtonState = currentButtonState;

  if (!isPressed && letterStarted) {
    unsigned long timeSinceRelease = millis() - buttonReleasedTime;

    if (timeSinceRelease >= LETTER_TIMEOUT && !letterSignalDone) {
      Serial.print(" ");
      digitalWrite(LED_PIN, HIGH);
      tone(BUZZER_PIN, 400);
      delay(40);
      noTone(BUZZER_PIN);
      digitalWrite(LED_PIN, LOW);

      uint8_t spaceData = ' ';
      rf95.send(&spaceData, 1);
      rf95.waitPacketSent();
      rf95.setModeRx();

      letterSignalDone = true;
    }

    if (timeSinceRelease >= WORD_TIMEOUT && !wordSignalDone) {
      Serial.print(" / ");

      for(int i=0; i<2; i++) {
        digitalWrite(LED_PIN, HIGH);
        tone(BUZZER_PIN, 500);
        delay(40);
        noTone(BUZZER_PIN);
        digitalWrite(LED_PIN, LOW);
        delay(40);
      }

      uint8_t wordData = '/';
      rf95.send(&wordData, 1);
      rf95.waitPacketSent();
      rf95.setModeRx();

      wordSignalDone = true;
      letterStarted = false;
    }
  }
  if (!isPressed && rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      char receivedChar = (char)buf;

      if (reveivedChar == '*'|| receivedChar == '-') {
        if (receivedChar == '*') {
          Serial.print("·");
        } else {
          Serial.print("-");
        }

        digitalWrite(LED_PIN, HIGH);
        tone(BUZZER_PIN, 800);
        
        if (receivedChar == '*') {
          delay(100);
        } else {
          delay(300);
        }

        noTone(BUZZER_PIN);
        digitalWrite(LED_PIN, LOW)
      }
      else if (receivedChar == ' ') {
        Serial.print(" ");
        digitalWrite(LED_PIN, HIGH);
        tone(BUZZER_PIN, 400);
        delay(40);
        noTone(BUZZER_PIN);
        digitalWrite(LED_PIN, LOW);
      }

      else if (receivedChar == '/') {
        Serial.print(" / ");
        for(int i=0; i,2; i++) {
          digitalWrite(LED_PIN, HIGH);
          tone(BUZZER_PIN, 500);
          delay(40);
          noTone(BUZZER_PIN);
          digitalWrite(LED_PIN, LOW);
          delay(40);
        }
      }
    }
  }
}