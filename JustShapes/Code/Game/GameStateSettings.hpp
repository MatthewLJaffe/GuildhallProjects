#pragma once
#include "Game/GameState.hpp"

class GameStateSettings : public GameState
{
public:
	GameStateSettings(GameStateType gameStateType);
	~GameStateSettings() override;
	void StartUp() override;
	void Render() override;
};

bool Event_IncreaseMasterButtonPressed(EventArgs& args);
bool Event_DecreaseMasterButtonPressed(EventArgs& args);

bool Event_IncreaseMusicButtonPressed(EventArgs& args);
bool Event_DecreaseMusicButtonPressed(EventArgs& args);

bool Event_IncreaseSFXButtonPressed(EventArgs& args);
bool Event_DecreaseSFXButtonPressed(EventArgs& args);

