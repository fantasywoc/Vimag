#include "VimagApp.h"
#include <iostream>

#if defined(_WIN32)
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
#elif defined(__APPLE__) || defined(__unix__) 
    #include <unistd.h>
    #include <cstdint>
#endif

int main(int argc, char** argv) {

#ifdef _WIN32
    // 隐藏控制台窗口
    FreeConsole();
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::cout << "Windows UTF-8" << std::endl;
#endif

    VimagApp app;
    
    if (!app.initialize(argc, argv)) {
        return -1;
    }
    
    app.run();
    
    return 0;
}
