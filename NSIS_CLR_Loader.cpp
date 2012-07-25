#include "exdll.h"
#include "MSCorEE.h"
#include "ezlogger_headers.hpp"
#include <tchar.h>
#include <string.h>
#include <stdlib.h>

HWND g_hwndParent;
HANDLE g_hInstance;

TCHAR *CA2W(LPSTR szSrcStr)
{
	if (szSrcStr == NULL)
		return NULL;

	int cw = strlen(szSrcStr);

	if (cw == 0) 
	{
		TCHAR *psz=new TCHAR[1];
		*psz = '\0';
		return psz;
	}

	int cc =  MultiByteToWideChar(CP_ACP, 0, szSrcStr, cw, NULL, 0);
	
	if (cc == 0) 
		return NULL;

	TCHAR *psz = new TCHAR[cc+1];
	cc = MultiByteToWideChar(CP_ACP, 0, szSrcStr, cw, psz, cc);

	if (cc == 0)
	{
		delete[] psz;
		return NULL;
	}

	psz[cc] = '\0';
	return psz;
}

LPSTR W2CA(TCHAR * szSrcStr)
{
	if (szSrcStr == NULL)
		return NULL;

	int cw = lstrlen(szSrcStr);

	if (cw == 0) 
	{
		CHAR *psz=new CHAR[1];
		*psz = '\0';
		return psz;
	}

	int cc = WideCharToMultiByte(CP_ACP, 0, szSrcStr, cw, NULL, 0, NULL, NULL);
	
	if (cc == 0) 
		return NULL;

	CHAR *psz = new CHAR[cc+1];
	cc = WideCharToMultiByte(CP_ACP, 0, szSrcStr, cw, psz, cc, NULL, NULL);

	if (cc == 0)
	{
		delete[] psz;
		return NULL;
	}

	psz[cc] = '\0';
	return psz;
}

char *returnChar(long value)
{
	char *strRetVal;
	strRetVal = (char *) malloc(sizeof(char) * 100);
	_ltoa(value, strRetVal, 10);

	return strRetVal;
}

char* CallCLR(TCHAR* DllName, TCHAR* ClassWithNamespace, TCHAR* MethodName, TCHAR* Args)
{
	// Bind to the CLR runtime..
	DWORD retVal = 0;
	DWORD appDomainId;

	ICLRRuntimeHost *pClrHost = NULL;

	HRESULT hr = CorBindToRuntimeEx(
        _T("v2.0.50727"),   // Load the latest 2.0 CLR version available
        _T("wks"), // Workstation GC ("wks" or "svr" overrides)
        0,      // No flags needed
        CLSID_CLRRuntimeHost,
        IID_ICLRRuntimeHost,
        (PVOID*)&pClrHost);
	if (hr != S_OK)
	{
		EZLOGGERPRINT("CorBindToRuntimeEx Call: 0x%08x", hr);
		return returnChar(hr);
	}

	hr = pClrHost->ExecuteInDefaultAppDomain(
					DllName, ClassWithNamespace,
					MethodName, Args,
					&retVal);
	if (hr != S_OK)
	{
		EZLOGGERPRINT("ExecuteInDefaultAppDomain: [%s] %s->%s(%s) 0x%08x", W2CA(DllName), W2CA(ClassWithNamespace), W2CA(MethodName), W2CA(Args), hr);
		return returnChar(hr);
	}

	// Don't forget to clean up.
    pClrHost->Release();
	if (hr != S_OK)
	{
		EZLOGGERPRINT("Release Call: 0x%08x", hr);
		return returnChar(hr);
	}

	return returnChar(retVal);
}

extern "C" __declspec(dllexport) void Call(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
	// expected parameters:
	// filename.dll, namespace.namespace...class, method, numparams, params...

	g_hwndParent=hwndParent;
	EXDLL_INIT();

	char buf[1024] = {0};

	// filename.dll
	popstring(buf);
	char dllname[1024] = {0};
	strcpy(dllname, buf);

	// namespace and class
	popstring(buf);
	char classwithnamespace[1024] = {0};
	strcpy(classwithnamespace, buf);

	// method
	popstring(buf);
	char method[1024] = {0};
	strcpy(method, buf);

	// num params
	popstring(buf);
	int numparams = atoi(buf);

	// params
	char args[65536] = {0};
	for (int i=0; i<numparams; i++)
	{
		popstring(buf);
		strcat(args, buf);		
	}
	
	char* result = (char*)CallCLR(CA2W(dllname), CA2W(classwithnamespace), CA2W(method), CA2W(args));
	pushstring(result);
	free(result);
}

extern "C" __declspec(dllexport) void Start(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
	g_hwndParent=hwndParent;
	EXDLL_INIT();

	char buf[1024] = {0};

	while (popstring(buf))
	{}

	char* result = NULL;
	ICLRRuntimeHost *pClrHost = NULL;

	HRESULT hr = CorBindToRuntimeEx(
        _T("v2.0.50727"),   // Load the latest 2.0 CLR version available
        _T("wks"), // Workstation GC ("wks" or "svr" overrides)
        0,      // No flags needed
        CLSID_CLRRuntimeHost,
        IID_ICLRRuntimeHost,
        (PVOID*)&pClrHost);
	if (hr != S_OK)
	{
		EZLOGGERPRINT("CorBindToRuntimeEx Start: 0x%08x", hr);
		result = returnChar(hr);
	}
 
    // Now, start the CLR.
    hr = pClrHost->Start();
	if (hr != S_OK)
	{
		EZLOGGERPRINT("Start: 0x%08x", hr);
		result = returnChar(hr);
	}

	// Don't forget to clean up.
    pClrHost->Release();
	if (hr != S_OK)
	{
		EZLOGGERPRINT("Release Star: 0x%08x", hr);
		result = returnChar(hr);
	}
	else
	{
		result = returnChar(0);
	}
	
	pushstring(result);
	free(result);
}

extern "C" __declspec(dllexport) void Stop(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
	g_hwndParent=hwndParent;
	EXDLL_INIT();

	char buf[1024] = {0};

	while (popstring(buf))
	{}

	char* result = NULL;
	ICLRRuntimeHost *pClrHost = NULL;

	HRESULT hr = CorBindToRuntimeEx(
        _T("v2.0.50727"),   // Load the latest 2.0 CLR version available
        _T("wks"), // Workstation GC ("wks" or "svr" overrides)
        0,      // No flags needed
        CLSID_CLRRuntimeHost,
        IID_ICLRRuntimeHost,
        (PVOID*)&pClrHost);
	if (hr != S_OK)
	{
		EZLOGGERPRINT("CorBindToRuntimeEx Stop: 0x%08x", hr);
		result = returnChar(hr);
	}

	// Optionally stop the CLR runtime (we could also leave it running)

	DWORD appDomainId;
	pClrHost->GetCurrentAppDomainId(&appDomainId);
	hr = pClrHost->UnloadAppDomain(appDomainId, true);

    hr = pClrHost->Stop();
	if (hr != S_OK)
	{
		EZLOGGERPRINT("Stop: 0x%08x", hr);
		result = returnChar(hr);
	}

    // Don't forget to clean up.
    pClrHost->Release();
	if (hr != S_OK)
	{
		EZLOGGERPRINT("Release Stop: 0x%08x", hr);
		result = returnChar(hr);
	}
	else
	{
		result = returnChar(0);
	}

	pushstring(result);
	free(result);
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance=hInst;
	return TRUE;
}