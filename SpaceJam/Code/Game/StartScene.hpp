#pragma once
#pragma once
#include "Game/Scene.hpp"
#include "Game/Widget.hpp"

enum class MenuState
{
	ATTRACT,
	MAIN,
	SETTINGS,
	CREDITS
};

class StartScene : public Scene
{
public:
	void StartUp() override;
	void Update(float deltaSeconds) override;
	void Render() const override;
	void ShutDown() override;
	std::vector<Widget*> m_attractWidgets;
	std::vector<Widget*> m_mainWidgets;
	std::vector<Widget*> m_settingsWidgets;
	std::vector<Widget*> m_creditsWidgets;
	MenuState m_menuState = MenuState::ATTRACT;
	std::vector<Widget*>& GetWidgetsForCurrentMenuState();
	std::vector<Widget*>const& GetWidgetsForCurrentMenuStateConst() const;

};