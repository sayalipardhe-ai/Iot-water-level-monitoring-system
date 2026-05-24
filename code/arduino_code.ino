unsigned long previousMillis = 0;
unsigned long interval = 30000;

#define BLYNK_TEMPLATE_ID "TMPL3o0kOUIUY"
#define BLYNK_TEMPLATE_NAME "water level"
#define BLYNK_AUTH_TOKEN "7D7Q-0jmQTOuGs7TP5QuKJxOIygVs9Iw"


#ifdef ESP32
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#endif

#include <LiquidCrystal_I2C.h>
#include <AceButton.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Your WiFi credentials.
// Set password to "" for open networks.
const char* ssid = "SuryaRMS";
const char* password = "E9WV07VA_@2018";

//Set Water Level Distance in CM
int emptyTankDistance = 70;  //Distance when tank is empty
int fullTankDistance = 30;   //Distance when tank is full

//Set trigger value in percentage
int triggerPer = 10;  //alarm will start when water level drop below triggerPer

// Define connections to sensor
#define TRIGPIN 14
#define ECHOPIN 12
#define wifiLed 13
#define ButtonPin1 3
#define BuzzerPin 2
#define GreenLed 16

//Change the virtual pins according the rooms
#define VPIN_BUTTON_1 V1
#define VPIN_BUTTON_2 V2

float duration;
float distance;
int waterLevelPer;
bool toggleBuzzer = HIGH;  //Define to remember the toggle state

char auth[] = BLYNK_AUTH_TOKEN;

using namespace ace_button;

ButtonConfig config1;
AceButton button1(&config1);

//void handleEvent1(aceButton*, uint8_t, uint8_t);

BlynkTimer timer;

void checkBlynkStatus() {  // called every 3 seconds by SimpleTimer

  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    //Serial.println("Blynk Not Connected");
    digitalWrite(wifiLed, LOW);
  }
  if (isconnected == true) {
    digitalWrite(wifiLed, HIGH);
    //Serial.println("Blynk Connected");
  }
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(VPIN_BUTTON_1);
  Blynk.syncVirtual(VPIN_BUTTON_2);
}

void displayData(int value)
{

  lcd.setCursor(2, 0);  //Set cursor to character 2 on line 0
  lcd.print("value");

  lcd.setCursor(2, 1);  //Move cursor to character 2 on line 1
  lcd.print(value);

  lcd.print("%");

}

void measureDistance() {
  // Set the trigger pin LOW for 2uS
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);

  // Set the trigger pin HIGH for 20us to send pulse
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(20);

  // Return the trigger pin to LOW
  digitalWrite(TRIGPIN, LOW);

  // Measure the width of the incoming pulse
  duration = pulseIn(ECHOPIN, HIGH);

  // Determine distance from duration
  // Use 343 metres per second as speed of sound
  // Divide by 1000 as we want millimeters

  distance = ((duration / 2) * 0.343) / 10;

  if (distance > (fullTankDistance - 10) && distance < emptyTankDistance) {
    waterLevelPer = map((int)distance, emptyTankDistance, fullTankDistance, 0, 100);
    displayData(waterLevelPer);
    Blynk.virtualWrite(VPIN_BUTTON_1, waterLevelPer);
    Blynk.virtualWrite(VPIN_BUTTON_2, (String(distance) + " cm"));

    // Print result to serial monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (waterLevelPer < triggerPer) {
      digitalWrite(GreenLed, HIGH);
      if (toggleBuzzer == HIGH) {
        digitalWrite(BuzzerPin, HIGH);
      }
    }
    if (distance < fullTankDistance) {
      digitalWrite(GreenLed, LOW);
      if (toggleBuzzer == HIGH) {
        digitalWrite(BuzzerPin, HIGH);
      }
    }

    if (distance > (fullTankDistance + 5) && waterLevelPer > (triggerPer + 5)) {
      toggleBuzzer = HIGH;
      digitalWrite(BuzzerPin, LOW);
    }
  }

  // Delay before repeating measurement
  delay(100);
}


void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void setup()
{
  // Set up serial monitor
  Serial.begin(115200);

  // Set pinmodes for sensor connections
  pinMode(ECHOPIN, INPUT);
  pinMode(TRIGPIN, OUTPUT);
  pinMode(wifiLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);

  pinMode(ButtonPin1, INPUT_PULLUP);

  digitalWrite(wifiLed, LOW);
  digitalWrite(GreenLed, LOW);
  digitalWrite(BuzzerPin, LOW);

  config1.setEventHandler(button1Handler);

  button1.init(ButtonPin1);

  lcd.init();
  lcd.clear();
  lcd.backlight();  // Make sure backlight is on

  // Print a message on both lines of the LCD.
  lcd.setCursor(2, 0);  //Set cursor to character 2 on line 0
  lcd.print("Hello world!");

  lcd.setCursor(2, 1);  //Move cursor to character 2 on line 1
  lcd.print("LCD Tutorial");

  initWiFi();
  timer.setInterval(2000L, checkBlynkStatus);  // check if Blynk server is connected every 2 seconds
  Blynk.config(auth);
  delay(1000);
}
void loop()
{

  measureDistance();

  Blynk.run();
  timer.run();  // Initiates SimpleTimer

  button1.check();

  wifi_reconnect();

}

void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState)
{
  Serial.println("EVENT1");
  switch (eventType)
  {
    case AceButton::kEventReleased:
      //Serial.println("kEventReleased");
      digitalWrite(BuzzerPin, LOW);
      toggleBuzzer = LOW;
      break;
  }
}

void wifi_reconnect()
{
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval))
  {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }

}
