#include "ForgeHttp.h"

BOOLEAN CheckFileExist(PCHAR lpszPath)
{
	if (GetFileAttributesA(lpszPath) == 0xFFFFFFFF && GetLastError() == ERROR_FILE_NOT_FOUND )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
BOOLEAN DownloadFileFromUrl(PCHAR pUrl,PCHAR pSaveFileName)
{
	BOOLEAN bRet;
	HINTERNET hInternet;
	HINTERNET hOpenUrl;
	ULONG ulStatusCode;
	ULONG ulStatusSize;
	ULONG ulSize;
	ULONG ulLengthSize;
	HANDLE hLocalFile;
	BOOLEAN bIsWrite;
	ULONG ulRetReadBytesSize,ulRetWriteBytesSize;
	CHAR Buffer[0x1000];


	bRet = FALSE;
	hInternet = NULL;
	hOpenUrl = NULL;
	bIsWrite = TRUE;
	ulRetReadBytesSize = 0;
	ulRetWriteBytesSize = 0;

	do 
	{
		hInternet = InternetOpenA(NULL,INTERNET_OPEN_TYPE_DIRECT,NULL,NULL,0);
		if (NULL == hInternet)
		{
			break;
		}
		hOpenUrl = InternetOpenUrlA(hInternet,pUrl,NULL,0,INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_PRAGMA_NOCACHE,0);
		if (NULL == hOpenUrl)
		{
			InternetCloseHandle(hInternet);
			break;
		}
		ulStatusSize = sizeof(ulStatusCode);
		HttpQueryInfoA(hOpenUrl,HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,&ulStatusCode,&ulStatusSize,NULL);
		if  (200 != ulStatusCode)
		{
			InternetCloseHandle(hOpenUrl);
			InternetCloseHandle(hInternet);
			break;
		}
		ulLengthSize = sizeof(ulSize);
		HttpQueryInfoA(hOpenUrl,HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,&ulSize,&ulLengthSize,NULL);
		if (ulSize < 0)
		{
			InternetCloseHandle(hOpenUrl);
			InternetCloseHandle(hInternet);
			break;
		}
		hLocalFile = CreateFileA(pSaveFileName,GENERIC_WRITE | GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
		if (hLocalFile == INVALID_HANDLE_VALUE)
		{
			InternetCloseHandle(hOpenUrl);
			InternetCloseHandle(hInternet);
			break;
		}
		while (TRUE)
		{
			InternetReadFile(hOpenUrl,Buffer,sizeof(Buffer),&ulRetReadBytesSize);
			if (ulRetReadBytesSize == 0)
			{
				break;
			}
			bIsWrite = WriteFile(hLocalFile,Buffer,ulRetReadBytesSize,&ulRetWriteBytesSize,NULL);
			if (bIsWrite == 0)
			{
				break;
			}
		}
		bRet = TRUE;
	} while (0);
	if (hLocalFile)
	{
		CloseHandle(hLocalFile);
	}
	if (hOpenUrl)
	{
		InternetCloseHandle(hOpenUrl);
	}
	if (hInternet)
	{
		InternetCloseHandle(hInternet);
	}
	return bRet;
}