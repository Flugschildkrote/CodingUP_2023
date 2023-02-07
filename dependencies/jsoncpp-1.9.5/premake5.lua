project "jsoncpp"
	kind "StaticLib"
	language "C++"

	targetdir (bin_dir)
	objdir (bin_obj_dir)

	files {
		"include/**.h",
		"src/lib_json/**.h",
		"src/lib_json/**.inl",
		"src/lib_json/**.cpp",
	}

	includedirs {
		"include"
	}
	
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		