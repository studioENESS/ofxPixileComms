#include "ofxPixileComms.h"
#include <iostream>

ofxPixileComms::ofxPixileComms()
{
	m_bSoundsOn = true;
	m_bLightsOn = true;
}

ofxPixileComms::~ofxPixileComms()
{

}

void ofxPixileComms::start()
{
	//if (m_bScanForDevices) {

	BYTE buf[12];
	memset(buf, '\0', 12);
	buf[0] = 'E';
	buf[1] = 'n';
	buf[2] = 'e';
	buf[3] = 's';
	buf[4] = 's';
	buf[5] = 'n';
	buf[6] = 'e';
	buf[7] = 't';
	buf[8] = 0x00;
	buf[9] = 0x00;
	buf[10] = m_bMaster ? 0x02 : 0x01;
	buf[11] = _computer_id;
	int slen = sizeof(browseraddr);

	memset(&browseraddr, '\0', slen);
	browseraddr.sin_family = PF_INET;
	browseraddr.sin_addr.s_addr = inet_addr("255.255.255.255");
	browseraddr.sin_port = htons(_server_port);

	if (sendto(browserSocket, (char*)buf, 12, 0, (struct sockaddr*)&browseraddr, slen) == SOCKET_ERROR) {
		//ns::debug::write_line("sendto() failed with error code : %d", WSAGetLastError());
		//exit(EXIT_FAILURE);
	}
}

void ofxPixileComms::SetupSockets()
{
	int slen;
	slen = sizeof(sender);

#ifdef _WIN32
	//Initialise winsock
	WSADATA wsa;

	//ns::debug::write("\nInhitialising Winsock...");

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		//ns::debug::write("Failed. Error Code : %d", WSAGetLastError());
		//exit(EXIT_FAILURE);
	}

	//ns::debug::write("Initialised.\n");
#endif
	//Create a socket
	if ((browserSocket = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		//ns::debug::write("Could not create socket : %d", WSAGetLastError());
	}


	//Prepare the sockaddr_in structure
	sender.sin_family = AF_INET;
	sender.sin_addr.s_addr = INADDR_ANY;//inet_addr("192.168.0.255");
	sender.sin_port = htons(_server_port);

	bool enabled = true;
	if (setsockopt(browserSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enabled, sizeof(BOOL)) < 0) {
		//ns::debug::write("Can't enable Broadcasting");
		//exit(EXIT_FAILURE);
	}

	//ns::debug::write("Socket created.\n");

	//Bind
	if (::bind(browserSocket, (struct sockaddr*)&sender, sizeof(sender)) == SOCKET_ERROR) {
		//ns::debug::write("Bind failed with error code : %d", WSAGetLastError());
		//exit(EXIT_FAILURE);
	}

	//ns::debug::write("Bind done");

	if (setsockopt(browserSocket, SOL_SOCKET, SO_BROADCAST, (char*)&enabled, sizeof(int32_t)) < 0) {
		//ns::debug::write("Can't enable Broadcasting");
		//exit(EXIT_FAILURE);
	}


	SetSocketBlockingEnabled(browserSocket, false);
}

bool ofxPixileComms::SetSocketBlockingEnabled(int fd, bool bBlocking)
{
	if (fd < 0) return false;
#ifdef WIN32
	unsigned long mode = bBlocking ? 0 : 1;
	return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) return false;
	flags = bBlocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
	return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;

#endif
}

bool ofxPixileComms::update(void)
{
	static int slen = sizeof(browseraddr);
	int recv_len = 0;
	recv_len = HandleScanResponse(recv_len, slen);

	return false;

}

bool ofxPixileComms::SendLightsMessage(bool status)
{
	m_bLightsOn = status;
	uint8_t msgID = MSG_LIGHTS;
	int32_t msgDataSize = 0x01;

	SendDataMessage(msgID, 0x00, msgDataSize, &status);

	return true;

}

bool ofxPixileComms::SendSoundsMessage(bool status)
{
	m_bSoundsOn = status;
	uint8_t msgID = MSG_SOUNDS;
	int32_t msgDataSize = 0x01;

	SendDataMessage(msgID, 0x00, msgDataSize, &status);

	return true;
}

bool ofxPixileComms::SendGameMessage(int gameID, int msgID, float param1 /*= 0.f*/, float param2 /*= 0.f*/, float param3 /*= 0.f*/, float param4 /*= 0.f*/)
{
	if (msgID == 0x0A)
		return false;

	SPixileMessage msg;

	msg._id = msgID;
	msg.param[0] = param1;
	msg.param[1] = param2;
	msg.param[2] = param3;
	msg.param[3] = param4;

	SendDataMessage(MSG_GAME, gameID, sizeof(SPixileMessage), &msg);

	return true;
}

void ofxPixileComms::SendDataMessage(uint8_t msgID, uint8_t gameID, int32_t msgDataSize, void* msgData)
{
	BYTE* buf;// [12];
	int msgSize = 13 + sizeof(int32_t) + msgDataSize;
	buf = new BYTE[msgSize];
	memset(buf, '\0', 13 + msgDataSize);
	buf[0] = 'E';
	buf[1] = 'n';
	buf[2] = 'e';
	buf[3] = 's';
	buf[4] = 's';
	buf[5] = 'n';
	buf[6] = 'e';
	buf[7] = 't';
	buf[8] = 0x00;
	buf[9] = 0x00;
	buf[10] = msgID;
	buf[11] = _computer_id;
	buf[12] = gameID;
	int slen = sizeof(browseraddr);

	memset(&browseraddr, '\0', slen);
	memcpy(buf + 13, &msgDataSize, sizeof(int32_t));
	memcpy(buf + 13 + sizeof(int32_t), msgData, msgDataSize);

	browseraddr.sin_family = PF_INET;
	//si_other.sin_addr.s_addr = inet_addr(m_sSrcIP.c_str());
	browseraddr.sin_addr.s_addr = inet_addr("255.255.255.255");
	browseraddr.sin_port = htons(_server_port);

	if (sendto(browserSocket, (char*)buf, msgSize, 0, (struct sockaddr*)&browseraddr, slen) == SOCKET_ERROR) {
		//ns::debug::write_line("sendto() failed with error code : %d", WSAGetLastError());
		//exit(EXIT_FAILURE);
	}

	delete[]buf;
}

void ofxPixileComms::Server_port(int val)
{
	_server_port = val;
}

int ofxPixileComms::Server_port() const
{
	return _server_port;
}

void ofxPixileComms::Computer_id(int val)
{
	_computer_id = val;
}

int ofxPixileComms::Computer_id() const
{
	return _computer_id;
}

void ofxPixileComms::Master(bool val)
{
	m_bMaster = val;
}

bool ofxPixileComms::Master() const
{
	return m_bMaster;
}

void ofxPixileComms::LightsOn(bool val)
{
	SendLightsMessage(val);
}

bool ofxPixileComms::LightsOn() const
{
	return m_bLightsOn;
}

void ofxPixileComms::SoundsOn(bool val)
{
	SendSoundsMessage(val);
}

bool ofxPixileComms::SoundsOn() const
{
	return m_bSoundsOn;
}

int ofxPixileComms::HandleScanResponse(int recv_len, socklen_t slen)
{

	BYTE buf[BUFLEN];
	recv_len = recvfrom(browserSocket, (char*)buf, BUFLEN, 0, (struct sockaddr*)&browseraddr, &slen);
	if (recv_len != SOCKET_ERROR) {


		/* if (m_bScanForDevices == false)5
			 return 0;*/
			 //print details of the client/peer and the data received
		int dataOffset = 12;
		char  ident[9];
		memcpy(ident, buf, 8);
		ident[8] = 0x00;
		std::string sIdent(ident);
		if (sIdent == std::string("Enessnet"))
		{
			BYTE msgType = buf[10];
			BYTE  senderID = buf[11];
			//ns::debug::write_line("Type: %x, senderID: %x", msgType, senderID);

			switch (msgType)
			{
			case 0x01: // Alive/Ident.
			{
				SetPeerInfo(senderID, false);
				break;
			}
			case 0x02: // Identify As Server
			{
				SetPeerInfo(senderID, true);
				break;
			}
			case 0x03: // Send/receive List.
			{
				std::string listData;
				int32_t strSize;
				memcpy(&strSize, buf + 12, sizeof(int32_t));
				char* cStr = new char[strSize];

				memcpy(cStr, buf + 12 + sizeof(int32_t), sizeof(int8_t) * strSize);
				// do something with the xml string.
				listData = std::string(cStr);
				delete[] cStr;

				break;
			}
			case 0x04: // Lights On/Off
			{
				int8_t status = buf[12];
				m_bLightsOn = status;
				break;
			}
			case 0x05: // Sounds On/Off
			{
				int8_t status = buf[12];
				m_bSoundsOn = status;
				break;
			}
			case 0x06: // Game Message
			{
				break;
			}
			default:
				break;
			}
		}
	}
	else
	{
		auto wsaLastError = WSAGetLastError();
		if (WSAGetLastError() != WSAEWOULDBLOCK)
			std::cout << "recvfrom() failed with error code : " << wsaLastError << std::endl;

	}
	return recv_len;
}

void ofxPixileComms::SetPeerInfo(uint8_t senderID, bool bServer)
{
	SPixilePeer* pPeer = nullptr;
	for (auto peer : m_vecPeer)
	{
		if (peer->_id == senderID)
		{
			pPeer = peer;
			break;
		}
	}

	if (pPeer == nullptr)
	{
		pPeer = new SPixilePeer();
		m_vecPeer.push_back(pPeer);
	}

	pPeer->_id = senderID;
	pPeer->_is_master = bServer;
	pPeer->_port = browseraddr.sin_port;
#ifdef WIN32
	pPeer->_ip[0] = browseraddr.sin_addr.S_un.S_un_b.s_b1;
	pPeer->_ip[1] = browseraddr.sin_addr.S_un.S_un_b.s_b2;
	pPeer->_ip[2] = browseraddr.sin_addr.S_un.S_un_b.s_b3;
	pPeer->_ip[3] = browseraddr.sin_addr.S_un.S_un_b.s_b4;
#endif
}