/*========================================
|Include all libraries needed for program|
========================================*/

// Include the jpeg decoder library
#include <TJpg_Decoder.h>

// Include SPIFFS
#include <FS.h>

// Include Regex
#include <regex>

//Include JSON
#include <ArduinoJson.h>

//Include base 64 encode
#include <base64.h>

// Include WiFi and http client
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecureBearSSL.h>

// Load tabs attached to this sketch
#include "List_SPIFFS.h"
#include "Web_Fetch.h"
#include "index.h"

// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include "SPI.h"
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
int imageOffsetX = 26, imageOffsetY = 20;

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
  if (y >= tft.height()) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

/*=========================
|User modifiable variables|
=========================*/
// WiFi credentials
#define WIFI_SSID "WiFi SSID Here"
#define PASSWORD "WiFi Password Here"

// Spotify API credentials
#define CLIENT_ID "Spotify Client ID Here"
#define CLIENT_SECRET "Spotify Client Secret Here"
#define REDIRECT_URI "http://'ADD THE IP DISPLAYED HERE'/callback"

// Rotary Encoder
#define CLK_PIN D1
#define DT_PIN D2

#define MAX_ATTEMPTS 50


/*=========================
|Non - modifiable variables|
==========================*/

String getValue(HTTPClient &http, String key) {
  bool found = false, look = false, seek = true;
  int ind = 0;
  String ret_str = "";

  int len = http.getSize();
  char char_buff[1];
  WiFiClient *stream = http.getStreamPtr();
  while (http.connected() && (len > 0 || len == -1)) {
    size_t size = stream->available();
    if (size) {
      int c = stream->readBytes(char_buff, ((size > sizeof(char_buff)) ? sizeof(char_buff) : size));
      if (found) {
        if (seek && char_buff[0] != ':') {
          continue;
        } else if (char_buff[0] != '\n') {
          if (seek && char_buff[0] == ':') {
            seek = false;
            int c = stream->readBytes(char_buff, 1);
          } else {
            ret_str += char_buff[0];
          }
        } else {
          break;
        }
      } else if ((!look) && (char_buff[0] == key[0])) {
        look = true;
        ind = 1;
      } else if (look && (char_buff[0] == key[ind])) {
        ind++;
        if (ind == key.length()) found = true;
      } else if (look && (char_buff[0] != key[ind])) {
        ind = 0;
        look = false;
      }
    }
  }
  if (*(ret_str.end() - 1) == ',') {
    ret_str = ret_str.substring(0, ret_str.length() - 1);
  }
  return ret_str;
}

// http response struct
struct httpResponse {
  int responseCode;
  String responseMessage;
};

struct songDetails {
  int durationMs;
  String album;
  String artist;
  String song;
  String Id;
};

char *parts[10];

void printSplitString(String text, int maxLineSize, int yPos) {
  int currentWordStart = 0;
  int spacedCounter = 0;
  int spaceIndex = text.indexOf(" ");

  while (spaceIndex != -1) {
    char *part = parts[spacedCounter];
    sprintf(part, text.substring(currentWordStart, spaceIndex).c_str());
    currentWordStart = spaceIndex;
    spacedCounter++;
    spaceIndex = text.indexOf(" ", spaceIndex + 1);
  }
  char *part = parts[spacedCounter];
  sprintf(part, text.substring(currentWordStart, text.length()).c_str());
  currentWordStart = spaceIndex;
  size_t counter = 0;
  currentWordStart = 0;
  while (counter <= spacedCounter) {
    char printable[maxLineSize];
    char *printablePointer = printable;

    sprintf(printablePointer, parts[counter]);

    int currentLen = 0;
    while (parts[counter][currentLen] != '\0') {
      currentLen++;
      printablePointer++;
    }
    counter++;
    while (counter <= spacedCounter) {
      int nextLen = 0;
      while (parts[counter][nextLen] != '\0') {
        nextLen++;
      }
      if (currentLen + nextLen > maxLineSize)
        break;
      sprintf(printablePointer, parts[counter]);
      currentLen += nextLen;
      printablePointer += nextLen;
      counter++;
    }
    String output = String(printable);
    if (output[0] == ' ')
      output = output.substring(1);
    tft.setCursor((int)(tft.width() / 2 - tft.textWidth(output) / 2), tft.getCursorY());
    tft.println(output);
  }
}

// Create spotify connection class
class SpotConn {
public:
  SpotConn() {
    client = std::make_unique<BearSSL::WiFiClientSecure>();
    client->setInsecure();
  }

  bool getUserCode(String serverCode) {
    https.begin(*client, "https://accounts.spotify.com/api/token");
    String auth = "Basic " + base64::encode(String(CLIENT_ID) + ":" + String(CLIENT_SECRET));
    https.addHeader("Authorization", auth);
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String requestBody = "grant_type=authorization_code&code=" + serverCode + "&redirect_uri=" + String(REDIRECT_URI);

    int httpResponseCode = https.POST(requestBody);

    if (httpResponseCode == HTTP_CODE_OK) {
      String response = https.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, response);
      accessToken = String((const char *)doc["access_token"]);
      refreshToken = String((const char *)doc["refresh_token"]);
      tokenExpireTime = doc["expires_in"];
      tokenStartTime = millis();
      accessTokenSet = true;
      Serial.println(accessToken);
      Serial.println(refreshToken);
    } else {
      Serial.println(https.getString());
    }

    https.end();
    return accessTokenSet;
  }

  bool refreshAuth() {
    https.begin(*client, "https://accounts.spotify.com/api/token");
    String auth = "Basic " + base64::encode(String(CLIENT_ID) + ":" + String(CLIENT_SECRET));
    https.addHeader("Authorization", auth);
    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String requestBody = "grant_type=refresh_token&refresh_token=" + String(refreshToken);

    int httpResponseCode = https.POST(requestBody);
    accessTokenSet = false;

    if (httpResponseCode == HTTP_CODE_OK) {
      String response = https.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, response);
      accessToken = String((const char *)doc["access_token"]);
      tokenExpireTime = doc["expires_in"];
      tokenStartTime = millis();
      accessTokenSet = true;
      Serial.println(accessToken);
      Serial.println(refreshToken);
    } else {
      Serial.println(https.getString());
    }

    https.end();
    return accessTokenSet;
  }

  bool getTrackInfo() {
    String url = "https://api.spotify.com/v1/me/player/currently-playing";
    https.useHTTP10(true);
    https.begin(*client, url);
    String auth = "Bearer " + String(accessToken);
    https.addHeader("Authorization", auth);
    int httpResponseCode = https.GET();
    bool success = false;
    String songId = "";
    bool refresh = false;

    if (httpResponseCode == 200) {
      String currentSongProgress = getValue(https, "progress_ms");
      currentSongPositionMs = currentSongProgress.toFloat();

      String imageLink = "";
      while (imageLink.indexOf("image") == -1) {
        String height = getValue(https, "height");
        if (height.toInt() > 300) {
          imageLink = "";
          continue;
        }
        imageLink = getValue(https, "url");
      }

      String albumName = getValue(https, "name");
      String artistName = getValue(https, "name");
      String songDuration = getValue(https, "duration_ms");
      currentSong.durationMs = songDuration.toInt();
      String songName = getValue(https, "name");
      songId = getValue(https, "uri");
      String isPlay = getValue(https, "is_playing");
      isPlaying = isPlay == "true";
      Serial.println(isPlay);
      songId = songId.substring(15, songId.length() - 1);
      https.end();
      if (songId != currentSong.Id) {
        if (SPIFFS.exists("/albumArt.jpg") == true) {
          SPIFFS.remove("/albumArt.jpg");
        }
        bool loaded_ok = getFile(imageLink.substring(1, imageLink.length() - 1).c_str(), "/albumArt.jpg");
        Serial.println("Image load was: ");
        Serial.println(loaded_ok);
        refresh = true;
        tft.fillScreen(TFT_BLACK);
      }
      currentSong.album = albumName.substring(1, albumName.length() - 1);
      currentSong.artist = artistName.substring(1, artistName.length() - 1);
      currentSong.song = songName.substring(1, songName.length() - 1);
      currentSong.Id = songId;
      success = true;
    } else {
      Serial.print("Error getting track info: ");
      Serial.println(httpResponseCode);
      https.end();
    }

    if (success) {
      drawScreen(refresh);
      lastSongPositionMs = currentSongPositionMs;
    }
    return success;
  }

  bool drawScreen(bool fullRefresh = false, bool likeRefresh = false) {
    int rectWidth = 120;
    int rectHeight = 10;
    static int scrollOffset = 0;

    String songTitle = cleanSongTitle(currentSong.song);

    if (fullRefresh) {
      if (SPIFFS.exists("/albumArt.jpg")) {
        TJpgDec.setSwapBytes(true);
        TJpgDec.setJpgScale(1);
        int xCenter = 10;
        TJpgDec.drawFsJpg(xCenter, 10, "/albumArt.jpg");
        Serial.println("Displaying album art.");
      } else {
        TJpgDec.setSwapBytes(false);
        TJpgDec.setJpgScale(2);
        int xCenter = (tft.width()) / 2;
        TJpgDec.drawFsJpg(xCenter, 10, "/Angry.jpg");
        Serial.println("Displaying default image.");
      }

      tft.setTextDatum(MC_DATUM);
      tft.setTextSize(3);
      tft.setCursor(tft.width() / 2, 180);
      tft.setTextWrap(false, false);

      if (songTitle.length() > 14 && songTitle.length() < 24) {
        tft.setTextSize(2);
        tft.drawString(songTitle, tft.width() / 2, 180);
      } else if (songTitle.length() > 24) {
        tft.setTextSize(2);
        String songTitleLong = songTitle.substring(0, 21) + "...";
        tft.drawString(songTitleLong, tft.width() / 2, 180);
      } else {
        tft.setTextSize(3);
        tft.drawString(songTitle, tft.width() / 2, 180);
      }

      Serial.print("Drawing song title: ");
      Serial.println(currentSong.song);

      tft.setTextSize(2);
      tft.setCursor(tft.width() / 2, 210);
      Serial.print("Drawing artist name: ");
      Serial.println(currentSong.artist);
      tft.drawString(currentSong.artist, tft.width() / 2, 210);

      tft.drawRoundRect(
        tft.width() / 2 - rectWidth / 2 + 2,
        230,
        rectWidth - 3,
        rectHeight - 3,
        10,
        TFT_DARKGREEN);
    }

    if (lastSongPositionMs > currentSongPositionMs) {
      tft.fillSmoothRoundRect(
        tft.width() / 2 - rectWidth / 2 + 2,
        230,
        rectWidth - 4,
        rectHeight - 4,
        10,
        TFT_BLACK);
      lastSongPositionMs = currentSongPositionMs;
      Serial.println("Updating song position.");
    }

    tft.fillSmoothRoundRect(
      (tft.width() / 2 - rectWidth / 2 + 2) + 10,
      230,
      rectWidth * (currentSongPositionMs / currentSong.durationMs) - 4,
      rectHeight - 4,
      10,
      TFT_GREEN);

    return true;
  }

  String cleanSongTitle(String songTitle) {
    // List of special characters to remove along with the text after them
    String specialChars = "()-";

    // Find the index of any special character
    int specialCharIndex = songTitle.length();
    for (char c : specialChars) {
      int index = songTitle.indexOf(c);
      if (index != -1 && index < specialCharIndex) {
        specialCharIndex = index;
      }
    }

    // If a special character is found, truncate the string
    if (specialCharIndex != songTitle.length()) {
      songTitle = songTitle.substring(0, specialCharIndex);
    }

    // Trim any whitespace from the start and end of the string
    songTitle.trim();

    return songTitle;
  }

  bool togglePlay() {
    String url = "https://api.spotify.com/v1/me/player/" + String(isPlaying ? "pause" : "play");
    isPlaying = !isPlaying;
    https.begin(*client, url);
    String auth = "Bearer " + String(accessToken);
    https.addHeader("Authorization", auth);
    int httpResponseCode = https.PUT("");
    bool success = false;

    if (httpResponseCode == 204) {
      Serial.println((isPlaying ? "Playing" : "Pausing"));
      success = true;
    } else {
      Serial.print("Error pausing or playing: ");
      Serial.println(httpResponseCode);
      String response = https.getString();
      Serial.println(response);
    }

    https.end();
    getTrackInfo();
    return success;
  }

  bool adjustVolume(int vol) {
    Serial.print("Volume Change: ");
    Serial.print(vol);
    String url = "https://api.spotify.com/v1/me/player/volume?volume_percent=" + String(vol);
    https.begin(*client, url);
    String auth = "Bearer " + String(accessToken);
    https.addHeader("Authorization", auth);
    int httpResponseCode = https.PUT("");
    bool success = false;

    if (httpResponseCode == 204) {
      currVol = vol;
      success = true;
    } else if (httpResponseCode == 403) {
      currVol = vol;
      success = false;
      Serial.print("Error setting volume: ");
      Serial.println(httpResponseCode);
      String response = https.getString();
      Serial.println(response);
    } else {
      Serial.print("Error setting volume: ");
      Serial.println(httpResponseCode);
      String response = https.getString();
      Serial.println(response);
    }
    https.end();

    // Display volume level in top-right corner
    tft.setTextSize(2);
    tft.setTextDatum(TR_DATUM);              // Align text to top-right corner
    tft.setCursor(tft.width() - 5, 5);       // Set position to top-right corner with some padding
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // White text with black background to clear area
    tft.drawString("Vol: " + String(vol), tft.width() - 5, 5);

    delay(3000);  // Show for 3 seconds

    return success;
  }

  bool skipForward() {
    String url = "https://api.spotify.com/v1/me/player/next";
    https.begin(*client, url);
    String auth = "Bearer " + String(accessToken);
    https.addHeader("Authorization", auth);
    int httpResponseCode = https.POST("");
    bool success = false;

    if (httpResponseCode == 204) {
      Serial.println("Skipping forward");
      success = true;
    } else {
      Serial.print("Error skipping forward: ");
      Serial.println(httpResponseCode);
      String response = https.getString();
      Serial.println(response);
    }

    https.end();
    getTrackInfo();
    return success;
  }

  bool skipBack() {
    String url = "https://api.spotify.com/v1/me/player/previous";
    https.begin(*client, url);
    String auth = "Bearer " + String(accessToken);
    https.addHeader("Authorization", auth);
    int httpResponseCode = https.POST("");
    bool success = false;

    if (httpResponseCode == 204) {
      Serial.println("Skipping backward");
      success = true;
    } else {
      Serial.print("Error skipping backward: ");
      Serial.println(httpResponseCode);
      String response = https.getString();
      Serial.println(response);
    }

    https.end();
    getTrackInfo();
    return success;
  }

  bool accessTokenSet = false;
  long tokenStartTime;
  int tokenExpireTime;
  songDetails currentSong;
  float currentSongPositionMs;
  float lastSongPositionMs;
  int currVol;

private:
  std::unique_ptr<BearSSL::WiFiClientSecure> client;
  HTTPClient https;
  bool isPlaying = false;
  String accessToken;
  String refreshToken;
};

bool buttonStates[] = { 1, 1, 1, 1 };
int debounceDelay = 50;
unsigned long debounceTimes[] = { 0, 0, 0, 0 };
int buttonPins[] = { D0, D6, 1, 3 };

ESP8266WebServer server(80);
SpotConn spotifyConnection;

void handleRoot() {
  Serial.println("Handling root");
  char page[500];
  sprintf(page, mainPage, CLIENT_ID, REDIRECT_URI);
  server.send(200, "text/html", String(page) + "\r\n");
}

void handleCallbackPage() {
  if (!spotifyConnection.accessTokenSet) {
    if (server.arg("code") == "") {
      char page[500];
      sprintf(page, errorPage, CLIENT_ID, REDIRECT_URI);
      Serial.println("Spotify setup start");
      server.send(200, "text/html", String(page));
    } else {
      Serial.println("Spotify setup start2");
      if (spotifyConnection.getUserCode(server.arg("code"))) {
        server.send(200, "text/html", "Spotify setup complete Auth refresh in :" + String(spotifyConnection.tokenExpireTime));
      } else {
        char page[500];
        sprintf(page, errorPage, CLIENT_ID, REDIRECT_URI);
        server.send(200, "text/html", String(page));
      }
    }
  } else {
    Serial.println("Spotify setup complete");
    server.send(200, "text/html", "Spotify setup complete");
  }
}

void handleButtonPress(int buttonIndex) {
  bool test = false;
  switch (buttonIndex) {
    case 0:  // D0
      // spotifyConnection.shuffleSong(spotifyConnection.currentSong.shuffleState);
      break;
    case 1:  // D6
      spotifyConnection.skipBack();
      break;
    case 2:  //TX
      spotifyConnection.skipForward();
      break;
    case 3:  //RX
      spotifyConnection.togglePlay();
      break;
    default:
      break;
  }
}

long timeLoop;
long refreshLoop;
bool serverOn = true;
int curVol = 50;  // Variable to store the counter value
int lastStateCLK;
unsigned long lastDebounceTime = 0;
unsigned long lastChangeTime = 0;         // Time of the last counter change
const unsigned long displayDelay = 1000;  // Delay to display the counter (1 second)
int lastCounter = 0;                      // Variable to track if the counter has changed

void setup() {
  // Set encoder pins as input
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);

  Serial.begin(38400);

  // Read the initial state of the CLK pin
  lastStateCLK = digitalRead(CLK_PIN);

  // Initialize SPIFFS with retries
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialization failed! Retrying...");
    int retries = 0;
    while (!SPIFFS.begin() && retries < 3) {
      retries++;
      delay(1000);
    }
    if (retries == 3) {
      Serial.println("SPIFFS failed after 3 retries. Restarting...");
      ESP.restart();  // Restart if SPIFFS fails
    }
  }

  Serial.println("\r\nInitialization done.");

  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  WiFi.begin(WIFI_SSID, PASSWORD);
  Serial.println("Connecting to WiFi...");

  // Attempt Wi-Fi connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_ATTEMPTS) {
    delay(1000);
    attempts++;
    Serial.println("Attempting Wi-Fi connection...");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to Wi-Fi");
    ESP.restart();
  } else {
    Serial.println("Connected to Wi-Fi. IP: " + WiFi.localIP().toString());
    tft.println(WiFi.localIP());
  }

  delay(100);

  server.on("/", handleRoot);
  server.on("/callback", handleCallbackPage);
  server.begin();
  Serial.println("HTTP server started");

  // Initialize buttons
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Allocate memory for parts array
  for (int i = 0; i < 10; i++) {
    parts[i] = (char *)malloc(sizeof(char) * 20);
  }
}

void loop() {
  if (spotifyConnection.accessTokenSet) {
    if (serverOn) {
      server.close();
      serverOn = false;
    }

    // Refresh token if expired
    if ((millis() - spotifyConnection.tokenStartTime) / 1000 > spotifyConnection.tokenExpireTime) {
      Serial.println("Refreshing token");
      if (spotifyConnection.refreshAuth()) {
        Serial.println("Refreshed token");
      }
    }

    // Refresh track info and screen every 5 seconds
    if ((millis() - refreshLoop) > 5000) {
      spotifyConnection.getTrackInfo();
      spotifyConnection.drawScreen();  // Update display
      refreshLoop = millis();

      // DEBUG: Indicate that screen is being updated
      Serial.println("Screen updated with track info.");
    }

    // Handle button presses with debouncing
    for (int i = 0; i < 4; i++) {
      int reading = digitalRead(buttonPins[i]);
      if (reading != buttonStates[i]) {
        if (millis() - debounceTimes[i] > debounceDelay) {
          buttonStates[i] = reading;
          if (reading == LOW) {
            handleButtonPress(i);  // Handle button action
          }
          debounceTimes[i] = millis();
        }
      }
    }
    // Read the current state of the CLK pin
    int currentStateCLK = digitalRead(CLK_PIN);

    // If the state of CLK has changed, a rotation occurred
    if (currentStateCLK != lastStateCLK) {
      // Check if enough time has passed to debounce
      if ((millis() - lastDebounceTime) > debounceDelay) {
        // Determine the direction based on DT pin
        if (digitalRead(DT_PIN) != currentStateCLK) {
          // Clockwise rotation
          curVol = curVol + 5;
        } else {
          // Counter-clockwise rotation
          curVol = curVol - 5;
        }

        // Ensure the counter stays within 0 to 100
        curVol = max(0, min(curVol, 100));

        // Print the updated counter value
        Serial.println(curVol);

        // Update the debounce time
        lastDebounceTime = millis();
        lastChangeTime = millis();  // Reset the last change time
      }
    }

    // Save the current state as the last state for the next loop iteration
    lastStateCLK = currentStateCLK;

    // Check if 1 second has passed since the last change
    if ((millis() - lastChangeTime) > displayDelay) {
      // No change for the last second, print the current Volume value
      if (curVol != lastCounter) {
        spotifyConnection.adjustVolume(curVol);
        lastCounter = curVol;
      }
      // Reset the last change time to avoid repetitive printing
      lastChangeTime = millis();
    }

    timeLoop = millis();  // Update the loop time
  } else {
    server.handleClient();
  }
}
