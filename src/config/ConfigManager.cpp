#include "ConfigManager.h"

// Get Singleton Instance
ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

// Private constructor
ConfigManager::ConfigManager() : ssid(""), pass(""), ip("") {}

ConfigManager::~ConfigManager() {}

void ConfigManager::begin(const std::string ssidRef, const std::string passRef, const std::string ipRef) {
    ssid = ssidRef;
    pass = passRef;
    ip   = ipRef;

    bool reset = false;
    #ifdef RESET
        reset = true;
    #endif

    preferences.begin("my-app", false);
    uint8_t firstBoot = preferences.getUChar("FIRST_BOOT", 0xFF);

    if (firstBoot != 0 || reset) {
        preferences.clear();
        preferences.putString("ssid", ssidRef.c_str());
        preferences.putString("pass", passRef.c_str());
        preferences.putString("ip",   ipRef.c_str());
        preferences.putUChar("FIRST_BOOT", 0x00);
    }

    String tempSsid = preferences.getString("ssid", String(ssidRef.c_str()));
    String tempPass = preferences.getString("pass", String(passRef.c_str()));
    String tempIp   = preferences.getString("ip",   String(ipRef.c_str()));

    ssid = std::string(tempSsid.c_str());
    pass = std::string(tempPass.c_str());
    ip   = std::string(tempIp.c_str());

    preferences.end();
}

void ConfigManager::save(const std::string& newSsid, const std::string& newPass, const std::string& newIp) {
    preferences.begin("my-app", false);
    preferences.putString("ssid", newSsid.c_str());
    preferences.putString("pass", newPass.c_str());
    preferences.putString("ip", newIp.c_str());
    preferences.end();

    ssid = newSsid;
    pass = newPass;
    ip   = newIp;
}

std::string ConfigManager::getSsid() const { return ssid; }
std::string ConfigManager::getPass() const { return pass; }
std::string ConfigManager::getIp()   const { return ip  ; }
