#include <Windows.h>
#include <iostream>
#include<string>

using namespace std;
int main(){
	string p;
	//定义创建子进程所需的参数；
	TCHAR * AppName = TEXT("C:\\sprocess\\Debug\\sprocess.exe");

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	//用0填充两个结构体的内存区域
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));


	//创建一个新进程  
	if (CreateProcess(
		AppName,   //  指向一个NULL结尾的、用来指定可执行模块的宽字节字符串  
		NULL, // 命令行字符串  
		NULL, //    指向一个SECURITY_ATTRIBUTES结构体，这个结构体决定是否返回的句柄可以被子进程继承。  
		NULL, //    如果lpProcessAttributes参数为空（NULL），那么句柄不能被继承。<同上>  
		false,//    指示新进程是否从调用进程处继承了句柄。   
		//0,指定附加的、用来控制优先类和进程的创建的标  
		CREATE_NEW_CONSOLE,  //新控制台打开子进程  
			//  CREATE_SUSPENDED    子进程创建后挂起，直到调用ResumeThread函数  
		NULL, //    指向一个新进程的环境块。如果此参数为空，新进程使用调用进程的环境  
		NULL, //    指定子进程的工作路径  
		&si, // 决定新进程的主窗体如何显示的STARTUPINFO结构体  
		&pi  // 接收新进程的识别信息的PROCESS_INFORMATION结构体  
	))
	{
		cout << "create process success" << endl;

		//下面两行关闭句柄，解除本进程和新进程的关系，不然有可能不小心调用TerminateProcess函数关掉子进程  
		CloseHandle(pi.hProcess);  
		CloseHandle(pi.hThread);  
	}
	else {
		cerr << "failed to create process" << endl;
	}


	//共享内存可以参考：http://blog.csdn.net/sszgg2006/article/details/8573348
	HANDLE hMutex = NULL;
	HANDLE hFileMapping = NULL;
	LPVOID lpShareMenmory = NULL;
	HANDLE hServerWriteOver = NULL;
	HANDLE hClientReadOver = NULL;
	//创建共享内存
	hFileMapping = CreateFileMapping(
		INVALID_HANDLE_VALUE,// 映射文件的句柄，若设为0xFFFFFFFF(即：INVALID_HANDLE_VALUE)则创建一个进程间共享的对象
		NULL,//安全属性
		PAGE_READWRITE, //保护方式
		0,//对象的大小
		1024 * 1024,
		L"ShareMemoryTest"// 映射文件名，即共享内存的名称
	);
	if (NULL == hFileMapping) {
		cerr << "Create File Mapping Fail:" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}
	//获取共享的内存地址
	lpShareMenmory = MapViewOfFile(hFileMapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0);
	if (NULL == lpShareMenmory) {
		cerr << "Map View Of File Faild " << GetLastError << endl;
		goto SERVER_SHARE_MEMORY_END;
	}
	//找出当前系统是否已经存在指定进程的实例。如果没有则创建一个互斥体

	hMutex = CreateMutex(NULL,// 指向安全属性的指针
		FALSE, // 初始化互斥对象的所有者
		L"SM_Mutex"// 指向互斥对象名的指针
	);
	//进程互斥
	if (NULL == hMutex || ERROR_ALREADY_EXISTS == GetLastError()) {
		cout << "CreateMutex Faild" << GetLastError() << endl;
		goto SERVER_SHARE_MEMORY_END;
	}
	//创建事件对象
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
		//检测hClientReadOver事件的信号状态
		if (WaitForSingleObject(hClientReadOver, 5 * 1000) != WAIT_OBJECT_0)
			goto SERVER_SHARE_MEMORY_END;
		gets_s(temp);
		p.assign(temp);//拷贝
		memcpy(lpShareMenmory, temp, MAX_PATH);
		if (!ResetEvent(hClientReadOver))
			goto SERVER_SHARE_MEMORY_END;
		if (!SetEvent(hServerWriteOver))
			goto SERVER_SHARE_MEMORY_END;
	} while (p != "END");

	//处理各异常问题，如关闭句柄
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
