#pragma once
#include "Game/GameState.hpp"

class GameStateLevelSelect : public GameState
{
public:
	GameStateLevelSelect(GameStateType gameStateType);
	~GameStateLevelSelect() override;
	void StartUp() override;
	void Render() override;
};

bool Event_Level1Pressed(EventArgs& args);
bool Event_Level2Pressed(EventArgs& args);
bool Event_Level3Pressed(EventArgs& args);


