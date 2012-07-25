#include "stdafx.h"
#include "MSCorEE.h"

_TCHAR* _assembly;
_TCHAR* _class;
_TCHAR* _method;
_TCHAR* _param;
int _repetitions = 1;

void StartTheDotNetRuntime();
void StopTheDotNetRuntime();
void CallAssemblyMethod(int i);

int _tmain(int argc, _TCHAR* argv[])
{
	if(argc < 5)
	{
		printf("\n\tMissing parameters\n\n\tUsage:\n\n\tcppdvm [DotNetAssembly] [Namespace.Class] [MethodName] [Param] [optional repetitions]\n\n");
		return 0;
	}
	
	_assembly = argv[1];
	_class = argv[2];
	_method = argv[3];
	_param = argv[4];

	if (argc >= 6)
	{
		_repetitions = _wtoi(argv[5]);
	}


	StartTheDotNetRuntime();
	for (int i = 0; i < _repetitions; i++ )
	{
		CallAssemblyMethod(i % 2);
	}

	StopTheDotNetRuntime();

	return 0;
}

void StartTheDotNetRuntime()
{	
	ICLRRuntimeHost *pClrHost;
    HRESULT hr = CorBindToRuntimeEx(NULL, L"wks", 0, CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, (PVOID*)&pClrHost);
	if (hr != S_OK)
	{
		printf("CorBindToRuntimeEx: 0x%08x\n", hr);
	}
	else
	{		
		hr = pClrHost->Start();
		if (hr != S_OK)
		{
			printf("pClrHost->Start(): 0x%08x\n", hr);
		}
	}
	pClrHost->Release();
	pClrHost = NULL;
}

void CallAssemblyMethod(int i)
{
	ICLRRuntimeHost *pClrHost;
	HRESULT hr = CorBindToRuntimeEx(NULL, L"wks", 0, CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, (PVOID*)&pClrHost);
	DWORD dwRet = 0;
	
	if(i == 1)
	{
		_class = _T("MyNamespace1.MyClass1");
		_method = _T("MyMethod1");
		_assembly = _T("Test1.dll");
	}
	else
	{
		_class = _T("MyNamespace2.MyClass2");
		_method = _T("MyMethod2");
		_assembly = _T("Test2.dll");
	}

	hr = pClrHost->ExecuteInDefaultAppDomain(_assembly,
										 _class, 
										 _method, 
										 _param, 
										 &dwRet);

	if (hr != S_OK)
	{
		printf("pClrHost->ExecuteInDefaultAppDomain(): 0x%08x\n", hr);
	}
	else
	{
		printf("pClrHost->ExecuteInDefaultAppDomain() ended with: %d\n", dwRet);
	}

	DWORD appDomainId;

	hr = pClrHost->GetCurrentAppDomainId(&appDomainId);
	if (hr != S_OK)
	{
		printf("pClrHost->GetCurrentAppDomainId(): 0x%08x\n", hr);
	}

	hr = pClrHost->UnloadAppDomain(appDomainId, true);
	if (hr != S_OK)
	{
		printf("pClrHost->UnloadAppDomain(): 0x%08x\n", hr);
	}

	pClrHost->Release();
	pClrHost = NULL;
}

void StopTheDotNetRuntime()
{ 
	ICLRRuntimeHost *pClrHost;
	HRESULT hr = CorBindToRuntimeEx(NULL, L"wks", 0, CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, (PVOID*)&pClrHost);
	hr = pClrHost->Stop();

	if (hr != S_OK)
	{
		printf("pClrHost->Stop(): 0x%08x\n", hr);
	} 
	pClrHost->Release();
	pClrHost = NULL;
}