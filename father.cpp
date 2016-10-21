#include <Windows.h>
#include <iostream>
#include<string>

using namespace std;
int main(){
	string p;
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


	//�����ڴ���Բο���http://blog.csdn.net/sszgg2006/article/details/8573348
	HANDLE hMutex = NULL;
	HANDLE hFileMapping = NULL;
	LPVOID lpShareMenmory = NULL;
	HANDLE hServerWriteOver = NULL;
	HANDLE hClientReadOver = NULL;
	//���������ڴ�
	hFileMapping = CreateFileMapping(
		INVALID_HANDLE_VALUE,// ӳ���ļ��ľ��������Ϊ0xFFFFFFFF(����INVALID_HANDLE_VALUE)�򴴽�һ�����̼乲��Ķ���
		NULL,//��ȫ����
		PAGE_READWRITE, //������ʽ
		0,//����Ĵ�С
		1024 * 1024,
		L"ShareMemoryTest"// ӳ���ļ������������ڴ������
	);
	if (NULL == hFileMapping) {
		cerr << "Create File Mapping Fail:" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}
	//��ȡ������ڴ��ַ
	lpShareMenmory = MapViewOfFile(hFileMapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0);
	if (NULL == lpShareMenmory) {
		cerr << "Map View Of File Faild " << GetLastError << endl;
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
	char *q = (char *)lpShareMenmory;
	do {
		char temp[MAX_PATH];
		//���hClientReadOver�¼����ź�״̬
		if (WaitForSingleObject(hClientReadOver, 5 * 1000) != WAIT_OBJECT_0)
			goto SERVER_SHARE_MEMORY_END;
		gets_s(temp);
		p.assign(temp);//����
		memcpy(lpShareMenmory, temp, MAX_PATH);
		if (!ResetEvent(hClientReadOver))
			goto SERVER_SHARE_MEMORY_END;
		if (!SetEvent(hServerWriteOver))
			goto SERVER_SHARE_MEMORY_END;
	} while (p != "END");

	//������쳣���⣬��رվ��
	SERVER_SHARE_MEMORY_END:
		if (NULL != hServerWriteOver)
			CloseHandle(hServerWriteOver);
		if (NULL != hClientReadOver)
			CloseHandle(hClientReadOver);
		if (NULL != lpShareMenmory)
			UnmapViewOfFile(lpShareMenmory);
		if (NULL != hFileMapping)
			CloseHandle(hFileMapping);
		if (NULL != hMutex)
			ReleaseMutex(hMutex);

	return 0;
}
