workspace_dir = ("projects/" .. _ACTION) 
project_dir = (workspace_dir .. "/%{prj.name}")

config_path = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
bin_dir = ("bin/" .. config_path)
lib_dir = ("lib/" .. config_path)
bin_obj_dir = ("bin-obj/" .. config_path)


workspace "Anubis_LoL"
	language "c++"
	cppdialect "c++20"
	location (workspace_dir)
	startproject "RE4_Fov_Hack"
	staticruntime "on"
	
	configurations 
	{
		"Debug",
		"Release",
		"RelWithDebInfo"
	}
	
	platforms {
		"Win32",
		"x64"
	}
	
	filter "platforms:Win32"
		system "Windows"
		architecture "x86"

	filter "platforms:Win32"
		system "Windows"
		architecture "x86_64"
	
	filter "configurations:Debug"
		symbols "On"
		optimize "Off"
		defines "INLINE_FLAG"

	filter "configurations:Release"
		symbols "Off"
		optimize "On"
		defines "NDEBUG"
		
	filter "configurations:RelWithDebInfo"
		symbols "On"
		optimize "On"
		defines "NDEBUG"


project "RE4_Fov_Hack"
	kind "ConsoleApp"
	location (project_dir)
	targetdir (bin_dir)

	files 
	{
		"./src/%{prj.name}/**",
		"./include/%{prj.name}/**"
	}
	
	includedirs 
	{
		"./include/%{prj.name}/",
        "./include/"
	}
	