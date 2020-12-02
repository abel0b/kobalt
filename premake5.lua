workspace "kobalt"
    configurations { "debug", "release" }
    
    if os.host() == "windows" then
        defines { "WINDOWS=1", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_WARNINGS" }
    else
        defines { "UNIX=1", "_POSIX_C_SOURCE=200809L" }
    end

    filter { "configurations:release", "toolset:clang or gcc" }
        buildoptions { "-Wall -Wextra" }

    filter { "configurations:debug", "toolset:clang or gcc" }
        buildoptions { "-std=c99", "-pedantic", "-ggdb3" }

    if os.host() == "linux" then
        filter { "configurations:debug", "toolset:clang" }
            buildoptions { "-funwind-tables", "-fasynchronous-unwind-tables", "-fno-omit-frame-pointer", "-fno-optimize-sibling-calls" }
            linkoptions { "-fsanitize=address,leak,undefined", "-Wl,--export-dynamic" }
    end

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
        includedirs { "lib/include", "extern/linenoise", "extern/sha-2", "extern/base32", "std/include" }
        files { "src/**.h", "src/**.c", "extern/linenoise/*.c", "extern/sha-2/*.c", "extern/base32/*.c" }
        links { "kb" }

newoption {
    trigger = "prefix",
    value = "/usr/local",
    description = "Install prefix",
    default = "/usr/local",
}

function install_action (prefix)
    if os.host() == "windows" then
        exe = ".exe"
    else
        exe = ""
    end        
    bindir = prefix .. "/bin"
    libdir = prefix .. "/lib"
    os.mkdir(bindir)
    os.mkdir(libdir)
    os.copyfile("bin/release/kobalt" .. exe, bindir)
    if os.host() == "windows" then
        os.copyfile("bin/release/kb.lib" .. exe, libdir)
    else
        os.copyfile("bin/release/libkb.a" .. exe, libdir)
    end

end

newaction {
    trigger = "install",
    description = "Install kobalt on unix",
    execute = function ()
        install_action(_OPTIONS["prefix"])
    end
}

newaction {
    trigger = "release",
    description = "Create release archive",
    execute = function ()
        slug = "kobalt-prerelease-" .. os.host()
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
        flags:write("-Iextern/base32\n")
        flags:write("-Ilib/include\n")
        flags:write("-Istd/include\n")
        flags:close()
    end
}
