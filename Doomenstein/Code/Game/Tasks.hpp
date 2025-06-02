#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#pragma once
class Task
{
public:
	Task(std::string name);
	std::string m_name;
	virtual void OnStart() = 0;
	virtual bool Tick() = 0;
	virtual void OnEnd() = 0;
	virtual ~Task();
};

class Task1_Start : public Task
{
public:
	Task1_Start(std::string name);
	void OnStart() override;
	bool Tick() override;
	void OnEnd() override;
};

class Task2_SpawnDemons : public Task
{
public:
	Task2_SpawnDemons(std::string name);
	void OnStart() override;
	bool Tick() override;
	void OnEnd() override;
	Timer m_demonSpawnTimer;

private:
	std::string pointLightMessage1 = "pointLights1";
	bool m_pointlightMessage1Started = false;
	int m_pointlightMessage1Threshold = 100;

	std::string pointLightMessage2 = "pointLights2";
	bool m_pointlightMessage2Started = false;
	int m_pointlightMessage2Threshold = 150;

	std::string pointLightMessage3 = "pointLights3";
	bool m_pointlightMessage3Started = false;
	int m_pointlightMessage3Threshold = 250;

	std::string pointLightMessage4 = "pointLights4";
	bool m_pointlightMessage4Started = false;
	int m_pointlightMessage4Threshold = 350;
};

class Task3_RemovePointLights : public Task
{
public:
	Task3_RemovePointLights(std::string name);
	void OnStart() override;
	bool Tick() override;
	void OnEnd() override;
};

class Task4_OutOfBoundsError : public Task
{
public:
	Task4_OutOfBoundsError(std::string name);
	void OnStart() override;
	bool Tick() override;
	void OnEnd() override;
	bool m_errorDialogStarted = false;
};

class Task5_CheckForBinkeyAlerted : public Task
{
public:
	Task5_CheckForBinkeyAlerted(std::string name);
	void OnStart() override;
	bool Tick() override;
	void OnEnd() override;
	bool m_binkeyHurt = false;
	bool m_binkeyAlerted = false;
	bool m_binkeyDialogStarted = false;
};

class Task6_BinkeyRunIntoWall : public Task
{
public:
	Task6_BinkeyRunIntoWall(std::string name);
	void OnStart() override;
	bool Tick() override;
	void OnEnd() override;
	bool m_holdW = false;
	Timer m_convo1Timer;
	bool dialogStarted = false;
};

class Task7_BinkeyRespawn : public Task
{
public:
	Task7_BinkeyRespawn(std::string name);
	void OnStart() override;
	bool Tick() override;
	void OnEnd() override;
	bool m_respawnConvoStared = false;
};

class Task8_BinkeyCombat : public Task
{
public:
	Task8_BinkeyCombat(std::string name);
	void OnStart() override;
	bool Tick() override;
	void OnEnd() override;
	Timer m_messageTimer;
};

class Task9_BinkeyRap : public Task
{
public:
	Task9_BinkeyRap(std::string name);
	void OnStart() override;
	bool Tick() override;
	void OnEnd() override;
	bool m_endRap = false;
	bool m_rapStarted = false;
	Timer m_rapTimer;
	SoundPlaybackID m_rap;
};

class Task10_ShrinkRayIntro : public Task
{
public:
	Task10_ShrinkRayIntro(std::string name);
	void OnStart() override;
	bool Tick() override;
	void OnEnd() override;
};


class Task11_ShrinkRayEnd : public Task
{
public:
	Task11_ShrinkRayEnd(std::string name);
	void OnStart() override;
	bool Tick() override;
	void OnEnd() override;
	bool m_warningConvoStarted = false;
	bool m_shrinkRayEquipped = false;
	bool m_lastShrinkRayConvoStarted = false;
};

class Task12_FOOM : public Task
{
public:
	Task12_FOOM(std::string name);
	void OnStart() override;
	bool Tick() override;
	void OnEnd() override;
	Timer m_failTimer;
};