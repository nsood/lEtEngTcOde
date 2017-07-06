#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LOG_NO_P(format, ...) do{ \
	char msg[256]; \
	char ti[32]; \
	FILE *fp = fopen("/tmp/debug.file", "a+"); \
	time_t now = time(NULL); \
	sprintf(msg, format, ##__VA_ARGS__); \
	strftime(ti, sizeof(ti), "%Y-%m-%d %H:%M:%S", localtime(&now)); \
	fprintf(fp, "[%s] %s", ti, msg); \
	fflush(fp); \
	fclose(fp); \
}while(0)

#define LOG_P(format, ...) do{ \
	char msg[256]; \
	char ti[32]; \
	FILE *fp = fopen("/tmp/debug.file", "a+"); \
	time_t now = time(NULL); \
	sprintf(msg, format, ##__VA_ARGS__); \
	strftime(ti, sizeof(ti), "%Y-%m-%d %H:%M:%S", localtime(&now)); \
	fprintf(fp, "[%s] %s", ti, msg); \
	fflush(fp); \
	fclose(fp); \
	printf("%s",msg); \
}while(0)

//#define DEBUG_INFO
#define DEBUG_FILE 1
#ifdef DEBUG_INFO
#define debug_info(format,...) printf("%s(%d):\t\t"format"\n",__func__, __LINE__, ##__VA_ARGS__)
#elif DEBUG_FILE
#define debug_info(format,...) LOG_P("%s(%d):\t\t"format"\n",__func__, __LINE__, ##__VA_ARGS__)
#else
#define debug_info(format,...) LOG_NO_P("%s(%d):\t\t"format"\n",__func__, __LINE__, ##__VA_ARGS__)
#endif

#endif
