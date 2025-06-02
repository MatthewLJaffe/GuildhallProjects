#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include <Vector>
#include <mutex>

typedef NamedProperties EventArgs;
typedef bool(*EventCallbackFunction)(EventArgs&);


class EventSystem;

extern EventSystem* g_theEventSystem;

struct EventSubscription
{
	EventSubscription(EventCallbackFunction funcitonPtr);
	EventCallbackFunction m_functionPtr;
};

class EventSubscriberBase
{
public:
	virtual bool Execute(EventArgs& args) = 0;
	virtual ~EventSubscriberBase() = default;
	void* m_ptrToObject = nullptr;
};


typedef std::vector<EventSubscriberBase*> SubscriptionList;


class EventSubscriberStandalone : public EventSubscriberBase
{
public:
	EventSubscriberStandalone(EventCallbackFunction funcitonPtr);
	virtual bool Execute(EventArgs& args) override;
	EventCallbackFunction m_functionPtr = nullptr;
};

template<typename T>
class EventSubscriberObjectMethod : public EventSubscriberBase
{
public:
	typedef bool(T::*EventCallbackObjectMethod)(EventArgs&);
	EventSubscriberObjectMethod(T& object, EventCallbackObjectMethod method) 
		: m_object(object)
		, m_objectMethod(method)
	{
		m_ptrToObject = (void*)&m_object;
	}
	virtual bool Execute(EventArgs& args) override { return (m_object.*m_objectMethod)(args); }

	T& m_object;
	EventCallbackObjectMethod m_objectMethod = nullptr;
};



struct EventSystemConfig
{

};

class EventSystem
{
public:
	EventSystem( EventSystemConfig const& config );
	~EventSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();
	void Execute(std::string const& command);
	void SubscribeEventCallbackFunction( std::string const& eventName, EventCallbackFunction functionPtr );
	void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
	void FireEvent (std::string const& eventName, EventArgs& args);
	void FireEvent(std::string const& eventName);
	int FireEventAndCountResponses(std::string const& eventName, EventArgs& args, bool& wasEventConsumed);
	std::vector<std::string> GetEventNames();

	template<typename T>
	void SubscribeEventCallbackObjectMethod(std::string const& eventName, T& object, bool(T::*objectMethodPtr)(EventArgs&))
	{
		std::string lowercaseEvent = ToLower(eventName);
		bool locked = m_eventSystemMutex.try_lock();
		for (int i = 0; i < (int)m_subscriptionListsByEventName[lowercaseEvent].size(); i++)
		{
			EventSubscriberBase* sub = m_subscriptionListsByEventName[lowercaseEvent][i];

			//don't subscribe a function twice
			EventSubscriberObjectMethod<T>* objectMethod = dynamic_cast<EventSubscriberObjectMethod<T>*>(sub);
			if (objectMethod != nullptr)
			{
				if (objectMethod->m_objectMethod == objectMethodPtr && &objectMethod->m_object == &object)
				{
					if (locked)
					{
						m_eventSystemMutex.unlock();
					}
					return;
				}
			}


		}
		m_subscriptionListsByEventName[lowercaseEvent].push_back(new EventSubscriberObjectMethod<T>(object, objectMethodPtr));
		m_eventNames.push_back(eventName);
		if (locked)
		{
			m_eventSystemMutex.unlock();
		}
	}

	template<typename T>
	void UnsubscribeEventObjectMethod(std::string const& eventName, T& object, bool(T::* objectMethodPtr)(EventArgs&))
	{
		std::string lowercaseEvent = ToLower(eventName);
		bool locked = m_eventSystemMutex.try_lock();
		SubscriptionList& listToUnsub = m_subscriptionListsByEventName[lowercaseEvent];
		for (size_t i = 0; i < listToUnsub.size(); i++)
		{
			EventSubscriberBase* base = listToUnsub[i];
			EventSubscriberObjectMethod<T>* objectMethod = dynamic_cast<EventSubscriberObjectMethod<T>*>(base);
			if (objectMethod != nullptr)
			{
				if (objectMethod->m_objectMethod == objectMethodPtr && &object == &objectMethod->m_object)
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

	template<typename T>
	void UnsubscribeAllEventsForObject(T& object)
	{
		bool locked = m_eventSystemMutex.try_lock();
		for (auto keyValuePair = m_subscriptionListsByEventName.begin(); keyValuePair != m_subscriptionListsByEventName.end(); keyValuePair++)
		{
			SubscriptionList& listToUnsub = keyValuePair->second;
			for (size_t i = 0; i < listToUnsub.size(); i++)
			{
				EventSubscriberBase* base = listToUnsub[i];
				if (base->m_ptrToObject == &object)
				{
					listToUnsub.erase(listToUnsub.begin() + i);
					i--;
				}
				
			}
		}
		if (locked)
		{
			m_eventSystemMutex.unlock();
		}
	}


protected:
	EventSystemConfig m_config;
	std::vector<std::string> m_eventNames;
	std::map<HCIS, SubscriptionList> m_subscriptionListsByEventName;
	std::mutex m_eventSystemMutex;
};

// Standalone global-namespace functions these forward to "the" event system
template <typename T>
void UnsubscribeAllEventsForObject(T& object)
{
	if (g_theEventSystem == nullptr)
	{
		return;
	}
	g_theEventSystem->UnsubscribeAllEventsForObject(object);
}

void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
void FireEvent(std::string const& eventName, EventArgs& args);
int FireEventAndCountResponses(std::string const& eventName, EventArgs& args, bool& wasEventConsumed);
void FireEvent(std::string const& eventName);

class EventRecipient
{
public:
	virtual ~EventRecipient() { UnsubscribeAllEventsForObject(*this); }
};
