
#include <Wire.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C_Hangul.h>
#include <Key.h>
#include <Keypad.h>
#include <Keypad_I2C.h>

#define FIREBASE_HOST "fuel-theft-61e65-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "n1a6nixIWGeyRlydWZmnNvlKUSX6C1nVMiIebtXD"
#define WIFI_SSID "Redmi"                                           // input your home or public wifi name 
#define WIFI_PASSWORD "12345678"
Servo myservo;
LiquidCrystal_I2C_Hangul lcd(0x3F, 16, 2);
int count=0;
const int vib_pin = 32;
const int relay = 25;
const int fuel = 34;
const int servoPin = 33;
/* Create an object named gps of the class TinyGPSPlus */
int val;
TinyGPSPlus gps;
void gpslocation();
#define I2CADDR 0x20 // Set the Address of the PCF8574

const byte ROWS = 4; // Set the number of Rows
const byte COLS = 4; // Set the number of Columns
String Status = "";
#define Password_Length 5
char Data[Password_Length];
char Master[Password_Length] = "1357";
byte data_count = 0, master_count = 0;
bool Pass_is_good;
char customKey;
// Set the Key at Use (4x4)
char keys [ROWS] [COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// define active Pin (4x4)
byte rowPins [ROWS] = {0, 1, 2, 3}; // Connect to Keyboard Row Pin
byte colPins [COLS] = {4, 5, 6, 7}; // Connect to Pin column of keypad.

// makeKeymap (keys): Define Keymap
// rowPins:Set Pin to Keyboard Row
// colPins: Set Pin Column of Keypad
// ROWS: Set Number of Rows.
// COLS: Set the number of Columns
// I2CADDR: Set the Address for i2C
// PCF8574: Set the number IC
Keypad_I2C keypad (makeKeymap (keys), rowPins, colPins, ROWS, COLS, I2CADDR, PCF8574);

void setup() {
  Serial.begin(115200);

  Wire .begin (); // Call the connection Wire
  keypad.begin (makeKeymap (keys)); // Call the connection

  lcd.init();
  lcd.backlight();
  lcd.setCursor(4, 0);
  lcd.print("WELCOME");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  delay(200);
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  delay(200);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial2.begin(9600);
  Firebase.setFloat("Lat", 12.954239);
  delay(1000);
  Firebase.setFloat("Long", 77.554671);
  delay(1000);
  pinMode(relay, OUTPUT);
  pinMode(fuel, INPUT);
  pinMode(vib_pin, INPUT);
  delay(300);
  digitalWrite(relay, LOW);
  delay(300);
  digitalWrite(relay, HIGH);
  delay(300);
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 500, 2400);
  servocall();
  delay(300);
}




void gpslocation()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial2.available() > 0)
    if (gps.encode(Serial2.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
}

void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);


  }
  else
  {
    Serial.print(F("INVALID"));
  }
  Serial.println();
}

void readkey()
{
  while( count<(Password_Length - 1))
  {
  Serial.println ("Reading Key....");
  lcd.clear();
  lcd.print("Enter Password");
  customKey = keypad.getKey (); // Create a variable named key of type char to hold the characters pressed

  if (customKey) {// if the key variable contains
    Serial.println (customKey); // output characters from Serial Monitor
    count=count+1;
  }

  //customKey = customKeypad.getKey();
  if (customKey) {
    Data[data_count] = customKey;
    lcd.clear();
    lcd.setCursor(data_count, 1);
    lcd.print(Data[data_count]);
    Serial.println (Data[data_count]);
    data_count++;
  }

  if (data_count == Password_Length - 1) {
    lcd.clear();

    if (!strcmp(Data, Master)) {
      lcd.clear();
      count=0;
      lcd.print("Correct");
      Serial.println ("Correct");
      Firebase.setString("ALERT", "Correct_Password");
      delay(500);
      myservo.write(0);
      delay(400);

    }
    else {
      lcd.clear();
      count=0;
      lcd.print("Incorrect");
      Serial.println ("Incorrect");
      Firebase.setString("ALERT", "InCorrect_Password");
      delay(500);
      myservo.write(90);
      delay(400);
    }

    lcd.clear();
    clearData();
  }
}

}

void clearData() {
  while (data_count != 0) {
    Data[data_count--] = 0;
  }
  return;
}

void fuelread()
{
  int fuelval = analogRead(fuel);
  Serial.println("Fuel Value:");
  Serial.println(fuelval);
  lcd.clear();
  lcd.print("FUEL");
  lcd.print(fuelval);
  Firebase.setFloat("FUEL", fuelval);
  delay(500);
}

void servocall() {
  myservo.write(0);
  delay(400);
  myservo.write(90);
  delay(400);
}

void readstatus() {
  Status = Firebase.getString("STATUS");                                      // get ld status input from firebase
  Serial.println(Status);
  if (Status == "1")

  { // compare the input of led status received from firebase
    Serial.println("Relay ON");
    delay(500);
    digitalWrite(relay, HIGH);
    delay(500);
  }
  else if (Status == "2")
  { // compare the input of led status received from firebase
    Serial.println("Relay OFF");
    delay(500);
    digitalWrite(relay, LOW);
    delay(500);
  }

  else if (Status == "3")
  { // compare the input of led status received from firebase
    Serial.println("Reading Password");
    readkey();
    delay(500);
  }
  else
  {
    fuelread();
    val = digitalRead(vib_pin);
    if (val == 0)
    {
      Serial.println("accident");
      lcd.clear();
      lcd.print("accident");
      Firebase.setString("ALERT", "accident DETECTED");
      delay(1000);
    }
    else
    {
      Serial.println("no accident");
      lcd.clear();
      lcd.print("normal");
      Firebase.setString("ALERT", "NORMAL");
      delay(1000);
    }
  }
}



void loop() {
  readstatus();
}
