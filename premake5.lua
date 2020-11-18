workspace "kobalt"
    configurations { "debug", "release" }
    
    if os.host() == "windows" then
        defines { "WINDOWS=1", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_WARNINGS" }
    end

    filter { "configurations:release", "toolset:clang or gcc" }
        buildoptions { "-Wall -Wextra" }

    filter { "configurations:debug", "toolset:clang" }
        buildoptions { "-std=c99 -pedantic -funwind-tables", "-fasynchronous-unwind-tables", "-ggdb3", "-fno-omit-frame-pointer", "-fno-optimize-sibling-calls" }
        linkoptions { "-Wl,--export-dynamic", "-fsanitize=address,leak,undefined" }

    filter { "configurations:debug", "toolset:gcc" }
        buildoptions { "-ggdb3", "-rdynamic" }

    filter "configurations:debug"
        defines { "DEBUG=1" }

    filter "configurations:release"
        defines { "DEBUG=0" }
        optimize "On"

    project "kb"
        kind "StaticLib"
        language "C"
        includedirs { "lib/include", "std/include" }
        files { "lib/**.h", "lib/**.c" }

    project "kobalt"
        kind "ConsoleApp"
        language "C"
        includedirs { "lib/include", "extern/linenoise", "std/include" }
        files { "src/**.h", "src/**.c", "extern/linenoise/*.c" }
        links { "kb" }

newoption {
    trigger = "prefix",
    value = "/usr/local",
    description = "Install prefix",
    default = "/usr/local",
}

newaction {
    trigger = "install",
    description = "Install kobalt",
    execute = function ()
        if os.host() == "windows" then
            io.stderr:write("install command is not available on windows")
            os.exit(1)
        end
        os.copyfile("./bin/release/kobalt", _OPTIONS["prefix"] .. "/bin")
    end
}

newaction {
    trigger = "export-compile-command",
    description = "Generate compile_flags.txt for clangd",
    execute = function ()
        flags = io.open("compile_flags.txt", "w")
        flags:write("-Wall\n")
        flags:write("-Wextra\n")
        flags:write("-Iextern/linenoise\n")
        flags:write("-Ilib/include\n")
        flags:write("-Istd/include\n")
        flags:close()
    end
}
