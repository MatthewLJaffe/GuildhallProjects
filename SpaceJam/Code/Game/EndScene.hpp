#pragma once
#pragma once
#pragma once
#include "Game/Scene.hpp"
#include "Game/Widget.hpp"



class EndScene : public Scene
{
public:
	virtual void StartUp() override;
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void ShutDown() override;
	std::vector<Widget*> m_winWidgets;
	std::vector<Widget*> m_loseWidgets;
	bool m_wonGame = false;
};