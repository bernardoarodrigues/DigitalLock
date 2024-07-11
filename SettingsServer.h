#ifndef SettingsServer_h
#define SettingsServer_h

#include "Arduino.h"
#include <ESP8266WebServer.h>

class SettingsServer {
  public:
    void start();
    
  private:
    ESP8266WebServer server;
    String serial;
    void createServerHandlers();

  SettingsServer(String serial) {
    ESP8266WebServer server(80);
    this->serial = serial;
  }
};

#endif
