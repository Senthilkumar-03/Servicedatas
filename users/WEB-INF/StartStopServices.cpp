#include<iostream>
#include<conio.h>
#include <windows.h>
#include<jni.h>
#include "Check.h"
BOOL __stdcall StopDependentServices(SC_HANDLE schService, SC_HANDLE schSCManager);

using namespace std;
JNIEXPORT jboolean JNICALL Java_Check_checks(JNIEnv* env, jobject obj, jstring serviceName)
{
	bool status = false;
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwOldCheckPoint;
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;
																	// Get a handle to the SCM databases 

	SC_HANDLE schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS); 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return status;
	}
                                                                     // Get a handle to the service
	LPCWSTR szSvcName = (LPWSTR)env->GetStringChars(serviceName, 0);
	SC_HANDLE schService = OpenService(schSCManager,szSvcName,SERVICE_ALL_ACCESS);  

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return status;
	}
																	// To checking the status if the service is not stopped 
	if (!QueryServiceStatusEx(schService,SC_STATUS_PROCESS_INFO,(LPBYTE)&ssStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded))              
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return status;
	}																//To check the services is already running

	if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
	{
		printf("Cannot start the service because it is already running\n");
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return status;
	}
																	// Save the tick count and initial checkpoint
	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;
																	// Waiting for the services to stop before attempting to start it

	while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
	{

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);
																	// Check the status of the services 
	if (!QueryServiceStatusEx(
			schService,                     
			SC_STATUS_PROCESS_INFO,         
			(LPBYTE)&ssStatus,             
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded))              
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return status;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{
			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				printf("Timeout waiting for service to stop\n");
				CloseServiceHandle(schService);
				CloseServiceHandle(schSCManager);
				return status;
			}
		}
	}
																		// Attempting to start the services

	if (!StartService(schService,0,NULL))      
	{
		printf("StartService failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return status;
	}
	else printf("Service start pending...\n");

	if (!QueryServiceStatusEx(schService,SC_STATUS_PROCESS_INFO,(LPBYTE)&ssStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded))              
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return status;
	}

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
	{

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		if (!QueryServiceStatusEx(schService,SC_STATUS_PROCESS_INFO,(LPBYTE)&ssStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded))             
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			break;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				break;
			}
		}
	}
																		// To check whether the service is running

	if (ssStatus.dwCurrentState == SERVICE_RUNNING)
	{
		status = true;
		printf("Service started successfully.\n");
	}
	else
	{
		printf("Service not started. \n");
		printf("  Current State: %d\n", ssStatus.dwCurrentState);
		printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
		printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
		printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return status;
}

JNIEXPORT jboolean JNICALL Java_Check_stops(JNIEnv* env, jobject obj, jstring serviceName)
{
	bool status = false;
	SERVICE_STATUS_PROCESS ssp;
	DWORD dwStartTime = GetTickCount();
	DWORD dwBytesNeeded;
	DWORD dwTimeout = 30000; 
	DWORD dwWaitTime;

	SC_HANDLE schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);  

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return status;
	}

	LPCWSTR szSvcName = (LPWSTR)env->GetStringChars(serviceName, 0);
	SC_HANDLE schService = OpenService(schSCManager,szSvcName,SERVICE_STOP |SERVICE_QUERY_STATUS |SERVICE_ENUMERATE_DEPENDENTS);

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return status;
	}
																// To check the services is not already stopped
	if (!QueryServiceStatusEx(
		schService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&ssp,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded))
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		goto stop_cleanup;
	}

	if (ssp.dwCurrentState == SERVICE_STOPPED)
	{
		printf("Service is already stopped.\n");
		goto stop_cleanup;
	}

	while (ssp.dwCurrentState == SERVICE_STOP_PENDING)
	{
		printf("Service stop pending...\n");

		dwWaitTime = ssp.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		if (!QueryServiceStatusEx(
			schService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded))
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			goto stop_cleanup;
		}

		if (ssp.dwCurrentState == SERVICE_STOPPED)
		{
			printf("Service stopped successfully.\n");
			goto stop_cleanup;
		}

		if (GetTickCount() - dwStartTime > dwTimeout)
		{
			printf("Service stop timed out.\n");
			goto stop_cleanup;
		}
	}
															// If the service is running, dependencies must be stopped first

	StopDependentServices(schService, schSCManager);


	if (!ControlService(schService,SERVICE_CONTROL_STOP,(LPSERVICE_STATUS)&ssp))
	{
		printf("ControlService failed (%d)\n", GetLastError());
		goto stop_cleanup;
	}
															// Wait for the service to stop

	while (ssp.dwCurrentState != SERVICE_STOPPED)
	{
		Sleep(ssp.dwWaitHint);
		if (!QueryServiceStatusEx(
			schService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded))
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			goto stop_cleanup;
		}

		if (ssp.dwCurrentState == SERVICE_STOPPED)
			break;

		if (GetTickCount() - dwStartTime > dwTimeout)
		{
			printf("Wait timed out\n");
			goto stop_cleanup;
		}
	}
	printf("Service stopped successfully\n");
	status = true;

stop_cleanup:
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return status;
}

BOOL __stdcall StopDependentServices(SC_HANDLE schService, SC_HANDLE schSCManager)
{
	DWORD i;
	DWORD dwBytesNeeded;
	DWORD dwCount;

	LPENUM_SERVICE_STATUS   lpDependencies = NULL;
	ENUM_SERVICE_STATUS     ess;
	SC_HANDLE               hDepService;
	SERVICE_STATUS_PROCESS  ssp;

	DWORD dwStartTime = GetTickCount();
	DWORD dwTimeout = 30000;							 // 30s timeout
														// Pass a zerolength buffer to get the required buffer size
	if (EnumDependentServices(schService, SERVICE_ACTIVE,pDependencies, 0, &dwBytesNeeded, &dwCount))
	{
		return TRUE;
	}
	else
	{
		if (GetLastError() != ERROR_MORE_DATA)
			return FALSE; 
		lpDependencies = (LPENUM_SERVICE_STATUS)HeapAlloc(
			GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);

		if (!lpDependencies)
			return FALSE;

		__try {
			if (!EnumDependentServices(schService, SERVICE_ACTIVE,
				lpDependencies, dwBytesNeeded, &dwBytesNeeded,
				&dwCount))
				return FALSE;

			for (i = 0; i < dwCount; i++)
			{
				ess = *(lpDependencies + i);
				hDepService = OpenService(schSCManager,
					ess.lpServiceName,
					SERVICE_STOP | SERVICE_QUERY_STATUS);

				if (!hDepService)
					return FALSE;

				__try {
					if (!ControlService(hDepService,
						SERVICE_CONTROL_STOP,
						(LPSERVICE_STATUS)&ssp))
						return FALSE;
						
					while (ssp.dwCurrentState != SERVICE_STOPPED)
					{
						Sleep(ssp.dwWaitHint);
						if (!QueryServiceStatusEx(
							hDepService,
							SC_STATUS_PROCESS_INFO,
							(LPBYTE)&ssp,
							sizeof(SERVICE_STATUS_PROCESS),
							&dwBytesNeeded))
							return FALSE;

						if (ssp.dwCurrentState == SERVICE_STOPPED)
							break;

						if (GetTickCount() - dwStartTime > dwTimeout)
							return FALSE;
					}
				}
				__finally
				{
					CloseServiceHandle(hDepService);
				}
			}
		}
		__finally
		{
			HeapFree(GetProcessHeap(), 0, lpDependencies);
		}
	}
}
