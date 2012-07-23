#include "exdll.h"
#include <string>
#include <vector>
using namespace std;
using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Reflection;

HINSTANCE g_hInstance;
HWND g_hwndParent;

System::Reflection::Assembly ^ LoadAssembly(String ^filename)
{
	Assembly ^assembly = nullptr;
	FileStream ^fs = File::Open(filename, FileMode::Open);
	if (fs != nullptr)
	{
		MemoryStream ^ms = gcnew MemoryStream();
		if (ms != nullptr)
		{
			cli::array<unsigned char> ^buffer = gcnew cli::array<unsigned char>(1024);
			int read = 0;
			while ((read = fs->Read(buffer, 0, 1024))>0)
				ms->Write(buffer, 0, read);
			assembly = Assembly::Load(ms->ToArray());
			ms->Close();
			delete ms;
		}
		fs->Close();
		delete fs;
	}

	return assembly;

	//http://forums.msdn.microsoft.com/en-US/clr/thread/093c3606-e68e-46f4-98a1-f2396d3f88ca/
}

char* CallCLR(string DllName, string ClassWithNamespace, string MethodName, vector<string> Args, bool loadInMemory)
{
	try
	{
		// get DLLName, Namespace, Class and Method
		String ^sDLLName = (gcnew String(DllName.c_str()))->Trim();
		if (!(sDLLName->ToLower()->EndsWith(".dll") || sDLLName->ToLower()->EndsWith(".exe")))
			sDLLName += ".dll";
		String ^sClassWithNamespace = (gcnew String(ClassWithNamespace.c_str()))->Trim();
		String ^sMethod = (gcnew String(MethodName.c_str()))->Trim();

		// load assembly (is closed on disk right away)
		System::Reflection::Assembly ^assembly;
		
		if (loadInMemory)
		{
			assembly = LoadAssembly(".\\" + sDLLName);
		}
		else
		{
			assembly = System::Reflection::Assembly::LoadFrom(".\\" + sDLLName);
		}
		if (assembly == nullptr)
			throw gcnew Exception("Error loading .NET assembly");

		// attempt to find the class
		bool found = false;
		cli::array<Type ^> ^types = assembly->GetExportedTypes();
		for (int i=0; i<types->Length; i++)
		{
			if (types[i]->FullName == sClassWithNamespace)
			{
				found = true;

				// found the class, create instance, get methodinfo and parameter info
				System::Object ^instance = assembly->CreateInstance(sClassWithNamespace);
				System::Reflection::MethodInfo ^methodinfo = instance->GetType()->GetMethod(sMethod);
				if (methodinfo == nullptr)
				{
					throw gcnew Exception(String::Format(
						"Method {0} not found in class {1}",
						sMethod, sClassWithNamespace));
				}
				cli::array<ParameterInfo ^> ^paraminfos = methodinfo->GetParameters();								

				// check that there are the same number of parameters in input and in dll to be called
				if (Args.size() != paraminfos->Length)
				{
					throw gcnew Exception("Number of parameters does not match");
				}

				// create a new Object^ array with data types that
				// corresponds to what the .NET method expects
				cli::array<Object ^> ^params = gcnew cli::array<Object ^>(Args.size());
				for (int i=0; i<(int)Args.size(); i++)
				{
					if (paraminfos[i]->ParameterType->FullName == "System.String")
						params[i] = Convert::ToString(gcnew String(Args[i].c_str()));
					else if (paraminfos[i]->ParameterType->FullName == "System.Char")
						params[i] = Convert::ToChar(gcnew String(Args[i].c_str()));
					else if (paraminfos[i]->ParameterType->FullName == "System.Int32")
						params[i] = Convert::ToInt32(gcnew String(Args[i].c_str()));
					else if (paraminfos[i]->ParameterType->FullName == "System.Int64")
						params[i] = Convert::ToInt64(gcnew String(Args[i].c_str()));
					else if (paraminfos[i]->ParameterType->FullName == "System.Int16")
						params[i] = Convert::ToInt16(gcnew String(Args[i].c_str()));
					else if ((paraminfos[i]->ParameterType->FullName == "System.Double") ||
						(paraminfos[i]->ParameterType->FullName == "System.Float"))
						params[i] = Convert::ToDouble(gcnew String(Args[i].c_str()));
					else if (paraminfos[i]->ParameterType->FullName == "System.Boolean")
						params[i] = Convert::ToBoolean(gcnew String(Args[i].c_str()));
					else if (paraminfos[i]->ParameterType->FullName == "System.IntPtr")
						params[i] = gcnew System::IntPtr(Convert::ToInt32(gcnew String(Args[i].c_str())));
				}

				char* result;

				// invoke method and return any value
				System::Object ^retValue = methodinfo->Invoke(instance, params);
				if (retValue != nullptr)
					result = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(retValue->ToString());
				else
					result = "";

				// cleanup
				DllName.clear();
				ClassWithNamespace.clear();
				MethodName.clear();
				Args.clear();
				delete sDLLName;
				delete sClassWithNamespace;
				delete sMethod;
				delete params;
				assembly = nullptr;
				delete assembly;
				delete instance;
				delete methodinfo;
				/*if (retValue != nullptr)
				delete retValue;*/

				return result;
			}
		}

		if (!found)
		{
			throw gcnew Exception(String::Format(
				"Class {0} not found in {1}",
				sClassWithNamespace, sDLLName));
		}

		return "";
	}
	catch (System::Exception ^ex)
	{
		String ^msg = "Error calling .NET DLL method\n\n" + ex->Message + "\n\n" + ex->StackTrace ;
		Windows::Forms::MessageBox::Show(msg);
		return "";
	}
}

extern "C" __declspec(dllexport) void CallMemory(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
	// expected parameters:
	// filename.dll, namespace.namespace...class, method, numparams, params...

	g_hwndParent=hwndParent;
	EXDLL_INIT();

	char buf[1024];

	// filename.dll
	popstring(buf);
	string dllname = string(buf);

	// namespace and class
	popstring(buf);
	string classwithnamespace = string(buf);

	// method
	popstring(buf);
	string method = string(buf);

	// num params
	popstring(buf);
	int numparams = atoi(buf);

	// params
	vector<string> args;
	for (int i=0; i<numparams; i++)
	{
		popstring(buf);
		string tmp = string(buf);
		args.push_back(tmp);
	}
	
	char* result = (char*)CallCLR(dllname, classwithnamespace, method, args, true);
	pushstring(result);
}

extern "C" __declspec(dllexport) void Call(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
	// expected parameters:
	// filename.dll, namespace.namespace...class, method, numparams, params...

	g_hwndParent=hwndParent;
	EXDLL_INIT();

	char buf[1024];

	// filename.dll
	popstring(buf);
	string dllname = string(buf);

	// namespace and class
	popstring(buf);
	string classwithnamespace = string(buf);

	// method
	popstring(buf);
	string method = string(buf);

	// num params
	popstring(buf);
	int numparams = atoi(buf);

	// params
	vector<string> args;
	for (int i=0; i<numparams; i++)
	{
		popstring(buf);
		string tmp = string(buf);
		args.push_back(tmp);
	}
	
	char* result = (char*)CallCLR(dllname, classwithnamespace, method, args, false);
	pushstring(result);
}

extern "C" __declspec(dllexport) void Destroy(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
}

//#pragma unmanaged
// Upgrade to Hosted CLR => http://msdn.microsoft.com/en-us/magazine/cc163567.aspx
//BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
//{
//	g_hInstance=(HINSTANCE)hInst;
//	return TRUE;
//}