
#define LOG_IP IP(192, 168, 0, 14)
#define LOG_PORT 9023
#define SERVER_PORT 9090
#define BACK_LOG 1
#undef LOG

#define PORT_RANGE_POSSIBILITY 3

int logSock = -1;
int serverSockFd = -1;
int clientSockFd;
#define PRINTS(format, ...)\
	do {\
		char bufferForTheSocket[512];\
		int size = sprintf(bufferForTheSocket, format, ##__VA_ARGS__);\
		sceNetSend(logSock, bufferForTheSocket, size, 0);\
	} while(0)

void initLog()
{
    struct sockaddr_in logSocket;
    logSocket.sin_family = AF_INET;
    logSocket.sin_port = sceNetHtons(LOG_PORT);
    logSocket.sin_addr.s_addr = LOG_IP;
    memset(logSocket.sin_zero, 0, sizeof(logSocket.sin_zero));
    logSocket.sin_len = sizeof(logSocket);
    logSock = sceNetSocket("SOCK", AF_INET, SOCK_STREAM, 0);

    if (logSock < 0)
        return;

    sceNetConnect(logSock, (struct sockaddr*)&logSocket, sizeof(logSocket));
}
void closeLog()
{
    sceNetSocketClose(logSock);
}
void initServer(int port)
{
	struct sockaddr_in serverSocket;
	serverSocket.sin_family = AF_INET;
	serverSocket.sin_port = sceNetHtons(port);
	serverSocket.sin_addr.s_addr = IN_ADDR_ANY;
	memset(serverSocket.sin_zero, 0, sizeof(serverSocket.sin_zero));
	serverSocket.sin_len = sizeof(serverSocket);
	int ret = sceNetSocket("SERVER", AF_INET, SOCK_STREAM, 0);

	if (ret < 0)
        return;
	int sock = ret;
	ret = sceNetBind(sock, (struct sockaddr*)&serverSocket, sizeof(serverSocket));
	if (ret < 0)
		return;

	ret = sceNetListen(sock, BACK_LOG);
	if (ret < 0)
		return;
	serverSockFd = sock;
}
void closeServer()
{
	sceNetSocketClose(serverSockFd);
}
int initSockets()
{
	#ifdef LOG
		initLog();
	#else
		#define PRINTS(...)
	#endif
	int severPort = SERVER_PORT;
	for(int i = 0; i < PORT_RANGE_POSSIBILITY; i++)
	{
		initServer(severPort);
		if (serverSockFd > 0)
			break;
		severPort += 1;
	}
	return serverSockFd > 0;
}
void closeSockets()
{
	#ifdef LOG
		closeLog();
	#endif
	closeServer();
}
void closeSocket(int sockFd)
{
	sceNetSocketClose(sockFd);
}
void abortSendRecv(int sockFd){
	sceNetSocketAbort(sockFd, 1 | 2);
}
int acceptClient(struct sockaddr_in* clientSocket)
{
	unsigned int sizeOfSock = sizeof(struct sockaddr_in);
	return sceNetAccept(serverSockFd, (struct sockaddr*)clientSocket, &sizeOfSock);
}
int sendToClient(int clientfd, const char* buffer, int length)
{
	return sceNetSend(clientfd, buffer, length, 0);
}

int receiveFromClient(int clientfd, char* buffer, int length)
{
	return sceNetRecv(clientfd, buffer, length, 0);
}
void clientIp(struct in_addr* inAddr, char* out)
{
	sceNetInetNtop(AF_INET, inAddr, out, /*sizeof(struct in_addr)*/ 16);
}
