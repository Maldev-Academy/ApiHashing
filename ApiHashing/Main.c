#include <Windows.h>
#include <stdio.h> // Demo

#include "StringHashing.h" // For The HASH_STRING_* Macros
#include "ReadThroughGadgets.h"	// For InitializeUnguardGadget
#include "ApiHashing.h" // For GetModuleHandleH, GetModuleNameFromHandle, and GetProcAddressH




/*
typedef struct _HASH_ENTRY
{
    PCHAR   pszDefine;
    PCHAR   pszString;
    BOOL    bCaseInsensitive;
} HASH_ENTRY, *PHASH_ENTRY;


int main()
{
    HASH_ENTRY Entries[] =
    {
        { "NTDLL",                           "ntdll.dll",                      TRUE  },
        { "KERNEL32",                        "kernel32.dll",                   TRUE  },
        { "NTALLOCATEVIRTUALMEMORY",         "NtAllocateVirtualMemory",        FALSE },
        { "HEAPALLOC",                       "HeapAlloc",                      FALSE },
        { "ACQUIRESRWLOCKEXCLUSIVE",         "AcquireSRWLockExclusive",        FALSE },
        { "I_QUERYTAGINFORMATION",           "I_QueryTagInformation",          FALSE },
        { "I_SCREGISTERPRESHUTDOWNRESTART",  "I_ScRegisterPreshutdownRestart", FALSE },
        { "D3DKMTACQUIREKEYEDMUTEX",         "D3DKMTAcquireKeyedMutex",        FALSE },
    };

    for (SIZE_T i = 0x00; i < sizeof(Entries) / sizeof(Entries[0]); i++)
    {
        UINT32 uHash = Entries[i].bCaseInsensitive ? HASH_STRING_A_CI(Entries[i].pszString)
            : HASH_STRING_A(Entries[i].pszString);

        printf("#define %s_CRC32B\t\t0x%08X\n", Entries[i].pszDefine, uHash);
    }

    return 0;
}
*/

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==



#define NTDLL_CRC32B                            0x84C05E40
#define KERNEL32_CRC32B                         0x6AE69F02

#define NTALLOCATEVIRTUALMEMORY_CRC32B          0xE0762FEB
#define HEAPALLOC_CRC32B                        0x5EDB1D72
#define ACQUIRESRWLOCKEXCLUSIVE_CRC32B          0x4784E83E
#define I_QUERYTAGINFORMATION_CRC32B            0xD89FDF0C
#define I_SCREGISTERPRESHUTDOWNRESTART_CRC32B   0xB94847F7
#define D3DKMTACQUIREKEYEDMUTEX_CRC32B          0x70F3545E



//
// Demo Helper: Walk The Export Directory To Find The Ordinal For A Given Export Name.
// Used Only For Demonstration Purposes To Avoid Hardcoding Version-Dependent Ordinals For kernel32/ntdll Exports.
//
WORD GetOrdinalForName(IN HMODULE hModule, IN PCSTR pszName)
{
    PIMAGE_DOS_HEADER       pImgDosHdr      = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS       pImgNtHdrs      = (PIMAGE_NT_HEADERS)((PBYTE)hModule + pImgDosHdr->e_lfanew);
    PIMAGE_DATA_DIRECTORY   pImgDataDir     = &pImgNtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    PIMAGE_EXPORT_DIRECTORY pImgExportDir   = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)hModule + pImgDataDir->VirtualAddress);
    PDWORD                  pFuncNamesArray = (PDWORD)((PBYTE)hModule + pImgExportDir->AddressOfNames);
    PWORD                   pNameOrdsArray  = (PWORD)((PBYTE)hModule + pImgExportDir->AddressOfNameOrdinals);

    for (DWORD i = 0; i < pImgExportDir->NumberOfNames; i++)
    {
        PCSTR pExportName = (PCSTR)((PBYTE)hModule + pFuncNamesArray[i]);

        // AddressOfNameOrdinals Gives An Index Into The Address Table; The Real Ordinal Is That Index Plus Base
        if (lstrcmpA(pExportName, pszName) == 0)
            return (WORD)(pNameOrdsArray[i] + pImgExportDir->Base);
    }

    return 0;
}



int main()
{
    HMODULE hAdvapi32   = LoadLibraryA("advapi32.dll");
    HMODULE hGdi32      = LoadLibraryA("gdi32.dll");
    if (!hAdvapi32 || !hGdi32) return -1;


    //
    // Scan ntdll's .text For QWORD/DWORD/WORD Read Gadgets.
    // This Step Is Optional And Only Needed To Bypass EAF.
    //
    // If InitializeUnguardGadget wasnt Called or Didnt Succeed, The Library
    // Still Functions Properly. It Just Wont Route Any Reads Through any Gadget,
    // So EAF Will Fire If Its Being Enforced On The Process.
    //
    //
    {
        //
        // We are Calling GetModuleHandleW/A Here Because Calling GetModuleHandleH Would 
        // Defeat the Purpose and Would Already Trigger EAF 
        //
        HMODULE hGadgetsDll = GetModuleHandle(L"NTDLL");

        if (hGadgetsDll)
        {
            if (!InitializeUnguardGadget(hGadgetsDll, 0x00))
            {
                printf("[!] No Suitable Gadget Found In Ntdll.dll \n");
                return -1;
            }
        }
    }


    HMODULE hNtdll      = GetModuleHandleH(NTDLL_CRC32B);
    HMODULE hKernel32   = GetModuleHandleH(KERNEL32_CRC32B);
    if (!hNtdll || !hKernel32) return -1;

    FARPROC pFunc1      = NULL,
            pFunc2      = NULL;
    WORD    wOrdinal    = 0x00;

    pFunc1 = GetProcAddressH(hNtdll, NTALLOCATEVIRTUALMEMORY_CRC32B);
    pFunc2 = GetProcAddress(hNtdll, "NtAllocateVirtualMemory");
    printf("[+] !ntdll.NtAllocateVirtualMemory | GetProcAddressH : 0x%p \n", pFunc1);
    printf("[+] !ntdll.NtAllocateVirtualMemory | GetProcAddress  : 0x%p \n", pFunc2);
    printf("\n\t----------------------------------------------------------------------------------\n\n");

    // Forwarded: KERNEL32!HeapAlloc -> NTDLL!RtlAllocateHeap
    pFunc1 = GetProcAddressH(hKernel32, HEAPALLOC_CRC32B);
    pFunc2 = GetProcAddress(hKernel32, "HeapAlloc");
    printf("[+] !kernel32.HeapAlloc | GetProcAddressH : 0x%p \n", pFunc1);
    printf("[+] !kernel32.HeapAlloc | GetProcAddress  : 0x%p \n", pFunc2);
    printf("\n\t----------------------------------------------------------------------------------\n\n");

    // Forwarded: KERNEL32!AcquireSRWLockExclusive -> NTDLL!RtlAcquireSRWLockExclusive
    pFunc1 = GetProcAddressH(hKernel32, ACQUIRESRWLOCKEXCLUSIVE_CRC32B);
    pFunc2 = GetProcAddress(hKernel32, "AcquireSRWLockExclusive");
    printf("[+] !kernel32.AcquireSRWLockExclusive | GetProcAddressH : 0x%p \n", pFunc1);
    printf("[+] !kernel32.AcquireSRWLockExclusive | GetProcAddress  : 0x%p \n", pFunc2);
    printf("\n\t----------------------------------------------------------------------------------\n\n");

    // Direct ordinal resolution: HeapCreate is a direct kernel32 export (not forwarded), resolved by its ordinal
    // Ordinals shift across Windows versions, so we look it up at runtime rather than hardcoding via our GetOrdinalForName helper
    wOrdinal = GetOrdinalForName(hKernel32, "HeapCreate");
    pFunc1   = GetProcAddressH(hKernel32, (UINT32)wOrdinal);
    pFunc2   = GetProcAddress(hKernel32, MAKEINTRESOURCEA(wOrdinal));
    printf("[+] !kernel32.#%u (HeapCreate) | GetProcAddressH : 0x%p \n", wOrdinal, pFunc1);
    printf("[+] !kernel32.#%u (HeapCreate) | GetProcAddress  : 0x%p \n", wOrdinal, pFunc2);
    printf("\n\t----------------------------------------------------------------------------------\n\n");

    // Ordinal-Through-Forwarder: HeapAlloc's ordinal lands on a forwarder string; the forwarder detection
    // fires regardless of whether we came in by name or by ordinal, and the recursive resolve goes by name
    wOrdinal = GetOrdinalForName(hKernel32, "HeapAlloc");
    pFunc1   = GetProcAddressH(hKernel32, (UINT32)wOrdinal);
    pFunc2   = GetProcAddress(hKernel32, MAKEINTRESOURCEA(wOrdinal));
    printf("[+] !kernel32.#%u (HeapAlloc, forwarded) | GetProcAddressH : 0x%p \n", wOrdinal, pFunc1);
    printf("[+] !kernel32.#%u (HeapAlloc, forwarded) | GetProcAddress  : 0x%p \n", wOrdinal, pFunc2);
    printf("\n\t----------------------------------------------------------------------------------\n\n");

    // API Set Test 1: advapi32!I_QueryTagInformation (api- prefix + name forwarder)
    // Forwards through api-ms-win-service-private-l1-1-0.dll -> sechost.dll (or kernelbase.dll)
    pFunc1 = GetProcAddressH(hAdvapi32, I_QUERYTAGINFORMATION_CRC32B);
    pFunc2 = GetProcAddress(hAdvapi32, "I_QueryTagInformation");
    printf("[+] !advapi32.I_QueryTagInformation | GetProcAddressH : 0x%p \n", pFunc1);
    printf("[+] !advapi32.I_QueryTagInformation | GetProcAddress  : 0x%p \n", pFunc2);
    printf("\n\t----------------------------------------------------------------------------------\n\n");

    // API Set Test 2: advapi32!I_ScRegisterPreshutdownRestart
    // Forwards through api-ms-win-service-private-l1-1-1.dll (a different contract version)
    pFunc1 = GetProcAddressH(hAdvapi32, I_SCREGISTERPRESHUTDOWNRESTART_CRC32B);
    pFunc2 = GetProcAddress(hAdvapi32, "I_ScRegisterPreshutdownRestart");
    printf("[+] !advapi32.I_ScRegisterPreshutdownRestart | GetProcAddressH : 0x%p \n", pFunc1);
    printf("[+] !advapi32.I_ScRegisterPreshutdownRestart | GetProcAddress  : 0x%p \n", pFunc2);
    printf("\n\t----------------------------------------------------------------------------------\n\n");

    // API Set Test 3: gdi32!D3DKMTAcquireKeyedMutex (ext- prefix + ORDINAL forwarder)
    // Forwards through ext-ms-win-dx-d3dkmt-dxcore-l1-1-5.#73 -> the resolved host, then by ordinal
    // Exercises: ext- prefix branch, ordinal parsing in the forwarder handler, recursive resolve by ordinal
    pFunc1 = GetProcAddressH(hGdi32, D3DKMTACQUIREKEYEDMUTEX_CRC32B);
    pFunc2 = GetProcAddress(hGdi32, "D3DKMTAcquireKeyedMutex");
    printf("[+] !gdi32.D3DKMTAcquireKeyedMutex | GetProcAddressH : 0x%p \n", pFunc1);
    printf("[+] !gdi32.D3DKMTAcquireKeyedMutex | GetProcAddress  : 0x%p \n", pFunc2);
    printf("\n\t----------------------------------------------------------------------------------\n\n");

    return 0;
}
