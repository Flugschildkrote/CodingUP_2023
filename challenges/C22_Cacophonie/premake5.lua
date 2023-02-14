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

	postbuildcommands (post_build_copy_commands)
