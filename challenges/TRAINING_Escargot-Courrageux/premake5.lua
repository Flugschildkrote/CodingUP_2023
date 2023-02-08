local prj_name = path.getname(os.getcwd())

project(prj_name)
	kind "ConsoleApp"
	location (project_dir)
	targetdir (bin_dir)
	objdir (bin_obj_dir)

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
	
	links {
        "anubis",
		"stb",
		"libssl",
		"libcrypto",
	}

	filter "configurations:Debug"
		links { "libcurl-d_imp" }
		postbuildcommands { ("{COPYFILE} " .. curl_dll_dir .. "libcurl-d.dll %{cfg.targetdir}") }

	filter "configurations: not Debug"
		links { "libcurl_imp" }
		postbuildcommands { ("{COPYFILE} " .. curl_dll_dir .. "libcurl.dll %{cfg.targetdir}") }
	filter {}

	postbuildcommands {
		("{COPYFILE} " .. _MAIN_SCRIPT_DIR .. "/dependencies/openssl-3.0.7/bin/libssl-3.dll %{cfg.targetdir}"),
		("{COPYFILE} " .. _MAIN_SCRIPT_DIR .. "/dependencies/openssl-3.0.7/bin/libcrypto-3.dll %{cfg.targetdir}"),
	}
