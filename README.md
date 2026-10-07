# ApiHashing

Hash-based x64 replacements for [GetModuleHandleA/W](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandlew), [GetProcAddress](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getprocaddress), and [GetModuleFileNameW](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew) WinAPIs. Supporting EAF bypass via read gadgets, forwarded functions, ordinal exports, and API set DLLs.

<br>

## Features

* Hash-based module and function resolution via the PEB loader list and PE export directory.

* Forwarded export resolution by both name and ordinal.

* API Set contract resolution (`api-ms-*`, `ext-ms-*`), covering versions V2, V3, V4, and V6.

* Parent-module hint honored when an API Set exposes version-specific host overrides.

* EAF-safe reads routed through QWORD/DWORD/WORD gadgets scanned out of a target module's `.text` section:

| Width | Primary                            | Alternate                          |
|-------|-------------------------------------|-------------------------------------|
| QWORD | `mov rax, [rax]; ret`               | `mov rax, [rcx]; ret`               |
| DWORD | `mov eax, [rax]; ret`               | `mov eax, [rcx]; ret`               |
| WORD  | `movzx eax, word ptr [rax]; ret`    | `movzx eax, word ptr [rcx]; ret`    |
 
<br>

## Usage

* [GetModuleHandleH](ApiHashing/ApiHashing.c) - Replaces `GetModuleHandleA/W`. Walks the PEB's `InLoadOrderModuleList` and matches on a CRC32B hash of the module name.

* [GetProcAddressH](ApiHashing/ApiHashing.c) - Replaces `GetProcAddress`. Resolves by name hash; a value whose `HIWORD` is 0 is treated as an ordinal. Automatically handles forwarders and API Set redirections.

* [GetModuleNameFromHandle](ApiHashing/ApiHashing.c) - Replaces `GetModuleFileNameW`. Reverse lookup from an `HMODULE` back to the base DLL name over the same loader list.

* [InitializeUnguardGadget](ApiHashing/ReadThroughGadgets.c) - Scans a specified module's `.text` for QWORD/DWORD/WORD read gadgets and installs them globally. *This function is Optional*; without it, reads fall back to a direct dereference and EAF will still fire if it's being enforced, so call it early for any EAF-sensitive workload.

* [HASH_STRING_A / HASH_STRING_A_CI / HASH_STRING_W / HASH_STRING_W_CI](ApiHashing/StringHashing.h) - Are the CRC32B macros that produce the hash constants `GetModuleHandleH` and `GetProcAddressH` consume.


<br>

## Demo

<img width="939" height="933" alt="image_2026-10-07_12-35-45" src="https://github.com/user-attachments/assets/101f6545-396d-413a-9f38-cf4af975a7d2" />
<br>


## Credits
 
API Set namespace structures and traversal are adapted from [ajkhoury/ApiSet](https://github.com/ajkhoury/ApiSet).


