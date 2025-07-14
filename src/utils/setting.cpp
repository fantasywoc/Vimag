#include "setting.h"

// 全局设置管理器实例定义
SettingManager g_settings;

// SettingManager类实现
SettingManager::SettingManager(const std::string& filePath) 
    : configFilePath(filePath) {
    std::filesystem::path path(filePath);
    if (!fs::exists(path)) {
        std::ofstream newFile(path);
        newFile.close();
        initDefaultSettings();
    }
    loadSettings();
}

SettingManager::~SettingManager() {
    saveSettings();
}

bool SettingManager::loadSettings() {
    try {
        config.load(configFilePath);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load settings: " << e.what() << std::endl;
        return false;
    }
}

bool SettingManager::saveSettings() {
    try {
        config.save(configFilePath);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to save settings: " << e.what() << std::endl;
        return false;
    }
}

std::string SettingManager::getString(const std::string& section, const std::string& key, const std::string& defaultValue) {
    try {
        if (config.count(section) && config[section].count(key)) {
            return config[section][key].as<std::string>();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting string: " << e.what() << std::endl;
    }
    return defaultValue;
}

void SettingManager::setString(const std::string& section, const std::string& key, const std::string& value) {
    config[section][key] = value;
}

int SettingManager::getInt(const std::string& section, const std::string& key, int defaultValue) {
    try {
        if (config.count(section) && config[section].count(key)) {
            return config[section][key].as<int>();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting int: " << e.what() << std::endl;
    }
    return defaultValue;
}

void SettingManager::setInt(const std::string& section, const std::string& key, int value) {
    config[section][key] = value;
}

bool SettingManager::getBool(const std::string& section, const std::string& key, bool defaultValue) {
    try {
        if (config.count(section) && config[section].count(key)) {
            return config[section][key].as<bool>();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting bool: " << e.what() << std::endl;
    }
    return defaultValue;
}

void SettingManager::setBool(const std::string& section, const std::string& key, bool value) {
    config[section][key] = value;
}

void SettingManager::initDefaultSettings() {
    // Display节默认配置
    setBool("Display", "image_cycle", true);
    setBool("Display", "image_name_display", true);
    setBool("Display", "image_EXIF", true);
    setBool("Display", "image_index", true);
    setBool("Display", "Enable_Exif_orientation", true);
    
    saveSettings();
}

// 全局便捷函数实现
void loadSetting() {
    g_settings.loadSettings();
}

void saveSetting() {
    g_settings.saveSettings();
}

// 字符串读写
std::string getSetting(const std::string& section, const std::string& key, const std::string& defaultValue) {
    return g_settings.getString(section, key, defaultValue);
}

void setSetting(const std::string& section, const std::string& key, const std::string& value) {
    g_settings.setString(section, key, value);
}

// 整数读写
int getSettingInt(const std::string& section, const std::string& key, int defaultValue) {
    return g_settings.getInt(section, key, defaultValue);
}

void setSettingInt(const std::string& section, const std::string& key, int value) {
    g_settings.setInt(section, key, value);
}

// 布尔值读写
bool getSettingBool(const std::string& section, const std::string& key, bool defaultValue) {
    return g_settings.getBool(section, key, defaultValue);
}

void setSettingBool(const std::string& section, const std::string& key, bool value) {
    g_settings.setBool(section, key, value);
}

// 初始化默认设置
void initDefaultSetting() {
    g_settings.initDefaultSettings();
}