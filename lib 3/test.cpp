#include "Windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <time.h>

PROCESS_INFORMATION StartClone(int nCloneID)
{
	TCHAR szFilename[MAX_PATH];
	TCHAR szCMDLine[MAX_PATH];
	::GetModuleFileName(NULL, szFilename, MAX_PATH);
	::sprintf(szCMDLine, "\"%s\" %d", szFilename, nCloneID);
	STARTUPINFO si;
	::ZeroMemory(reinterpret_cast<void *>(&si), sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	//创建子进程
	bool flag = CreateProcess(szFilename, szCMDLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	return pi;
}

int main(int argc, char *argv[])
{
	int nClone = 0;
	if (argc > 1)
	{
		sscanf(argv[1], "%d", &nClone);
	}
	if (nClone == 0)
	{
		HANDLE mutex = CreateSemaphore(NULL, 1, 1, (LPCSTR) "sp_MUTEX");
		//CloseHandle(mutex);
		PROCESS_INFORMATION pi1 = StartClone(1);
		PROCESS_INFORMATION Pi2 = StartClone(2);
		HANDLE h[2];
		h[0]=pi1.hProcess;
		h[1]=Pi2.hProcess;
		WaitForMultipleObjects(2,h, true,INFINITE);
		

		printf("press any key to end:");
	
		if (getchar())
		
		{
			CloseHandle(pi1.hProcess);
			CloseHandle(Pi2.hProcess);
			return 0;
		}
	}
	else if (nClone == 1)
	{	
		Sleep(2000);
		HANDLE mutex = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, (LPCSTR) "sp_MUTEX");
		WaitForSingleObject(mutex, INFINITE);
		printf("xxxx1\n");
		//ReleaseSemaphore(mutex, 1, NULL);
		CloseHandle(mutex);
	}
	else if (nClone == 2)
	{
	
		HANDLE mutex = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, (LPCSTR) "sp_MUTEX");
		WaitForSingleObject(mutex, INFINITE);
		printf("xxxx2\n");
		
		ReleaseSemaphore(mutex, 1, NULL);
		CloseHandle(mutex);
	}
}
