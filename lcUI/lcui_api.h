#pragma once

// lcui is the shared test-facing UI library; librecad compiles the same
// sources directly. Automatic Windows exports do not import data symbols
// (including Qt's staticMetaObject) into DLL consumers.
#if defined(_WIN32) && defined(LC_UI_SHARED)
#if defined(lcui_EXPORTS)
#define LC_UI_API __declspec(dllexport)
#else
#define LC_UI_API __declspec(dllimport)
#endif
#else
#define LC_UI_API
#endif
