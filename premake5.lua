workspace "newport"
    architecture "x64"
    startproject "newport"
    
    configurations
    {
        "Debug",
        "Release"
    }

	targetdir ("bin")
    objdir ("bin")
    debugdir ("bin")
	characterset("ASCII")

	include "libs/ch_stl"


project "newport"
    language "C++"
	dependson "ch_stl"

    cppdialect "C++17"
    systemversion "latest"
    architecture "x64"
	kind "WindowedApp"
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

    files
    {
        "src/*.h",
        "src/*.cpp",
		"res/**.glsl",
		"todo.txt",
		"gdd.txt"
    }

    includedirs
    {
        "src/**",
        "libs/"
    }

    links
    {
        "opengl32",
        "user32",
        "kernel32",
		"shlwapi",
		"bin/ch_stl"
    }

    filter "configurations:Debug"
		defines 
		{
			"BUILD_DEBUG#1",
			"BUILD_RELEASE#0",
			"CH_BUILD_DEBUG#1"
		}
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines 
		{
			"BUILD_RELEASE#1",
			"BUILD_DEBUG#0",
			"NDEBUG"
		}
		runtime "Release"
        optimize "On"
        
    filter "system:windows"

		files 
		{
			"src/win32/**.h",
			"src/win32/**.cpp"
		}