workspace "kobalt"
    configurations { "debug", "release" }
    
    if os.host() == "windows" then
        defines { "WINDOWS=1", "_CRT_SECURE_NO_WARNINGS" }
    end

    filter { "configurations:debug", "toolset:clang" }
        buildoptions { "-ggdb3", "-fno-omit-frame-pointer", "-fno-optimize-sibling-calls" }
        linkoptions { "-fsanitize=address,leak,undefined" }

    filter { "configurations:debug", "toolset:gcc" }
        buildoptions { "-ggdb3" }

    filter "configurations:debug"
        defines { "DEBUG=1" }

    filter "configurations:release"
        defines { "DEBUG=0" }
        optimize "On"

    project "kb"
        kind "StaticLib"
        language "C"
        includedirs { "lib/include" }
        files { "lib/**.h", "lib/**.c" }

    project "kobalt"
        kind "ConsoleApp"
        language "C"
        includedirs { "lib/include", "extern/linenoise" }
        files { "src/**.h", "src/**.c", "extern/linenoise/*.c" }
        links { "kb" }
