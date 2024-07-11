#include "Arduino.h"
#include "SettingsServer.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

void SettingsServer::start() {
  // Changes Wifi mode and creates AP
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.softAP("Chave Digital", "");
  
  // Sets server route handlers
  createServerHandlers();

  // Begins server
  server.begin();

  // While not connected
  while ((WiFi.status() != WL_CONNECTED))
  {
    // Waits
    delay(100);
    
    // Handles requests
    server.handleClient();
  }
}

// Server Handlers
void SettingsServer::createServerHandlers() {
  // Index
  server.on("/", [this]()
  {
    // Variables
    String indexAP = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Chave Digital</title><style type=\"text/css\"> .cont { position: absolute; left: 50%; top: 50%; transform: translate(-50%, -50%); -ms-transform: translate(-50%, -50%); -webkit-transform: translate(-50%, -50%); text-align: center; } .lbl { color: #87857e; } </style></head><body><div class=\"cont\"><h1>Chave Digital</h1><h3 class=\"lbl\">Configure seu dispositivo pelo aplicativo Chave Digital</h3></div></body></html>";

    // Index page
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", indexAP); 
  });

  // Networks
  server.on("/networks", [this]()
  { 
    // Variables
    int pot;
    String st, stJ, potF;

    // Scans near networks
    int n = WiFi.scanNetworks();
    if(n > 0){
      stJ = "{ \"networks\": [";
      for (int i = 0; i < n; ++i)
      {
        // Network Name
        stJ += "{ \"name\": \"";
        stJ += WiFi.SSID(i);
        stJ += "\", ";
    
        pot = WiFi.RSSI(i);
        if(pot <= -77) {
          potF = "1";
        }
        if(pot > -77 and pot <= -67) {
          potF = "2";
        }
        if(pot > -67 and pot <= -60) {
          potF = "3"; 
        }
        if(pot > -60) {
          potF = "4"; 
        }

        // Network Strength
        stJ += "\"strength\": \"";
        stJ += potF;
        stJ += "\", ";

        // Network Security
        stJ += "\"secure\": \"";
        stJ += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "0" : "1";
        if(i != (n-1)) {
          stJ += "\" },";
        } else {
          stJ += "\" }";
        }
      }
      stJ += "] }";
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "application/json", stJ);
    }
    else {
      // No Networks found
      stJ = "{ \"networks\": [] }";
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "application/json", stJ);
    } 
  });

  // Setting
  server.on("/settings", [this]()
  {
    // Get ssid, pass and adminUid from request parameters
    String qsid = server.arg("ssid");
    String qpass = server.arg("pass");
    String qadmin = server.arg("admin");

    // If data is valid
    if (qsid.length() > 0 && qpass.length() > 0 && qadmin.length() > 0) {
      // Cleans EEPROM
      for (int i = 0; i < 512; ++i) {
        EEPROM.write(i, 0);
      }

      // Writes ssid
      for (int i = 0; i < qsid.length(); ++i)
      {
        EEPROM.write(i, qsid[i]);
      }

      // Writes password
      for (int i = 0; i < qpass.length(); ++i)
      {
        EEPROM.write(32 + i, qpass[i]);
      }

      // Writes adminUid
      for (int i = 0; i < qadmin.length(); ++i)
      {
        EEPROM.write(100 + i, qadmin[i]);
      }

      // Writes config state
      EEPROM.write(200, 1);
      EEPROM.commit();

      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", serial);
      delay(2000);
      ESP.restart();
    } 
    // If data isn't valid
    else {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(404, "text/plain", "Error");
    } 
  });
}
