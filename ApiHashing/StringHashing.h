#pragma once
#ifndef STRING_HASHING_H
#define STRING_HASHING_H

#include <Windows.h>


#ifdef __cplusplus
extern "C" {
#endif

	UINT32 HashStringCrc32bA(IN PCHAR pszString);

	UINT32 HashIStringCrc32bA(IN PCHAR pszString);

	UINT32 HashStringCrc32bW(IN PWCHAR pwszString);

	UINT32 HashIStringCrc32bW(IN PWCHAR pwszString);

#ifdef __cplusplus
}
#endif



#define HASH_STRING_A(STR)			HashStringCrc32bA((PCHAR)(STR))

#define HASH_STRING_A_CI(STR)		HashIStringCrc32bA((PCHAR)(STR))

#define HASH_STRING_W(STR)			HashStringCrc32bW((PWCHAR)(STR))

#define HASH_STRING_W_CI(STR)		HashIStringCrc32bW((PWCHAR)(STR))



#endif // !STRING_HASHING_H
