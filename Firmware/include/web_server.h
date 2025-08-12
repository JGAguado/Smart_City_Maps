#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "config_manager.h"

class WebConfigServer {
private:
    WebServer server;
    DNSServer dnsServer;
    ConfigManager* configManager;
    bool serverStarted;
    
    // HTML pages
    String getIndexPage();
    String getSuccessPage();
    
    // Request handlers
    void handleRoot();
    void handleConfig();
    void handleStatus();
    
public:
    WebConfigServer(ConfigManager* configMgr);
    bool startConfigAP();
    void stopServer();
    bool isServerStarted() const { return serverStarted; }
    void handleClient();
    void handleDNS();
};

#endif // WEB_SERVER_H
