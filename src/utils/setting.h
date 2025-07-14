#ifndef SETTING_H
#define SETTING_H

#include "inicpp.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class SettingManager {
private:
    ini::IniFile config;
    std::string configFilePath;

public:
    // 构造函数
    SettingManager(const std::string& filePath = "config.ini");
    
    // 析构函数
    ~SettingManager();

    // 加载设置文件
    bool loadSettings();

    // 保存设置文件
    bool saveSettings();

    // 获取字符串值
    std::string getString(const std::string& section, const std::string& key, const std::string& defaultValue = "");

    // 设置字符串值
    void setString(const std::string& section, const std::string& key, const std::string& value);

    // 获取整数值
    int getInt(const std::string& section, const std::string& key, int defaultValue = 0);

    // 设置整数值
    void setInt(const std::string& section, const std::string& key, int value);

    // 获取布尔值
    bool getBool(const std::string& section, const std::string& key, bool defaultValue = false);

    // 设置布尔值
    void setBool(const std::string& section, const std::string& key, bool value);
    
    // 初始化默认设置
    void initDefaultSettings();
};

// 全局设置管理器实例声明
extern SettingManager g_settings;

// 便捷函数声明
void loadSetting();
void saveSetting();

// 字符串读写
std::string getSetting(const std::string& section, const std::string& key, const std::string& defaultValue = "");
void setSetting(const std::string& section, const std::string& key, const std::string& value);

// 整数读写
int getSettingInt(const std::string& section, const std::string& key, int defaultValue = 0);
void setSettingInt(const std::string& section, const std::string& key, int value);

// 布尔值读写
bool getSettingBool(const std::string& section, const std::string& key, bool defaultValue = true);
void setSettingBool(const std::string& section, const std::string& key, bool value);

// 初始化默认设置
void initDefaultSetting();

#endif // SETTING_H


