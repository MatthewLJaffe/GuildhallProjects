#include "Engine/Network/NetSystem.hpp"
NetSystem* g_theNetSystem = nullptr;

#if !defined( ENGINE_DISABLE_NETWORK )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")



NetSystem::NetSystem(NetSystemConfig const& netSystemConfig)
	: m_config(netSystemConfig)
{

}

void NetSystem::StartUp()
{
	if (m_config.m_mode == Mode::SERVER)
	{
		StartUpServer();
	}
	if (m_config.m_mode == Mode::CLIENT)
	{
		StartUpClient();
	}
	m_sendBuffer = new char[m_config.m_sendBufferSize];
	m_recvBuffer = new char[m_config.m_recvBufferSize];
}

void NetSystem::StartUpServer()
{
	m_serverState = ServerState::BEGIN;
	//Startup Windows Sockets.
	WSADATA data;
	int result = WSAStartup(MAKEWORD(2, 2), &data);
	if (result == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		ERROR_RECOVERABLE(Stringf("WSAStartup error %d", errorCode));
	}

	//Create a listen socket.
	m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Set the listen socket to non-blocking.
	unsigned long blockingMode = 1;
	result = ioctlsocket(m_listenSocket, FIONBIO, &blockingMode);
	if (result == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		ERROR_RECOVERABLE(Stringf("WSAStartup error %d", errorCode));
	}

	//Bind the listen socket to a port.
	Strings hostAddrSplit = SplitStringOnDelimiter(m_config.m_hostAdressString, ':');

	m_hostAddress = INADDR_ANY;
	m_hostPort = (unsigned short)(atoi(hostAddrSplit[1].c_str()));
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = htonl(m_hostAddress);
	addr.sin_port = htons(m_hostPort);
	result = bind(m_listenSocket, (sockaddr*)&addr, (int)sizeof(addr));
	if (result == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		ERROR_RECOVERABLE(Stringf("WSAStartup error %d", errorCode));
	}

	//Tell the listen socket to start listening for connections to accept.
	result = listen(m_listenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSAEWOULDBLOCK)
		{
			ERROR_RECOVERABLE(Stringf("WSAStartup error %d", errorCode));
		}
	}
	m_serverState = ServerState::LISTENING;
}

void NetSystem::StartUpClient()
{
	WSADATA data;
	int result = WSAStartup(MAKEWORD(2, 2), &data);
	if (result == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		ERROR_RECOVERABLE(Stringf("WSAStartup error %d", errorCode));
	}
	m_clientState = ClientState::BEGIN;
}


void NetSystem::BeginFrame()
{
	if (m_config.m_mode == Mode::SERVER)
	{
		BeginFrameServer();
	}
	if (m_config.m_mode == Mode::CLIENT)
	{
		BeginFrameClient();
	}
}

void NetSystem::BeginFrameServer()
{
	if (m_serverState == ServerState::LISTENING)
	{
		m_clientSocket = accept(m_listenSocket, NULL, NULL);
		if (m_clientSocket != INVALID_SOCKET)
		{
			unsigned long blockingMode = 1;
			ioctlsocket(m_clientSocket, FIONBIO, &blockingMode);
			m_serverState = ServerState::CONNECTED;
		}
	}
}

void NetSystem::BeginFrameClient()
{
	if (m_clientState == ClientState::BEGIN)
	{
		//Create client socket.
		m_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		//Set the client socket to non-blocking.
		unsigned long blockingMode = 1;
		int result = ioctlsocket(m_clientSocket, FIONBIO, &blockingMode);
		if (result != 0)
		{
			int errorCode = WSAGetLastError();
			ERROR_RECOVERABLE(Stringf("ioctlsocket error %d", errorCode));
		}

		//Get host address and port from string, using hard coded string literals, which you should not do.
		Strings hostAddrSplit = SplitStringOnDelimiter(m_config.m_hostAdressString, ':');
		IN_ADDR addr;
		result = inet_pton(AF_INET, hostAddrSplit[0].c_str(), &addr);
		if (result != 1)
		{
			int errorCode = WSAGetLastError();
			ERROR_RECOVERABLE(Stringf("inet_pton error %d", errorCode));
		}
		m_hostAddress = ntohl(addr.S_un.S_addr);
		m_hostPort = (unsigned short)(atoi(hostAddrSplit[1].c_str()));
		m_clientState = ClientState::CONNECTION_ATTEMPT_NOT_STARTED;
	}

	//if we have an unconnected client socket.
	if (m_clientState != ClientState::CONNECTED)
	{
		//If we haven’t already started a connection attempt, do so.
		if (m_clientState == ClientState::CONNECTION_ATTEMPT_NOT_STARTED)
		{
			sockaddr_in socaddr;
			socaddr.sin_family = AF_INET;
			socaddr.sin_addr.S_un.S_addr = htonl(m_hostAddress);
			socaddr.sin_port = htons(m_hostPort);
			int result = connect(m_clientSocket, (sockaddr*)(&socaddr), (int)sizeof(socaddr));
			if (result != 0)
			{
				int errorCode = WSAGetLastError();
				if (errorCode != WSAEWOULDBLOCK)
				{
					ERROR_RECOVERABLE(Stringf("connect error %d", errorCode));
				}
			}
			m_clientState = ClientState::ATTEMPTING_CONNECT;
		}
		//If we have already started a connection attempt, check the status.
		if (m_clientState == ClientState::ATTEMPTING_CONNECT)
		{
			fd_set writeSockets;
			fd_set exceptSockets;
			FD_ZERO(&writeSockets);
			FD_ZERO(&exceptSockets);
			FD_SET(m_clientSocket, &writeSockets);
			FD_SET(m_clientSocket, &exceptSockets);
			timeval waitTime = { };
			int result = select(0, NULL, &writeSockets, &exceptSockets, &waitTime);

			//If the return code is SOCKET_ERROR the connection attempt failed and you should call connect again.
			if (result == SOCKET_ERROR)
			{
				m_clientState = ClientState::CONNECTION_ATTEMPT_NOT_STARTED;
			}
			else if (result > 0)
			{
				if (FD_ISSET(m_clientSocket, &exceptSockets))
				{
					m_clientState = ClientState::CONNECTION_ATTEMPT_NOT_STARTED;
				}
				else if (FD_ISSET(m_clientSocket, &writeSockets))
				{
					m_clientState = ClientState::CONNECTED;
				}
			}
		}
	}
}


void NetSystem::Shutdown()
{
	if (m_config.m_mode == Mode::SERVER)
	{
		ShutDownServer();
	}
	if (m_config.m_mode == Mode::CLIENT)
	{
		ShutDownClient();
	}
}

void NetSystem::ShutDownServer()
{
	closesocket(m_clientSocket);
	closesocket(m_listenSocket);

	WSACleanup();
}

void NetSystem::SendAndRecieveData()
{
	for (int i = 0; i < (int)m_sendQueue.size(); i++)
	{
		std::string sendStr = m_sendQueue[i];
		int sendStartOffset = 0;
		bool finishedSendingString = false;
		while (!finishedSendingString)
		{
			finishedSendingString = true;
			int sendLength = (int)(sendStr.size() + 1) - sendStartOffset;
			if (sendLength > m_config.m_sendBufferSize)
			{
				sendLength = m_config.m_sendBufferSize;
				finishedSendingString = false;
			}
			for (int c = 0; c < sendLength; c++)
			{
				m_sendBuffer[c] = sendStr[sendStartOffset + c];
			}
			int result;
			if (finishedSendingString)
			{
				m_sendBuffer[sendLength] = '\0';
			}
			result = send(m_clientSocket, m_sendBuffer, sendLength, 0);
			

			if (result <= 0)
			{
				int errorCode = WSAGetLastError();
				if (errorCode == 0)
				{
					ResetConnectionState();
					return;
				}
				ERROR_RECOVERABLE(Stringf("WSAStartup error %d", errorCode));
			}
			else if (result != sendLength)
			{
				ERROR_RECOVERABLE("Did not send as many bytes as expected");
			}
			sendStartOffset += sendLength;
		}
	}
	m_sendQueue.clear();

	bool recievedAllData = false;
	while (!recievedAllData)
	{
		int result = recv(m_clientSocket, m_recvBuffer, m_config.m_recvBufferSize, 0);
		if (result <= 0)
		{
			int errorCode = WSAGetLastError();
			if (errorCode == 0)
			{
				ResetConnectionState();
				return;
			}
			if (errorCode == WSAEWOULDBLOCK)
			{
				recievedAllData = true;
				break;
			}
			else
			{
				ERROR_RECOVERABLE(Stringf("WSAStartup error %d", errorCode));
			}
		}

		//copy that much data into the receive queue
		else
		{
			for (int i = 0; i < result; i++)
			{
				if (m_recvBuffer[i] == '\0')
				{
					ToLower(m_recvQueue);
					if (m_recvQueue == "quit")
					{
						for (int j = 0; j < 1000; j++)
						{
							m_sendQueue.push_back("Echo Message=\"Know thyself know thy enemy - Sun Tzu\"");
						}
						m_sendQueue.push_back("Quit");
					}
					else
					{
						g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, m_recvQueue);
						g_theDevConsole->Execute(m_recvQueue);
					}
					m_recvQueue = "";
				}
				else
				{
					m_recvQueue += m_recvBuffer[i];
				}
			}
		}
	}
}

void NetSystem::ResetConnectionState()
{
	closesocket(m_clientSocket);
	if (m_config.m_mode == Mode::CLIENT)
	{
		m_clientState = ClientState::BEGIN;
	}
	else if (m_config.m_mode == Mode::SERVER)
	{
		m_serverState = ServerState::LISTENING;
	}
}

void NetSystem::ShutDownClient()
{
	closesocket(m_clientSocket);
	WSACleanup();
}

void NetSystem::EndFrame()
{
	if (m_config.m_mode == Mode::SERVER && m_serverState == ServerState::CONNECTED)
	{
		SendAndRecieveData();
	}

	//If we have a connected client socket, send all queued outgoing data and receive all incoming data.
	if (m_config.m_mode == Mode::CLIENT && m_clientState == ClientState::CONNECTED)
	{
		SendAndRecieveData();
	}
}

#endif // !defined( ENGINE_DISABLE_NETWORK )
