#include "UIWindow.h"
#include "component/UIPanel.h"
#include "component/UIButton.h"
#include "component/UILabel.h"
#include "component/FlexLayout.h"
#include "component/UITexture.h"
#include "component/UITextInput.h"
#include "component/TextureCacheData.h"
// 添加动画系统头文件
#include "animation/UIAnimationManager.h"
#include "animation/UIAnimation.h"
#include "utils/utils.h"
#include "utils/setting.h"
#include "TinyEXIF/EXIF.h"
#include <iostream>
#include <locale>
#include <filesystem>
#include <memory>
#include <vector>
#include <algorithm>
#include <set>


namespace fs = std::filesystem;


#if defined(_WIN32)
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
#elif defined(__APPLE__) || defined(__unix__) 
    #include <unistd.h>
    #include <cstdint>

#endif







int main(int argc, char** argv) {
 
    // FreeConsole();  //关闭控制台
    #ifdef _WIN32
        // Windows专用代码
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
      
        std::cout << "windos UTF-8" << std::endl;
    #endif

    //////////////////////////////  setting ini  ////////////////////////////////////////
    loadSetting();
    // 读取Display配置
    bool imageCycle = getSettingBool("Display", "image_cycle", true);  //图像循环切换
    bool imageNameDisplay = getSettingBool("Display", "image_name_display", false);  //显示文件名
    bool showIndex = getSettingBool("Display", "image_index", true);  //显示图像索引
    bool showExif = getSettingBool("Display", "image_EXIF", false);  //显示图像EXIF



    // 获取图片文件
    fs::path directory = "./";
    std::vector<fs::path> image_paths;
    std::vector<std::string> image_names;
    size_t current_index = 0;
    if(argc >1) {
        std::string filePath = argv[1];
      
        getImages(filePath,directory,image_paths,image_names,current_index);

        std::cout << "Received file path: " << filePath << std::endl;
        
    } else {
        std::cout << "Usage: " << argv[0] << " <file_path>" << std::endl;
    }
    if (image_paths.empty()) {
        image_paths.push_back("Vimag.png");
        image_names.push_back("Vimag.png");
        current_index = 0;
    }

    size_t limit_index = image_paths.size(); 
    std::string imagPath = image_paths[current_index].generic_string();
    // std::string imagPath =image_paths[current_index].u8string();
    int change_speed = 0;

    UIWindow window(1600, 900, "VIMAG");

    // 启用动态标题栏功能
    window.enableDynamicTitleBar(true, 15.0); // 鼠标距离顶部15像素时显示
    // 设置透明帧缓冲区
    window.setTransparentFramebuffer(true);
    
    // 初始化窗口
    if (!window.initialize()) {
        std::cerr << "Failed to initialize window" << std::endl;
        return -1;
    }

    int currentWindowWidth, currentWindowHeight;
    window.getFramebufferSize(currentWindowWidth, currentWindowHeight);
    int windowWidth{currentWindowWidth},windowHeight{currentWindowHeight};
    // 创建主面板
    auto mainPanel = std::make_shared<UIPanel>(0, 0, windowWidth, windowHeight);
    mainPanel->setHorizontalLayoutWithAlignment(FlexLayout::X_CENTER, FlexLayout::Y_CENTER, 0.0f, 0.0f);
    mainPanel->setBackgroundColor(nvgRGBA(255, 255, 255, 230));
    mainPanel->setBorderColor(nvgRGBA(255, 255, 255, 230));
    mainPanel->setBorderWidth(0.0f);
    mainPanel->setCornerRadius(0.0f);

    
    // 创建右图像面板
    auto rightPanel1 = std::make_shared<UIPanel>(0, 0,  windowWidth, windowHeight);
    rightPanel1->setVerticalLayoutWithAlignment(FlexLayout::X_CENTER, FlexLayout::Y_CENTER, 10.0f, 10.0f);
    rightPanel1->setBackgroundColor(nvgRGBA(255, 255, 255, 230));

    // 先创建纹理组件
    // auto texture = std::make_shared<UITexture>(0, 0, 400, 600, "D:\\Picture\\JEPG\\20250216\\20250216-P1013191-.jpg");
    auto texture = std::make_shared<UITexture>(0, 0, windowWidth*0.9f, windowHeight*0.9, imagPath);

    texture->setScaleMode(UITexture::ScaleMode::KEEP_ASPECT);
    texture->setAlpha(1.0f);
    texture->setCornerRadius(10.0f);

// 方法2: 设置窗口大小变化监听
   window.setWindowSizeCallback([&,mainPanel,rightPanel1](int width, int height) {
    std::cout << "窗口大小实时更新: " << width << "x" << height << std::endl;

    mainPanel->setSize(width,height);
    rightPanel1->setSize(width,height);

    texture->setSize(width*0.9,height*0.9);
    texture->setOriginSize(width*0.9,height*0.9);
    texture->setPaintValid(false);


    mainPanel->updateLayout();
   
});
   
    // 添加标签到左面板 - 修改宽度为150px
    auto label = std::make_shared<UILabel>(0, 0, 200, 50, " ");
    label->setTextAlign(UILabel::TextAlign::CENTER);
    label->setTextColor(nvgRGBA(50, 50, 50, 250));
    label->setFontSize(24.0f);

    auto indexlabel = std::make_shared<UILabel>(2, 2, 100, 50, " ");
    indexlabel->setTextAlign(UILabel::TextAlign::RIGHT);
    indexlabel->setTextColor(nvgRGBA(50, 50, 50, 250));
    indexlabel->setFontSize(24.0f);
    // indexlabel->setAnimationOpacity(1.0f); 
 

    ////////////////////////////////////////   setting    //////////////////////////////////
     // 创建左面板 - 使用具体坐标（由布局系统自动计算）
    auto settingPanel = std::make_shared<UIPanel>(0, 0, 500, 480);
    settingPanel->setVerticalLayoutWithAlignment(FlexLayout::X_START, FlexLayout::Y_CENTER, 10.0f, 10.0f);
    settingPanel->setBackgroundColor(nvgRGBA(200, 200, 200,150));
    settingPanel->setBorderColor(nvgRGBA(150, 150, 150, 10));

    settingPanel->setBorderWidth(5.0f);
    settingPanel->setCornerRadius(5.0f);
    auto index_button = std::make_shared<UIButton>(0, 0, 200, 50, "Show Image Index");
    index_button->setTextColor(nvgRGBA(249, 125, 3, 255));
    index_button->setFontSize(20.0f);
    index_button->setCornerRadius(8.0f);
    index_button->setOnClick([index_button,indexlabel]() {
        std::cout << "Button 1 clicked! Playing fade animation..." << std::endl;
        // 淡出再淡入的动画效果
        UIAnimationManager::getInstance().fadeOut(index_button.get(), 0.25f, UIAnimation::EASE_IN);
        
        // 创建延迟淡入动画
        auto fadeInAnim = std::make_shared<UIAnimation>(UIAnimation::FADE, 0.3f, UIAnimation::EASE_OUT);
        fadeInAnim->setValues(0.0f, 1.0f);
        fadeInAnim->setOnUpdate([index_button](float value) {
            index_button->setAnimationOpacity(value);
        });

        // 延迟0.1秒后开始淡入
        auto delayAnim = std::make_shared<UIAnimation>(UIAnimation::CUSTOM, 0.1f);
        delayAnim->setOnComplete([fadeInAnim, index_button]() {
            UIAnimationManager::getInstance().addAnimation(fadeInAnim, index_button.get());
        });
        UIAnimationManager::getInstance().addAnimation(delayAnim, index_button.get());

        if(index_button->getText() == "Show Image Index"){
            index_button->setTextColor(nvgRGBA(170, 95, 3, 255));
            index_button->setText("Hide Image Index");
            UIAnimationManager::getInstance().fadeIn(indexlabel.get(), 0.25f, UIAnimation::EASE_OUT);
            indexlabel->setDisplay(false);
        }else{
            index_button->setText("Show Image Index");
            UIAnimationManager::getInstance().fadeOut(indexlabel.get(), 0.25f, UIAnimation::EASE_OUT);
            index_button->setTextColor(nvgRGBA(249, 125, 3, 255));
            indexlabel->setDisplay(true);
        }
        
    });
    
    ///////////////////////////////////////////////////////////////////
    rightPanel1->addChild(texture);
    // rightPanel1->addChild(label);
    mainPanel->addChild(rightPanel1);

    settingPanel->addChild(index_button);
    mainPanel->addChild(settingPanel);
    mainPanel->updateLayout();



    float scalex{1}, scaley{1};
    // 事件处理 //
    // // 设置拖拽回调
    static float initialX = rightPanel1->getX();
    static float initialY = rightPanel1->getY();
    static float totalDeltaX = 0.0f;
    static float totalDeltaY = 0.0f;
    
    texture->setOnDrag([&,rightPanel1,texture](float deltaX, float deltaY) {
        std::cout << "拖拽偏移: (" << deltaX << ", " << deltaY << ")" << std::endl;
        
        totalDeltaX += deltaX * 2 ;
        totalDeltaY += deltaY * 2;
        
    
        // mainPanel->updateLayout();
        rightPanel1->moveTo(totalDeltaX , totalDeltaY, 0.3f);
        // texture->setPaintValid(false);
        // mainPanel->updateLayout();
        mainPanel->updateLayout();
    });
  
    // 设置滚动回调（需要在事件系统中添加滚动事件支持）
    texture->setOnScroll([&,texture,rightPanel1](float scrollX, float scrollY) {
        std::cout << "滚动: (" << scrollX << ", " << scrollY << ")" << std::endl;
        scalex += scrollY*0.15;
        scaley += scrollY*0.15; 
        std::cout << "缩放: (" << scalex << ", " << scaley << ")" << std::endl;
        if (scalex >= 13.0f) {

            scalex = 13.0f;
            scaley = 13.0f; 
        }
        else if (scalex <= 0.2f) {
            scalex = 0.3f;
            scaley = 0.3f; 
        }
       
        UIAnimationManager::getInstance().scaleTo(texture.get(), 1.0f * scalex, 1.0f *scaley, 0.2f, UIAnimation::EASE_OUT);
        // texture->setPaintValid(false);
        // mainPanel->updateLayout();
    });

    // 设置按键回调
    texture->setOnKeyPress([](int keyCode, int modifiers) {
        std::cout << "按键: " << keyCode << ", 修饰键: " << modifiers << std::endl;
    });

    // 设置拖拽时滚轮回调
    // 在图像切换逻辑中添加预加载
    texture->setOnDragScroll([&, texture, rightPanel1, mainPanel,label](float scrollX, float scrollY) {
    
    change_speed += -scrollY;
        if (change_speed%2 != 0){
            return ;
        }

        current_index += change_speed / 2;
        change_speed = 0;

        enableImageCycle(current_index,limit_index,imageCycle);
        // 图片信息获取
        imagPath =image_paths[current_index].generic_string();
        
        std::string indexString = "["+ std::to_string(current_index+1)+"/"+  std::to_string(limit_index) +"]" ;
        std::string image_exif = indexString + getExifInfo(imagPath);
        std::string imagName = image_names[current_index];
        texture->setImagePath(window.getNVGContext(), imagPath);
        if(texture->isLoadError()){
            label->setText(indexString + imagName+" LOAD FAIL!");
        }else{
            if(!showIndex){
                indexString = "";
            }
            if(!imageNameDisplay){
                imagName = "";
            }
            label->setText(indexString + imagName);
        }

        mainPanel->setSize(currentWindowWidth,currentWindowHeight);
        rightPanel1->setSize(currentWindowWidth,currentWindowHeight);

        texture->setSize(currentWindowWidth*0.9,currentWindowHeight*0.9);
        texture->setOriginSize(currentWindowWidth*0.9,currentWindowHeight*0.9);
        texture->setPaintValid(false);
        // mainPanel->updateLayout();
        rightPanel1->updateLayout ();
        
    });

    // 设置双击回调
    texture->setOnDoubleClick([&,texture,rightPanel1,mainPanel,settingPanel](int mouseButton) {
        if (mouseButton == 0) {
            std::cout << "左键双击" << std::endl;
            scalex = 1.0f;
            scaley = 1.0f; 
            UIAnimationManager::getInstance().scaleTo(texture.get(), 1.0f * scalex, 1.0f *scaley, 0.35f, UIAnimation::EASE_OUT);
            
            rightPanel1->moveTo(rightPanel1->getX() , rightPanel1->getY(), 0.2f);

  
            texture->setPaintValid(false);

            // mainPanel->updateLayout();
            // rightPanel1->updateLayout ();
        } 
        texture->setPaintValid(false);
        mainPanel->updateLayout();
    });

    // 设置中键点击回调
    texture->setOnMiddleClick([&,texture](float mouseX, float mouseY) {
        std::cout << "Middle mouse button clicked at (" << mouseX << ", " << mouseY << ")" << std::endl;

    });

    // 启用所有事件
    texture->setDragEnabled(true);
    texture->setScrollEnabled(true);
    texture->setDoubleClickEnabled(true);
    texture->setDragScrollEnabled(true);
    texture->setMiddleClickEnabled(true);
    



    window.setDropCallback([&, mainPanel, rightPanel1](int count, const char** paths) {
        std::cout << "Drop event: " << count << " files dropped" << std::endl;
        for (int i = 0; i < count; ++i) {
            std::cout << "File path: " << paths[i] << std::endl;
        }
    });


    // 设置鼠标事件回调
    window.setMouseButtonCallback([&,rightPanel1,texture,mainPanel,settingPanel](int button, int action, int mods) {

        // 处理所有鼠标按键，不只是左键
        double xpos, ypos;
        glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
        
        UIEvent event;
        event.type = (action == GLFW_PRESS) ? UIEvent::MOUSE_PRESS : UIEvent::MOUSE_RELEASE;
        event.mouseX = static_cast<float>(xpos);
        event.mouseY = static_cast<float>(ypos);
        event.mouseButton = button;  // 直接传递button值
        
        // 添加时间戳用于双击检测
        event.clickTime = glfwGetTime();
        
        // 添加调试输出
        std::cout << "Mouse button " << button << " action " << action << std::endl;
        
        if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {

            window.toggleFullscreen();
            window.getFramebufferSize(currentWindowWidth, currentWindowHeight);
            mainPanel->setSize(currentWindowWidth,currentWindowHeight);
            rightPanel1->setSize(currentWindowWidth,currentWindowHeight);
        
            texture->setSize(currentWindowWidth*0.9,currentWindowHeight*0.9);
            texture->setOriginSize(currentWindowWidth*0.9,currentWindowHeight*0.9);
            

        }else if (button == GLFW_MOUSE_BUTTON_RIGHT  && action == GLFW_PRESS) {
            std::cout << "右键单击 打开设置" << std::endl;
            if (settingPanel->isDisplay()){
            
                settingPanel->setDisplay(false);

            }
            else{
                settingPanel->setDisplay(true);
                rightPanel1->moveTo(rightPanel1->getX() , rightPanel1->getY(), 0.35f);
                mainPanel->updateLayout();
                
                rightPanel1->moveTo(rightPanel1->getX() , rightPanel1->getY(), 0.35f);
            }
        

        }

        texture->setPaintValid(false);
        mainPanel->handleEvent(event);
        settingPanel->handleEvent(event);
    });
    
    window.setCursorPosCallback([mainPanel,settingPanel](double xpos, double ypos) {
        UIEvent event;
        event.type = UIEvent::MOUSE_MOVE;
        event.mouseX = static_cast<float>(xpos);
        event.mouseY = static_cast<float>(ypos);
        event.mouseButton = -1;
        
        mainPanel->handleEvent(event);
        settingPanel->handleEvent(event);
    });
    
    // 在设置鼠标事件回调后添加键盘事件处理
    window.setKeyCallback([&, texture, rightPanel1, mainPanel,label,settingPanel](int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            UIEvent event;
            event.type = UIEvent::KEY_PRESS;
            event.keyCode = key;
            event.modifiers = mods;
            
            // 添加方向键检查
            if (key == GLFW_KEY_LEFT) {
                // 左方向键处理
                event.keyCode = GLFW_KEY_LEFT;
                std::cout << "Left key pressed   "<<event.keyCode  << std::endl;
                change_speed = -1 ;
            } 
            else if (key == GLFW_KEY_RIGHT ) {
                // 右方向键处理
                event.keyCode = GLFW_KEY_RIGHT;
                std::cout << "Right key pressed  "<<event.keyCode << std::endl;
                change_speed =1;
            }else if(key == GLFW_KEY_F ){
                event.keyCode = GLFW_KEY_F;
                window.toggleFullscreen();
                window.getFramebufferSize(currentWindowWidth, currentWindowHeight);
                mainPanel->setSize(currentWindowWidth,currentWindowHeight);
                rightPanel1->setSize(currentWindowWidth,currentWindowHeight);
            
                texture->setSize(currentWindowWidth*0.9,currentWindowHeight*0.9);
                texture->setOriginSize(currentWindowWidth*0.9,currentWindowHeight*0.9);
                texture->setPaintValid(false);
                rightPanel1->updateLayout();
                mainPanel->handleEvent(event);
                return;
            }
            
/////////////////////////////////////////////
 
        current_index += change_speed ;
        change_speed = 0;

        enableImageCycle(current_index,limit_index,imageCycle);
        // 图片信息获取
        imagPath =image_paths[current_index].generic_string();

        std::string indexString = "["+ std::to_string(current_index+1)+"/"+  std::to_string(limit_index) +"]" ;
        std::string image_exif ="";
        // std::string image_exif =  getExifInfo(imagPath);
        std::string imagName = image_names[current_index];
        texture->setImagePath(window.getNVGContext(), imagPath);
        if(texture->isLoadError()){
            label->setText(indexString + imagName+" LOAD FAIL!");
        }else{
            if(!showIndex){
                indexString = "";
            }
            if(!imageNameDisplay){
                imagName = "";
            }
            indexlabel->setText(indexString);
            label->setText(image_exif);
        }
        
        mainPanel->setSize(currentWindowWidth,currentWindowHeight);
        rightPanel1->setSize(currentWindowWidth,currentWindowHeight);

        texture->setSize(currentWindowWidth*0.9,currentWindowHeight*0.9);
        texture->setOriginSize(currentWindowWidth*0.9,currentWindowHeight*0.9);
        texture->setPaintValid(false);
        
        rightPanel1->updateLayout();


//////////////////////////////////////////////////
        mainPanel->handleEvent(event);
        settingPanel->handleEvent(event);
        }
    });

    // 添加字符输入事件处理
    window.setCharCallback([mainPanel,settingPanel](unsigned int codepoint) {
        UIEvent event;
        event.type = UIEvent::CHAR_INPUT;
        event.character = codepoint;
        
        mainPanel->handleEvent(event);
    });
    
    // 添加滚轮事件处理
    window.setScrollCallback([&,mainPanel,settingPanel](double xoffset, double yoffset) {
        UIEvent event;
        event.type = UIEvent::MOUSE_SCROLL;
        
        // 获取当前鼠标位置
        double xpos, ypos;
        glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
        event.mouseX = static_cast<float>(xpos);
        event.mouseY = static_cast<float>(ypos);
        
        // 设置滚轮偏移量
        event.scrollX = static_cast<float>(xoffset);
        event.scrollY = static_cast<float>(yoffset);
        
        // 分发事件到主面板
        mainPanel->handleEvent(event);
        // settingPanel->handleEvent(event);
    });
    

    // 创建定时器实例
    OneTimeTimer timer;
    timer.start(1.5);
    // 主渲染循环
    const double targetFrameTime = 1.0 / 60.0;
    auto lastTime = glfwGetTime();
    while (!window.shouldClose()) {
        

        
        // 计算时间差
        // 在初始化时设置垂直同步
        glfwSwapInterval(1);  // 已经设置了

        window.getFramebufferSize(currentWindowWidth, currentWindowHeight);
        if(timer.check()){
            mainPanel->setSize(currentWindowWidth,currentWindowHeight);
            rightPanel1->setSize(currentWindowWidth,currentWindowHeight);
        
            texture->setSize(currentWindowWidth*0.9,currentWindowHeight*0.9);
            texture->setOriginSize(currentWindowWidth*0.9,currentWindowHeight*0.9);
            texture->setPaintValid(false);
        }

        // std::cout << "窗口大小: " << currentWindowWidth << "x" << currentWindowHeight << std::endl;
        auto currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        
        if (deltaTime < targetFrameTime) {
            // 计算需要休眠的时间（毫秒）
            double sleepTime = (targetFrameTime - deltaTime) * 1000;
            if (sleepTime > 1) {  // 只在休眠时间大于1ms时才休眠
                //Sleep(static_cast<DWORD>(sleepTime - 1));  // 预留1ms的误差
 
                #ifdef _WIN32
                    Sleep(static_cast<DWORD>(sleepTime - 1));
                #else
                    usleep((sleepTime - 1) * 1000); // 毫秒转微秒
                #endif
            }
            continue;
        }
        lastTime = currentTime;
        texture->update(deltaTime);
        window.pollEvents();
        // std::cout<<"UIAnimationManager::getInstance().getAnimationCount():"<<UIAnimationManager::getInstance().getAnimationCount() <<std::endl;
        // std::cout<<"texture->isPaintValid():"<<texture->isPaintValid() <<std::endl;
        // 更新动画系统
        UIAnimationManager::getInstance().update(deltaTime);
        if (UIAnimationManager::getInstance().getAnimationCount() != 0 || texture->isPaintValid()== false || texture->isGif()) {
            window.beginFrame();
            window.clearBackground(1.0f, 1.0f, 1.0f, 0.9f);
            
            // 渲染主面板（会递归渲染所有子组件）
            mainPanel->updateLayout();
            mainPanel->render(window.getNVGContext());
            indexlabel->render(window.getNVGContext());
            // settingPanel->render(window.getNVGContext());

            // texture->render(window.getNVGContext());
            window.endFrame();
            window.swapBuffers();
        }
    }
    
    // 清理所有纹理资源
    UITexture::cleanupAll(window.getNVGContext());
    window.cleanup();
    return 0;
}
