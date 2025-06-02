#pragma once
#include "Game/GameStateLevel.hpp"

class GameStateLevel3 : public GameStateLevel
{
public:
	GameStateLevel3(GameStateType gameType, SoundID levelMusic);
	void StartUp() override;
	void OnEnable() override;
};