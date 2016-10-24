#include <Windows.h>
#include <iostream>
#include <string>

using namespace std;
int main()
{
	string p;
	HANDLE hMutex = NULL;
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

	do
	{
		if (!SetEvent(hClientReadOver))
			goto CLIENT_SHARE_MEMORY_END;

		if (WaitForSingleObject(hServerWriteOver, INFINITE) != WAIT_OBJECT_0)
			goto CLIENT_SHARE_MEMORY_END;

		if (OpenClipboard(NULL))
		{
			//判断剪贴板中的数据格式是否为 CF_TEXT          
			if (IsClipboardFormatAvailable(CF_TEXT))
			{
				char*     pDataBuf;
				HGLOBAL   hGlobalClip;
				//从剪贴板中获取格式为 CF_TEXT 的数据   
				hGlobalClip = GetClipboardData(CF_TEXT);
				/*cout << hGlobalClip;*/
				pDataBuf = (char *)GlobalLock(hGlobalClip);
				GlobalUnlock(hGlobalClip);
				cout << "从剪贴板中获取到数据：    " << pDataBuf << endl << endl;
			}
			CloseClipboard();
		}
		if (!ResetEvent(hServerWriteOver))
			goto CLIENT_SHARE_MEMORY_END;
	} while (p != "END");
CLIENT_SHARE_MEMORY_END:
	if (NULL != hServerWriteOver) CloseHandle(hServerWriteOver);
	if (NULL != hClientReadOver)  CloseHandle(hClientReadOver);
	if (NULL != hMutex)          ReleaseMutex(hMutex);
	return 0;
}


