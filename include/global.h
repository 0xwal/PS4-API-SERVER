#define NOTIFYT(type, format, ...)\
	do {\
		char bufferForTheSocket[512];\
		sprintf(bufferForTheSocket, format, ##__VA_ARGS__);\
		sceSysUtilSendSystemNotificationWithText(type, bufferForTheSocket);\
	} while(0)
#define NOTIFY(format, ...) NOTIFYT(222, format, ##__VA_ARGS__)


int countChars(char* source, char c )
{
    int count = 0;
    int index = 0;
    char theChar;
    while((theChar = *(source + index++)) != '\0')
        if (theChar == c)
            count++;
    return count;
}

int cmp(char* val1, char* val2){
    return strcmp(val1, val2) == 0;
}