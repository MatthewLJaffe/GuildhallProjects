#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Timer.hpp"

class ParticleEmitter;
class Entity;
class Player;
class Texture;
class BitmapFont;
class App;
class CPUMesh;
class GPUMesh;
class ParticleEffect;
class IndividualParticleEffect;
class ParticlePhysicsAABB3;
class Model;
class Prop;
class Scene;

class Game
{
public:
	Game(App* app);
	~Game();
	void StartUp();
	void Update(float deltaSeconds);
	void Render();
	void EndFrame();
	void ShutDown();
	Scene* GetCurrentScene();
	Player* m_player = nullptr;
	bool m_controllingPlayer = false;
	std::vector<Entity*> m_allEntities;

private:
	void AddGrid();
	void AddGroundPlane();
	void ToggleDebugDrawVectorField();
	void CheckForDebugCommands();
	void DeleteDeadEntities();
	void ShootBullet();
	void ShootBigBullet();
	Vec3 reconstructPos(Vec2 const& uv, float z, Mat44 const& inverseViewProj);
	float computeLinearDepth(float z);
	void AddTestCube();
	App* m_theApp;
	float m_attractAlpha = 0.f;
	float m_playRadius = 0.f;
	float m_currT = 0.0f;
	Texture* m_testTexture = nullptr;
	Camera m_screenCamera;
	BitmapFont* font;
	GPUMesh* m_debugArrowsGPU = nullptr;
	SoundPlaybackID m_musicPlayback;
	bool m_showAudioZoo = false;
	int m_sceneIndex = 0;
	const int m_numScenes = 5;
	Prop* m_groundPlane = nullptr;
	std::vector<Scene*> m_scenes;
	ParticlePhysicsAABB3* m_testWallCollider = nullptr;
	Texture* m_skyboxTexture = nullptr;
	void SwitchScenes(bool forward);
	Prop* m_grid = nullptr;
	
};

