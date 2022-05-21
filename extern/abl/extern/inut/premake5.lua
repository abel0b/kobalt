workspace "inut"
    configurations { "debug", "release" }
    if os.host() == "windows" then
        defines { "WINDOWS=1", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_WARNINGS" }
    else
        defines { "UNIX=1", "_POSIX_C_SOURCE=200809L" }
    end
    require "inut.lua"
