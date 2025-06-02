#pragma once
#include "Game/GameState.hpp"

class GameStateHowToPlay: public GameState
{
public:
	GameStateHowToPlay(GameStateType gameStateType);
	~GameStateHowToPlay() override;
	void StartUp() override;
	void Render() override;
};


