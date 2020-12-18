workspace "kobalt"
    configurations { "debug", "release", "debug-vg" }
    
    if os.host() == "windows" then
        defines { "WINDOWS=1", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_WARNINGS" }
    else
        defines { "UNIX=1", "_POSIX_C_SOURCE=200809L" }
    end

    filter { "configurations:release", "toolset:clang or gcc" }
        buildoptions { "-Wall -Wextra" }

    filter { "configurations:debug", "toolset:clang or gcc" }
        buildoptions { "-std=c99", "-pedantic" }
        
    if os.host() == "linux" then
        filter { "configurations:debug", "toolset:clang" }
            buildoptions { "-funwind-tables", "-fasynchronous-unwind-tables", "-fno-omit-frame-pointer", "-fno-optimize-sibling-calls" }
            linkoptions { "-fsanitize=address,leak,undefined", "-Wl,--export-dynamic" }
            defines { "DEBUG_SAN=1" }
        filter { "configurations:debug or debug-vg", "toolset:clang or gcc" }
            buildoptions { "-ggdb3" }
    end

    filter "configurations:debug or debug-vg"
        defines { "DEBUG=1" }

    filter "configurations:release"
        defines { "DEBUG=0" }
        optimize "On"

    project "kl"
        kind "StaticLib"
        language "C"
        includedirs { "include" }
        files { "include/**.h", "lib/**.h", "lib/**.c" }

    project "kobalt"
        kind "ConsoleApp"
        language "C"
        includedirs { "include", "extern/linenoise", "extern/sha-2", "extern/basenc" }
        files { "src/kobalt/**.h", "src/kobalt/**.c", "extern/linenoise/*.c", "extern/sha-2/*.c", "extern/basenc/*.c" }
        links { "kl" }

newoption {
    trigger = "prefix",
    value = "/usr/local",
    description = "Install prefix",
    default = "/usr/local",
}

function install_action (prefix)
    if os.host() == "windows" then
        exe = "kobalt.exe"
    else
        exe = "kobalt"
    end        
    bindir = prefix .. "/bin/"
    libdir = prefix .. "/lib/"
    os.mkdir(bindir)
    os.mkdir(libdir)
    assert(os.copyfile("bin/release/" .. exe, bindir .. exe))
    if os.host() == "windows" then
        assert(os.copyfile("bin/release/kl.lib", libdir .. "kl.lib"))
    else
        assert(os.copyfile("bin/release/libkl.a", libdir .. "libkl.a"))
    end
end

newaction {
    trigger = "install",
    description = "Install kobalt on unix",
    execute = function ()
        install_action(_OPTIONS["prefix"])
    end
}

function os.capture(cmd)
    local f = assert(io.popen(cmd, 'r'))
    local s = assert(f:read('*a'))
    f:close()
    s = string.gsub(s, '^%s+', '')
    s = string.gsub(s, '%s+$', '')
    s = string.gsub(s, '[\n\r]+', ' ')
    return s
end

newaction {
    trigger = "release",
    description = "Create release archive",
    execute = function ()
        if os.host() == "macosx" then
            slug = "kobalt-macos"
        else
            slug = "kobalt-" .. os.host()
        end
        reldir = "tmp/" .. slug
        install_action(reldir)
        os.mkdir("dist")
        os.chdir(reldir)
        if os.host() == "windows" then
            tarcmd = "tar.exe -a -c -f ../../dist/" .. slug .. ".zip " .. "bin lib"
        else
            tarcmd = "tar czvf ../../dist/" .. slug .. ".tar.gz " .. "bin lib"
        end
        print(tarcmd)
        os.execute(tarcmd)
    end
}

newaction {
    trigger = "gen-compile-flags",
    description = "Generate compile_flags.txt for clangd",
    execute = function ()
        flags = io.open("compile_flags.txt", "w")
        flags:write("-Wall\n")
        flags:write("-Wextra\n")
        flags:write("-Iextern/linenoise\n")
        flags:write("-Iextern/levenshtein\n")
        flags:write("-Iextern/sha-2\n")
        flags:write("-Iextern/base85\n")
        flags:write("-Iextern/basenc\n")
        flags:write("-Iinclude\n")
        flags:close()
    end
}
