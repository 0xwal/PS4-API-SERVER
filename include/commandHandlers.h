#define MAX_PAYLOAD_LENGTH 512
#define MAX_READ_LENGTH 1024 //NOT APPLIED FOR THE READER YET
#define MAX_RECEIVE_LENGTH 1024

#define GAME_PROCESS_1 "eboot.bin"
#define GAME_PROCESS_2 "default_mp.elf"
#define GAME_PROCESS_3 "default.elf"


#define LENGTH_IS_NOT_CORRECT -2
#define NO_PAYLOAD_RECEIVED -3
#define READ_WRITE_FAILED -4
#define UNKNOWN_COMMAND -5

int attachedPid;

typedef struct command{
	char commandChar;
    int minLength;
	void (*handler)(const char* in, unsigned int inLength);
} command_s;

typedef struct client_io{
    char commandChar;
    unsigned int length;
    unsigned long int address;
} client_io_s;

typedef struct clientReplyDate{
    char code;
    int datalength;
    char*returnData;
}clientReplyDate_s;


void replyToClient(int clientfd, const clientReplyDate_s* clientData)
{
    unsigned int dataToReplyLen = clientData->datalength;
    if (!clientData->returnData)
    {
        dataToReplyLen = 0;
    }
    char dataToReply[dataToReplyLen + 1];

    dataToReply[0] = clientData->code;

    if (dataToReplyLen > 0)
    {
        for (unsigned int i = 0; i < dataToReplyLen; i++) {
            dataToReply[i + 1] = clientData->returnData[i];
        }
    }
    unsigned int dataToSendLength = dataToReplyLen ? dataToReplyLen + 1 : 1;
    sendToClient(clientfd, dataToReply, dataToSendLength);
}
void writerHandler(const char* in, unsigned int inDataLength)
{
    processAttach(attachedPid);
    client_io_s cmdLine = *(client_io_s*)in; // length & address, beyond that is the payload
    clientReplyDate_s clientReply = {0, 0, 0};
    if (cmdLine.length < 1)
    {
        clientReply.code = LENGTH_IS_NOT_CORRECT;
        goto exitMe;
    }

    int client_ioSize = sizeof(client_io_s);
    int payloadLength = inDataLength - client_ioSize;
    payloadLength = payloadLength > MAX_PAYLOAD_LENGTH ? MAX_PAYLOAD_LENGTH : payloadLength;
    if (!payloadLength)
    {
        clientReply.code = NO_PAYLOAD_RECEIVED;
        goto exitMe;
    }
    if (payloadLength < cmdLine.length){
        cmdLine.length = payloadLength;
    };

    unsigned char* payload = ((unsigned char*)in) + client_ioSize;

    if (writeMemory(attachedPid, (void*)cmdLine.address, payload, cmdLine.length) != 0)
    {
        clientReply.code = READ_WRITE_FAILED;
        goto exitMe;
    }
    clientReply.code = 0;
    exitMe:
    processDetach(attachedPid);
    replyToClient(clientSockFd, &clientReply);
}

void readerHandler(const  char* in, unsigned int inDataLength)
{
    processAttach(attachedPid);
    client_io_s cmdLine = *(client_io_s*)in;
    clientReplyDate_s clientReply = {0, 0, 0};
    if (cmdLine.length < 1)
    {
        clientReply.code = LENGTH_IS_NOT_CORRECT;
        goto exitMe;
    }
    int lengthToAllocate = cmdLine.length + 1;
    clientReply.returnData = (char*)calloc(lengthToAllocate, 1);
    clientReply.datalength = cmdLine.length;
    errno = 0;
    if (readMemory(attachedPid, (void*)cmdLine.address, clientReply.returnData, clientReply.datalength) < 0)
    {
        clientReply.code = READ_WRITE_FAILED;
        PRINTS("process: %d, address: %lx - malloc: %p - [errno=%d]", attachedPid, cmdLine.address, clientReply.returnData, errno);
        goto exitMe;
    }

    exitMe:
    replyToClient(clientSockFd, &clientReply);
    processDetach(attachedPid);
    free(clientReply.returnData);
}

int matchProcess(char* name, int pid){
    if (cmp(name, GAME_PROCESS_1) || cmp(name, GAME_PROCESS_2) || cmp(name, GAME_PROCESS_3))
        return true;
    return false;
}
void attachHandler(const  char* in, unsigned int inDataLength)
{
    int pid = getAllProcess(NULL, matchProcess);
    attachedPid = pid;
    int result = -1;
    errno = 0;
    int isAlreadyAttached = processgetVMTimeStamp(pid);
	if (isAlreadyAttached == -1 && pid != -1)
		result = processAttach(pid);
    result = (isAlreadyAttached != -1 || (result == -1 && errno == 16)) ? 0 : result;
    clientReplyDate_s clientReply = {result, 0, 0};
    processDetach(attachedPid);
    replyToClient(clientSockFd, &clientReply);
}
void detachHandler(const  char* in, unsigned int inDataLength)
{
    int result = -1;
	int pid = attachedPid;
	if (pid > 1)
		result = processDetach(pid);
    clientReplyDate_s clientReply = {result, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
void suspendHandler(const  char* in, unsigned int inDataLength)
{
	int result = -1;
	int pid = attachedPid;
	if (pid > 1)
		result = processSuspend(pid);
    clientReplyDate_s clientReply = {result, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
void resumeHandler(const  char* in, unsigned int inDataLength)
{
	int result = -1;
	int pid = attachedPid;
	if (pid > 1)
		result = processResume(pid);
    clientReplyDate_s clientReply = {result, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
void killHandler(const  char* in, unsigned int inDataLength)
{
	int result = -1;
	int pid = attachedPid;
	if (pid > 1)
		result = processKill(pid);
    clientReplyDate_s clientReply = {result, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
void continueHandler(const  char* in, unsigned int inDataLength)
{
	int result = -1;
	int pid = attachedPid;
	if (pid > 1){
		struct reg rg;
		processGetRegs(pid, &rg);
		result = processContinue(pid, (void*)rg.r_rip);
	}
    clientReplyDate_s clientReply = {result, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
void notfiyHandler(const  char* in, unsigned int inDataLength){
	void* commandEntry = (void*)in + 1;
    int notificationType = *(int*)commandEntry;
    char* text = commandEntry+sizeof(int);
    NOTIFYT(notificationType, text);
    clientReplyDate_s clientReply = {0, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}

command_s commands[] = {
        {'w', sizeof(client_io_s),  writerHandler},
        {'r', sizeof(client_io_s), readerHandler},
        {'a', 1, attachHandler},
        {'d', 1, detachHandler},
		{'s', 1, suspendHandler},
		{'u', 1, resumeHandler},
		{'k', 1, killHandler},
		{'c', 1, continueHandler},
		{'n', 5, notfiyHandler},
        
};
int lenOfCommands = sizeof(commands)/sizeof(commands[0]);

void unknownCommandHandler()
{
    clientReplyDate_s clientReply = {UNKNOWN_COMMAND, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
void quitCommandHandler()
{
    clientReplyDate_s clientReply = {0, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
