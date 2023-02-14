workspace_dir = (_MAIN_SCRIPT_DIR .. "/projects/" .. _ACTION) 
project_dir = ("%{wks.location}" .. "/%{prj.name}")

config_path = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
dependencies_config_path = "%{cfg.system}/%{cfg.platform}/"
bin_dir = (_MAIN_SCRIPT_DIR .. "/bin/%{prj.name}/" .. config_path)
lib_dir = (_MAIN_SCRIPT_DIR .. "/lib/" .. config_path)
bin_obj_dir = (_MAIN_SCRIPT_DIR .. "/bin-obj/" .. config_path)

deps_include_dirs = {
	(_MAIN_SCRIPT_DIR .. "/dependencies/*/include"),
	(_MAIN_SCRIPT_DIR .. "/dependencies/boost-1.81.0/libs/**/include")
}

lib_dirs = {
	(_MAIN_SCRIPT_DIR .. "/lib/" .. config_path),
	(_MAIN_SCRIPT_DIR .. "/dependencies/openssl-3.0.7/lib/" .. dependencies_config_path),
	(_MAIN_SCRIPT_DIR .. "/dependencies/curl-7.87.1/lib/" .. dependencies_config_path),
	(_MAIN_SCRIPT_DIR .. "/dependencies/libsndfile-1.2.0/lib/" .. dependencies_config_path)
}

link_libs = {
    "anubis",
	"stb",
	"libssl",
	"libcrypto",
	"sndfile",
	"libcurl_imp"
}

post_build_copy_commands = {
	("{COPYFILE} " .. _MAIN_SCRIPT_DIR .. "/dependencies/dependencies/curl-7.87.1/bin/" .. dependencies_config_path .. "libcurl-d.dll %{cfg.targetdir}"),
	("{COPYFILE} " .. _MAIN_SCRIPT_DIR .. "/dependencies/openssl-3.0.7/bin/libssl-3.dll %{cfg.targetdir}"),
	("{COPYFILE} " .. _MAIN_SCRIPT_DIR .. "/dependencies/openssl-3.0.7/bin/libcrypto-3.dll %{cfg.targetdir}"),
	("{COPYFILE} " .. _MAIN_SCRIPT_DIR .. "/dependencies/libsndfile-1.2.0/bin/" .. dependencies_config_path .. "sndfile.dll %{cfg.targetdir}"),
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
		"x86",
		"x64"
	}
	
	filter "platforms:x86"
		architecture "x86"

	filter "platforms:x64"
		architecture "x64"
	
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