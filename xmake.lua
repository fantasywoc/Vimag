-- 全局配置
add_rules("plugin.compile_commands.autoupdate", { outputdir = ".vscode" })
-- set_policy("build.merge_archive", true)

-- 设置项目基本信息
add_rules("mode.debug", "mode.release")
set_project("LiteUI")
set_languages("c++17")
set_version("1.0.0")

includes("@builtin/xpack")

if is_plat("linux") then
    add_ldflags("-Wl,-rpath=$ORIGIN")
end

-- 添加第三方库依赖
add_requires("glfw 3.3.8", {configs = {shared = true}})
add_requires("nanovg", {configs = {shared = true}})
add_requires("glew", {configs = {shared = true}})

-- 定义 UI 静态库目标
target("ui")
    set_kind("static")
    add_files("src/UIWindow.cpp")
    add_files("src/component/*.cpp")
    add_files("src/animation/*.cpp")
    add_files("src/utils/*.cpp")
    add_files("src/TinyEXIF/*.cpp")
    add_includedirs("src", "src/component", "src/animation","src/utils","src/TinyEXIF")
    add_packages("glfw", "nanovg", "glew")

-- 定义图片查看器演示程序目标
target("VIMAG")
    set_kind("binary")
    add_rpathdirs("$ORIGIN")
    add_files("src/Vimag.cpp","src/TinyEXIF/TinyEXIF.cpp","src/component/TextureCache.cpp","src/VimagApp.cpp")
    add_deps("ui")
    add_packages("glfw", "nanovg", "glew")
    
    add_includedirs("src", "src/component", "src/widget", "src/animation", "src/TinyEXIF")
    
    if is_plat("windows") then
        add_cxflags("/utf-8")
        add_links("opengl32", "gdi32", "user32", "kernel32")
    else
        add_links("GL", "X11", "pthread", "Xrandr", "Xi", "dl","m")
    end
    
    set_rundir("$(projectdir)")
    
    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
        add_defines("DEBUG")
    else
        set_optimize("fast")
    end
    
    add_cxxflags("/EHsc")

-- 在 dist_package target 中直接定义函数
target("dist_package")
    set_kind("phony")
    add_deps("VIMAG")
    on_build(function (target)
        -- 智能查找和复制依赖库函数 - 在这里定义
        local function smart_copy_package_dlls()
            local user_home = os.getenv("USERPROFILE") or os.getenv("HOME")
            local xmake_packages_dir = path.join(user_home, "AppData", "Local", ".xmake", "packages")
            
            print("📦 智能查找依赖库...")
            
            -- 包名映射到搜索路径和 DLL 名称
            local packages = {
                glfw = {search_dir = "g/glfw", dll_name = "glfw3.dll"},
                nanovg = {search_dir = "n/nanovg", dll_name = "nanovg.dll"},
                glew = {search_dir = "g/glew", dll_name = "glew32.dll"}
            }
            
            -- 确保 dist 目录存在
            if not os.isdir("dist") then
                os.mkdir("dist")
            end
            
            for pkg_name, pkg_info in pairs(packages) do
                local pkg_base_dir = path.join(xmake_packages_dir, pkg_info.search_dir)
                
                if os.isdir(pkg_base_dir) then
                    -- 查找所有版本目录
                    local version_dirs = os.dirs(path.join(pkg_base_dir, "*"))
                    
                    for _, version_dir in ipairs(version_dirs) do
                        -- 查找哈希目录
                        local hash_dirs = os.dirs(path.join(version_dir, "*"))
                        
                        for _, hash_dir in ipairs(hash_dirs) do
                            local dll_path = path.join(hash_dir, "bin", pkg_info.dll_name)
                            
                            if os.isfile(dll_path) then
                                local dest_path = path.join("dist", pkg_info.dll_name)
                                
                                if os.cp(dll_path, dest_path) then
                                    print("✅ 已复制: " .. pkg_name .. " -> " .. dest_path)
                                    goto next_package
                                end
                            end
                        end
                    end
                    
                    print("⚠️ 未找到: " .. pkg_name .. " 的 DLL 文件")
                else
                    print("⚠️ 包目录不存在: " .. pkg_base_dir)
                end
                
                ::next_package::
            end
            
            print("🚀 依赖库复制完成！")
        end
        
        -- 1. 复制可执行文件
        if is_plat("windows") then
            os.trycp("build/windows/x64/release/*.exe", "dist")
        end

        -- 2. 智能复制依赖库
        smart_copy_package_dlls()

        -- 3. 复制资源文件
        os.trycp("src/font/*.ttc", "dist")
        os.trycp("src/icons/*.png", "dist")
        os.trycp("config.ini", "dist")
        print("🚀 打包完成！")
    end)
