#pragma once

#include <Windows.h>

namespace HookUtils {
    PVOID ResolveMethod(const char* assemblyName, const char* namespaze, const char* className, const char* methodName, int argsCount);
    bool HookFunction(PVOID* ppPointer, PVOID pDetour, const char* functionName);
    bool UnhookFunction(PVOID* ppPointer, PVOID pDetour, const char* functionName);
}
