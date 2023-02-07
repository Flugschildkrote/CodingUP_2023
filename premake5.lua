workspace_dir = (_MAIN_SCRIPT_DIR .. "/projects/" .. _ACTION) 
project_dir = ("%{wks.location}" .. "/%{prj.name}")

config_path = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
bin_dir = (_MAIN_SCRIPT_DIR .. "/bin/%{prj.name}/" .. config_path)
lib_dir = (_MAIN_SCRIPT_DIR .. "/lib/" .. config_path)
bin_obj_dir = (_MAIN_SCRIPT_DIR .. "/bin-obj/" .. config_path)
deps_include_dirs = (_MAIN_SCRIPT_DIR .. "/dependencies/*/include")
curl_dll_dir = (_MAIN_SCRIPT_DIR .. "/dependencies/curl-7.87.1/lib/" .. config_path .. "/")

lib_dirs = {
	(_MAIN_SCRIPT_DIR .. "/lib/" .. config_path),
	(_MAIN_SCRIPT_DIR .. "/dependencies/openssl-3.0.7/lib"),
	(_MAIN_SCRIPT_DIR .. "/dependencies/curl-7.87.1/lib/" .. config_path)
}

workspace "CodingGames_2023"
	language "c++"
	cppdialect "c++20"
	location (workspace_dir)
	startproject "CopyAndPasteSandbox"
	
	configurations 
	{
		"Debug",
		"Release",
		"RelWithDebInfo"
	}

	platforms {
		"Win32"
	}
	
	filter "platforms:Win32"
		architecture "x86"
	
	filter "configurations:Debug"
		symbols "On"
		optimize "Off"

	filter "configurations:Release"
		symbols "Off"
		optimize "On"
		defines "NDEBUG"
		
	filter "configurations:RelWithDebInfo"
		symbols "On"
		optimize "On"
		defines "NDEBUG"



include "dependencies/anubis"
include "dependencies/stb"

-------------------------------------------------
-----------[Sub projects detection]--------------
-------------------------------------------------
local subProjects = os.matchfiles("./challenges/*/premake5.lua")
for k,v in pairs(subProjects) do
 	local subProjectDir = path.getdirectory(v)
	print("Detected sub project \"" .. subProjectDir .. "\"")
	include(subProjectDir)
end