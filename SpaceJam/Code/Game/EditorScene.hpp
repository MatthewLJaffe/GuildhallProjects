#pragma once
#include "Game/Scene.hpp"

class EditorScene : public Scene 
{
public:
	void StartUp() override;
	void Update(float deltaSeconds) override;
	void Render() const override;
	void DeleteDeadEntities();
	void ShutDown() override;
	std::vector<Entity*> m_allEntities;
};