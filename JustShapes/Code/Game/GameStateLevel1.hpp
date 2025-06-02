#pragma once
#include "Game/GameStateLevel.hpp"

class GameStateLevel1 : public GameStateLevel
{
public:
	GameStateLevel1(GameStateType gameType, SoundID levelMusic);
	void StartUp() override;
	void OnEnable() override;
};