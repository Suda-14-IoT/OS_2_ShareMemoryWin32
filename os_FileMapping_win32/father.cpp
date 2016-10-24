#include <Windows.h>
#include <iostream>
#include<string>

using namespace std;
int main() {
	//������
	string end_mark;
	//���崴���ӽ�������Ĳ�����
	TCHAR * AppName = TEXT("C:\\sprocess\\Debug\\sprocess.exe");

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	//��0��������ṹ����ڴ�����
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));


	//����һ���½���  
	if (CreateProcess(
		AppName,   //  ָ��һ��NULL��β�ġ�����ָ����ִ��ģ��Ŀ��ֽ��ַ���  
		NULL, // �������ַ���  
		NULL, //    ָ��һ��SECURITY_ATTRIBUTES�ṹ�壬����ṹ������Ƿ񷵻صľ�����Ա��ӽ��̼̳С�  
		NULL, //    ���lpProcessAttributes����Ϊ�գ�NULL������ô������ܱ��̳С�<ͬ��>  
		false,//    ָʾ�½����Ƿ�ӵ��ý��̴��̳��˾����   
			  //0,ָ�����ӵġ���������������ͽ��̵Ĵ����ı�  
		CREATE_NEW_CONSOLE,  //�¿���̨���ӽ���  
							 //  CREATE_SUSPENDED    �ӽ��̴��������ֱ������ResumeThread����  
		NULL, //    ָ��һ���½��̵Ļ����顣����˲���Ϊ�գ��½���ʹ�õ��ý��̵Ļ���  
		NULL, //    ָ���ӽ��̵Ĺ���·��  
		&si, // �����½��̵������������ʾ��STARTUPINFO�ṹ��  
		&pi  // �����½��̵�ʶ����Ϣ��PROCESS_INFORMATION�ṹ��  
	))
	{
		cout << "create process success" << endl;

		//�������йرվ������������̺��½��̵Ĺ�ϵ����Ȼ�п��ܲ�С�ĵ���TerminateProcess�����ص��ӽ���  
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		cerr << "failed to create process" << endl;
	}


	HANDLE hMutex = NULL;
	HANDLE hServerWriteOver = NULL;
	HANDLE hClientReadOver = NULL;
	HANDLE hFileMapping = NULL;
	LPVOID lpShareMemory = NULL;
	//create share memory  
	hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		1024 * 1024,
		L"ShareMemoryTest");
	if (NULL == hFileMapping)
	{
		cout << "CreateFileMapping fail:" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}

	lpShareMemory = MapViewOfFile(hFileMapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,      //memory start address  
		0);     //all memory space  
	if (NULL == lpShareMemory)
	{
		cout << "MapViewOfFile" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}

	//�ҳ���ǰϵͳ�Ƿ��Ѿ�����ָ�����̵�ʵ�������û���򴴽�һ��������

	hMutex = CreateMutex(NULL,// ָ��ȫ���Ե�ָ��
		FALSE, // ��ʼ����������������
		L"SM_Mutex"// ָ�򻥳��������ָ��
	);
	//���̻���
	if (NULL == hMutex || ERROR_ALREADY_EXISTS == GetLastError()) {
		cout << "CreateMutex Faild" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}
	//�����¼�����
	hServerWriteOver = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"ServerWriteOver"
	);
	hClientReadOver = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		L"ClientReadOver"
	);
	if (NULL == hServerWriteOver ||
		NULL == hClientReadOver) {
		cerr << "CreateEvent" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}
	cout << "Please enter:\n";
	
	do {
		char temp[MAX_PATH];
		//���hClientReadOver�¼����ź�״̬
		if (WaitForSingleObject(hClientReadOver, 5 * 1000) != WAIT_OBJECT_0)
			goto SERVER_SHARE_MEMORY_END;
		gets_s(temp);
		end_mark.assign(temp);
		memcpy(lpShareMemory, temp, MAX_PATH);
		if (!ResetEvent(hClientReadOver))
			goto SERVER_SHARE_MEMORY_END;
		if (!SetEvent(hServerWriteOver))
			goto SERVER_SHARE_MEMORY_END;
	} while (end_mark != "END");

	//�������쳣���⣬��رվ��
SERVER_SHARE_MEMORY_END:
	if (NULL != hServerWriteOver)
		CloseHandle(hServerWriteOver);
	if (NULL != hClientReadOver)
		CloseHandle(hClientReadOver);
	if (NULL != hMutex)
		ReleaseMutex(hMutex);
	if (NULL != lpShareMemory)      
		UnmapViewOfFile(lpShareMemory);
	if (NULL != hFileMapping)       
		CloseHandle(hFileMapping);

	return 0;
}


