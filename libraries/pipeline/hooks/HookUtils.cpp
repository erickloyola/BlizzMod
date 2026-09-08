#include "pch-il2cpp.h"
#include "HookUtils.h"
#include <iostream>
#include "detours/detours.h"

#include "il2cpp-appdata.h"

namespace HookUtils {
    PVOID ResolveMethod(const char* assemblyName, const char* namespaze, const char* className, const char* methodName, int argsCount) {
        if (!il2cpp_domain_get) {
            return nullptr;
        }
        Il2CppDomain* domain = il2cpp_domain_get();
        if (!domain) return nullptr;

        const Il2CppImage* image = nullptr;

        // 1. Try il2cpp_domain_assembly_open with original name and without/with .dll
        if (il2cpp_domain_assembly_open && il2cpp_assembly_get_image) {
            const Il2CppAssembly* assembly = il2cpp_domain_assembly_open(domain, assemblyName);
            if (!assembly) {
                std::string sName = assemblyName;
                if (sName.length() > 4 && sName.substr(sName.length() - 4) == ".dll") {
                    assembly = il2cpp_domain_assembly_open(domain, sName.substr(0, sName.length() - 4).c_str());
                } else {
                    assembly = il2cpp_domain_assembly_open(domain, (sName + ".dll").c_str());
                }
            }
            if (assembly) {
                image = il2cpp_assembly_get_image(assembly);
            }
        }

        // 2. Fallback: iterate over all loaded domain assemblies
        if (!image && il2cpp_domain_get_assemblies && il2cpp_assembly_get_image && il2cpp_image_get_name) {
            size_t count = 0;
            const Il2CppAssembly** assemblies = il2cpp_domain_get_assemblies(domain, &count);
            if (assemblies) {
                std::string target = assemblyName;
                if (target.length() > 4 && target.substr(target.length() - 4) == ".dll") {
                    target = target.substr(0, target.length() - 4);
                }
                for (size_t i = 0; i < count; i++) {
                    const Il2CppImage* img = il2cpp_assembly_get_image(assemblies[i]);
                    if (!img) continue;
                    const char* imgName = il2cpp_image_get_name(img);
                    if (!imgName) continue;
                    std::string curName = imgName;
                    if (curName.length() > 4 && curName.substr(curName.length() - 4) == ".dll") {
                        curName = curName.substr(0, curName.length() - 4);
                    }
                    if (_stricmp(curName.c_str(), target.c_str()) == 0) {
                        image = img;
                        break;
                    }
                }
            }
        }

        if (!image || !il2cpp_class_from_name || !il2cpp_class_get_method_from_name) {
            return nullptr;
        }

        Il2CppClass* klass = il2cpp_class_from_name(image, (namespaze && *namespaze) ? namespaze : "", className);
        if (!klass) return nullptr;

        const MethodInfo* method = il2cpp_class_get_method_from_name(klass, methodName, argsCount);
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
