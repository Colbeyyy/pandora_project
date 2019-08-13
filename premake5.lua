workspace "pandora_project"
    architecture "x64"
    startproject "pandora_project"
    
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


project "pandora_project"
    language "C++"
	dependson "ch_stl"

    cppdialect "C++17"
    systemversion "latest"
    architecture "x64"
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

    files
    {
        "src/*.h",
        "src/*.cpp",
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
		kind "ConsoleApp"

	filter "configurations:Release"
		defines 
		{
			"BUILD_RELEASE#1",
			"BUILD_DEBUG#0",
			"NDEBUG"
		}
		runtime "Release"
        optimize "On"
		kind "WindowedApp"

        
    filter "system:windows"

		files 
		{
			"src/win32/**.h",
			"src/win32/**.cpp"
		}