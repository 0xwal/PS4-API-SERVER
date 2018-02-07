#include "ps4.h"
#include "include/global.h"
#include "include/kernel.h"
#include "include/sock.h"
#include "include/process.h"
#include "include/commandHandlers.h"


int _main(void) {
	initKernel();
	initLibc();
	initNetwork();
	initSysUtil();
	if (initSockets())
			NOTIFY("PS4API By BISOON STARTED\n");
	else{
		NOTIFY("PS4API: Failed to run.\n");
	}
	kexec(kernelPayload, NULL);
	struct sockaddr_in clientStruc;


	for ( ;; ) {
		char bufferOfClient[MAX_RECEIVE_LENGTH] = {0};
		command_s *localCommands = commands;
		int localCommandsLength = lenOfCommands;
		clientSockFd = acceptClient(&clientStruc);
		while (1) {
			int lenOfRecievedData = receiveFromClient(clientSockFd, bufferOfClient, MAX_RECEIVE_LENGTH);
			if (lenOfRecievedData == 0)//Client Disconnected ?
				break;

			if (bufferOfClient[0] == 'q')
			{
				closeSocket(clientSockFd);
				break;
			}
			for (size_t i = 0; i < localCommandsLength; i++) {
		        if(
					localCommands[i].commandChar == bufferOfClient[0] &&
					localCommands[i].minLength <= lenOfRecievedData  &&
					localCommands[i].handler != NULL
				)
		        {
		            localCommands[i].handler(bufferOfClient, lenOfRecievedData);
					break;
		        }
		    }
		}
	}
	closeSockets();
	return 0;
}
