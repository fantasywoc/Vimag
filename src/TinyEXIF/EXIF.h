#pragma once
#define TINYEXIF_NO_XMP_SUPPORT  // 在包含头文件前定义 禁止xmp
#include "TinyEXIF.h"  // 使用相对路径
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class EXIF {
private:
    std::string m_imagePath;
    int m_imageWidth;
    int m_imageHeight;
    TinyEXIF::EXIFInfo info;
    bool m_isValid;  // 添加有效性标志
    
public:
EXIF(const std::string& imagePath) : m_imageWidth(0), m_imageHeight(0), m_isValid(false) 
{
    std::ifstream input_file(imagePath, std::ios::binary);
    if (!input_file) {
        std::cerr << "Error: cannot open input file" << std::endl;
        return;
    }

    // 获取文件大小（带错误检查）
    input_file.seekg(0, std::ios::end);
    size_t file_size = input_file.tellg();
    if (file_size == static_cast<size_t>(-1)) {
        std::cerr << "Error: failed to get file size" << std::endl;
        return;
    }
    input_file.seekg(0, std::ios::beg);

    // 读取文件内容
    std::vector<uint8_t> file_data(file_size);
    if (!input_file.read(reinterpret_cast<char*>(file_data.data()), file_size)) {
        std::cerr << "Error: failed to read file content" << std::endl;
        return;
}

    // 解析EXIF
    if (info.parseFrom(file_data.data(), file_size) != TinyEXIF::PARSE_SUCCESS) {
        std::cerr << "Error: EXIF parsing failed" << std::endl;
        m_isValid=false;
        return;
    }

    m_imageWidth = info.ImageWidth;
    m_imageHeight = info.ImageHeight;
    m_isValid = true;
}
    
    ~EXIF() = default;  // 使用默认析构函数
    
    // 添加有效性检查方法
    bool isValid() const { return m_isValid; }
    
    std::vector<int> getWH() { return {m_imageWidth, m_imageHeight}; }
    
    // 获取EXIF信息
    const TinyEXIF::EXIFInfo& getInfo() const { return info; }
    
    // 输出EXIF元数据 - 只使用存在的成员变量
    const TinyEXIF::EXIFInfo& printAllInfo() {

        // if (info.RelatedImageWidth || info.RelatedImageHeight){
		//     std::cout << "RelatedImageResolution " << info.RelatedImageWidth << "x" << info.RelatedImageHeight << " pixels" << "\n";
        // }// 检查基础字段是否存在

        // // if (info.ImageWidth == 0) {
        // //     std::cerr << "警告: ImageWidth未解析到有效值" << std::endl;
        // // }

        if (info.ImageWidth || info.ImageHeight){
		    std::cout << "ImageResolution " << info.ImageWidth << "x" << info.ImageHeight << " pixels" << "\n";
        }
        std::cout << "Make: " << info.Make << std::endl;
        std::cout << "Model: " << info.Model << std::endl;
        std::cout << "Software: " << info.Software << std::endl;
        std::cout << "BitsPerSample: " << info.BitsPerSample << std::endl;
        std::cout << "ImageWidth: " << info.ImageWidth << std::endl;
        std::cout << "ImageHeight: " << info.ImageHeight << std::endl;
        std::cout << "ImageDescription: " << info.ImageDescription << std::endl;
        std::cout << "Orientation: " << info.Orientation << std::endl;
        std::cout << "XResolution: " << info.XResolution << std::endl;
        std::cout << "YResolution: " << info.YResolution << std::endl;
        std::cout << "ResolutionUnit: " << info.ResolutionUnit << std::endl;
        std::cout << "DateTime: " << info.DateTime << std::endl;
        std::cout << "DateTimeOriginal: " << info.DateTimeOriginal << std::endl;
        std::cout << "DateTimeDigitized: " << info.DateTimeDigitized << std::endl;
        std::cout << "SubSecTimeOriginal: " << info.SubSecTimeOriginal << std::endl;
        std::cout << "ExposureTime: " << info.ExposureTime << std::endl;
        std::cout << "FNumber: " << info.FNumber << std::endl;
        std::cout << "ExposureProgram: " << info.ExposureProgram << std::endl;
        std::cout << "ISOSpeedRatings: " << info.ISOSpeedRatings << std::endl;
        std::cout << "ShutterSpeedValue: " << info.ShutterSpeedValue << std::endl;
        std::cout << "ApertureValue: " << info.ApertureValue << std::endl;
        std::cout << "BrightnessValue: " << info.BrightnessValue << std::endl;
        std::cout << "ExposureBiasValue: " << info.ExposureBiasValue << std::endl;
        std::cout << "SubjectDistance: " << info.SubjectDistance << std::endl;
        std::cout << "MeteringMode: " << info.MeteringMode << std::endl;
        std::cout << "LightSource: " << info.LightSource << std::endl;
        std::cout << "Flash: " << info.Flash << std::endl;
        std::cout << "FocalLength: " << info.FocalLength << std::endl;
        
        // 正确访问SubjectArea
        if (!info.SubjectArea.empty()) {
            std::cout << "SubjectArea: ";
            for (size_t i = 0; i < info.SubjectArea.size(); ++i) {
                std::cout << info.SubjectArea[i];
                if (i < info.SubjectArea.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
        }
        
        // 正确访问LensInfo结构体中的成员
        std::cout << "LensInfo.FStopMin: " << info.LensInfo.FStopMin << std::endl;
        std::cout << "LensInfo.FStopMax: " << info.LensInfo.FStopMax << std::endl;
        std::cout << "LensInfo.FocalLengthMin: " << info.LensInfo.FocalLengthMin << std::endl;
        std::cout << "LensInfo.FocalLengthMax: " << info.LensInfo.FocalLengthMax << std::endl;
        std::cout << "LensInfo.DigitalZoomRatio: " << info.LensInfo.DigitalZoomRatio << std::endl;
        std::cout << "LensInfo.FocalLengthIn35mm: " << info.LensInfo.FocalLengthIn35mm << std::endl;
        std::cout << "LensInfo.FocalPlaneXResolution: " << info.LensInfo.FocalPlaneXResolution << std::endl;
        std::cout << "LensInfo.FocalPlaneYResolution: " << info.LensInfo.FocalPlaneYResolution << std::endl;
        std::cout << "LensInfo.FocalPlaneResolutionUnit: " << info.LensInfo.FocalPlaneResolutionUnit << std::endl;
        std::cout << "LensInfo.Make: " << info.LensInfo.Make << std::endl;
        std::cout << "LensInfo.Model: " << info.LensInfo.Model << std::endl;
        
        // 序列号在主结构体中
        std::cout << "SerialNumber: " << info.SerialNumber << std::endl;
        
        return info;
    }
};
