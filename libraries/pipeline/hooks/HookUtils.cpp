#include "pch-il2cpp.h"
#include "HookUtils.h"
#include <iostream>
#include "detours/detours.h"

#include "il2cpp-appdata.h"

namespace HookUtils {
    PVOID ResolveMethod(const char* assemblyName, const char* namespaze, const char* className, const char* methodName, int argsCount) {
        if (!app::il2cpp_domain_get || !app::il2cpp_domain_assembly_open || !app::il2cpp_assembly_get_image || !app::il2cpp_class_from_name || !app::il2cpp_class_get_method_from_name) {
            return nullptr;
        }
        app::Il2CppDomain* domain = app::il2cpp_domain_get();
        if (!domain) return nullptr;
        const app::Il2CppAssembly* assembly = app::il2cpp_domain_assembly_open(domain, assemblyName);
        if (!assembly) return nullptr;
        const app::Il2CppImage* image = app::il2cpp_assembly_get_image(assembly);
        if (!image) return nullptr;
        app::Il2CppClass* klass = app::il2cpp_class_from_name(image, namespaze, className);
        if (!klass) return nullptr;
        const app::MethodInfo* method = app::il2cpp_class_get_method_from_name(klass, methodName, argsCount);
        if (!method) return nullptr;
        return (PVOID)method->methodPointer;
    }

    bool HookFunction(PVOID* ppPointer, PVOID pDetour, const char* functionName) {
        if (!ppPointer || !*ppPointer) {
            std::cout << "[INFO]: Target function pointer for " << functionName << " is null, skipping." << std::endl;
            return false;
        }
        if (const auto error = DetourAttach(ppPointer, pDetour); error != NO_ERROR) {
            std::cout << "[WARNING]: Could not hook " << functionName << ", error " << error << " (skipping)" << std::endl;
            return false;
        }
        std::cout << "[HOOKED]: " << functionName << std::endl;
        return true;
    }

    bool UnhookFunction(PVOID* ppPointer, PVOID pDetour, const char* functionName) {
        if (!ppPointer || !*ppPointer) {
            return false;
        }
        if (const auto error = DetourDetach(ppPointer, pDetour); error != NO_ERROR) {
            return false;
        }
        std::cout << "[UNHOOKED]: " << functionName << std::endl;
        return true;
    }
}
