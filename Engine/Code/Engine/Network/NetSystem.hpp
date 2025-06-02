#pragma once
#include "Engine/Core/EngineCommon.hpp"

constexpr uintptr_t INVALID_SOCKET = ~0ull;

enum class Mode
{
	NONE = 0,
	CLIENT,
	SERVER
};

enum class ServerState
{
	NONE = 0,
	BEGIN,
	LISTENING,
	CONNECTED,
};

enum class ClientState
{
	NONE = 0,
	BEGIN,
	CONNECTION_ATTEMPT_NOT_STARTED,
	ATTEMPTING_CONNECT,
	CONNECTED,
};

struct NetSystemConfig
{
	Mode m_mode = Mode::NONE;
	std::string m_hostAdressString = "127.0.0.1:27015";
	int m_sendBufferSize = 2048;
	int m_recvBufferSize = 2048;
};

class NetSystem
{
public:
	NetSystem(NetSystemConfig const& netSystemConfig);
	void StartUp();
	void Shutdown();
	void BeginFrame();
	void EndFrame();
private:
	void StartUpServer();
	void StartUpClient();
	void BeginFrameClient();
	void BeginFrameServer();
	void ShutDownClient();
	void ShutDownServer();
	void SendAndRecieveData();
	void ResetConnectionState();
public:
	NetSystemConfig m_config;
	std::vector<std::string> m_sendQueue;
	std::string m_recvQueue;

private:
	ServerState m_serverState = ServerState::NONE;
	ClientState m_clientState = ClientState::NONE;
	uintptr_t m_clientSocket = INVALID_SOCKET;
	uintptr_t m_listenSocket = INVALID_SOCKET;
	unsigned long m_hostAddress = 0;
	unsigned short m_hostPort = 0;
	char* m_sendBuffer = nullptr;
	char* m_recvBuffer = nullptr;
};