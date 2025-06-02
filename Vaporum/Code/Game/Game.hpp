#pragma once
#include "Game/GameCommon.hpp"


class Entity;
class Player;
class Texture;
class BitmapFont;
class Prop;
class Unit;
class Widget;

Vec2 GetHexPosFromGridCoord(IntVec2 const& gridGoord);
IntVec2 GetGridCoordFromIndex(int index);
int GetIndexFromGridCoord(IntVec2 const& gridCoord);
bool IsInBounds(IntVec2 const& gridCoord);
bool IsBlocked(IntVec2 const& gridCoord);
IntVec2 GetHexCoordFromWorldPos(Vec3 const& worldPosition);


struct TileDefinition
{
	char m_symbol = 0;
	std::string m_name = "";
	bool m_isBlocked = false;
};

struct UnitInMap
{
	char m_unitType;
	IntVec2 m_startCoords;
};

struct MapDefinition
{
	std::vector<char> m_tiles;
	std::vector<UnitInMap> m_player1Units;
	std::vector<UnitInMap> m_player2Units;
	std::string m_name = "";
	Shader* m_overlayShader = nullptr;
	IntVec2 m_gridSize;
	Vec3 m_worldBoundsMin;
	Vec3 m_worldBoundsMax;
};

enum class GameState
{
	START_SCREEN = 0,
	MAIN_MENU,
	WAITING_FOR_PLAYER,
	PLAYING,
	PAUSE_MENU,
	GAME_COMPLETE
};

enum class TurnState
{
	TURN_BEGIN,
	SELECTING,
	MOVING,
	ATTACKING,
	TURN_END
};

enum class Team
{
	NONE,
	RED,
	BLUE
};


class Game
{
public:
	Game(App* app);
	~Game();
	void StartUp();
	void Update(float deltaSeconds);
	void Update_StartScreen(float deltaSeconds);
	void Update_MainMenu(float deltaSeconds);
	void Update_WaitingForPlayer(float deltaSeconds);
	void Update_Playing(float deltaSeconds);
	void Update_PauseMenu(float deltaSeconds);
	void Update_GameComplete(float deltaSeconds);
	void UpdateBlueUnitWidget();
	void UpdateRedUnitWidget();

	void Render() const;
	void ShutDown();
	bool IsControllingCurrentTurn() const;
	bool IsMultiplayerTeamsTurn();
	static bool Event_LoadMap(EventArgs& args);
	static bool Event_PlayerReady(EventArgs& args);
	static bool Event_SetFocusedHex(EventArgs& args);
	static bool Event_StartTurn(EventArgs& args);
	static bool Event_EndTurn(EventArgs& args);
	static bool Event_SelectFocusedUnit(EventArgs& args);
	static bool Event_SelectNextUnit(EventArgs& args);
	static bool Event_SelectPreviousUnit(EventArgs& args);
	static bool Event_Move(EventArgs& args);
	static bool Event_Cancel(EventArgs& args);
	static bool Event_Stay(EventArgs& args);
	static bool Event_Attack(EventArgs& args);
	static bool Event_HoldFire(EventArgs& args);
	static bool Event_PlayerQuit(EventArgs& args);
	void CheckForGameComplete();
	void AttackComplete();


	void LoadMap(std::string mapToLoad);
	TileDefinition GetTileDefinitionBySymbol(char symbol) const;
	void AddVertsForHexCell(std::vector<Vertex_PCU>& verts, IntVec2 const& cellCoords, Rgba8 const& color, bool outline, float size = 1.f) const;
	MapDefinition m_currentMapDef;
	std::vector<Entity*> m_allEntities;
	std::vector<Unit*> m_allUnits;
	std::vector<Unit*> m_player1Units;
	std::vector<Unit*> m_player2Units;
	Unit* m_selectedUnit = nullptr;
	IntVec2 m_focusedHexCoords = IntVec2(-1, -1);
	TurnState m_currentTurnState;
	GameState m_currentGameState;
	Widget* m_startScreenWidget = nullptr;
	Widget* m_mainMenuWidget = nullptr;
	Widget* m_pauseMenuWidget = nullptr;
	std::vector<Widget*> m_mainMenuButtons;
	int m_selectedMainMenuButton = 0;
	std::vector<Widget*> m_pauseMenuButtons;
	std::vector<Widget*> m_controlWidgets;
	Widget* m_blueTeamUnitInfo = nullptr;
	Widget* m_redTeamUnitInfo = nullptr;
	Widget* m_waitingForPlayersWidget = nullptr;
	Widget* m_turnBeginWidget = nullptr;
	Widget* m_controlsWidget = nullptr;
	Widget* m_playerTurnIndicator = nullptr;
	Widget* m_gameCompleteWidget = nullptr;
	int m_selectedPauseMenuButton = 0;
	Camera m_screenCamera;
	Player* m_player = nullptr;


	//commands that must be supported
	void PlayerReady();
	void StartTurn();
	void SetFocusedHex(IntVec2 const& coords);
	void SelectFocusedUnit();
	void SelectPreviousUnit();
	void SelectNextUnit();
	void Move();
	void Stay();
	void HoldFire();
	void Attack();
	void Cancel();
	void EndTurn();
	void PlayerQuit();
	std::string m_multiplayerMapToLoad = "";
	Team m_multiplayerTeam = Team::NONE;
	Team m_currentPlayerTurn = Team::BLUE;
	bool m_selfInGame = false;
	bool m_remotePlayerInGame = false;
	bool m_redTeamWon = false;
	bool m_blueTeamWon = false;
	bool m_isQuitting = false;
	BitmapFont* m_bitmapFont;

private:
	void StartGame();
	void CheckForDebugCommands(float deltaSeconds);
	RaycastResult3D RaycastVsHexGrid();
	void DrawHilightedTile() const;
	void LoadUnits();
	App* m_theApp;
	bool m_attractScreen = true;
	float m_attractAlpha = 0.f;
	float m_playRadius = 0.f;
	float m_currT = 0.0f;
	Texture* m_testTexture = nullptr;
	Prop* m_hexGrid = nullptr;
	EulerAngles m_sunOrientaiton = EulerAngles(120.f, 45.f, 0.f);
	float m_sunOrientaitonRate = 45.f;
	float m_sunIntensity = .5f;
	bool m_useAmbient = true;
	bool m_useDiffuse = true;
	bool m_useSpecular = true;
	bool m_useEmissive = true;
	bool m_diffuseMap = true;
	bool m_normalMap = true;
	bool m_specularMap = true;
	bool m_glossinessMap = true;
	bool m_emissiveMap = true;
	bool m_renderDebug = false;
};

