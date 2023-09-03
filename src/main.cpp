// Program T-Beam Receiver / Gateway
// Versi 1.0
// Wenda Yusup
// PT. Makerindo Prima Solusi




#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <TinyGPS++.h>

#define SS   18          // LoRa radio chip select
#define RST  14       // LoRa radio reset
#define DIO0 26         // change for your board; must be a hardware interrupt pin
#define SCK  5
#define MISO 19
#define MOSI 27
#define LEDRED 14
#define LEDGREEN 13
#define LEDBLUE 25

String LAT, LON, SOG, COG,ALT;
String ID = "MKRRMP001222";
String RSI = "NAN";
String SNR = "NAN";
String data1 = "";
String data2 = "";

byte msgCount = 0;            // count of outgoing messages
int localAddress = 0;     // address of this device
int Destination = 0;
long lastSendTime = 0;        // last send time

LiquidCrystal_I2C lcd(0x27, 16, 2);
TinyGPSPlus gps;
HardwareSerial GPS(1);
AXP20X_Class axp;



void LEDCOLOR(String color) {
  if (color == "RED") {
    digitalWrite(LEDRED, HIGH);
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDBLUE, LOW);
  } else if (color == "GREEN") {
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, HIGH);
    digitalWrite(LEDBLUE, LOW);
  } else if (color == "BLUE") {
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDBLUE, HIGH);
  } else if (color == "OFF") {
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDBLUE, LOW);
  }
}



void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }
  LEDCOLOR("BLUE");
  Serial.println(incoming);
  //  Serial.println(packetSize);
  RSI = String(LoRa.packetRssi());
  SNR = String(LoRa.packetSnr());
}





void setup() {
  Serial.begin(115200);                   // initialize serial
  pinMode(LEDRED, OUTPUT);
  pinMode(LEDGREEN, OUTPUT);
  pinMode(LEDBLUE, OUTPUT);
  LEDCOLOR("OFF");

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(915E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  GPS.begin(9600, SERIAL_8N1, 34, 12);   //17-TX 18-RX
  lcd.init();
  lcd.backlight();
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(Destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}




void loop() {
  if (millis() - lastSendTime >= 100 ) {
    LEDCOLOR("OFF");
    lcd.setCursor(0,0);
    lcd.print(String() + "  " + ID);
    lcd.setCursor(0,1);
    lcd.print(String() + "RSSI=" + String(LoRa.packetRssi()) + "SNR=" + String(LoRa.packetSnr()));
    if (gps.location.isValid()) {
      LEDCOLOR("BLUE");
      LAT = String(gps.location.lat(), 9);
      LON = String(gps.location.lng(), 9);
      ALT = String(gps.altitude.feet() / 3.2808);
    } else {
      LEDCOLOR("RED");
      LAT = "NAN";
      LON = "NAN";
      ALT = "NAN";
    }
      if (Destination == localAddress) {
      
      // Destination++;Serial.println(String() + ID + "," + LAT + "," + LON + "," + SOG + "," + COG + "," + RSI + "," + SNR + ",*");
    } else if (Destination == 1) {
      String message = "REQ,*";   // send a message
      sendMessage(message);
      Destination++;
    } else if (Destination >= 2) {
      String message = "REQ,*";   // send a message
      sendMessage(message);
      Destination = 0;
    }
     lastSendTime = millis();            // timestamp the message
  } 
  else {
    while (GPS.available()) {
      gps.encode(GPS.read());
      onReceive(LoRa.parsePacket());
    }

  } 

     onReceive(LoRa.parsePacket());

}

 









