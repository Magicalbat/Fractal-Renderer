workspace "Fractal-Renderer"
    configurations { "debug", "release" }
    startproject "Fractal-Renderer"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Fractal-Renderer"
    language "C"
    location "src"
    kind "ConsoleApp"

    includedirs {
        "src",
        "src/third_party"
    }

    files {
        "src/**.h",
        "src/**.c",
    }

    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    targetdir ("bin/" .. outputdir)
    targetprefix ""

    warnings "Extra"
    architecture "x64"
    toolset "clang"

    --filter { "action:*gmake*" } 
        buildoptions { "-msse4.1 -mpclmul" }

    filter "system:linux"
        links {
            "m", "X11", "GL", "GLX", "pthread"
        }

    filter { "system:windows", "action:*gmake*", "configurations:debug" }
        linkoptions { "-g" }

    filter "configurations:debug"
        symbols "On"

        defines {
            "DEBUG"
        }

    filter "configurations:release"
        optimize "On"
        defines { "NDEBUG" }

    filter "system:windows"
        systemversion "latest"

        links {
            "gdi32", "kernel32", "user32", "opengl32"
        }
