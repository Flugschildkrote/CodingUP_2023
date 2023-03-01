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

	filter "platforms:x86"
		postbuildcommands(GetPostPostBuildCommands("x86"))
	filter "platforms:x64"
		postbuildcommands(GetPostPostBuildCommands("x64"))
