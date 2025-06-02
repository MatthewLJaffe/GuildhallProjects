#pragma once
#include "Game/GameState.hpp"

class GameStateMainMenu : public GameState
{
	public:
		GameStateMainMenu(GameStateType gameStateType);
		~GameStateMainMenu() override;
		void StartUp() override;
		void Render() override;
		void OnEnable() override;
};

bool Event_PlayButtonPressed(EventArgs& args);
bool Event_HowToPlayButtonPressed(EventArgs& args);
bool Event_SettingsButtonPressed(EventArgs& args);
bool Event_BackButtonPressed(EventArgs& args);
