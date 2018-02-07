#define MAX_PAYLOAD_LENGTH 512
#define MAX_READ_LENGTH 1024 //NOT APPLIED FOR THE READER YET
#define MAX_RECEIVE_LENGTH 1024
#define DEFAULT_PROCESS "eboot.bin"

#define LENGTH_IS_NOT_CORRECT -2
#define NO_PAYLOAD_RECEIVED -3
#define READ_WRITE_FAILED -4

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
    replyToClient(clientSockFd, &clientReply);
}

void readerHandler(const  char* in, unsigned int inDataLength)
{
    client_io_s cmdLine = *(client_io_s*)in;
    clientReplyDate_s clientReply = {0, 0, 0};
    if (cmdLine.length < 1)
    {
        clientReply.code = LENGTH_IS_NOT_CORRECT;
        goto exitMe;
    }

    clientReply.returnData = (char*)calloc(cmdLine.length+1, 1);
    clientReply.datalength = cmdLine.length;

    if (readMemory(attachedPid, (void*)cmdLine.address, clientReply.returnData, cmdLine.length) < 0)
    {
        clientReply.code = READ_WRITE_FAILED;
        goto exitMe;
    }

    exitMe:
    replyToClient(clientSockFd, &clientReply);
    free(clientReply.returnData);
}
void attachHandler(const  char* in, unsigned int inDataLength)
{
    int procPid = getProcess(DEFAULT_PROCESS);
    int result = -1;
	if (procPid != -1)
		result = processAttach(procPid);
    clientReplyDate_s clientReply = {result, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
void detachHandler(const  char* in, unsigned int inDataLength)
{
    int result = -1;
	int pid = attachedPid;
	if (pid!=-1 || pid != 0)
		result = processDetach(pid);
    clientReplyDate_s clientReply = {result, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
void suspendHandler(const  char* in, unsigned int inDataLength)
{
	int result = -1;
	int pid = attachedPid;
	if (pid !=-1 || pid != 0)
		result = processSuspend(pid);
    clientReplyDate_s clientReply = {result, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
void resumeHandler(const  char* in, unsigned int inDataLength)
{
	int result = -1;
	int pid = attachedPid;
	if (pid !=-1 || pid != 0)
		result = processResume(pid);
    clientReplyDate_s clientReply = {result, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
void killHandler(const  char* in, unsigned int inDataLength)
{
	int result = -1;
	int pid = attachedPid;
	if (pid !=-1 || pid != 0)
		result = processKill(pid);
    clientReplyDate_s clientReply = {result, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
void continueHandler(const  char* in, unsigned int inDataLength)
{
	int result = -1;
	int pid = attachedPid;
	if (pid !=-1 || pid != 0){
		struct reg rg;
		processGetRegs(pid, &rg);
		result = processContinue(pid, (void*)rg.r_rip);
	}
    clientReplyDate_s clientReply = {result, 0, 0};
    replyToClient(clientSockFd, &clientReply);
}
void notfiyHandler(const  char* in, unsigned int inDataLength){
	NOTIFY((in + 1));
}
command_s commands[] = {
        {'w', sizeof(client_io_s),  writerHandler},
        {'r', sizeof(client_io_s), readerHandler},
        {'a', 5, attachHandler},
        {'d', 1, detachHandler},
		{'s', 1, suspendHandler},
		{'u', 1, resumeHandler},
		{'k', 1, killHandler},
		{'c', 1, continueHandler},
		{'n', 2, notfiyHandler},
};
int lenOfCommands = sizeof(commands)/sizeof(commands[0]);
