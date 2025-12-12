#include "WiFiHandler.h"

WiFiHandler::WiFiHandler() {
    Preferences preferences;
    preferences.begin("my-app", false);
    #if RESET_PREFERENCES == true
        preferences.clear();
        DEBUG_PRINTLN("[Preferences] NVS cleared");
    #endif
    ssid      = preferences.getString("ssid", STA_SSID);
    password  = preferences.getString("pass", STA_PASS);
    volumioIP = preferences.getString("ip", VOLUMIO_IP);
    preferences.end();

    StartSTA();

    // Initialize DNS server (will be started only in AP mode)
    dns = new DNSServer();

    // Start mDNS
    if(!MDNS.begin(DNS_NAME)) {
        DEBUG_PRINTLN("[mDNS] Failed to start");
    } else {
        DEBUG_PRINTLN("[mDNS] Started: " << DNS_NAME);
    }

    // Initialize web server
    server = new AsyncWebServer(80);

    // Setup OTA
    #if DISABLE_OTA == true
        DEBUG_PRINTLN("[OTA] DISABLED");
    #elif defined(OTA_SSID) && defined(OTA_PASS)
        ElegantOTA.begin(server, OTA_SSID, OTA_PASS);
    #else
        ElegantOTA.begin(server);
    #endif

    ElegantOTA.onStart(
        [this]() {
            NotificationManager::getInstance().postNotification( "Updating", "Starting...", 0 );
        }
    );
    ElegantOTA.onProgress(
        [this](size_t current, size_t final) {
            std::string progress = std::to_string(current/1000) + " / " + std::to_string(final/1000) + " KB";
            NotificationManager::getInstance().postNotification( "Updating", progress.c_str(), 0 );
        }
    );
    ElegantOTA.onEnd(
        [this](bool success) {
            NotificationManager::getInstance().postNotification( "Updating", success ? "Finished" : "Failed", 5000 );
        }
    );

    // Setup web server callbacks
    IPAddress localIP = connected ? WiFi.localIP() : WiFi.softAPIP(); // Default AP IP if not connected
    webServerCallbacks(*server, localIP);

    // Start web server
    server->begin();
    DEBUG_PRINTLN("[WebServer] Started on port 80");

    // Initialize Volumio
    std::string ipStr = std::string(volumioIP.c_str());
    volumio = new Volumio(ipStr);
}

WiFiHandler::~WiFiHandler() {
    if (volumio) {
        delete volumio;
        volumio = nullptr;
    }
    if (server) {
        server->end();
        delete server;
        server = nullptr;
    }
    if (dns) {
        dns->stop();
        delete dns;
        dns = nullptr;
    }
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

void WiFiHandler::RunTask(void){
    xTaskCreatePinnedToCore(TaskEntry,      // Task entry point
                            "WiFiTask",     // Task name
                            4096,           // Stack depth
                            this,           // Task parameters - pointer to this instance
                            2,              // Task priority
                            NULL,           // Task handle
                            1);             // Core ID (Core 0 - Fast Core)
}

void WiFiHandler::TaskEntry(void* param) {
    WiFiHandler* instance = static_cast<WiFiHandler*>(param);

    while (true) {
        instance->Update();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    vTaskDelete(NULL);
}

void WiFiHandler::StartSTA(TickType_t timeout) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    connected = false;

    TickType_t startTime = xTaskGetTickCount();
    const TickType_t timeoutTicks = (timeout == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout);

    while (WiFi.status() != WL_CONNECTED && (xTaskGetTickCount() - startTime) < timeout) {
        vTaskDelay(500);
    }

    if (WiFi.status() != WL_CONNECTED){
        DEBUG_PRINTLN("[WiFi] Failed to connect to STA, timeout reached");
        return;
    }

    connected = true;
    DEBUG_PRINTLN("[WiFi] Connected to STA: " << ssid.c_str() << " (" << WiFi.localIP().toString().c_str() << ")");
    std::string notifyContent = std::string(ssid.c_str()) + "\n" + WiFi.localIP().toString().c_str();
    NotificationManager::getInstance().postNotification(
        "Connected",
        notifyContent,
        5000
    );
}

void WiFiHandler::StartAP(void) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    vTaskDelay(500); // Give AP time to start

    IPAddress apIP = WiFi.softAPIP();
    DEBUG_PRINTLN("[WiFi] AP Started: " << AP_SSID);
    DEBUG_PRINTLN("[WiFi] AP IP Address: " << apIP.toString());

    // Start DNS server for captive portal
    if (dns) {
        dns->start(53, "*", apIP);
        DEBUG_PRINTLN("[DNS] Started for captive portal");
    }

    // Update web server callbacks with AP IP
    webServerCallbacks(*server, apIP);
    connected = true;

    // Post notification: "Configuration, MDNS\nAddressIP" (no timeout)
    std::string apContent = std::string(DNS_NAME) + "\n" + apIP.toString().c_str();
    NotificationManager::getInstance().postNotification(
        "Configuration",
        apContent,
        0
    );
}

void WiFiHandler::Update() {
    // Process DNS server requests (needed for captive portal in AP mode)
    if (dns && (mode == WIFI_AP || mode == WIFI_AP_STA)) {
        dns->processNextRequest();
    }

    // Update connection status in STA mode
    if (mode == WIFI_STA) {
        bool wasConnected = connected;
        connected = (WiFi.status() == WL_CONNECTED);

        // Popup notification on network state changes
        if (wasConnected != connected) {
            if (connected) {
                DEBUG_PRINTLN("[WiFi] Reconnected to STA");
                DEBUG_PRINTLN("[WiFi] IP Address: " << WiFi.localIP().toString());
                std::string notifyContent = std::string(ssid.c_str()) + "\n" + WiFi.localIP().toString().c_str();
                NotificationManager::getInstance().postNotification(
                    "Connected",
                    notifyContent,
                    5000
                );
                lastAttempt = 0;
            } else {
                DEBUG_PRINTLN("[WiFi] Disconnected from STA");
                NotificationManager::getInstance().postNotification(
                    "Disconnected",
                    "Connection lost",
                    5000
                );
                lastAttempt = xTaskGetTickCount();
            }
        }

        // Auto-reconnect if disconnected (retry every 5 seconds)
        if (!connected) {
            TickType_t now = xTaskGetTickCount();
            if (lastAttempt == 0 || (now - lastAttempt) >= RECONNECT_INTERVAL) {
                DEBUG_PRINTLN("[WiFi] Attempting to reconnect...");
                StartSTA(30000);  // 30 second timeout
                lastAttempt = now;
            }
        }
    }

    // Update Volumio if connected and initialized
    if (volumio != nullptr) {
        volumio->Update();

        Info trackData;
        volumio->ParseResponse(&trackData);
        TrackDataQueue::getInstance().postTrackData(trackData);

        processVolumioCommands();
    }
}

void WiFiHandler::processVolumioCommands(void) {
    VolumioCommand cmd;
    while (CommandQueue::getInstance().getNextCommand(cmd)) {
        std::string commandStr;

        // Convert command type to Volumio HTTP API string
        switch (cmd.type) {
            case VolumioCommandType::PLAY:
                commandStr = VOLUMIO_CMD_PLAY;
                break;
            case VolumioCommandType::PAUSE:
                commandStr = VOLUMIO_CMD_PAUSE;
                break;
            case VolumioCommandType::TOGGLE:
                commandStr = VOLUMIO_CMD_TOGGLE;
                break;
            case VolumioCommandType::NEXT:
                commandStr = VOLUMIO_CMD_NEXT;
                break;
            case VolumioCommandType::PREV:
                commandStr = VOLUMIO_CMD_PREV;
                break;
            case VolumioCommandType::SEEK:
                commandStr = VOLUMIO_CMD_SEEK(cmd.value);
                break;
            case VolumioCommandType::RANDOM:
                commandStr = VOLUMIO_CMD_RANDOM;
                break;
            case VolumioCommandType::REPEAT:
                commandStr = VOLUMIO_CMD_REPEAT;
                break;
        }
        volumio->SendCommand(commandStr);
    }
}

void WiFiHandler::ToggleMode() {
    // Stop DNS server if running
    if (dns && (mode == WIFI_AP || mode == WIFI_AP_STA)) {
        dns->stop();
        DEBUG_PRINTLN("[DNS] Stopped");
    }

    // Disconnect current connection
    WiFi.disconnect();
    vTaskDelay(100);

    // Set new mode
    mode = (mode == WIFI_STA) ? WIFI_AP : WIFI_STA;
    if (mode == WIFI_STA) {
        StartSTA();
    } else {
        StartAP();
    }
}