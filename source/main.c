#include "ps4.h"
#include "include/global.h"
#include "include/kernel.h"
#include "include/sock.h"
#include "include/process.h"
#include "include/commandHandlers.h"
#define VERSION "1.0"

int _main(void) {
	initKernel();
	initLibc();
	initNetwork();
	initSysUtil();
	if (initSockets())
			NOTIFY("PS4API By BISOON STARTED v%s\n", VERSION);
	else{
		NOTIFY("PS4API: Failed to run on PORT [%d], try again later\n", SERVER_PORT);
		closeSockets();
		return 1;
	}

	kexec(kernelPayload, NULL);
	struct sockaddr_in clientStruc;
	command_s *localCommands = commands;
	int localCommandsLength = lenOfCommands;
	int gotUnknownCommand = 1;
	for ( ;; ) {
		
		clientSockFd = acceptClient(&clientStruc);
		while (1) {
			char bufferOfClient[MAX_RECEIVE_LENGTH] = {0};
			int lenOfReceivedData = receiveFromClient(clientSockFd, bufferOfClient, MAX_RECEIVE_LENGTH);
			if (lenOfReceivedData < 1)//Client Disconnected ?
				break;

			if (bufferOfClient[0] == 'q')
			{
				closeSocket(clientSockFd);
				quitCommandHandler();
				break;
			}
			for (size_t i = 0; i < localCommandsLength; i++) 
			{
		    	if(localCommands[i].commandChar == bufferOfClient[0] &&
					localCommands[i].minLength <= lenOfReceivedData  &&
					localCommands[i].handler != NULL)
		        {
		            localCommands[i].handler(bufferOfClient, lenOfReceivedData);
					gotUnknownCommand = 0;
					break;
		        }
		    }
			if (gotUnknownCommand){
				unknownCommandHandler();
			}
			gotUnknownCommand = 1;
		}
	}
	closeSockets();
	return 0;
}
