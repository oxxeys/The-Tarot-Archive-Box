/*
*
* All the resources for this project:
* Modified by Handson Technology
* www.handsontec.com
* Created by Handsontec Tech team
*
*/
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 27
#define RST_PIN 33

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin(14, 12, 13, SS_PIN); // SCK=14, MISO=12, MOSI=13, SS=27
  mfrc522.PCD_Init();
  Serial.println("Put your card close to the reader...");
}

void loop() {
 // Look for new cards
 if ( ! mfrc522.PICC_IsNewCardPresent())
 {
 return;
 }
 // Select one of the cards
 if ( ! mfrc522.PICC_ReadCardSerial())
 {
 return;
 }
 //Show UID on serial monitor
 Serial.print("UID tag :");
 String content= "";
 byte letter;
 for (byte i = 0; i < mfrc522.uid.size; i++)
 {
 Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
 Serial.print(mfrc522.uid.uidByte[i], HEX);
 content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
 content.concat(String(mfrc522.uid.uidByte[i], HEX));
 }
 Serial.println();
 Serial.print("Message : ");
 content.toUpperCase();
 if (content.substring(1) == "89 9E CA B0") //change here the UID of the card/cards that you want to give access
 {
 Serial.println("Authorized access");
 Serial.println();
 delay(3000);
 }
else {
 Serial.println(" Access denied");
 Serial.println(content.substring(1));
 delay(3000);
 }
}