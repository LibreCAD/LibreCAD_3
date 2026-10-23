#pragma once

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <cstdlib>
#include <fstream>
#include <string>

// Opt-in CI evidence before --help exits; never creates a graphics context or
// changes normal CLI behavior. Observe loaded DLLs, not guessed import paths.
inline bool writeRendererModuleInventory() {
    const char* output = std::getenv("LC_RENDERER_DLL_REPORT");
    if (!output) return true;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if (snapshot == INVALID_HANDLE_VALUE) return false;
    std::ofstream stream(output, std::ios::binary);
    MODULEENTRY32W module = {};
    module.dwSize = sizeof(module);
    bool ok = stream.good() && Module32FirstW(snapshot, &module);
    if (ok) do {
        int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, module.szExePath,
                                      -1, nullptr, 0, nullptr, nullptr);
        if (size <= 1) { ok = false; break; }
        std::string path(static_cast<size_t>(size), '\0');
        if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, module.szExePath,
                                 -1, &path[0], size, nullptr, nullptr)) { ok = false; break; }
        stream.write(path.data(), size - 1);
        stream.put('\n');
    } while (Module32NextW(snapshot, &module));
    if (ok && GetLastError() != ERROR_NO_MORE_FILES) ok = false;
    CloseHandle(snapshot);
    stream.flush();
    return ok && stream.good();
}
#endif
