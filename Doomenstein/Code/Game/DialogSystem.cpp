#include "Game/DialogSystem.hpp"
#include "Game/GameCommon.hpp"

DialogMessage::DialogMessage(std::string message, SoundID typeWriterSound, float typeSpeed, float advanceTime)
	: m_message(message)
	, m_typeWriterSound(typeWriterSound)
	, m_typeTimer(1.f/typeSpeed, g_theApp->GetGameClock())
	, m_advanceTimer(advanceTime, g_theApp->GetGameClock())
{
}

DialogSequence::DialogSequence(std::string name)
	: m_name(name)
{

}

DialogSystem::DialogSystem()
{
	float scale = 3.8f;
	m_textBoxTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TextBox.png");
	m_textBoxBounds.m_mins.x = GetScreenDimensions().x - 192 * scale;
	m_textBoxBounds.m_mins.y = 0;

	m_textBoxBounds.m_maxs.x = GetScreenDimensions().x;
	m_textBoxBounds.m_maxs.y = 64 * scale;

	g_theEventSystem->SubscribeEventCallbackFunction("StartDialog", DialogSystem::Event_Start_Dialog);

	DialogSequence intro("intro");
	intro.m_stopGame = true;
	intro.m_dialogMessages.push_back(DialogMessage("Welcome to my Doomenstin Gold!", SOUND_ID_TYPEWRITER_1));
	intro.m_dialogMessages.push_back(DialogMessage("Butler if you're playing this I went ahead and graded my assignment for you", SOUND_ID_TYPEWRITER_1));
	intro.m_dialogMessages.push_back(DialogMessage("I know that you're pretty busy with end of semester grades so I figured I would do my part and make grading this one easy on you", SOUND_ID_TYPEWRITER_1));
	intro.m_dialogMessages.push_back(DialogMessage("I went ahead and gave myself a 90. I think that's pretty fair.", SOUND_ID_TYPEWRITER_1));
	intro.m_dialogMessages.push_back(DialogMessage("Don't get me wrong, it's not the best Gold in the world, but everything is working as intended,", SOUND_ID_TYPEWRITER_1));
	intro.m_dialogMessages.push_back(DialogMessage("so I can't imagine there will be anything to take off points for", SOUND_ID_TYPEWRITER_1));
	intro.m_dialogMessages.push_back(DialogMessage("So go ahead, no need to stick around here, you can just give me a 90 and be on your way", SOUND_ID_TYPEWRITER_1));
	intro.m_dialogMessages.push_back(DialogMessage("...", SOUND_ID_TYPEWRITER_1));
	intro.m_dialogMessages.push_back(DialogMessage(".......", SOUND_ID_TYPEWRITER_1));
	intro.m_dialogMessages.push_back(DialogMessage(".............................", SOUND_ID_TYPEWRITER_1));
	intro.m_dialogMessages.push_back(DialogMessage("Damn I really thought that was going to work.", SOUND_ID_TYPEWRITER_1));
	intro.m_dialogMessages.push_back(DialogMessage("I mean no problem! Yeah go and head and stick around and I will show you my assignment.", SOUND_ID_TYPEWRITER_1));
	intro.m_dialogMessages.push_back(DialogMessage("Nothing to hide here, haha.", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(intro);

	DialogSequence flamethrower("flamethrower");
	flamethrower.m_dialogMessages.push_back(DialogMessage("This is my flamethrower. It even temporarily leaves behind flames that light the ground up.", SOUND_ID_TYPEWRITER_1));
	flamethrower.m_dialogMessages.push_back(DialogMessage("Go on, torch some demons.", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(flamethrower);

	DialogSequence pointLights1("pointLights1");
	pointLights1.m_stopGame = true;
	pointLights1.m_dialogMessages.push_back(DialogMessage("Huh that's strange...", SOUND_ID_TYPEWRITER_1));
	pointLights1.m_dialogMessages.push_back(DialogMessage("Those flames on the ground are supposed to extinguish after some time.", SOUND_ID_TYPEWRITER_1));
	pointLights1.m_dialogMessages.push_back(DialogMessage("That's ok. I mean it looks cooler this way. No harm in leaving a couple extra point lights around, right?", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(pointLights1);

	DialogSequence pointLights2("pointLights2");
	pointLights2.m_stopGame = true;
	pointLights2.m_dialogMessages.push_back(DialogMessage("Ok that's a lot of point lights maybe you should stop spawning those.", SOUND_ID_TYPEWRITER_1));
	pointLights2.m_dialogMessages.push_back(DialogMessage("I mean I'm sure my game can handle it.", SOUND_ID_TYPEWRITER_1));
	pointLights2.m_dialogMessages.push_back(DialogMessage("Not a big deal haha.", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(pointLights2);

	DialogSequence pointLights3("pointLights3");
	pointLights3.m_stopGame = true;
	pointLights3.m_dialogMessages.push_back(DialogMessage("QUIT IT WITH THOSE POINT LIGHTS!", SOUND_ID_TYPEWRITER_1));
	pointLights3.m_dialogMessages.push_back(DialogMessage("WHAT ARE YOU A LEVEL DESIGNER?", SOUND_ID_TYPEWRITER_1));
	pointLights3.m_dialogMessages.push_back(DialogMessage("STOP LAGGING MY GAME", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(pointLights3);

	DialogSequence pointLights4("pointLights4");
	pointLights4.m_stopGame = true;
	pointLights4.m_dialogMessages.push_back(DialogMessage("OH GOD THE FRAMES", SOUND_ID_TYPEWRITER_1));
	pointLights4.m_dialogMessages.push_back(DialogMessage("ALRIGHT I'M GETTING RID OF THESE DAMN POINT LIGHTS", SOUND_ID_TYPEWRITER_1));
	pointLights4.m_dialogMessages.push_back(DialogMessage("AND NO MORE FLAME THROWER FOR YOU!", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(pointLights4);

	DialogSequence wallsDestroyed("wallsDestroyed");
	wallsDestroyed.m_dialogMessages.push_back(DialogMessage("Wait... why did those walls just disappear?", SOUND_ID_TYPEWRITER_1));
	wallsDestroyed.m_dialogMessages.push_back(DialogMessage("I mean that's fine maybe don't go over there ", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(wallsDestroyed);

	DialogSequence indexOutOfBounds("indexOutOfBounds");
	indexOutOfBounds.m_stopGame = true;
	indexOutOfBounds.m_dialogMessages.push_back(DialogMessage("Oh god... this is not going well", SOUND_ID_TYPEWRITER_1));
	indexOutOfBounds.m_dialogMessages.push_back(DialogMessage("That's ok I can bring this back", SOUND_ID_TYPEWRITER_1));
	indexOutOfBounds.m_dialogMessages.push_back(DialogMessage("Let's go to another map, still plenty of good stuff to show off in this assignment.",
		SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(indexOutOfBounds);

	DialogSequence binkeyHurt("binkeyHurt");
	binkeyHurt.m_dialogMessages.push_back(DialogMessage("Binkey: Owww why did you do that, that hurt!", SOUND_ID_TYPEWRITER_1));
	binkeyHurt.m_dialogMessages.push_back(DialogMessage("Matthew: Don't shoot at that creature! That's Binkey your friendly AI companion", SOUND_ID_TYPEWRITER_1));
	binkeyHurt.m_dialogMessages.push_back(DialogMessage("Matthew: Binkey's intelligent AI allows him to perform all sorts of neat tricks.", SOUND_ID_TYPEWRITER_1));
	binkeyHurt.m_dialogMessages.push_back(DialogMessage("Matthew: Binkey, show our friend here something cool.", SOUND_ID_TYPEWRITER_1));
	binkeyHurt.m_dialogMessages.push_back(DialogMessage("Binkey: But I wasn't programmed to do anything coo-", SOUND_ID_TYPEWRITER_1));
	binkeyHurt.m_dialogMessages.push_back(DialogMessage("Matthew: He's just being modest!", SOUND_ID_TYPEWRITER_1));
	binkeyHurt.m_dialogMessages.push_back(DialogMessage("Matthew: show off your path finding capabilities Binkey!", SOUND_ID_TYPEWRITER_1));
	binkeyHurt.m_dialogMessages.push_back(DialogMessage("Now watch as Binkey navigates the maze in front of you.", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(binkeyHurt);

	DialogSequence binkeyAlerted("binkeyAlerted");
	binkeyAlerted.m_dialogMessages.push_back(DialogMessage("Binkey: Hey there I'm Binkey, a friendly AI companion!", SOUND_ID_TYPEWRITER_1));
	binkeyAlerted.m_dialogMessages.push_back(DialogMessage("Matthew: Binkey's intelligent AI allows him to perform all sorts of neat tricks.", SOUND_ID_TYPEWRITER_1));
	binkeyAlerted.m_dialogMessages.push_back(DialogMessage("Matthew: Binkey, show our friend here something cool.", SOUND_ID_TYPEWRITER_1));
	binkeyAlerted.m_dialogMessages.push_back(DialogMessage("Binkey: But I wasn't programmed to do anything coo-", SOUND_ID_TYPEWRITER_1));
	binkeyAlerted.m_dialogMessages.push_back(DialogMessage("Matthew: HE'S JUST BEING MODEST!", SOUND_ID_TYPEWRITER_1));
	binkeyAlerted.m_dialogMessages.push_back(DialogMessage("Matthew: show off your path finding capabilities Binkey!", SOUND_ID_TYPEWRITER_1));
	binkeyAlerted.m_dialogMessages.push_back(DialogMessage("Now watch as Binkey navigates the maze in front of you.", SOUND_ID_TYPEWRITER_1));
	binkeyAlerted.m_dialogMessages.push_back(DialogMessage("Binkey: Ok, I'm going to give it my best shot", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(binkeyAlerted);

	DialogSequence binkeyRunIntoWall1("binkeyRunIntoWall");
	binkeyRunIntoWall1.m_dialogMessages.push_back(DialogMessage("...", SOUND_ID_TYPEWRITER_1));
	binkeyRunIntoWall1.m_dialogMessages.push_back(DialogMessage("Matthew: Binkey what are you doing?", SOUND_ID_TYPEWRITER_1));
	binkeyRunIntoWall1.m_dialogMessages.push_back(DialogMessage("Binkey: Hold on... I think I've almost got it", SOUND_ID_TYPEWRITER_1));
	binkeyRunIntoWall1.m_dialogMessages.push_back(DialogMessage("Matthew: No Binkey you do not almost have it, you're running straight into a wall.", SOUND_ID_TYPEWRITER_1));
	binkeyRunIntoWall1.m_dialogMessages.push_back(DialogMessage("Binkey: A wall? What's that?", SOUND_ID_TYPEWRITER_1));
	binkeyRunIntoWall1.m_dialogMessages.push_back(DialogMessage("Matthew: Binkey you need to turn left to avoid the wall.", SOUND_ID_TYPEWRITER_1));
	binkeyRunIntoWall1.m_dialogMessages.push_back(DialogMessage("Binkey: Nah I don't think so, my goal is in front of me so I just need to keep walking towards it.", SOUND_ID_TYPEWRITER_1));
	binkeyRunIntoWall1.m_dialogMessages.push_back(DialogMessage("Matthew: Binkey that's not going to work. Remember the heat map thing we talked about?", SOUND_ID_TYPEWRITER_1));
	binkeyRunIntoWall1.m_dialogMessages.push_back(DialogMessage("Matthew: Walk towards the lowest heat tile, to your LEFT.", SOUND_ID_TYPEWRITER_1));
	binkeyRunIntoWall1.m_dialogMessages.push_back(DialogMessage("Binkey: That sounds too complicated, I think I have a pretty solid plan here.", SOUND_ID_TYPEWRITER_1));
	binkeyRunIntoWall1.m_dialogMessages.push_back(DialogMessage("Binkey: I'm just going to keep doing this until it works out.", SOUND_ID_TYPEWRITER_1));
	binkeyRunIntoWall1.m_dialogMessages.push_back(DialogMessage("Matthew: OH FOR THE LOVE OF GOD!", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(binkeyRunIntoWall1);

	DialogSequence binkeyRespawn("binkeyRespawn");
	binkeyRespawn.m_dialogMessages.push_back(DialogMessage("Matthew: Look he got it. He's so fast you looked away for one second and he just pathed his way right through there.", SOUND_ID_TYPEWRITER_1));
	binkeyRespawn.m_dialogMessages.push_back(DialogMessage("Matthew: Haha...", SOUND_ID_TYPEWRITER_1));
	binkeyRespawn.m_dialogMessages.push_back(DialogMessage("Binkey: Did you just kill me?", SOUND_ID_TYPEWRITER_1));
	binkeyRespawn.m_dialogMessages.push_back(DialogMessage("Matthew: ...", SOUND_ID_TYPEWRITER_1));
	binkeyRespawn.m_dialogMessages.push_back(DialogMessage("Matthew: Anyways, Binkey is also a fearsome warrior.", SOUND_ID_TYPEWRITER_1));
	binkeyRespawn.m_dialogMessages.push_back(DialogMessage("Matthew: Watch him make quick work of these demons.", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(binkeyRespawn);

	DialogSequence binkeyHelp("binkeyHelp");
	binkeyHelp.m_dialogMessages.push_back(DialogMessage("Binkey: SOMBODY HELP ME!", SOUND_ID_TYPEWRITER_1, 30.f, 1.f));
	m_allDialogSequences.push_back(binkeyHelp);

	DialogSequence binkeyAgony("binkeyAgony");
	binkeyAgony.m_dialogMessages.push_back(DialogMessage("Binkey: OH THE AGONY!", SOUND_ID_TYPEWRITER_1, 30.f, 1.f));
	m_allDialogSequences.push_back(binkeyAgony);

	DialogSequence binkeyStop("binkeyStop");
	binkeyStop.m_dialogMessages.push_back(DialogMessage("Binkey: MAKE IT STOP MAKE IT STOP!!", SOUND_ID_TYPEWRITER_1, 30.f, 1.f));
	m_allDialogSequences.push_back(binkeyStop);

	DialogSequence binkeyRap("binkeyRap");
	binkeyRap.m_dialogMessages.push_back(DialogMessage("Matthew: Ok look that was pretty pathetic I'm not going to lie.", SOUND_ID_TYPEWRITER_1));
	binkeyRap.m_dialogMessages.push_back(DialogMessage("Matthew: But there is one one last trick Binkey can do.", SOUND_ID_TYPEWRITER_1));
	binkeyRap.m_dialogMessages.push_back(DialogMessage("Matthew: He can rap.", SOUND_ID_TYPEWRITER_1));
	binkeyRap.m_dialogMessages.push_back(DialogMessage("Matthew: Go on Binkey just like we practiced", SOUND_ID_TYPEWRITER_1));
	binkeyRap.m_dialogMessages.push_back(DialogMessage("Binkey: Ahem...", SOUND_ID_TYPEWRITER_1, 15.f));
	m_allDialogSequences.push_back(binkeyRap);

	DialogSequence shrinkRayIntro("shrinkRayIntro");
	shrinkRayIntro.m_dialogMessages.push_back(DialogMessage("Oh come on that was funny", SOUND_ID_TYPEWRITER_1));
	shrinkRayIntro.m_dialogMessages.push_back(DialogMessage("Alright things are looking pretty bleak for my grade", SOUND_ID_TYPEWRITER_1));
	shrinkRayIntro.m_dialogMessages.push_back(DialogMessage("But I do have one last attempt at getting some points back", SOUND_ID_TYPEWRITER_1));
	shrinkRayIntro.m_dialogMessages.push_back(DialogMessage("Let's load up the next map and I'll show you", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(shrinkRayIntro);

	DialogSequence shrinkRay2("shrinkRay2");
	shrinkRay2.m_stopGame = true;
	shrinkRay2.m_dialogMessages.push_back(DialogMessage("Look It's a Shrink Ray", SOUND_ID_TYPEWRITER_1));
	shrinkRay2.m_dialogMessages.push_back(DialogMessage("What a cool and interesting weapon to have implemented for this assignment", SOUND_ID_TYPEWRITER_1));
	shrinkRay2.m_dialogMessages.push_back(DialogMessage("Surely this will keep me from failing", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(shrinkRay2);

	DialogSequence shrinkRay3("shrinkRay3");
	shrinkRay3.m_stopGame = true;
	shrinkRay3.m_dialogMessages.push_back(DialogMessage("Probably best to just admire it from here", SOUND_ID_TYPEWRITER_1));
	shrinkRay3.m_dialogMessages.push_back(DialogMessage("I mean sure you could pick it up by why bother?", SOUND_ID_TYPEWRITER_1));
	shrinkRay3.m_dialogMessages.push_back(DialogMessage("It's a Shrink Ray you know what that does.", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(shrinkRay3);

	DialogSequence shrinkRay4("shrinkRay4");
	shrinkRay4.m_dialogMessages.push_back(DialogMessage("Ok maybe I didn't have time to implement the Shrink Ray", SOUND_ID_TYPEWRITER_1));
	shrinkRay4.m_dialogMessages.push_back(DialogMessage("But what's so wrong with shotguns? Those are cool too.", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(shrinkRay4);

	DialogSequence fail("fail");
	fail.m_dialogMessages.push_back(DialogMessage("Damn looks like I failed this assignment", SOUND_ID_TYPEWRITER_1));
	fail.m_dialogMessages.push_back(DialogMessage("Curse you Butler!", SOUND_ID_TYPEWRITER_1));
	m_allDialogSequences.push_back(fail);
}	


DialogSequence* DialogSystem::GetDialogSequenceByName(std::string name)
{
	for (size_t i = 0; i < m_allDialogSequences.size(); i++)
	{
		if (m_allDialogSequences[i].m_name == name)
		{
			return &m_allDialogSequences[i];
		}
	}
	ERROR_RECOVERABLE(Stringf("Cound not find dialog with name %s", name.c_str()));
	return nullptr;
}

void DialogSystem::StartNewDialogue(std::string dialogueName)
{
	m_currentDialogueSequence = GetDialogSequenceByName(dialogueName);
	if (m_currentDialogueSequence != nullptr)
	{
		m_currentDialogueSequence->Start();
	}
}

void DialogSystem::ShowNewMessage()
{
	if (m_currentDialogueSequence == nullptr)
	{
		return;
	}
	if (m_currentDialogueSequence->IncrementMessage() == false)
	{
		m_currentDialogueSequence = nullptr;
	}
}

void DialogSystem::AdvanceMessage()
{
	if (m_currentDialogueSequence == nullptr)
	{
		return;
	}
	if (m_currentDialogueSequence->IsMessageComplete())
	{
		ShowNewMessage();
	}
	else
	{
		m_currentDialogueSequence->AdvanceCurrentMessage();
	}

}

void DialogSystem::AdvanceSequence()
{
	if (m_currentDialogueSequence == nullptr)
	{
		return;
	}
	for (size_t i = 0; i < m_currentDialogueSequence->m_dialogMessages.size(); i++)
	{
		m_currentDialogueSequence->m_dialogMessages[i].m_typeTimer.Stop();
	}
	m_currentDialogueSequence = nullptr;
}

void DialogSystem::Update()
{
	if (m_currentDialogueSequence != nullptr)
	{
		m_currentDialogueSequence->Update();

		if (m_currentDialogueSequence->GetCurrentMessage().m_advanceTimer.HasPeriodElapsed())
		{
			ShowNewMessage();
		}
	}
}

void DialogSystem::Render()
{
	if (m_currentDialogueSequence == nullptr)
	{
		return;
	}

	std::vector<Vertex_PCU> textBoxVerts;
	AddVertsForAABB2D(textBoxVerts, m_textBoxBounds, Rgba8::WHITE);
	g_theRenderer->BindTexture(m_textBoxTexture);
	g_theRenderer->DrawVertexArray(textBoxVerts.size(), textBoxVerts.data());

	std::vector<Vertex_PCU> textVerts;
	g_bitMapFont->AddVertsForTextInBox2D(textVerts, m_textBoxBounds, 20.f, m_currentDialogueSequence->GetCurrentMessagePortion(), 
		Rgba8::WHITE, 1.f, Vec2(0.f, 1.f), WRAP_WORDS, 99999, m_textBoxBounds.GetDimensions() * .1f);

	g_theRenderer->BindTexture(g_bitMapFont->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts.size(), textVerts.data());
}

bool DialogSystem::IsGameStopped()
{
	return m_currentDialogueSequence != nullptr && m_currentDialogueSequence->m_stopGame;
}

DialogSequence* DialogSystem::GetCurrentDialogSequence()
{
	return m_currentDialogueSequence;
}

bool DialogSystem::Event_Start_Dialog(EventArgs& args)
{
	std::string dialogName = args.GetValue("name", "");
	if (dialogName == "")
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Invocation: StartDialog name={dialogName}");
	}
	else
	{
		g_dialogSystem->StartNewDialogue(dialogName);
	}
	return true;
}


void DialogSequence::Update()
{
	DialogMessage& currMesage = m_dialogMessages[m_currentMessageIndex];
	if (currMesage.m_typeTimer.HasPeriodElapsed())
	{
		if (m_currentMessageChars < (int)currMesage.m_message.size())
		{
			m_currentMessageChars++;
			if (m_currentMessageChars % currMesage.m_soundFrequency == 0)
			{
				g_theAudio->StartSound(currMesage.m_typeWriterSound, false, 1.f, 0.f, g_randGen->RollRandomFloatInRange(.9f, 1.1f));
			}
			currMesage.m_typeTimer.Start();
		}
		else
		{
			currMesage.m_typeTimer.Stop();
		}
	}
}

void DialogSequence::AdvanceCurrentMessage()
{
	DialogMessage const& currMessage = m_dialogMessages[m_currentMessageIndex];
	m_currentMessageChars = (int)currMessage.m_message.size();
}

bool DialogSequence::IncrementMessage()
{
	m_currentMessageChars = 0;
	m_dialogMessages[m_currentMessageIndex].m_typeTimer.Stop();
	m_dialogMessages[m_currentMessageIndex].m_advanceTimer.Stop();

	m_currentMessageIndex++;
	if (m_currentMessageIndex == (int)m_dialogMessages.size())
	{
		m_currentMessageIndex = 0;
		return false;
	}

	m_dialogMessages[m_currentMessageIndex].m_typeTimer.Start();
	if (m_dialogMessages[m_currentMessageIndex].m_advanceTimer.m_period > 0)
	{
		m_dialogMessages[m_currentMessageIndex].m_advanceTimer.Start();
	}
	return true;
}

DialogMessage& DialogSequence::GetCurrentMessage()
{
	return m_dialogMessages[m_currentMessageIndex];
}

void DialogSequence::Start()
{
	m_currentMessageIndex = 0;
	m_currentMessageChars = 0;
	m_dialogMessages[m_currentMessageIndex].m_typeTimer.Start();
	if (m_dialogMessages[m_currentMessageIndex].m_advanceTimer.m_period > 0)
	{
		m_dialogMessages[m_currentMessageIndex].m_advanceTimer.Start();
	}

}

std::string DialogSequence::GetCurrentMessagePortion()
{
	std::string messagePortion = GetCurrentMessage().m_message.substr(0, m_currentMessageChars);
	//adding padding to the last word so that it ends up in the correct place
	for (int i = m_currentMessageChars; i < (int)GetCurrentMessage().m_message.size() && GetCurrentMessage().m_message[i] != ' '; i++)
	{
		messagePortion += "#";
	}
	return messagePortion;
}

bool DialogSequence::IsMessageComplete()
{
	return (int)GetCurrentMessage().m_message.size() == m_currentMessageChars;
}
