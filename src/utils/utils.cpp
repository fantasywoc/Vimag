#include "utils.h"

#define STBI_MAX_DIMENSIONS 32768  // 扩展到 32768x32768 ,默认最大支持尺寸为 ​16,777,216 像素
// 需要包含 stb_image
#define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"
#include "stb_image.h" // 需先下载stb_image.h







bool getImageInfo(const std::string& filePath, int& w, int& h) {
    int channels;
    // 检查文件是否存在
    if (!fs::exists(filePath)) {
        std::cerr << "文件不存在: " << filePath << std::endl;
        return false;
    }

    // 打开文件并定位到末尾
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return false;
    }

    // 检查文件大小是否有效
    const long fileSize = file.tellg();
    if (fileSize <= 0) {
        std::cerr << "无效的文件大小: " << fileSize << std::endl;
        file.close();
        return false;
    }

    // 动态计算头部大小（上限2KB）
    // 修改headerSize的计算逻辑
    const size_t headerSize = (fileSize > 51768) ? 51768 : static_cast<size_t>(fileSize); // 32KB覆盖99%的EXIF偏移
    file.seekg(0, std::ios::beg);

    // 读取头部数据到vector（自动内存管理）
    std::vector<unsigned char> header(headerSize);
    if (!file.read(reinterpret_cast<char*>(header.data()), headerSize)) {
        std::cerr << "读取文件头失败: " << filePath << std::endl;
        file.close();
        return false;
    }
    file.close();

    // 解析图像元数据
    if (stbi_info_from_memory(header.data(), headerSize, &w, &h, &channels) == 0) {
        std::cerr << "不支持的图像格式或损坏的文件: " << filePath << std::endl;
        return false;
    }
    return true;
}


// 查找指定目录下的图片文件路径和文件名
void find_image_files(
    const fs::path& directory, 
    std::vector<fs::path>& image_paths,
    std::vector<std::string>& image_names
) {
    // 清空结果容器确保每次调用都是全新结果
    image_paths.clear();
    image_names.clear();

    // 验证目录有效性 
    if (!fs::exists(directory)) {
        std::cerr << "错误：路径不存在 - " << directory << std::endl;
        return;
    }
    if (!fs::is_directory(directory)) {
        std::cerr << "错误：目标不是目录 - " << directory << std::endl;
        return;
    }

    try {
        // 创建递归迭代器
        auto options = fs::directory_options::skip_permission_denied;
        fs::recursive_directory_iterator dir_iter(directory, options);
        fs::recursive_directory_iterator end_iter;

        while (dir_iter != end_iter) {
            try {
                const auto& entry = *dir_iter;
                
                // 跳过非常规文件（目录/符号链接等）
                if (entry.is_regular_file()) {
                    // 获取小写扩展名以统一比较
                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), 
                                  [](unsigned char c) { return std::tolower(c); });
                    
                    // 检查是否为图片格式 
                    if (imageExtensions.find(ext) != imageExtensions.end() && std::filesystem::exists(entry.path())) {
                       
                       
                        try{
                            std::string filePath = entry.path().generic_string();
                            image_paths.push_back(entry.path());
                            image_names.push_back(entry.path().filename().u8string());

                        }
                        catch (const std::exception& e) {
                            std::cerr << entry.path() <<"  -------: " << std::endl;
                     
                        }
                        

                    }
                }
                ++dir_iter;
            } 
            // 捕获单个文件处理中的异常（不影响整体遍历）
            catch (const fs::filesystem_error& e) {
                std::cerr << "跳过无法访问的文件: " << e.what() << std::endl;
                dir_iter.disable_recursion_pending();  // 继续遍历其他文件 [7](@ref)
                ++dir_iter;
            }
        }
    } 
    // 捕获全局性异常（如目录打开失败）
    catch (const fs::filesystem_error& e) {
        std::cerr << "文件系统错误: " << e.what() << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "未知错误: " << e.what() << std::endl;
    }
}

size_t findPathIndex(const std::vector<fs::path>& paths, const fs::path& target) 
{
    auto it = std::find(paths.begin(), paths.end(), target);
    if (it != paths.end()) {
        return std::distance(paths.begin(), it); // 返回索引
    }
    return static_cast<size_t>(-1); // 未找到时返回无效索引
}



void removeZero(std::string& str) {
    if (str.empty()) return;

    // 使用反向迭代器从末尾开始扫描
    auto it = str.rbegin();
    while (it != str.rend()) {
        if (*it == '0') {
            // 删除当前字符（将反向迭代器转为正向迭代器）
            str.erase(std::next(it).base());
            // 重置迭代器（因删除后原迭代器失效）
            it = str.rbegin();
        } else if (*it == '.') {
            // 删除小数点并终止循环
            str.erase(std::next(it).base());
            break;
        } else {
            // 遇到非0非.字符，终止处理
            break;
        }
    }
}

//格式化曝光时间
std::string fomatExposureTime(double& exposureTime) {
    std::string str;
    if (exposureTime < 1.0f) {
        exposureTime = 1/ exposureTime;
        str = std::to_string(exposureTime);
        removeZero(str);
    }else{
        str = std::to_string(exposureTime);
        removeZero(str);
        str += "s";
    }
    return str;
}


bool isFile(const std::string& path) {
    try {
        return fs::is_regular_file(path);
    } catch (...) {
        return false;
    }
}

bool isGifPath(const std::string& path) {
    return fs::path(path).extension() == ".gif";
}

bool isDirectory(const std::string& path) {
    try {
        return fs::is_directory(path);
    } catch (...) {
        return false;
    }
}

std::string getDirectoryFromPath(const std::string& path) {
    try {
        if (fs::is_directory(path)) {
            return path;
        }

        std::string ext = fs::path(path).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        if (imageExtensions.count(ext) > 0) {
            return fs::path(path).parent_path().string();
        }
        
        return path; // 如果不是图像文件，返回原路径
    } catch (...) {
        return path; // 异常情况下返回原路径
    }
}



void getImages( const std::string& filePath,fs::path& directory,std::vector<fs::path>& image_paths, std::vector<std::string>& image_names,size_t& current_index ){
    if(isDirectory(filePath)){
        directory = filePath;
        find_image_files(directory,image_paths,image_names);
    }else if(isFile(filePath)){
        directory = getDirectoryFromPath(filePath);
        std::cout << "Received file path: " << filePath << std::endl;
        find_image_files(directory,image_paths,image_names);
        current_index = findPathIndex(image_paths,filePath );
    } 
}
//////////////////////////////  gif   //////////////////////////////////////////
unsigned char* loadGifImage(const std::string& path, int& outWidth, int& outHeight, int& channels, int& frames,std::vector<int>& outDelays) { 
    // 1. 读取文件到内存 
    std::ifstream file(path, std::ios::binary | std::ios::ate); 
    if (!file.is_open()) { 
        std::cerr << "Failed to open GIF: " << path << std::endl; 
        return nullptr;  // 修正：返回nullptr而不是false
    } 
    
    size_t size = file.tellg(); 
    file.seekg(0); 
    std::vector<char> buffer(size); 
    file.read(buffer.data(), size); 
    
    int* delays = nullptr;
    unsigned char* data = nullptr;  // 初始化为nullptr
    
    try { 
        // 2. 使用stb_image加载GIF 
        data = stbi_load_gif_from_memory( 
            reinterpret_cast<unsigned char*>(buffer.data()), 
            static_cast<int>(size), 
            &delays, &outWidth, &outHeight, &frames, &channels, 0); 

        // 直接使用参数引用，不需要局部变量
        std::cout << "GIF width: " << outWidth << ", height: " << outHeight 
                  << ", frames: " << frames << ", channels: " << channels << std::endl; 
        
        if (!data) { 
            std::cerr << "Failed to load GIF: " << stbi_failure_reason() << std::endl; 
            if (delays) free(delays);  // 清理delays
            return nullptr; 
        } 
        
        // 将C数组复制到vector中
        if (delays && frames > 0) {
            outDelays.assign(delays, delays + frames);
            free(delays); // 释放stbi分配的内存
        }

        return data; 
    } catch (const std::bad_alloc& e) { 
        std::cerr << "Memory allocation failed: " << e.what() << std::endl; 
        if (delays) free(delays);  // 安全清理
        if (data) stbi_image_free(data);  // 安全清理
        return nullptr; 
    } 
}


////////////////////////////////   image   ///////////////////////////////
unsigned char* LoadImage(const std::string& path, int& outWidth, int& outHeight, int& channels, int desiredChannels) {
    // 检查文件是否存在
    if (!std::filesystem::exists(path)) {
        std::cerr << "Error: Image file not found: " << path << std::endl;
        return nullptr;
    }
    
    // 先尝试获取图像信息
    if (!stbi_info(path.c_str(), &outWidth, &outHeight, &channels)) {
        std::cerr << "Failed to get image info: " << path << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }
    
    std::cout << "Loading image: " << path << std::endl;
    unsigned char* outData = nullptr;

    if (!isGifPath(path)) {
        try {
            outData = stbi_load(path.c_str(), &outWidth, &outHeight, &channels, desiredChannels);
            if (!outData) {
                std::cerr << "Failed to load image: " << path << std::endl;
                std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
                return nullptr;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: Failed to load image: " << e.what() << std::endl;
            return nullptr;
        }
    } else {
        // try {
        //     int frame_count;
        //     GifImage gif = loadGif(path, outWidth, outHeight, frame_count);
        //     if (gif.frame_count > 0 && gif.frames) {
        //         size_t frameSize = gif.width * gif.height * 4;
        //         outData = new unsigned char[frameSize];
        //         memcpy(outData, gif.frames.get()+ 60 * frameSize, frameSize);
        //     } else {
        //         std::cerr << "Failed to load GIF: " << path << std::endl;
        //         return nullptr;
        //     }
        // } catch (const std::exception& e) {
        //     std::cerr << "Error: Failed to load GIF: " << e.what() << std::endl;
        //     return nullptr;
        // }
    }

    return outData;
}

// 修改函数定义
void FreeImage(unsigned char*& data, const std::string& path) {
    if (!data) return;
    
    if (isGifPath(path)) {
        stbi_image_free(data);
        // GIF内存由unique_ptr自动管理，只需重置指针
        data = nullptr;
    } else {
        // 普通图像使用STB释放
        stbi_image_free(data);
        data = nullptr;
    }
}




// GifImage loadGif(const std::string& path,  int& outWidth, int& outHeight,int& frame_count) {
//     GifImage gif;
    
//     if (!loadGifImage(path, &gif)) {
//         std::cerr << "加载GIF失败: " << path << std::endl;
//         return {};
//     }
//     outWidth=gif.width;
//     outHeight=gif.height;
//     frame_count=gif.frame_count;
//     std::cout << "GIF加载成功: " << path << std::endl;
//     std::cout<<"width: " << gif.width << std::endl;
//     std::cout<<"height: " << gif.height << std::endl;
//     std::cout<<"frame_count: " << gif.frame_count << std::endl;
//     return gif;
// }
int get_Orientation(int orientation){
     if (orientation == -1) {
        std::cerr << "Failed to parse orientation." << std::endl;
    } else {
        switch (orientation) {
            case 1: return 0; 
            case 3: return 180; 
            case 6: return 90; 
            case 8: return -90; 
            default: return 0; // 2,4,5,7 较少见
        }
    }
}

bool getExifInfo(const std::string& imagPath,std::string& image_exif,int& orientation){


    EXIF exif(imagPath);
    if (!exif.isValid()) {
        // std::cerr << "EXIF信息无效: " << imagPath << std::endl;
        image_exif = "EXIF info is invalid";
        orientation = 0;
        return false ;
    }
    TinyEXIF::EXIFInfo info = exif.getInfo();
    std::string Fnumber = std::to_string(info.FNumber);
    // std::cout<< "Fnumber =" << Fnumber <<std::endl;
    removeZero(Fnumber);
    // std::cout<< "Fnumber =" << Fnumber <<std::endl;
    orientation = get_Orientation(info.Orientation);
    image_exif =  info.Make + "\n光圈 f/" + Fnumber + "\n快门 1/" + fomatExposureTime(info.ExposureTime) + "\nISO " + std::to_string(info.ISOSpeedRatings) +"\n旋转 "+ std::to_string(orientation)+"°"  ;
  
    return true;

}

//图片切换循环
void enableImageCycle(size_t& current_index,size_t& limit_index, bool& is_cycle){
    if (limit_index==1) {
        current_index = 0;
        return;
    }
    if(is_cycle){
        if (current_index > limit_index) {
            current_index = limit_index - 1;
            // return;
        }else if (current_index==limit_index) {
            current_index = 0;
            // return;
        }
    }else{
        if (current_index > limit_index) {
            current_index = 0;
            return;
        }else if (current_index==limit_index) {
            current_index = limit_index - 1;
            return;
        }
    }

}
void playGif(int& currentFrame, int& gifFramesCount,double& m_frameTimeAccumulator,double deltaTime,std::vector<int>& gifDelays, bool& is_cycle){
     if ( gifFramesCount <= 1 || gifDelays.empty()) {
        return;
    }
     // 将deltaTime转换为毫秒并累积
    m_frameTimeAccumulator += deltaTime * 1000.0;
    
    // 获取当前帧的延迟时间（毫秒）
    int currentFrameDelay = gifDelays[currentFrame];
    
    // 如果累积时间超过当前帧的延迟时间，切换到下一帧
    if (m_frameTimeAccumulator >= currentFrameDelay) {
         // 减去当前帧的延迟时间，保留多余的时间用于下一帧
        m_frameTimeAccumulator -= currentFrameDelay;
        
        currentFrame++;
        if (gifFramesCount==1) {
            currentFrame = 0;
            return;
        }
        if(is_cycle){
            if (currentFrame > gifFramesCount) {
                currentFrame = gifFramesCount - 1;
                // return;
            }else if (currentFrame==gifFramesCount) {
                currentFrame = 0;
                // return;
            }
        }else{
            if (currentFrame > gifFramesCount) {
                currentFrame = 0;
                return;
            }else if (currentFrame==gifFramesCount) {
                currentFrame = gifFramesCount - 1;
                return;
            }
        }
    }

}


