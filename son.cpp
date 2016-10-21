#include <Windows.h>
#include <iostream>
#include <string>

using namespace std;
int main()
{
	string p;
	HANDLE hMutex = NULL;
	HANDLE hFileMapping = NULL;
	LPVOID lpShareMemory = NULL;
	HANDLE hServerWriteOver = NULL;
	HANDLE hClientReadOver = NULL;
	//打开父程序中互斥体
	hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, L"SM_Mutex");
	if (NULL == hMutex)
	{
		if (ERROR_FILE_NOT_FOUND == GetLastError())
		{
			cout << "OpenMutex fail:file not found!" << endl;
			goto CLIENT_SHARE_MEMORY_END;
		}
		else
		{
			cout << "OpenMutex fail:" << GetLastError() << endl;
			goto CLIENT_SHARE_MEMORY_END;
		}
	}
	//检测hMutex的信号状态超时，报错
	if (WaitForSingleObject(hMutex, 5000) != WAIT_OBJECT_0)
	{
		DWORD dwErr = GetLastError();
		goto CLIENT_SHARE_MEMORY_END;
	}

	//打开共享内存，获得共享内存的句柄
	hFileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"ShareMemoryTest");
	if (NULL == hFileMapping)
	{
		cout << "OpenFileMapping" << GetLastError() << endl;
		goto CLIENT_SHARE_MEMORY_END;
	}
	//内存映射
	lpShareMemory = MapViewOfFile(hFileMapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0);
	if (NULL == lpShareMemory)
	{
		cout << "MapViewOfFile" << GetLastError() << endl;
		goto CLIENT_SHARE_MEMORY_END;
	}

	//打开父进程中ServerWriteOver和ClientReadOver两个事件对象
	hServerWriteOver = CreateEvent(NULL,
		TRUE,
		FALSE,
		L"ServerWriteOver");
	hClientReadOver = CreateEvent(NULL,
		TRUE,
		FALSE,
		L"ClientReadOver");
	if (NULL == hServerWriteOver || NULL == hClientReadOver)
	{
		cout << "CreateEvent" << GetLastError() << endl;
		goto CLIENT_SHARE_MEMORY_END;
	}
	
	char *q = (char*)lpShareMemory;
	do
	{
		char temp[MAX_PATH];
		if (!SetEvent(hClientReadOver))
			goto CLIENT_SHARE_MEMORY_END;

		if (WaitForSingleObject(hServerWriteOver, INFINITE) != WAIT_OBJECT_0)
			goto CLIENT_SHARE_MEMORY_END;
		memcpy(temp, lpShareMemory, MAX_PATH);
		p.assign(temp);
		cout << p << endl;
		if (!ResetEvent(hServerWriteOver))
			goto CLIENT_SHARE_MEMORY_END;
	} while (p != "END");
	CLIENT_SHARE_MEMORY_END:
		if (NULL != hServerWriteOver) CloseHandle(hServerWriteOver);
		if (NULL != hClientReadOver)  CloseHandle(hClientReadOver);
		if (NULL != lpShareMemory)    UnmapViewOfFile(lpShareMemory);
		if (NULL != hFileMapping)	 CloseHandle(hFileMapping);
		if (NULL != hMutex)          ReleaseMutex(hMutex);
	return 0;
}

