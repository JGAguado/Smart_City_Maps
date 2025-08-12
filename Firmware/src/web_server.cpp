#include "web_server.h"
#include "serial_config.h"  // Must be included first
#include "config.h"

WebConfigServer::WebConfigServer(ConfigManager* configMgr) : 
    server(WEB_SERVER_PORT), configManager(configMgr), serverStarted(false) {
}

bool WebConfigServer::startConfigAP() {
    Serial.println("Starting configuration Access Point...");
    
    // Start WiFi in AP mode
    WiFi.mode(WIFI_AP);
    
    if (!WiFi.softAP(AP_SSID, AP_PASSWORD)) {
        Serial.println("Failed to start Access Point");
        return false;
    }
    
    IPAddress IP = WiFi.softAPIP();
    Serial.printf("Access Point started: %s\n", AP_SSID);
    Serial.printf("IP address: %s\n", IP.toString().c_str());
    Serial.printf("Password: %s\n", AP_PASSWORD);
    
    // Start DNS server for captive portal
    dnsServer.start(DNS_PORT, "*", IP);
    Serial.println("DNS server started for captive portal");
    
    // Set up web server routes
    server.on("/", [this]() { handleRoot(); });
    server.on("/config", HTTP_POST, [this]() { handleConfig(); });
    server.on("/status", [this]() { handleStatus(); });
    server.onNotFound([this]() { handleRoot(); }); // Redirect all unknown requests to root
    
    // Start the server
    server.begin();
    serverStarted = true;
    
    Serial.println("Web configuration server started on port 80");
    Serial.println("Captive portal active - all requests redirect to configuration page");
    Serial.println("Open http://192.168.4.1 in your browser to configure");
    
    return true;
}

void WebConfigServer::stopServer() {
    if (serverStarted) {
        server.stop();
        dnsServer.stop();
        WiFi.softAPdisconnect(true);
        serverStarted = false;
        Serial.println("Configuration server and DNS stopped");
    }
}

void WebConfigServer::handleClient() {
    if (serverStarted) {
        dnsServer.processNextRequest();
        server.handleClient();
    }
}

void WebConfigServer::handleDNS() {
    if (serverStarted) {
        dnsServer.processNextRequest();
    }
}

void WebConfigServer::handleRoot() {
    // Send captive portal headers
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send(200, "text/html", getIndexPage());
}

void WebConfigServer::handleConfig() {
    Serial.println("Received configuration request");
    
    if (!server.hasArg("wifi_ssid") || !server.hasArg("github_repo") || 
        !server.hasArg("github_path")) {
        server.send(400, "text/plain", "Missing required parameters");
        return;
    }
    
    String wifiSSID = server.arg("wifi_ssid");
    String wifiPassword = server.arg("wifi_password");
    String githubRepo = server.arg("github_repo");
    String githubPath = server.arg("github_path");
    
    Serial.printf("Configuring: SSID=%s, Repo=%s, Path=%s\n", 
                  wifiSSID.c_str(), githubRepo.c_str(), githubPath.c_str());
    
    // Validate and save configuration
    if (configManager->setWiFiCredentials(wifiSSID.c_str(), wifiPassword.c_str()) &&
        configManager->setGitHubInfo(githubRepo.c_str(), githubPath.c_str())) {
        
        if (configManager->saveConfig()) {
            server.send(200, "text/html", getSuccessPage());
            Serial.println("Configuration saved successfully");
            
            // Schedule restart after a short delay
            delay(2000);
            ESP.restart();
        } else {
            server.send(500, "text/plain", "Failed to save configuration");
        }
    } else {
        server.send(400, "text/plain", "Invalid configuration parameters");
    }
}

void WebConfigServer::handleStatus() {
    JsonDocument doc;
    doc["configured"] = configManager->isConfigured();
    doc["wifi_ssid"] = configManager->getWiFiSSID();
    doc["github_repo"] = configManager->getGitHubRepo();
    doc["github_path"] = configManager->getGitHubImagePath();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["uptime"] = millis();
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

String WebConfigServer::getIndexPage() {
    return R"HTML(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Dashboard Configuration</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }
        .container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        h1 { color: #333; text-align: center; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        input[type="text"], input[type="password"] { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
        button { background: #007cba; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; width: 100%; }
        button:hover { background: #005a87; }
        .info { background: #e7f3ff; padding: 10px; border-radius: 4px; margin-top: 20px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Smart Dashboard Setup</h1>
        <form action="/config" method="POST">
            <div class="form-group">
                <label for="wifi_ssid">WiFi Network:</label>
                <input type="text" id="wifi_ssid" name="wifi_ssid" required>
            </div>
            <div class="form-group">
                <label for="wifi_password">WiFi Password:</label>
                <input type="password" id="wifi_password" name="wifi_password">
            </div>
            <div class="form-group">
                <label for="github_repo">GitHub Repository:</label>
                <input type="text" id="github_repo" name="github_repo" required placeholder="owner/repository">
            </div>
            <div class="form-group">
                <label for="github_path">Image Path:</label>
                <input type="text" id="github_path" name="github_path" required placeholder="dashboard_480x800.png">
            </div>
            <button type="submit">Save Configuration</button>
        </form>
        <div class="info">
            <strong>Device:</strong> ESP32-S2 Smart Dashboard<br>
            <strong>Display:</strong> 7.3" E-Paper<br>
            <strong>Update:</strong> Every hour
        </div>
    </div>
</body>
</html>
)HTML";
}

String WebConfigServer::getSuccessPage() {
    return R"HTML(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Configuration Saved</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <h1>Configuration Saved Successfully!</h1>
        <div class="success-message">
            <p>Your Smart Dashboard has been configured and will restart shortly.</p>
            <p>The device will now connect to your WiFi network and start fetching images from GitHub.</p>
        </div>
        
        <div class="info-section">
            <h2>What happens next?</h2>
            <ul>
                <li>Device will restart and connect to your WiFi</li>
                <li>First image will be downloaded and displayed</li>
                <li>Images will update automatically every hour</li>
                <li>The configuration portal will no longer be available</li>
            </ul>
        </div>
        
        <p class="note">If the device doesn't connect to WiFi, it will return to configuration mode after a few minutes.</p>
    </div>
    
    <script>
        setTimeout(function() {
            document.body.innerHTML = '<div class="container"><h1>Device Restarting...</h1><p>Please wait while the device restarts and connects to your WiFi network.</p></div>';
        }, 3000);
    </script>
</body>
</html>
)HTML";
}
