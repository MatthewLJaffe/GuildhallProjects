#include "EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

EventSystem* g_theEventSystem = nullptr;


EventSubscription::EventSubscription(EventCallbackFunction funcitonPtr)
	: m_functionPtr(funcitonPtr)
{

}

EventSystem::EventSystem(EventSystemConfig const& config)
	: m_config(config)
{
}

EventSystem::~EventSystem()
{
}

void EventSystem::Startup()
{
}

void EventSystem::Shutdown()
{
}

void EventSystem::BeginFrame()
{
}

void EventSystem::EndFrame()
{
}

void EventSystem::Execute(std::string const& consoleCommandText)
{
	EventArgs eventArgs;
	Strings commands = SplitStringOnDelimiter(consoleCommandText, '\n');
	for (size_t commandIdx = 0; commandIdx < commands.size(); commandIdx++)
	{
		Strings commandArgs = SplitStringWithQuotes(commands[commandIdx], ' ');
		std::string eventName = ToLower(commandArgs[0]);
		for (size_t argsIdx = 1; argsIdx < commandArgs.size(); argsIdx++)
		{
			Strings argKeyValue = SplitStringWithQuotes(commandArgs[argsIdx], '=');
			if (argKeyValue.size() == 2)
			{
				std::string value = argKeyValue[1];
				TrimString(value, '\"');
				eventArgs.SetValue(ToLower(argKeyValue[0]), value);
			}
		}
		bool wasConsumed = false;
		int numResponses = FireEventAndCountResponses(eventName, eventArgs, wasConsumed);

		if (numResponses == 0)
		{
			ERROR_RECOVERABLE("ERROR: unknown command " + consoleCommandText);
		}
	}
}

void EventSystem::SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	std::string lowercaseEvent = ToLower(eventName);
	bool locked = m_eventSystemMutex.try_lock();
	for (int i = 0; i < (int)m_subscriptionListsByEventName[lowercaseEvent].size(); i++)
	{
		EventSubscriberBase* sub = m_subscriptionListsByEventName[lowercaseEvent][i];

		//don't subscribe a function twice
		EventSubscriberStandalone* standalone = dynamic_cast<EventSubscriberStandalone*>(sub);
		if (standalone != nullptr)
		{
			if (standalone->m_functionPtr == functionPtr)
			{
				if (locked)
				{
					m_eventSystemMutex.unlock();
				}
				return;
			}
		}

	
	}
	m_subscriptionListsByEventName[lowercaseEvent].push_back( new EventSubscriberStandalone(functionPtr));
	m_eventNames.push_back(eventName);
	if (locked)
	{
		m_eventSystemMutex.unlock();
	}
}

void EventSystem::UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	
	std::string lowercaseEvent = ToLower(eventName);
	bool locked = m_eventSystemMutex.try_lock();
	SubscriptionList& listToUnsub = m_subscriptionListsByEventName[lowercaseEvent];
	for (size_t i = 0; i < listToUnsub.size(); i++)
	{
		EventSubscriberBase* base = listToUnsub[i];
		EventSubscriberStandalone* standalone = dynamic_cast<EventSubscriberStandalone*>(base);
		if (standalone != nullptr)
		{
			if (standalone->m_functionPtr == functionPtr)
			{
				if (locked)
				{
					m_eventSystemMutex.unlock();
				}
				listToUnsub.erase(listToUnsub.begin() + i);
				return;
			}
		}		
	}
	if (locked)
	{
		m_eventSystemMutex.unlock();
	}
	
}

void EventSystem::FireEvent(std::string const& eventName, EventArgs& args)
{
	std::string lowercaseEvent = ToLower(eventName);
	bool locked = m_eventSystemMutex.try_lock();
	SubscriptionList const& listToFire = m_subscriptionListsByEventName[lowercaseEvent];
	for (size_t i = 0; i < listToFire.size(); i++)
	{
		if (listToFire[i]->Execute(args))
		{
			break;
		}
	}
	if (locked)
	{
		m_eventSystemMutex.unlock();
	}
}

int EventSystem::FireEventAndCountResponses(std::string const& eventName, EventArgs& args, bool& wasEventConsumed)
{
	std::string lowercaseEvent = ToLower(eventName);
	bool locked = m_eventSystemMutex.try_lock();
	int functionsCalled = 0;
	SubscriptionList const& listToFire = m_subscriptionListsByEventName[lowercaseEvent];
	for (size_t i = 0; i < listToFire.size(); i++)
	{
		functionsCalled++;
		if (listToFire[i]->Execute(args))
		{
			wasEventConsumed = true;
			if (locked)
			{
				m_eventSystemMutex.unlock();
			}
			return functionsCalled;
		}
	}
	wasEventConsumed = false;
	if (locked)
	{
		m_eventSystemMutex.unlock();
	}
	return functionsCalled;
}

std::vector<std::string> EventSystem::GetEventNames()
{
	return m_eventNames;
}

void EventSystem::FireEvent(std::string const& eventName)
{
	std::string lowercaseEvent = ToLower(eventName);
	EventArgs defaultArgs;
	FireEvent(lowercaseEvent, defaultArgs);
}

void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	std::string lowercaseEvent = ToLower(eventName);
	if (g_theEventSystem == nullptr)
	{
		return;
	}
	g_theEventSystem->SubscribeEventCallbackFunction(lowercaseEvent, functionPtr);
}

void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	std::string lowercaseEvent = ToLower(eventName);
	if (g_theEventSystem == nullptr)
	{
		return;
	}
	g_theEventSystem->UnsubscribeEventCallbackFunction(lowercaseEvent, functionPtr);
}

void FireEvent(std::string const& eventName, EventArgs& args)
{
	std::string lowercaseEvent = ToLower(eventName);
	if (g_theEventSystem == nullptr)
	{
		return;
	}
	g_theEventSystem->FireEvent(lowercaseEvent, args);
}

int FireEventAndCountResponses(std::string const& eventName, EventArgs& args, bool& wasEventConsumed)
{
	std::string lowercaseEvent = ToLower(eventName);
	if (g_theEventSystem == nullptr)
	{
		return 0;
	}
	return g_theEventSystem->FireEventAndCountResponses(lowercaseEvent, args, wasEventConsumed);
}

void FireEvent(std::string const& eventName)
{
	std::string lowercaseEvent = ToLower(eventName);
	if (g_theEventSystem == nullptr)
	{
		return;
	}
	g_theEventSystem->FireEvent(lowercaseEvent);
}

EventSubscriberStandalone::EventSubscriberStandalone(EventCallbackFunction funcitonPtr)
	: m_functionPtr(funcitonPtr)
{
}

bool EventSubscriberStandalone::Execute(EventArgs& args)
{
	return m_functionPtr(args);
}
