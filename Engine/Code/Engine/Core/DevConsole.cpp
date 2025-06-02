#include "DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Timer.hpp"	
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Network/NetSystem.hpp"
#include "ThirdParty/TinyXML2/tinyxml2.h"

DevConsole* g_theDevConsole;

Rgba8 const DevConsole::INFO_ERROR = Rgba8(255, 0, 0);
Rgba8 const DevConsole::INFO_WARNING = Rgba8(255, 255, 0);
Rgba8 const DevConsole::INFO_MAJOR = Rgba8(0, 255, 255);
Rgba8 const DevConsole::INFO_MINOR =  Rgba8(255, 255, 255);
Rgba8 const DevConsole::INPUT_COLOR = Rgba8(200, 220, 255);
Rgba8 const DevConsole::COMMAND_COLOR = Rgba8(140, 3, 252);

DevConsole::DevConsole(DevConsoleConfig const& config)
	: m_config(config)
{
	m_insertionPointTimer = new Timer(.5f);
}

DevConsole::~DevConsole()
{
}

void DevConsole::Startup()
{
	g_theEventSystem->SubscribeEventCallbackFunction("KeyPressed", DevConsole::Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFunction("KeyReleased", DevConsole::Event_KeyReleased);
	g_theEventSystem->SubscribeEventCallbackFunction("CharSent", DevConsole::Event_CharSent);
	g_theEventSystem->SubscribeEventCallbackFunction("help", DevConsole::Event_Help);
	g_theEventSystem->SubscribeEventCallbackFunction("clear", DevConsole::Event_Clear);
	g_theEventSystem->SubscribeEventCallbackFunction("quit", DevConsole::Event_Quit);
	g_theEventSystem->SubscribeEventCallbackFunction("echo", DevConsole::Event_Echo);
	g_theEventSystem->SubscribeEventCallbackFunction("RemoteCommand", DevConsole::Event_RemoteCommand);
	g_theEventSystem->SubscribeEventCallbackFunction("BurstTest", DevConsole::Event_BurstTest);


	g_theDevConsole->AddLine(INFO_MAJOR, "Type help for a list of commands");
}

void DevConsole::Shutdown()
{
	delete m_insertionPointTimer;
	m_insertionPointTimer = nullptr;
}

void DevConsole::BeginFrame()
{
	if (m_insertionPointTimer->IsStopped())
	{
		m_insertionPointTimer->Start();
	}

	m_frameNumber++;
	while (m_insertionPointTimer->DecrementPeriodIfElapsed())
	{
		m_insertionPointVisible = ! m_insertionPointVisible;
	}
}

void DevConsole::EndFrame()
{
}

void DevConsole::ExecuteXmlCommandScriptNode(XmlElement const& commandScriptXmlElement)
{
	for (XmlElement const* currentCommand = commandScriptXmlElement.FirstChildElement();
		currentCommand != nullptr; currentCommand = currentCommand->NextSiblingElement())
	{
		std::string command = currentCommand->Name();
		for (XmlAttribute const* currentAttribute = currentCommand->FirstAttribute(); currentAttribute != nullptr; currentAttribute = currentAttribute->Next())
		{
			command += " " + std::string(currentAttribute->Name()) + "=" + std::string(currentAttribute->Value());
		}
		Execute(command);
	}
}

void DevConsole::ExecuteXmlCommandScript(std::string commandScriptFilePath)
{
	XmlDocument commandScriptDocument;
	GUARANTEE_OR_DIE(commandScriptDocument.LoadFile(commandScriptFilePath.c_str()) == 0, Stringf("Failed to load file"));
	XmlElement* rootElement = commandScriptDocument.FirstChildElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root element"));
	ExecuteXmlCommandScriptNode(*rootElement);
}

void DevConsole::Execute(std::string const& consoleCommandText)
{
	EventArgs eventArgs;
	Strings commands = SplitStringOnDelimiter(consoleCommandText, '\n');
	for (size_t commandIdx = 0; commandIdx < commands.size(); commandIdx++)
	{
		Strings commandArgs = SplitStringWithQuotes(commands[commandIdx], ' ');
		std::string eventName = ToLower( commandArgs[0] );
		for (size_t argsIdx = 1; argsIdx < commandArgs.size(); argsIdx++)
		{
			Strings argKeyValue = SplitStringWithQuotes(commandArgs[argsIdx], '=');
			if (argKeyValue.size() == 2)
			{
				std::string value = argKeyValue[1];
				TrimString(value, '\"');
				eventArgs.SetValue(ToLower(argKeyValue[0]), value);
			}
		}
		bool wasConsumed = false;
		int numResponses = FireEventAndCountResponses(eventName, eventArgs, wasConsumed);

		if (numResponses == 0)
		{
			AddLine(INFO_ERROR, "ERROR: unknown command " + consoleCommandText);
		}
	}
}

void DevConsole::AddLine(Rgba8 const& color, std::string const& text)
{
	m_devConsoleMutex.lock();
	m_lines.push_back(DevConsoleLine(color, text, m_frameNumber, GetCurrentTimeSeconds()));
	m_devConsoleMutex.unlock();
}

void DevConsole::AddToCurrentLine(unsigned char text)
{
	m_insertionPointTimer->Stop();
	m_insertionPointVisible = true;
	std::string beforeInsertionPoint = m_inputLine.substr(0, m_insertionPointIdx);
	std::string atInsertionPoint = "";
	atInsertionPoint += text;
	std::string afterInsertionPoint =  m_inputLine.substr(m_insertionPointIdx);
	m_inputLine =beforeInsertionPoint + atInsertionPoint + afterInsertionPoint;
	m_insertionPointIdx++;
}

void DevConsole::Render(AABB2 const& bounds, Renderer* renderOverride)
{
	m_devConsoleMutex.lock();
	if (m_mode == DevConsoleMode::HIDDEN)
	{
		m_devConsoleMutex.unlock();
		return;
	}
	Renderer* rendererToUse = m_config.m_defaultRenderer;
	if (renderOverride != nullptr)
	{
		rendererToUse = renderOverride;
	}
	rendererToUse->BeginCamera(m_config.m_devConsoleCamera);
	float clientAspect = Window::GetTheWindowInstance()->GetConfig().m_clientAspect;
	Render_OpenFull(bounds, *rendererToUse, *rendererToUse->CreateOrGetBitmapFontFromFile(m_config.m_fontFilePath.c_str()), 1.f / clientAspect);
	rendererToUse->EndCamera(m_config.m_devConsoleCamera);
	m_devConsoleMutex.unlock();
}

void DevConsole::ClearLines()
{
	m_devConsoleMutex.lock();
	m_lines.clear();
	m_devConsoleMutex.unlock();
}

DevConsoleMode DevConsole::GetMode() const
{
	return m_mode;
}

void DevConsole::SetMode(DevConsoleMode mode)
{
	m_mode = mode;
}

void DevConsole::ToggleMode(DevConsoleMode mode)
{
	if (m_mode != mode)
	{
		m_mode = mode;
	}
	else
	{
		m_mode = DevConsoleMode::HIDDEN;
	}
}

void DevConsole::Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect) const
{
	
	std::vector<Vertex_PCU> transparentQuadVerts;
	AddVertsForAABB2D(transparentQuadVerts, bounds, Rgba8(0, 0, 0, 120));
	renderer.SetDepthMode(DepthMode::DISABLED);
	renderer.SetBlendMode(BlendMode::ALPHA);
	renderer.SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	renderer.SetModelConstants();
	renderer.BindTexture(nullptr);
	renderer.DrawVertexArray(transparentQuadVerts.size(), transparentQuadVerts.data());
	
	Vec2 textBorderOffset = Vec2::DOWN * .02f;
	
	//add verts for input line
	float lineHeight = bounds.GetDimensions().y / m_config.m_linesToDisplay;
	float currLineYMin = lineHeight;
	std::vector<Vertex_PCU> devConsoleVerts;
	AABB2 currLineBox(Vec2(bounds.m_mins.x, 0), Vec2(bounds.m_maxs.x, lineHeight));
	AABB2 offsetLineBox(Vec2(bounds.m_mins.x, 0) + textBorderOffset, Vec2(bounds.m_maxs.x, lineHeight) + textBorderOffset);

	//add verts for current input line or line from history
	if (m_currLineIdx == (int)m_sentLines.size() || (int)m_sentLines.size() == 0)
	{
		font.AddVertsForTextInBox2D(devConsoleVerts, offsetLineBox, lineHeight, m_inputLine, Rgba8::BLACK, fontAspect, Vec2(0.f, .5f));
		font.AddVertsForTextInBox2D(devConsoleVerts, currLineBox, lineHeight, m_inputLine, INPUT_COLOR, fontAspect, Vec2(0.f, .5f));
	}
	else
	{
		font.AddVertsForTextInBox2D(devConsoleVerts, offsetLineBox, lineHeight, m_inputLine, Rgba8::BLACK, fontAspect, Vec2(0.f, .5f));
		font.AddVertsForTextInBox2D(devConsoleVerts, currLineBox, lineHeight, m_inputLine, INPUT_COLOR, fontAspect, Vec2(0.f, .5f));
	}

	//insertion point verts
	if (m_insertionPointVisible)
	{
		std::vector<Vertex_PCU> insertionPointVerts;

		//some dumb math
		float insertionPointX =  font.GetTextWidth(lineHeight, m_inputLine, fontAspect);
		if (m_inputLine.size() != 0)
		{
			insertionPointX *= (float)(m_insertionPointIdx) / (float)(m_inputLine.size());
		}
		float insertionPointWidth = lineHeight * .05f;
		if (insertionPointWidth < .025f)
		{
			insertionPointWidth = .025f;
		}
		Vec2 cursorBottomLeft = Vec2(bounds.m_mins.x + insertionPointX - (insertionPointWidth * .5f), bounds.m_mins.y);
		cursorBottomLeft.x = Clamp(cursorBottomLeft.x, 0, 99999);
		AddVertsForAABB2D(insertionPointVerts, AABB2(cursorBottomLeft, cursorBottomLeft + Vec2(insertionPointWidth, lineHeight)), Rgba8::WHITE);
		renderer.DrawVertexArray(insertionPointVerts.size(), insertionPointVerts.data());
	}

	//add verts for other lines
	for (int i = static_cast<int>(m_lines.size() - 1); i >= 0; i--)
	{
		std::string message = m_lines[i].m_message;
		if (g_gameConfigBlackboard.GetValue("debugMode", false))
		{
			message += Stringf("\tFrame Number: %d Time: %.3f", m_lines[i].m_frameNumber, m_lines[i].m_timeLogged);
		}
		currLineBox = AABB2(Vec2(bounds.m_mins.x, currLineYMin), Vec2(bounds.m_maxs.x, currLineYMin + lineHeight));
		offsetLineBox = AABB2(currLineBox.m_mins + textBorderOffset, currLineBox.m_maxs + textBorderOffset);
		font.AddVertsForTextInBox2D(devConsoleVerts, offsetLineBox, lineHeight, message, Rgba8::BLACK, fontAspect, Vec2(0.f, .5f));
		font.AddVertsForTextInBox2D(devConsoleVerts, currLineBox, lineHeight, message, m_lines[i].m_color, fontAspect, Vec2(0.f, .5f));
		currLineYMin += lineHeight;
		if (currLineYMin + lineHeight > bounds.m_maxs.y)
		{
			break;
		}
	}

	renderer.BindTexture(font.GetTexture());
	renderer.DrawVertexArray(devConsoleVerts.size(), devConsoleVerts.data());
	renderer.BindTexture(nullptr);
	
}

bool DevConsole::Event_KeyPressed(EventArgs& args)
{
	if (g_theDevConsole->GetMode() == DevConsoleMode::HIDDEN)
	{
		return false;
	}	
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);

	if (keyCode == KEYCODE_TILDE)
	{
		g_theDevConsole->ToggleMode(DevConsoleMode::FULLSCREEN);
	}
	if (keyCode == KEYCODE_ESC)
	{
		if (g_theDevConsole->m_inputLine == "")
		{
			g_theDevConsole->ToggleMode(DevConsoleMode::FULLSCREEN);
		}
		g_theDevConsole->m_inputLine = "";
	}
	if (keyCode == '\r')
	{
		if (g_theDevConsole->m_inputLine == "")
		{
			g_theDevConsole->ToggleMode(DevConsoleMode::FULLSCREEN);
		}
		else
		{
			Rgba8 lineColor = INFO_MINOR;
			//check for if input line is a command
			std::vector<std::string> commands = g_theEventSystem->GetEventNames();
			Strings splitCommand = SplitStringOnDelimiter(g_theDevConsole->m_inputLine, ' ');
			if (splitCommand.size() > 0)
			{
				for (size_t i = 0; i < commands.size(); i++)
				{
					if (commands[i] == splitCommand[0])
					{
						lineColor = COMMAND_COLOR;
					}
				}
			}
			g_theDevConsole->AddLine(lineColor, g_theDevConsole->m_inputLine);
			g_theDevConsole->Execute(g_theDevConsole->m_inputLine);

			//remember for history
			g_theDevConsole->m_sentLines.push_back(DevConsoleLine(INFO_MINOR, g_theDevConsole->m_inputLine, g_theDevConsole->m_frameNumber, GetCurrentTimeSeconds()));
			g_theDevConsole->m_currLineIdx = (int)g_theDevConsole->m_sentLines.size();

			g_theDevConsole->m_inputLine = "";
		}
	}

	if (keyCode == KEYCODE_LEFT_ARROW)
	{
		g_theDevConsole->m_insertionPointTimer->Stop();
		g_theDevConsole->m_insertionPointVisible = true;
		g_theDevConsole->m_insertionPointIdx--;
	}
	if (keyCode == KEYCODE_RIGHT_ARROW)
	{
		g_theDevConsole->m_insertionPointTimer->Stop();
		g_theDevConsole->m_insertionPointVisible = true;
		g_theDevConsole->m_insertionPointIdx++;
	}
	if (keyCode == KEYCODE_HOME)
	{
		g_theDevConsole->m_insertionPointTimer->Stop();
		g_theDevConsole->m_insertionPointVisible = true;
		g_theDevConsole->m_insertionPointIdx = 0;
	}
	if (keyCode == KEYCODE_END)
	{
		g_theDevConsole->m_insertionPointTimer->Stop();
		g_theDevConsole->m_insertionPointVisible = true;
		g_theDevConsole->m_insertionPointIdx = (int)g_theDevConsole->m_inputLine.size();
	}
	if (keyCode == KEYCODE_DELETE)
	{
		if (g_theDevConsole->m_insertionPointIdx < (int)g_theDevConsole->m_inputLine.size())
		{
			std::string newInputLine = g_theDevConsole->m_inputLine.substr(0, g_theDevConsole->m_insertionPointIdx) + g_theDevConsole->m_inputLine.substr(g_theDevConsole->m_insertionPointIdx + 1);
			g_theDevConsole->m_inputLine = newInputLine;
			if (g_theDevConsole->m_insertionPointIdx > (int)g_theDevConsole->m_inputLine.size())
			{
				g_theDevConsole->m_insertionPointIdx = (int)g_theDevConsole->m_inputLine.size();
			}
		}
	}
	if (keyCode == KEYCODE_BACKSPACE)
	{
		if (g_theDevConsole->m_insertionPointIdx > 0)
		{
			g_theDevConsole->m_inputLine = g_theDevConsole->m_inputLine.substr(0, g_theDevConsole->m_insertionPointIdx - 1) + g_theDevConsole->m_inputLine.substr(g_theDevConsole->m_insertionPointIdx);
			g_theDevConsole->m_insertionPointIdx--;
		}
	}

	//cycle command history
	if (keyCode == KEYCODE_UP_ARROW)
	{
		g_theDevConsole->m_currLineIdx--;
		g_theDevConsole->m_currLineIdx = ClampInt(g_theDevConsole->m_currLineIdx, 0, (int)g_theDevConsole->m_sentLines.size());
		if (g_theDevConsole->m_sentLines.size() > 0)
		{
			g_theDevConsole->m_inputLine = g_theDevConsole->m_sentLines[g_theDevConsole->m_currLineIdx].m_message;
			g_theDevConsole->m_insertionPointIdx = (int)g_theDevConsole->m_inputLine.size();
		}
	}
	if (keyCode == KEYCODE_DOWN_ARROW)
	{
		g_theDevConsole->m_currLineIdx++;

		//clamp and erase current line if needed
		if (g_theDevConsole->m_currLineIdx >= (int)g_theDevConsole->m_sentLines.size())
		{
			g_theDevConsole->m_inputLine = "";
			g_theDevConsole->m_currLineIdx = (int)g_theDevConsole->m_sentLines.size();
			g_theDevConsole->m_insertionPointIdx = 0;

		}
		else
		{
			if (g_theDevConsole->m_sentLines.size() > 0)
			{
				g_theDevConsole->m_inputLine = g_theDevConsole->m_sentLines[g_theDevConsole->m_currLineIdx].m_message;
				g_theDevConsole->m_insertionPointIdx = (int)g_theDevConsole->m_inputLine.size();
			}
		}
	}
	g_theDevConsole->m_insertionPointIdx = ClampInt(g_theDevConsole->m_insertionPointIdx, 0, (int)g_theDevConsole->m_inputLine.size());
	return true;
}

bool DevConsole::Event_KeyReleased(EventArgs& args)
{
	UNUSED(args);
	return false;
}

bool DevConsole::Event_CharSent(EventArgs& args)
{
	unsigned char keyCode = (unsigned char)args.GetValue("Char", -1);
	if (keyCode != '`' && keyCode != '~' && keyCode >= 32 && keyCode <= 126)
	{
		if (g_theDevConsole->m_mode != DevConsoleMode::HIDDEN)
		{
			g_theDevConsole->AddToCurrentLine(keyCode);
		}
	}
	return true;
}

bool DevConsole::Event_Help(EventArgs& args)
{
	UNUSED(args);
	std::vector commands = g_theEventSystem->GetEventNames();
	for (size_t i = 0; i < commands.size(); i++)
	{
		g_theDevConsole->AddLine(INFO_WARNING, commands[i]);
	}
	return true;
}

bool DevConsole::Event_Clear(EventArgs& args)
{
	UNUSED(args);
	g_theDevConsole->ClearLines();
	DebugRenderClear();
	return true;
}

bool DevConsole::Event_Quit(EventArgs& args)
{
	UNUSED(args);
	g_theEventSystem->UnsubscribeEventCallbackFunction("Quit", DevConsole::Event_Quit);
	g_theEventSystem->FireEvent("Quit");
	return true;
}

bool DevConsole::Event_Echo(EventArgs& args)
{
	std::string message = args.GetValue("Message", "");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, message);
	return true;
}

bool DevConsole::Event_RemoteCommand(EventArgs& args)
{
	std::string command = args.GetValue("Command", "");
	g_theNetSystem->m_sendQueue.push_back(command);
	return true;
}

bool DevConsole::Event_BurstTest(EventArgs& args)
{
	UNUSED(args);
	for (int i = 1; i <= 20; i++)
	{
		g_theNetSystem->m_sendQueue.push_back(Stringf("Echo Message=%d", i));
	}
	return true;
}


DevConsoleLine::DevConsoleLine(Rgba8 color, std::string message, int frameNumber, double timeLogged)
	: m_color(color)
	, m_message(message)
	, m_frameNumber(frameNumber)
	, m_timeLogged(timeLogged)
{
}
