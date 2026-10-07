#pragma once
#ifndef API_HASHING_H
#define API_HASHING_H

#include <Windows.h>


#ifdef __cplusplus
extern "C" {
#endif

	HMODULE GetModuleHandleH(IN UINT32 uModuleNameHash);

	BOOL GetModuleNameFromHandle(IN HMODULE hModule, OUT PWSTR pwszModuleName);

	FARPROC GetProcAddressH(IN HMODULE hModule, IN UINT32 uFunctionNameHash);

#ifdef __cplusplus
}
#endif


#endif // !API_HASHING_H
