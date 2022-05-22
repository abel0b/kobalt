project "abl"
	kind "SharedLib"
	language "C"
	
	includedirs { "include" }
	defines { "ABL_DLL", "ABL_DLL_EXPORTS" }
	files { "include/**.h", "abl/**.h", "abl/**.c" }

function libabl()
	links { "abl" }
end

