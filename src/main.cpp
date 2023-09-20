// Program T-Beam Receiver / Gateway
// Versi 1.0
// Wenda Yusup
// PT. Makerindo Prima Solusi

//--------------------------------------------------------------------LIBRARY------------------------------------------------
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>

//-------------------------------------------------------------------DEFINITION-----------------------------------------------

//------------Definition For Lora
#define SS 18
#define RST 14
#define DIO0 26
#define SCK 5
#define MISO 19
#define MOSI 27
#define LEDRED 14
#define LEDGREEN 13
#define LEDBLUE 25

//------------Definition For GPS
#define RX 34
#define TX 12
TinyGPSPlus gps;
HardwareSerial GPS(1);

//-------------------------------------------------------------------SETUP SECTION-----------------------------------------------
String LAT, LON, SOG, COG, ALT;
String ID = "MKRRMP001222";
String RSI = "NAN";
String SNR = "NAN";

byte msgCount = 0;
int localAddress = 0;
int Destination = 0;
long lastSendTime = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

//-------------------------------------------------------------------PROCEDURE SECTION--------------------------------------------
// MAIN CODE FOR COLOR LED
void LEDCOLOR(String color)
{
  if (color == "RED")
  {
    digitalWrite(LEDRED, HIGH);
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDBLUE, LOW);
  }
  else if (color == "GREEN")
  {
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, HIGH);
    digitalWrite(LEDBLUE, LOW);
  }
  else if (color == "BLUE")
  {
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDBLUE, HIGH);
  }
  else if (color == "OFF")
  {
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDBLUE, LOW);
  }
}
// MAIN CODE SETUP MESSAGE FOR SENDING
void onReceive(int packetSize)
{
  if (packetSize == 0)
    return;

  // read packet header bytes:
  int recipient = LoRa.read();
  byte sender = LoRa.read();
  byte incomingMsgId = LoRa.read();
  byte incomingLength = LoRa.read();
  String incoming = "";

  while (LoRa.available())
  {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length())
  { // check length for error
    Serial.println("error: message length does not match length");
    return; // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF)
  {
    Serial.println("This message is not for me.");
    return; // skip rest of function
  }
  LEDCOLOR("BLUE");
  Serial.println(incoming);
  //  Serial.println(packetSize);
  RSI = String(LoRa.packetRssi());
  SNR = String(LoRa.packetSnr());
}
// MAIN CODE START LORA
void startLoRa()
{
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(915E6))
  { // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true)
      ; // if failed, do nothing
  }
}
// MAIN CODE SEND MESSAGE LORA
void sendMessage(String outgoing)
{
  LoRa.beginPacket();            // start packet
  LoRa.write(Destination);       // add destination address
  LoRa.write(localAddress);      // add sender address
  LoRa.write(msgCount);          // add message ID
  LoRa.write(outgoing.length()); // add payload length
  LoRa.print(outgoing);          // add payload
  LoRa.endPacket();              // finish packet and send it
  msgCount++;                    // increment message ID
}


//--------------------------------------------------------------------VOID SETUP--------------------------------------------------
void setup()
{

  //---------BEGIN SECTION
  Serial.begin(115200);
  GPS.begin(9600, SERIAL_8N1, RX, TX); 
  lcd.init();
  lcd.backlight();
  startLoRa();

  //---------PINMODE LED
  pinMode(LEDRED, OUTPUT);
  pinMode(LEDGREEN, OUTPUT);
  pinMode(LEDBLUE, OUTPUT);
  LEDCOLOR("OFF");


}



//--------------------------------------------------------------------VOID lOOP--------------------------------------------------
void loop()
{
  if (millis() - lastSendTime >= 100)
  {

    if (Destination == localAddress)
    {

      Destination++;
      Serial.println(String() + ID + "," + LAT + "," + LON + "," + SOG + "," + COG + "," + RSI + "," + SNR + ",*");
    }
    else if (Destination == 1)
    {
      String message = "REQ,*"; // send a message
      sendMessage(message);
      Destination++;
    }
    else if (Destination >= 2)
    {
      String message = "REQ,*"; // send a message
      sendMessage(message);
      Destination = 0;
    }
    lastSendTime = millis(); // timestamp the message
  }
  onReceive(LoRa.parsePacket());
}

// #include <Arduino.h>
// #include <Wire.h>
// #include <SPI.h> // include libraries
// #include <LoRa.h>

// #define SS 18   // LoRa radio chip select
// #define RST 14  // LoRa radio reset
// #define DIO0 26 // change for your board; must be a hardware interrupt pin
// #define SCK 5
// #define MISO 19
// #define MOSI 27

// void setup()
// {
//   Serial.begin(115200);
//   SPI.begin(SCK, MISO, MOSI, SS);
//   LoRa.setPins(SS, RST, DIO0);
//   LoRa.begin(915E6);
// }

// void loop()
// {
//   int packetsize = LoRa.parsePacket();
//   if (packetsize)
//   {
//     while (LoRa.available())
//     {
//       String message = LoRa.readString();
//       Serial.println(String() + "PESAN: " + message);
//       Serial.println(String() + "RSSI : " + LoRa.packetRssi());
//     }
//   }
// }
