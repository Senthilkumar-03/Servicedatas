#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
#include "Services.h"


JNIEXPORT jobject JNICALL Java_Services_services(JNIEnv *env, jobject obj)
{
    HRESULT hres;   // 32-bit value used to describe error or warning

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);    // Initiliaze the COM library on the current thread
    if (FAILED(hres))
    {
        cout << "Failed to initialize COM library. Error code = 0x"
            << hex << hres << endl;
        return NULL;                
    }


    hres = CoInitializeSecurity(NULL,-1,NULL,NULL,RPC_C_AUTHN_LEVEL_DEFAULT,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE,NULL);
																				
																				//To authenticate the securitylevel

    if (FAILED(hres))
    {
        cout << "Failed to initialize security. Error code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return NULL;                    
    }

    IWbemLocator* pLoc = NULL;                             // Connector Servrmethod

														   //Cocreate to create only oneobject on the localsystem
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres))
    {
        cout << "Failed to create IWbemLocator object."
            << " Err code = 0x"
            << hex << hres << endl;
        CoUninitialize();
        return NULL;                
    }


    IWbemServices* pSvc = NULL;                            // Interface used by clients and provides to access WMI services
	
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL,NULL,0,0L,0,0,&pSvc);

    if (FAILED(hres))
    {
        cout << "Could not connect. Error code = 0x"
            << hex << hres << endl;
        pLoc->Release();
        CoUninitialize();
        return NULL;        
    }

															// Set the authentication information that will be used to make calls on specify proxy
    hres = CoSetProxyBlanket(pSvc,RPC_C_AUTHN_WINNT,RPC_C_AUTHZ_NONE,NULL,RPC_C_AUTHN_LEVEL_CALL,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE);

    if (FAILED(hres))
    {
        cout << "Could not set proxy blanket. Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return NULL;            
    }

    IEnumWbemClassObject* pEnumerator = NULL;                   // Userdefined type that consists of a setof named integral constraints
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_Service"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        cout << "Query for operating system name failed."
            << " Error code = 0x"
            << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return NULL;            
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;                                    
    jclass classHm = env->FindClass("java/util/HashMap");     // getting HashMap class
    jmethodID construtorHashMap = env->GetMethodID(classHm, "<init>", "()V");  // Construtor for hashmap
    jmethodID putId = env->GetMethodID(classHm, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");  // Method id for HassMap.put method
    jclass classAL = env->FindClass("java/util/ArrayList");  // getting HashMap class
    jmethodID constructorAL = env->GetMethodID(classAL, "<init>", "()V");
    jmethodID addId = env->GetMethodID(classAL, "add", "(Ljava/lang/Object;)Z");
    jobject serviceDetails = env->NewObject(classAL, constructorAL);   

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;                                    // Tagged Union that can create string

															
															
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);        // Get the value of the Name property
        //wcout << " Service Name : " << vtProp.bstrVal << endl;
        jstring jServiceName = env->NewString((jchar*)((LPWSTR)vtProp.bstrVal), wcslen(vtProp.bstrVal));
        VariantClear(&vtProp);
		

       /* VARIANT vtPropDes;
        hr = pclsObj->Get(L"Description", 0, &vtPropDes, 0, 0);        
        wcout << "Description :" << vtPropDes.bstrVal << endl;
        VariantClear(&vtPropDes); */
		
		

        VARIANT vtPropStat;                                  
        hr = pclsObj->Get(L"State", 0, &vtPropStat, 0, 0);             //Get the value of the status of the service
        //wcout << "Status :" << vtPropStat.bstrVal << endl;
        jstring jstate = env->NewString((jchar*)((LPWSTR)vtPropStat.bstrVal), wcslen(vtPropStat.bstrVal));
        VariantClear(&vtPropStat);
		

        VARIANT vtPropStart;
        hr = pclsObj->Get(L"StartMode", 0, &vtPropStart, 0, 0);
       // wcout << "StartUp Type: " << vtPropStart.bstrVal << endl;
        jstring jstartup = env->NewString((jchar*)((LPWSTR)vtPropStart.bstrVal), wcslen(vtPropStart.bstrVal));
        VariantClear(&vtPropStart);

        VARIANT vtPropName;
        hr = pclsObj->Get(L"StartName", 0, &vtPropName, 0, 0);           
        //wcout << "LogOnas: " << vtPropName.bstrVal << endl;
        //jstring jlogonas = env->NewString((jchar*)((LPWSTR)vtPropName.bstrVal), wcslen(vtPropName.bstrVal));
        LPWSTR logonas = vtPropName.bstrVal;
        jstring jlogonas = NULL;
        if (logonas != NULL)
        {
            jlogonas = env->NewString((jchar*)vtPropName.bstrVal, wcslen(vtPropName.bstrVal));
        }
        else {
            jlogonas = (env)->NewStringUTF("---");
        }
        VariantClear(&vtPropName);          
			                                           // Inserting the datas into HashMap
        jobject serviceMap = env->NewObject(classHm, construtorHashMap);    
        env->CallObjectMethod(serviceMap, putId, (env)->NewStringUTF("Logon as"), jlogonas);
        env->CallObjectMethod(serviceMap, putId, (env)->NewStringUTF("Service Name"), jServiceName);
        env->CallObjectMethod(serviceMap, putId, (env)->NewStringUTF("Status"), jstate);
        env->CallObjectMethod(serviceMap, putId, (env)->NewStringUTF("StartUp Type"), jstartup);
        
        env->CallObjectMethod(serviceDetails, addId, serviceMap);
        env->DeleteLocalRef(serviceMap);
        pclsObj->Release();
    }


    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
    

    return serviceDetails;                       // Returning the arrayList to the javaFile

}
