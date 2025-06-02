#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"


class Game;
class Clock;
class Material;
struct MapDefinition;
struct TileDefinition;


class App
{
public: 
	App();
	~App();

	void Run();
	void StartUp(std::string const& commandLineArgs);
	void Shutdown();
	static bool QuitGame(EventArgs& args);
	void HandleSpecialCommands();
	void ResetGame();
	bool m_debugMode = false;
	Clock* m_clock = nullptr;
	bool m_isQuitting = false;
	Material* m_moonMaterial = nullptr;
	std::vector<MapDefinition> m_mapDefinitions;
	std::vector<TileDefinition> m_tileDefinitions;

private:
	void RunFrame();
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
	void PrintControlsToConsole();
	void HandleMouseMode();
	void LoadTileDefinitions();
	void LoadMapDefinitions();
	void GetUnitData(XmlElement const* unitsXML, MapDefinition& mapDef);


private:
	static bool LoadGameConfig(EventArgs& args);
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_runOnce = false;
	double m_timeLastFrame = 0.0;
	float m_deltaTime = 0.0f;
};