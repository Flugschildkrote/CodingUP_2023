project "anubis"
	kind "StaticLib"
	language "C++"

	location (project_dir)
	targetdir (bin_dir)
	objdir (bin_obj_dir)

	files
	{
		"src/**",
		"include/**"
	}

	includedirs {
		"include",
		deps_include_dirs
	}
	
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		