#define NOTIFY(format, ...)\
	do {\
		char bufferForTheSocket[512];\
		sprintf(bufferForTheSocket, format, ##__VA_ARGS__);\
		strcat(bufferForTheSocket, "\n\n\n\n\n\n");\
		sceSysUtilSendSystemNotificationWithText(0x81, bufferForTheSocket);\
	} while(0)


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
