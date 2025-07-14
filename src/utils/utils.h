#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <iostream> // \以包含 std::cout 和 std::cerr
#include <cstring> // 包含 std::strerror 函数的头文件
#include "../TinyEXIF/EXIF.h" 

#include <filesystem>
namespace fs = std::filesystem;


// 检查是否是支持的图像文件
static const std::set<std::string> imageExtensions = {
    ".jpg", ".jpeg", ".png", ".bmp", ".gif", ".tga","hdr",".psd"
};


bool getImageInfo(const std::string& filePath, int& w, int& h) ;


// 查找指定目录下的图片文件路径和文件名
void find_image_files( const fs::path& directory, std::vector<fs::path>& image_paths,std::vector<std::string>& image_names) ;

size_t findPathIndex(const std::vector<fs::path>& paths, const fs::path& target) ;
void removeZero(std::string& str);
//格式化曝光时间
std::string fomatExposureTime(double& exposureTime) ;
bool isFile(const std::string& path) ;
bool isGifPath(const std::string& path);
bool isDirectory(const std::string& path) ;
std::string getDirectoryFromPath(const std::string& path) ;

void getImages( const std::string& filePath,fs::path& directory,std::vector<fs::path>& image_paths, std::vector<std::string>& image_names,size_t& current_index );


class OneTimeTimer {
private:
    std::chrono::steady_clock::time_point start_time;
    std::chrono::seconds duration;
    bool is_running;

public:
    // 构造函数，初始化时不启动定时器
    OneTimeTimer() : is_running(false) {}

    // 启动定时器，设置持续时间（秒）
    bool start(int seconds) {
        if (is_running) return false;
        
        duration = std::chrono::seconds(seconds);
        start_time = std::chrono::steady_clock::now();
        is_running = true;
        return true;
    }

    // 检查定时器状态
    // 返回true表示定时器正在运行，false表示定时器已结束
    bool check() {
        if (!is_running) return false;

        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = current_time - start_time;

        if (elapsed >= duration) {
            is_running = false;
            return false;
        }
        return true;
    }

    // 手动停止定时器
    void stop() {
        is_running = false;
    }

    // 获取剩余时间（秒）
    double getRemainingTime() {
        if (!is_running) return 0.0;

        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
        auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(duration) - elapsed;

        return remaining.count() / 1000.0;
    }
};





///////////////////////////////////   image   ///////////////////////////////
    /**
     * @brief 加载图像文件
     * @param path 图像文件路径
     * @param outData 输出图像数据指针（需要调用者释放内存）
     * @param outWidth 输出图像宽度
     * @param outHeight 输出图像高度
     * @param desiredChannels 期望的通道数（0=原样,1=灰度,3=RGB,4=RGBA）
     * @return 是否加载成功
     */
// bool LoadImage(const std::string& path, unsigned char** outData,  int* outWidth,  int* outHeight);
unsigned char* LoadImage(const std::string& path,  int& outWidth, int& outHeight, int& channels ,int desiredChannels = 4); 
  /**
     * @brief 安全释放由LoadImage加载的图像数据
     * @param data 图像数据指针（会被置为nullptr）
     */
// 修改函数声明
void FreeImage(unsigned char*& data, const std::string& path);


// GIF
unsigned char* loadGifImage(const std::string& path, int& outWidth, int& outHeight, int& channels, int& frames,std::vector<int>& outDelays) ;
// GifImage loadGif(const std::string& path,  int& outWidth, int& outHeight,int& frame_count);


void enableImageCycle(size_t& current_index,size_t& limit_index, bool& is_cycle);
bool getExifInfo(const std::string& imagPath,std::string& image_exif,int& orientation);
int get_Orientation(int orientation);
void playGif(int& currentFrame, int& gifFramesCount,double& m_frameTimeAccumulator,double deltaTime,std::vector<int>& gifDelays, bool& is_cycle);



class Timer {
public:
    Timer() : start_time(std::chrono::high_resolution_clock::now()) {}
    
    double elapsed() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0; // 返回毫秒
    }
    
    void reset() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
private:
    std::chrono::high_resolution_clock::time_point start_time;
};