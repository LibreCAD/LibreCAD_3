#pragma once

#ifdef WINDOWS_MSYS2
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif