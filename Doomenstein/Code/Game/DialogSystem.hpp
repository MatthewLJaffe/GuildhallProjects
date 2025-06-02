#pragma once
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/EventSystem.hpp"

class Texture;

struct DialogMessage
{
	DialogMessage(std::string message, SoundID m_typeWriterSound, float typeSpeed = 30.f, float advanceTime = -1.f);
	std::string m_message;
	SoundID m_typeWriterSound;
	Timer m_typeTimer;
	Timer m_advanceTimer;
	int m_soundFrequency = 2;
};

class DialogSequence
{
public:
	DialogSequence(std::string name);
	std::string m_name;
	std::vector<DialogMessage> m_dialogMessages;
	void Update();
	void AdvanceCurrentMessage();
	bool IncrementMessage();
	DialogMessage& GetCurrentMessage();
	void Start();
	std::string GetCurrentMessagePortion();
	bool IsMessageComplete();
	bool m_stopGame = false;
private:
	int m_currentMessageIndex = 0;
	int m_currentMessageChars = 0;
};

class DialogSystem
{
public:
	DialogSystem();
	DialogSequence* GetDialogSequenceByName(std::string name);
	void StartNewDialogue(std::string dialogueName);
	void AdvanceMessage();
	void AdvanceSequence();
	void Update();
	void Render();
	bool IsGameStopped();
	DialogSequence* GetCurrentDialogSequence();

	static bool Event_Start_Dialog(EventArgs& args);
private:
	void ShowNewMessage();
	std::vector<DialogSequence> m_allDialogSequences;
	DialogSequence* m_currentDialogueSequence;
	Texture* m_textBoxTexture = nullptr;
	AABB2 m_textBoxBounds;
};