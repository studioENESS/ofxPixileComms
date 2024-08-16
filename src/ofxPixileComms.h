#pragma once

#include <vector>
#include <cstring>

#ifdef _WIN32
#include <WinSock2.h>
#define socklen_t int
#else
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h> 
#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#include <errno.h>
#define WSAGetLastError() errno
#include <linux/if_packet.h> 
#define BYTE uint8_t
#define BOOL bool
#define WSAEWOULDBLOCK EWOULDBLOCK
#endif

#define BUFLEN 2048  //Max length of buffer
#define MSG_PING 0x01
#define MSG_PONG 0x02
#define MSG_SCAN 0x03
#define MSG_LIGHTS 0x04
#define MSG_SOUNDS 0x05
#define MSG_GAME 0x06


struct SPixilePeer
{
	int _id;
	int _port;
	uint8_t _ip[4];
	bool _is_master;
};

struct SPixileMessage
{
	int32_t _id{ -1 };
	int param[4]{0,0,0,0};
};

typedef void (*fpMessageCallbackFunc)(SPixileMessage*, void*);

class ofxPixileComms
{
public:
	ofxPixileComms();
	~ofxPixileComms();

	void start();
	void SetupSockets();
	bool SetSocketBlockingEnabled(int fd, bool bBlocking);


	bool update(void);
	bool SendLightsMessage(bool status);
	bool SendSoundsMessage(bool status);
	bool SendGameMessage(int gameID, int msgID, float param1 = 0.f, float param2 = 0.f, float param3 = 0.f, float param4 = 0.f);
	void SendDataMessage(uint8_t msgID, uint8_t gameID, int32_t msgDataSize, void* msgData);
	void SendClientMessage(uint8_t msgID, int32_t* params);
	int Server_port() const;
	void Server_port(int val);
	int Computer_id() const;
	void Computer_id(int val);
	bool Master() const;
	void Master(bool val);

	bool LightsOn() const;
	void LightsOn(bool val);
	bool SoundsOn() const;
	void SoundsOn(bool val);

	void SetMessageHandler(fpMessageCallbackFunc pFunc, void* pUserData);

protected:
	int HandleScanResponse(int recv_len, socklen_t slen);

	void SetPeerInfo(uint8_t senderID, bool bServer);

private:
	int _server_port{ 3637 };

	int _computer_id;
	bool m_bMaster;

	SOCKET browserSocket;
	sockaddr_in sender, browseraddr;
	std::vector<SPixilePeer*> m_vecPeer;

	bool m_bLightsOn{ true };
	bool m_bSoundsOn{ true };

	fpMessageCallbackFunc m_pMessageHandler = nullptr;
	void* m_pUserData = nullptr;

};

