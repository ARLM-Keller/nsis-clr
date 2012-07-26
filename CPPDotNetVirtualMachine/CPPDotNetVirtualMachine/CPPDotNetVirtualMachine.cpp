#include "stdafx.h"
#include "MSCorEE.h"
#include <conio.h>

#import "mscorlib.tlb" raw_interfaces_only  high_property_prefixes("_get","_put","_putref") rename("ReportEvent", "InteropServices_ReportEvent") 
using namespace mscorlib; 

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

	getch();

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
	ICorRuntimeHost *pClrHost;
	HRESULT hr = CorBindToRuntimeEx(NULL, L"wks", 0, CLSID_CorRuntimeHost, IID_ICorRuntimeHost, (PVOID*)&pClrHost);
	DWORD dwRet = 0;
	
	if(i == 1)
	{
		_class = _T("MyNamespace1.MyClass1");
		_method = _T("MyMethod1");
		_assembly = _T("Test1");
	}
	else
	{
		_class = _T("MyNamespace2.MyClass2");
		_method = _T("MyMethod2");
		_assembly = _T("Test2");
	}

	IUnknownPtr spAppDomainThunk = NULL; 
	_AppDomainPtr spDefaultAppDomain = NULL; 

	// The .NET assembly to load. 
	bstr_t bstrAssemblyName(_assembly); 
	_AssemblyPtr spAssembly = NULL; 

	// The .NET class to instantiate. 
	bstr_t bstrClassName(_class); 
	_TypePtr spType = NULL; 
	variant_t vtObject; 
	variant_t vtEmpty; 

	// The static method in the .NET class to invoke. 
	bstr_t bstrStaticMethodName(_method); 
	SAFEARRAY *psaStaticMethodArgs = NULL; 
	variant_t vtStringArg(_param); 
	variant_t vtLengthRet; 

	hr = pClrHost->GetDefaultDomain(&spAppDomainThunk);
	if (hr != S_OK)
	{
		printf("pClrHost->GetDefaultDomain: 0x%08x\n", hr);
	}

	hr = spAppDomainThunk->QueryInterface(IID_PPV_ARGS(&spDefaultAppDomain)); 
	if (hr != S_OK)
	{
		printf("QueryInterface->spDefaultAppDomain: 0x%08x\n", hr);
	} 

	// Load the .NET assembly. 
	hr = spDefaultAppDomain->Load_2(bstrAssemblyName, &spAssembly); 
	if (hr != S_OK)
	{
		printf("spDefaultAppDomain->Load_2: 0x%08x\n", hr);
	}  

	

	// Get the Type of the object. 
	hr = spAssembly->GetType_2(bstrClassName, &spType); 
	if (hr != S_OK)
	{
		printf("spAssembly->GetType_2: 0x%08x\n", hr);
	} 

	// Call the static method of the class: 
	//   public static int Method(string str); 
	 
	 
	// Create a safe array to contain the arguments of the method. The safe  
	// array must be created with vt = VT_VARIANT because .NET reflection  
	// expects an array of Object - VT_VARIANT. There is only one argument,  
	// so cElements = 1. 
	psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1); 
	LONG index = 0; 
	hr = SafeArrayPutElement(psaStaticMethodArgs, &index, &vtStringArg); 
	if (hr != S_OK)
	{
		printf("SafeArrayPutElement: 0x%08x\n", hr);
	}
	 
	 
	// Invoke the "Method" method from the Type interface. 
	hr = spType->InvokeMember_3(bstrStaticMethodName, (BindingFlags) 
		(BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),  
		NULL, vtEmpty, psaStaticMethodArgs, &vtLengthRet); 
	if (hr != S_OK)
	{
		printf("spType->InvokeMember_3: 0x%08x\n", hr);
	}
	 
	 
	// Print the call result of the static method. 
	wprintf(L"Call %s.%s(\"%s\") => %d\n", (PCWSTR)bstrClassName,  
		(PCWSTR)bstrStaticMethodName, (PCWSTR)vtStringArg.bstrVal,  
		vtLengthRet.lVal); 

	/*hr = pClrHost->ExecuteInDefaultAppDomain(_assembly,
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
	}*/

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