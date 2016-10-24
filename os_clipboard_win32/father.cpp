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


	HANDLE hMutex = NULL;
	HANDLE hServerWriteOver = NULL;
	HANDLE hClientReadOver = NULL;
	
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
	do {
		char temp[MAX_PATH];
		//检测hClientReadOver事件的信号状态
		if (WaitForSingleObject(hClientReadOver, 5 * 1000) != WAIT_OBJECT_0)
			goto SERVER_SHARE_MEMORY_END;
		gets_s(temp);
		p.assign(temp);//拷贝

		if (!OpenClipboard(NULL)) {
			wprintf(L"open clipboard failed!\n");
			exit(0);
		}
		else {
			//给全局内存对象分配全局内存          
			HGLOBAL hGlobalClip = GlobalAlloc(GHND, p.length() + 1);
			//通过给全局内存对象加锁获得对全局内存块的引用          
			char * pDataBuf = (char *)GlobalLock(hGlobalClip);
			strcpy(pDataBuf, temp);
			//使用完全局内存块后需要对全局内存块解锁          
			GlobalUnlock(hGlobalClip);
			//清空剪贴板          
			EmptyClipboard();
			//设置剪贴板数据，这里直接将数据放到了剪贴板中，而没有使用延迟提交技术          
			SetClipboardData(CF_TEXT, hGlobalClip);
			//关闭剪贴板          
			CloseClipboard();
			cout << "设置剪贴板为：    " << p << endl << endl;
		}
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
		if (NULL != hMutex)
			ReleaseMutex(hMutex);

	return 0;
}


