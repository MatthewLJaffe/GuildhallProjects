#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)


int WINAPI WinMain( _In_ HINSTANCE applicationInstanceHandle, _In_opt_ HINSTANCE previosInstance, _In_ LPSTR commandLineString, _In_ int nShowCmd)
{	
	UNUSED( applicationInstanceHandle );
	UNUSED( commandLineString );
	UNUSED( previosInstance );
	UNUSED( nShowCmd );

	std::string commandLineArgs = std::string(commandLineString);
	g_theApp = new App();
	g_theApp->StartUp(commandLineArgs);
	g_theApp->Run();
	g_theApp->Shutdown();
	delete g_theApp;
	g_theApp = nullptr;
	return 0;
}