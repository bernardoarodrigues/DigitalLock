// Author: Bernardo Alexandre Alves Rodrigues - Control

// *Libraries*

// ESP8266
#include <ESP8266WiFi.h>
#include <EEPROM.h>

// NTP
#include <NTPClient.h>
#include <WiFiUdp.h>

// Tasks
#include <TaskScheduler.h>

// Firebase
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// SettingsServer
#include "SettingsServer.h"

// *Variables*

// NTP
const long utcOffsetInSeconds = -10800; // GMT -3
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Scheduler & Tasks
Scheduler runner;
void sendTime();
Task SendTime(10000, TASK_FOREVER, &sendTime);

String serial = "000000000001";

// Various
int timeCounter = 0;
String adminUid = "";
bool conToDB = false;

// GPIOs
int relay = 3;

// Firebase
#define API_KEY "FIREBASE_API_KEY"
#define USER_PASSWORD "FIREBASE_USER_PASSWORD"
#define DATABASE_URL "FIREBASE_DATABASE_URL"
String childPath[2] = {"/state", "/reset"};
FirebaseData fbdo;
FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;

// Root CA Certificate
const char rootCACert[] PROGMEM = "-----BEGIN CERTIFICATE-----\n"
                                  "MIIFVzCCAz+gAwIBAgINAgPlk28xsBNJiGuiFzANBgkqhkiG9w0BAQwFADBHMQsw\n"
                                  "CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n"
                                  "MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAw\n"
                                  "MDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n"
                                  "Y2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEBAQUA\n"
                                  "A4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaMf/vo\n"
                                  "27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vXmX7w\n"
                                  "Cl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSMHobTSPS5g4M/SCYe7zUjw\n"
                                  "TcLCeoiKu7rPWRnWr4+wB7CeMfGCwcDfLqZtbBkOtdh+JhpFAz2weaSUKK0Pfybl\n"
                                  "qAj+lug8aJRT7oM6iCsVlgmy4HqMLnXWnOunVmSPlk9orj2XwoSPwLxAwAtcvfaH\n"
                                  "szVsrBhQf4TgTM2S0yDpM7xSma8ytSmzJSq0SPly4cpk9+aCEI3oncKKiPo4Zor8\n"
                                  "Y/kB+Xj9e1x3+naH+uzfsQ55lVe0vSbv1gHR6xYKu44LtcXFilWr06zqkUspzBmk\n"
                                  "MiVOKvFlRNACzqrOSbTqn3yDsEB750Orp2yjj32JgfpMpf/VjsPOS+C12LOORc92\n"
                                  "wO1AK/1TD7Cn1TsNsYqiA94xrcx36m97PtbfkSIS5r762DL8EGMUUXLeXdYWk70p\n"
                                  "aDPvOmbsB4om3xPXV2V4J95eSRQAogB/mqghtqmxlbCluQ0WEdrHbEg8QOB+DVrN\n"
                                  "VjzRlwW5y0vtOUucxD/SVRNuJLDWcfr0wbrM7Rv1/oFB2ACYPTrIrnqYNxgFlQID\n"
                                  "AQABo0IwQDAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4E\n"
                                  "FgQU5K8rJnEaK0gnhS9SZizv8IkTcT4wDQYJKoZIhvcNAQEMBQADggIBAJ+qQibb\n"
                                  "C5u+/x6Wki4+omVKapi6Ist9wTrYggoGxval3sBOh2Z5ofmmWJyq+bXmYOfg6LEe\n"
                                  "QkEzCzc9zolwFcq1JKjPa7XSQCGYzyI0zzvFIoTgxQ6KfF2I5DUkzps+GlQebtuy\n"
                                  "h6f88/qBVRRiClmpIgUxPoLW7ttXNLwzldMXG+gnoot7TiYaelpkttGsN/H9oPM4\n"
                                  "7HLwEXWdyzRSjeZ2axfG34arJ45JK3VmgRAhpuo+9K4l/3wV3s6MJT/KYnAK9y8J\n"
                                  "ZgfIPxz88NtFMN9iiMG1D53Dn0reWVlHxYciNuaCp+0KueIHoI17eko8cdLiA6Ef\n"
                                  "MgfdG+RCzgwARWGAtQsgWSl4vflVy2PFPEz0tv/bal8xa5meLMFrUKTX5hgUvYU/\n"
                                  "Z6tGn6D/Qqc6f1zLXbBwHSs09dR2CQzreExZBfMzQsNhFRAbd03OIozUhfJFfbdT\n"
                                  "6u9AWpQKXCBfTkBdYiJ23//OYb2MI3jSNwLgjt7RETeJ9r/tSQdirpLsQBqvFAnZ\n"
                                  "0E6yove+7u7Y/9waLd64NnHi/Hm3lCXRSHNboTXns5lndcEZOitHTtNCjv0xyBZm\n"
                                  "2tIMPNuzjsmhDYAPexZ3FL//2wmUspO8IFgV6dtxQ/PeEMMA3KgqlbbC1j+Qa3bb\n"
                                  "bP6MvPJwNQzcmRk13NfIRmPVNnGuV/u3gm3c\n"
                                  "-----END CERTIFICATE-----\n";

// *Functions*

// Checks Wifi connection
bool testWifi(void)
{
  int c = 0;
  while (c < 20)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    c++;
  }
  return false;
}

// Reset
void handle_reset(void)
{
  // Clean Wifi credentials from EEPROM
  EEPROM.begin(512);
  for (int i = 0; i < 512; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();

  // Sets configured as no
  if (Firebase.ready())
    Firebase.RTDB.setString(&fbdo, ("/devices/" + serial + "/config").c_str(), F("no"));

  // Restarts
  delay(1000);
  ESP.reset();
}

// Firebase Callback
void streamCallback(MultiPathStream stream)
{
  size_t numChild = sizeof(childPath) / sizeof(childPath[0]);

  for (size_t i = 0; i < numChild; i++)
  {
    if (stream.get(childPath[i]))
    {
      String node = stream.dataPath.c_str();
      String value = stream.value.c_str();

      // If state changes (on/off)
      if (node == "/state")
      {
        // If 1, activates the relay
        if (value == "1")
        {
          digitalWrite(relay, LOW);
          delay(1000);
          digitalWrite(relay, HIGH);

          // Sets back to 0
          if (Firebase.ready())
            Firebase.RTDB.setString(&fbdo, ("/devices/" + serial + "/state").c_str(), F("0"));
        }
      }
      // If reset changes (yes/no)
      else if (node == "/reset")
      {
        // If yes, cleans EEPROM (ssid/pass) and restarts
        if (value == "yes")
        {
          // Sets back to no
          if (Firebase.ready())
            Firebase.RTDB.setString(&fbdo, ("/devices/" + serial + "/reset").c_str(), F("no"));

          handle_reset();
        }
      }
    }
  }
}

// Lost Firebase Connection Callback
void streamTimeoutCallback(bool timeout)
{
}

// Sends lasttime to Firebase
void sendTime()
{
  // Gets EpochTime with NTP
  unsigned long secs = timeClient.getEpochTime();

  if (Firebase.ready()) {
    // Sends time to Firebase
    Firebase.RTDB.setString(&fbdo, ("/devices/" + serial + "/lasttime").c_str(), String(secs));

    // If 10 mins passed
    if (timeCounter >= 60)
    {
      // Resets counter
      timeCounter = 0;
  
      // Restarts multipath stream
      Firebase.RTDB.removeMultiPathStreamCallback(&stream);
      Firebase.RTDB.beginMultiPathStream(&stream, "/devices/" + serial);
      Firebase.RTDB.setMultiPathStreamCallback(&stream, streamCallback, streamTimeoutCallback);
    }
    else
    {
      // Increases counter
      timeCounter++;
    }
  }
}

// Updates Firebase data and starts stream
void startDB()
{
  if (Firebase.ready())
  {
    // Adds time task to Scheduler
    runner.addTask(SendTime);

    // Enables the task
    SendTime.enable();

    // Sets device admin
    Firebase.RTDB.setString(&fbdo, ("/devices/" + serial + "/admin").c_str(), adminUid.c_str());
    // Confirms device is configured to Wifi
    Firebase.RTDB.setString(&fbdo, ("/devices/" + serial + "/config").c_str(), F("yes"));
    // Sets relay state to off
    Firebase.RTDB.setString(&fbdo, ("/devices/" + serial + "/state").c_str(), F("0"));

    // Changes database connection state
    conToDB = true;

    // Starts Stream
    Firebase.RTDB.beginMultiPathStream(&stream, ("/devices/" + serial).c_str());

    // Sets stream callback
    Firebase.RTDB.setMultiPathStreamCallback(&stream, streamCallback, streamTimeoutCallback);
  }
}

// *Setup*
void setup()
{
  // Begins Serial
  Serial.begin(9600);

  // Relay setup
  pinMode(relay, FUNCTION_3);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);

  // Disconnects Wifi and starts EEPROM
  WiFi.disconnect();
  EEPROM.begin(512);
  delay(10);

  // Checks if any data in EEPROM
  boolean hasData = false;
  for (int i = 0; i < 150; ++i)
  {
    if(char(EEPROM.read(i)) != 0) {
      hasData = true;
      break;
    }
  }

  // If any credential saved in the memory
  if(hasData) {
    // Reads EEPROM

    // SSID
    String esid;
    for (int i = 0; i < 32; ++i)
    {
      esid += char(EEPROM.read(i));
    }
    // Password
    String epass;
    for (int i = 32; i < 64; ++i)
    {
      epass += char(EEPROM.read(i));
    }
    // AdminUid
    for (int i = 100; i < 132; ++i)
    {
      adminUid += char(EEPROM.read(i));
    }
    
    boolean ok = false;

    // While not connected
    while(!ok) {
      // Tries to connect to Wifi
      WiFi.begin(esid.c_str(), epass.c_str());
    
      // If connection was successful
      if (testWifi())
      {
        // Resets config state
        EEPROM.write(200, 0);
        EEPROM.commit();
      
        // Starts NTP connection
        timeClient.begin();
    
        // Firebase Configuration
        config.api_key = API_KEY;
        auth.user.email = serial + "@gmail.com";
        auth.user.password = USER_PASSWORD;
        config.database_url = DATABASE_URL;
        config.cert.data = rootCACert;
        config.token_status_callback = tokenStatusCallback;
        config.max_token_generation_retry = 5;
        config.signer.preRefreshSeconds = 5 * 60;
        fbdo.setCert(rootCACert);
        fbdo.setResponseSize(100000);
    
        // Begins Firebase
        Firebase.begin(&config, &auth);
    
        // Starts DB connection
        startDB();

        ok = true;
      }
      // If couldn't connect, waits 3s to try again
      else {
        // If in config, and coutldn't connect - wrong password
        if(char(EEPROM.read(200)) == 1)
          break;
        // Else - wifi network not working
        else 
          delay(3000);
      }
    }

    // Resets config state, and restarts configuring wifi
    if(char(EEPROM.read(200)) == 1) {
      EEPROM.write(200, 0);
      EEPROM.commit();
    }
    // Finishes setup and proceeds
    else {
      return;
    }
  }
  
  // If no credentials saved, starts server
  SettingsServer server = SettingsServer(serial);
  server.start();
}

// *Loop*
void loop()
{
  // If connected to Wifi
  if (WiFi.status() == WL_CONNECTED)
  {
    // If couldn't connect to DB, tries again
    if (conToDB == false)
    {
      startDB();
    }
    // If already connected
    else
    {
      // Firebase.ready() should be called repeatedly to handle authentication tasks.
      Firebase.ready();

      // Executes tasks
      runner.execute();
    }

    // Updates NTP
    timeClient.update();
  }
}
