#pragma once

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Preferences.h>
#include <Arduino.h>
#include <string>

/**
 *  _____ _____ _____ _____ _____
 * | __  |   __|   __|   __|_   _|
 * |    -|   __|__   |   __| | |
 * |__|__|_____|_____|_____| |_|
 *
 * FORCE RESET SPIFFS VALUES
 */
//#define RESET

class ConfigManager {
private:
    Preferences preferences;

    std::string ssid;
    std::string pass;
    std::string ip;

    // Private Constructor (Singleton)
    ConfigManager();

public:
    // Delete copy constructor and assignment operator (singleton)
    ConfigManager(const ConfigManager&) = delete;
    ~ConfigManager();
    ConfigManager& operator=(const ConfigManager&) = delete;

    // Get Singleton Instance
    static ConfigManager& getInstance();

    void begin(const std::string ssidRef, const std::string passRef, const std::string ipRef);
    void save(const std::string& newSsid, const std::string& newPass, const std::string& newIp);

    std::string getSsid() const;
    std::string getPass() const;
    std::string getIp() const;
};

#endif // CONFIG_MANAGER_H
