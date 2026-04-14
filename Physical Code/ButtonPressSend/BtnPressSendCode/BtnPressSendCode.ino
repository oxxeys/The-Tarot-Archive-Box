#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Preferences.h>

WebServer server(80);
Preferences preferences;

const int buttonPin = 14;
bool shouldRestart = false;

String ssid;
String password;

// ===== HTML SETUP PAGE =====
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

// ===== HANDLE ROOT =====
void handleRoot() {
  server.send(200, "text/html", setupPage);
}

// ===== SAVE WIFI =====
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

// ===== START AP MODE =====
void startAPMode() {
  WiFi.softAP("BoxSetup123");

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/save", handleSave);

  server.begin();

  Serial.println("AP Mode Started");
  Serial.println("Connect to: BoxSetup123");
}

// ===== CONNECT TO WIFI =====
void connectToWiFi() {
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

// ===== SEND POST REQUEST =====
void sendEvent() {
  Serial.println("Sending POST...");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi OK");
// https://webhook.site/9481ee50-bb48-4464-a1cb-9a8699894ac3
    HTTPClient http;
    http.begin("https://the-tarot-archive-box.onrender.com/data");
    http.addHeader("Content-Type", "application/json");

    String json = R"({
      "box_id": "TEST123",
      "event": "button_pressed"
    })";

    int httpResponseCode = http.POST(json);

    Serial.print("Response: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == -11){
      delay(5000);
      httpResponseCode = http.POST(json);
    }

    http.end();
  } else {
    Serial.println("WiFi NOT connected");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);

  connectToWiFi();
}

bool lastState = HIGH;

void loop() {
  server.handleClient();

  // Restart safely outside request
  if (shouldRestart) {
    Serial.println("Restarting...");
    delay(3000);
    ESP.restart();
  }

  // Button detection (fixed debounce)
  bool currentState = digitalRead(buttonPin);

  if (lastState == HIGH && currentState == LOW) {
    Serial.println("Button Pressed!");
    sendEvent();
  }

  lastState = currentState;
}