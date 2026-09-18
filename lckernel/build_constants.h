#define VERSION_MAJOR 3
#define VERSION_MINOR 0

#ifdef _MSC_VER
#define EXPORT_API __declspec(dllexport)
#define IMPORT_API __declspec(dllimport)
#else
#define EXPORT_API
#define IMPORT_API
#endif

extern IMPORT_API const char* BUILD_DATE;
extern IMPORT_API const char* BUILD_INFO;
extern IMPORT_API const char* CMAKE_VERSION;
extern IMPORT_API const char* SETTINGS_PATH;
extern IMPORT_API const char* PATTERN_RESOURCE_PATH;
// libdxfrw package version + submodule revision; DRW_VERSION is a frozen
// legacy banner ("0.6.3") and does not identify the library that is linked.
extern IMPORT_API const char* LIBDXFRW_BUILD_VERSION;
