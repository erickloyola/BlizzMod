// Helper functions

#include "pch-il2cpp.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <codecvt>
#include "helpers.h"

// Log file location
extern const LPCWSTR LOG_FILE;

// Helper function to get the module base address
uintptr_t il2cppi_get_base_address() {
    return (uintptr_t) GetModuleHandleW(L"GameAssembly.dll");
}

// Helper function to get exported function address from GameAssembly.dll
void* il2cppi_get_proc_address(const char* procName) {
    static HMODULE hGameAssembly = (HMODULE)il2cppi_get_base_address();
    if (!hGameAssembly) {
        hGameAssembly = (HMODULE)il2cppi_get_base_address();
    }
    return (void*)GetProcAddress(hGameAssembly, procName);
}

// Helper function to append text to a file
void il2cppi_log_write(std::string text) {
    HANDLE hfile = CreateFileW(LOG_FILE, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfile == INVALID_HANDLE_VALUE)
        MessageBoxW(0, L"Could not open log file", 0, 0);

    DWORD written;
    WriteFile(hfile, text.c_str(), (DWORD) text.length(), &written, NULL);
    WriteFile(hfile, "\r\n", 2, &written, NULL);
    CloseHandle(hfile);
}

// Helper function to open a new console window and redirect stdout there
void il2cppi_new_console() {
    AllocConsole();
    freopen_s((FILE**) stdout, "CONOUT$", "w", stdout);
}

#if _MSC_VER >= 1920
static bool IsValidIl2CppString(Il2CppString* str) {
    if (!str) return false;
    __try {
        int32_t len = str->length;
        if (len < 0 || len > 65536) return false;
        if (len > 0) {
            volatile Il2CppChar first = str->chars[0];
            (void)first;
        }
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// Helper function to convert Il2CppString to std::string
std::string il2cppi_to_string(Il2CppString* str) {
    if (!IsValidIl2CppString(str) || str->length <= 0) {
        return "";
    }
    std::u16string u16(reinterpret_cast<const char16_t*>(str->chars), str->length);
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16);
}

// Helper function to convert System.String to std::string
std::string il2cppi_to_string(app::String* str) {
    if (!str) return "";
    return il2cppi_to_string(reinterpret_cast<Il2CppString*>(str));
}
#endif