#define VERSION_MAJOR 3
#define VERSION_MINOR 0

#ifdef Q_CC_MSVC
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
