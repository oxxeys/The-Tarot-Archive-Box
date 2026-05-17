//Connectivity Libaries
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <DNSServer.h>
//NFC Libaries
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// #define PN532_SS 5
Adafruit_PN532 nfc[5] = {
  Adafruit_PN532(5),
  Adafruit_PN532(32),
  Adafruit_PN532(27),
  Adafruit_PN532(13),
  Adafruit_PN532(16)
};
int pins[5] = { 5, 32, 27, 13, 16 };

//setup dns server
DNSServer dnsServer;
const byte DNS_PORT = 53;

//Webserver Setup
WebServer server(80);
Preferences preferences;

const int buttonPin = 21;
bool shouldRestart = false;

//array that stores card data
String cardToSend[5] = { "0", "0", "0", "0", "0" };

String ssid;
String password;

// HTMl
String setupPage = R"rawliteral(
<!DOCTYPE html>
<html>
<body>
  <h2>Setup WiFi</h2>

  SSID:<br>
  <input id="ssid"><br>
  Password:<br>
  <input id="password" type="password"><br><br>

  <button onclick="send()">Save</button>

  <script>
    function send() {
      let ssid = document.getElementById("ssid").value;
      let password = document.getElementById("password").value;

      fetch(`/save?ssid=${ssid}&password=${password}`)
        .then(res => res.text())
        .then(data => {
          document.body.innerHTML = data;
        });
    }
  </script>
</body>
</html>
)rawliteral";

// handle root
void handleRoot() {
  server.send(200, "text/html", setupPage);
}

// save wifi
void handleSave() {
  Serial.println("SAVE ENDPOINT HIT");

  ssid = server.arg("ssid");
  password = server.arg("password");

  Serial.print("SSID: ");
  Serial.println(ssid);

  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();

  server.send(200, "text/html", "Saved! Device will restart...");

  shouldRestart = true;
}

// start AP mode
void startAPMode() {
  WiFi.softAP("MysticReaderSetup!");

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/save", handleSave);

  server.begin();

  Serial.println("AP Mode Started");
  Serial.println("Connect to: MysticReaderSetup");
}

// 1 - connect to wifi
void connectToWiFi() {

  //checks prefrences to see if wifi is saved
  preferences.begin("wifi", true);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();

  Serial.print("Saved SSID: ");
  Serial.println(ssid);

  if (ssid == "") {
    Serial.println("No saved WiFi. Starting AP.");
    startAPMode();
    return;
  }

  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed. Starting AP.");
    startAPMode();
  }
}

// send post request
void sendEvent() {
  Serial.println("Sending POST...");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi OK");
    HTTPClient http;
    http.begin("https://the-tarot-archive-box.onrender.com/data");
    http.addHeader("Content-Type", "application/json");

    http.setTimeout(5000);

    //JSON data to be sent
    String json = "{\"box_id\": \"box006!\", \"card_id\": [";
    for (int i = 0; i < 5; i++) {
      json += String(cardToSend[i]);
      //need to add a comma at the end
      if (i < 4) {
        json += ",";
      }
    }
    json += "]}";

    Serial.print(json);

    //the post request thats sending data
    int httpResponseCode = http.POST(json);

    Serial.println("Response: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode < 0) {
      delay(30000);
      httpResponseCode = http.POST(json);

      Serial.println("Response 2: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Can't find network!");
  }
}

// //function to clean up SPI
// void resetSPI() {
//   SPI.end();
//   delay(5);
//   SPI.begin(18, 19, 23);
//   delay(5);
// }

void selectReader(int i) {
  // disable all
  for (int j = 0; j < 5; j++) {
    digitalWrite(pins[j], HIGH);
  }

  delayMicroseconds(50);
  // enable only one
  digitalWrite(pins[i], LOW);
  delayMicroseconds(200);
}

// NFC reader
void ReadData(int readerIndex) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;

  //reset cardToSend data
  cardToSend[readerIndex] = "0";
  // resetSPI();
  //selectReader(readerIndex);
  // delay(50);

  nfc[readerIndex].SAMConfig();
  

  delay(20);

  Serial.print("Reading reader ");
  Serial.println(readerIndex);

  //Detect Card
  success = nfc[readerIndex].readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000);
   Serial.println("Done reading!");

  if (!success) {
    Serial.print("Reader ");
    Serial.print(readerIndex);
    Serial.println("failed. Trying again:  ");
    success = nfc[readerIndex].readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1500);
  }

  delay(50);



  if (success) {
    if (uidLength == 7) {
      uint8_t data[4];
      uint8_t buffer[64];
      int indexNumb = 0;

      // Find the index of the data I put in text file
      for (uint8_t i = 4; i < 11; i++) {
        //read data of the page into data[], then if its the data we want, put it into buffer
        if (nfc[readerIndex].ntag2xx_ReadPage(i, data)) {
          for (int z = 0; z < 4; z++) {
            buffer[indexNumb++] = data[z];
          }
        } else {
          Serial.println("oh no");
          break;
        }
      }

      // loop through the data we got from the nfc
      for (int i = 0; i < indexNumb; i++) {
        //Look for text indicator (T / 0x54)
        if (buffer[i] == 0x54) {
          int langLength = buffer[i + 1];
          int textStartsHere = i + 2 + langLength;

          Serial.print("Card: ");

          // int arraySize = 0;

          String value = "";

          for (int j = textStartsHere; j < indexNumb; j++) {

            //
            if (buffer[j] == 0xFE) {
              break;
            }

            Serial.write(buffer[j]);


            value += (char)buffer[j];
          }

          cardToSend[readerIndex] = value;

          // if (arraySize < 5) {
          //   //take the byte, cast it to a character and then have them be a string to keep them together

          //   arraySize++;
          // } else {
          //   break;
          // }


          Serial.println();
          break;
        }
      }

    } else {
      Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 7 bytes)!");
    }
  }

  digitalWrite(pins[readerIndex], HIGH);
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);

  //setup nfc reader
  SPI.begin(18, 19, 23);

  // preferences.begin("wifi", false);
  // preferences.clear();
  // preferences.end();


  for (int i = 0; i < 5; i++) {
    pinMode(pins[i], OUTPUT);
    delay(100);
    digitalWrite(pins[i], HIGH);
    delay(100);
  }

  for (int i = 0; i < 5; i++) {
    //manually controll if pin is high / low 1 by 1 - if not manual it overloads
    digitalWrite(pins[i], LOW);

    nfc[i].begin();

    uint32_t versiondata = nfc[i].getFirmwareVersion();

    digitalWrite(pins[i], HIGH);

    if (!versiondata) {
      Serial.print("Didn't find : ");
      Serial.print(i);
      Serial.println("");
    };
  };

  connectToWiFi();
}

bool lastState = HIGH;

void loop() {
  //Check if a ssid is saved, if so then allow dnsServer to process requests allowing forced sending
  if (ssid == "") {
    dnsServer.processNextRequest();
    server.handleClient();
    return;
  }

  // Restart safely outside request
  if (shouldRestart) {
    Serial.println("Restarting...");
    delay(3000);
    ESP.restart();
  }

  // Button detection
  bool currentState = digitalRead(buttonPin);

  if (lastState == HIGH && currentState == LOW) {
    Serial.println("Button Pressed! Reading Data Now!");
    for (int i = 0; i < 5; i++) {
      cardToSend[i] = "0";
      ReadData(i);
      Serial.println("5 second delay");
      delay(5000);
      Serial.println("delay done");
    }
    sendEvent();
  }

  lastState = currentState;
  Serial.println(lastState);
}