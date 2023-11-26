#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "certs.h"
#include "creds.h"

#ifndef STASSID
#define STASSID "examplessid"
#define STAPSK "examplepassword"
#endif

#ifndef TOGGLUSERNAME
#define TOGGLUSERNAME "exampleusername"
#define TOGGLPASSWORD "examplepassword"
#endif

#ifndef TOGGLWORKSPACEID
#define TOGGLWORKSPACEID 1337
#define TOGGLUSERID 111
#define TOGGLPROJECT1ID 123
#define TOGGLPROJECT2ID 124
#define TOGGLPAYMENTRATE 100.0
#endif

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <ArduinoJson.h>

#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 60 * 60 * 3, 5 * 60 * 1000);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    continue;
  // Serial.setDebugOutput(true);
  Serial.println();

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();

  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  timeClient.begin();
}

void loop()
{
  if ((WiFi.status() == WL_CONNECTED))
  {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setFingerprint(fingerprint_api_track_toggl_com);

    HTTPClient https;
    String url = "/reports/api/v3/workspace/";
    url.concat(TOGGLWORKSPACEID);
    url.concat("/projects/summary");
    if (https.begin(*client, "api.track.toggl.com", 443, url, true))
    {
      https.setAuthorization(TOGGLUSERNAME, TOGGLPASSWORD);
      https.addHeader("Content-Type", "application/json");

      int httpCode = https.POST("{\"end_date\":\"2023-12-05\",\"start_date\":\"2023-11-17\"}");
      Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

      // httpCode will be negative on error
      if (httpCode > 0)
      {
        if (httpCode == HTTP_CODE_OK)
        {
          WiFiClient &stream = https.getStream();

          StaticJsonDocument<256> doc;
          DeserializationError error = deserializeJson(doc, stream);

          if (error)
          {
            Serial.print(F("[JSON] deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
          }

          display.clearDisplay();
          display.setTextSize(2);              // Normal 1:1 pixel scale
          display.setTextColor(SSD1306_WHITE); // Draw white text
          display.setCursor(0, 0);             // Start at top-left corner
          long total = 0;
          for (JsonObject item : doc.as<JsonArray>())
          {
            long user_id = item["user_id"];
            if (user_id == TOGGLUSERID)
            {
              long tracked_seconds = item["tracked_seconds"];
              total += tracked_seconds;
            }
          }
          display.print(F("T:"));
          display.println(total / 60.0 / 60.0 * TOGGLPAYMENTRATE, 0);
          for (JsonObject item : doc.as<JsonArray>())
          {
            long user_id = item["user_id"];
            if (user_id == TOGGLUSERID)
            {
              long project_id = item["project_id"];
              long tracked_seconds = item["tracked_seconds"];
              Serial.printf("[JSON] Tracked Seconds: (%ld) %ld\n", project_id, tracked_seconds);

              if (project_id == TOGGLPROJECT1ID)
              {
                display.print(F("V:"));
              }
              else if (project_id == TOGGLPROJECT2ID)
              {
                display.print(F("M:"));
              }
              display.println(tracked_seconds / 60.0 / 60.0 * TOGGLPAYMENTRATE, 0);
            }
          }
          timeClient.update();
          int day = timeClient.getDay();
          if (day == 0)
          {
            day = 7;
          }
          display.print(day);
          display.print(F(" "));
          display.println(timeClient.getFormattedTime());
          display.display();
        }
      }
      else
      {
        Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
    }
    else
    {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
    delay(5 * 60 * 1000);
  }
  else
  {
    Serial.println("[WIFI] Wi-Fi not connected, waiting 5 seconds!");
    delay(5 * 1000);
  }
}
