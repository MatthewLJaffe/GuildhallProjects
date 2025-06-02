#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include <mutex>
#include <atomic>

struct Rgba8;
class AABB2;
class Renderer;
class BitmapFont;
class Timer;

struct DevConsoleLine
{
	DevConsoleLine(Rgba8 color, std::string message, int frameNumber, double timeLogged);
	Rgba8 m_color;
	std::string m_message;
	int m_frameNumber;
	double m_timeLogged;
};

enum class DevConsoleMode
{
	FULLSCREEN,
	HIDDEN
};

struct DevConsoleConfig
{
	float m_linesToDisplay = 12.5f;
	std::string m_fontFilePath;
	Renderer* m_defaultRenderer = nullptr;
	Camera m_devConsoleCamera;
};

class DevConsole
{
public:
	DevConsole( DevConsoleConfig const& config );
	~DevConsole();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void ExecuteXmlCommandScriptNode(XmlElement const& commandScriptXmlElement);
	void ExecuteXmlCommandScript(std::string commandScriptFilePath);


	void Execute( std::string const& consoleCommandText );
	void AddLine( Rgba8 const& color, std::string const& text );
	void AddToCurrentLine(unsigned char charToAdd);
	void Render( AABB2 const& bounds, Renderer* renderOverride = nullptr );
	void ClearLines();

	DevConsoleMode GetMode() const;
	void SetMode( DevConsoleMode mode );
	void ToggleMode( DevConsoleMode mode );

	DevConsoleConfig m_config;

	static const Rgba8 INFO_ERROR;
	static const Rgba8 INFO_WARNING;
	static const Rgba8 INFO_MAJOR;
	static const Rgba8 INFO_MINOR;
	static const Rgba8 INPUT_COLOR;
	static const Rgba8 COMMAND_COLOR;
	std::vector<std::string> m_commands;
	Timer* m_insertionPointTimer = nullptr;
	bool m_insertionPointVisible = true;
	int m_insertionPointIdx = 0;
	int m_currLineIdx = 0;
	std::mutex m_devConsoleMutex;


protected:
	void Render_OpenFull( AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect=1.f) const;

protected:
	std::atomic<DevConsoleMode> m_mode = DevConsoleMode::HIDDEN;
	std::vector<DevConsoleLine> m_lines; //#ToDo: support a max limited # of lines (e.g. fixed circular buffer)
	std::vector<DevConsoleLine> m_sentLines;
	int m_frameNumber = 0;
	std::string m_inputLine = "";
	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_KeyReleased(EventArgs& args);
	static bool Event_CharSent(EventArgs& args);
	static bool Event_Help(EventArgs& args);
	static bool Event_Clear(EventArgs& args);
	static bool Event_Quit(EventArgs& args);
	static bool Event_Echo(EventArgs& args);
	static bool Event_RemoteCommand(EventArgs& args);
	static bool Event_BurstTest(EventArgs& args);


};
