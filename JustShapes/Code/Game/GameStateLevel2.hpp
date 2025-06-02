#pragma once
#include "Game/GameStateLevel.hpp"

class GameStateLevel2 : public GameStateLevel
{
public:
	GameStateLevel2(GameStateType gameType, SoundID levelMusic);
	void StartUp() override;
	void OnEnable() override;
};