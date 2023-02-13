local prj_name = path.getname(os.getcwd())

project(prj_name)
	kind "ConsoleApp"
	location (project_dir)
	targetdir (bin_dir)
	objdir (bin_obj_dir)
	debugdir ("./")

	files 
	{
		"./src/**",
		"./include/**",
	}
	
	includedirs 
	{
		"./include/",
		deps_include_dirs,
	}

	libdirs {
		lib_dirs,
	}
	
	links (link_libs)

	filter "configurations:Debug"
		links { "libcurl-d_imp" }
		postbuildcommands { ("{COPYFILE} " .. curl_dll_dir .. "libcurl-d.dll %{cfg.targetdir}") }

	filter "configurations: not Debug"
		links { "libcurl_imp" }
		postbuildcommands { ("{COPYFILE} " .. curl_dll_dir .. "libcurl.dll %{cfg.targetdir}") }
	filter {}

	postbuildcommands (post_build_copy_commands)
