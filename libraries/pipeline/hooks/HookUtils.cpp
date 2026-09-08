#include "pch-il2cpp.h"
#include "HookUtils.h"
#include <iostream>
#include <string>
#include <cstring>
#include "detours/detours.h"

#include "il2cpp-appdata.h"

namespace HookUtils {

    static bool MatchAssemblyName(const char* candidate, const char* target) {
        if (!candidate || !target) return false;
        std::string c = candidate;
        std::string t = target;
        if (c.length() > 4 && c.substr(c.length() - 4) == ".dll") c = c.substr(0, c.length() - 4);
        if (t.length() > 4 && t.substr(t.length() - 4) == ".dll") t = t.substr(0, t.length() - 4);
        return (_stricmp(c.c_str(), t.c_str()) == 0);
    }

    static const Il2CppImage* FindImage(Il2CppDomain* domain, const char* assemblyName) {
        if (!domain) return nullptr;

        // 1. Try direct open
        if (il2cpp_domain_assembly_open && il2cpp_assembly_get_image) {
            std::string sName = assemblyName ? assemblyName : "";
            if (sName.length() > 4 && sName.substr(sName.length() - 4) == ".dll") {
                sName = sName.substr(0, sName.length() - 4);
            }
            const Il2CppAssembly* asmPtr = il2cpp_domain_assembly_open(domain, sName.c_str());
            if (!asmPtr) {
                asmPtr = il2cpp_domain_assembly_open(domain, (sName + ".dll").c_str());
            }
            if (asmPtr) {
                const Il2CppImage* img = il2cpp_assembly_get_image(asmPtr);
                if (img) return img;
            }
        }

        // 2. Iterate domain assemblies
        if (il2cpp_domain_get_assemblies && il2cpp_assembly_get_image && il2cpp_image_get_name) {
            size_t count = 0;
            const Il2CppAssembly** assemblies = il2cpp_domain_get_assemblies(domain, &count);
            if (assemblies) {
                for (size_t i = 0; i < count; i++) {
                    const Il2CppImage* img = il2cpp_assembly_get_image(assemblies[i]);
                    if (!img) continue;
                    const char* imgName = il2cpp_image_get_name(img);
                    if (MatchAssemblyName(imgName, assemblyName)) {
                        return img;
                    }
                }
            }
        }

        return nullptr;
    }

    static Il2CppClass* FindClass(const Il2CppImage* image, const char* namespaze, const char* className) {
        if (!image || !className) return nullptr;

        Il2CppClass* klass = nullptr;

        // 1. Try il2cpp_class_from_name
        if (il2cpp_class_from_name) {
            klass = il2cpp_class_from_name(image, (namespaze && *namespaze) ? namespaze : "", className);
            if (!klass && (!namespaze || !*namespaze)) {
                klass = il2cpp_class_from_name(image, nullptr, className);
            }
        }
        if (klass) return klass;

        // 2. Iterate all classes in image
        if (il2cpp_image_get_class_count && il2cpp_image_get_class && il2cpp_class_get_name) {
            size_t numClasses = il2cpp_image_get_class_count(image);
            for (size_t i = 0; i < numClasses; i++) {
                const Il2CppClass* k = il2cpp_image_get_class(image, i);
                if (!k) continue;
                const char* cName = il2cpp_class_get_name(const_cast<Il2CppClass*>(k));
                if (cName && strcmp(cName, className) == 0) {
                    if (namespaze && *namespaze && il2cpp_class_get_namespace) {
                        const char* cNs = il2cpp_class_get_namespace(const_cast<Il2CppClass*>(k));
                        if (cNs && strcmp(cNs, namespaze) != 0) continue;
                    }
                    return const_cast<Il2CppClass*>(k);
                }
            }
        }

        return nullptr;
    }

    static const MethodInfo* FindMethod(Il2CppClass* klass, const char* methodName, int argsCount) {
        if (!klass || !methodName) return nullptr;

        // 1. Try il2cpp_class_get_method_from_name if argsCount >= 0
        if (argsCount >= 0 && il2cpp_class_get_method_from_name) {
            const MethodInfo* m = il2cpp_class_get_method_from_name(klass, methodName, argsCount);
            if (m) return m;
        }

        // 2. Iterate methods of klass
        if (il2cpp_class_get_methods && il2cpp_method_get_name) {
            void* iter = nullptr;
            const MethodInfo* fallbackByName = nullptr;
            while (const MethodInfo* m = il2cpp_class_get_methods(klass, &iter)) {
                const char* mName = il2cpp_method_get_name(m);
                if (mName && strcmp(mName, methodName) == 0) {
                    int pCount = il2cpp_method_get_param_count ? (int)il2cpp_method_get_param_count(m) : -1;
                    if (argsCount < 0 || pCount == argsCount) {
                        return m;
                    }
                    if (!fallbackByName) {
                        fallbackByName = m;
                    }
                }
            }
            if (fallbackByName) {
                return fallbackByName;
            }
        }

        return nullptr;
    }

    PVOID ResolveMethod(const char* assemblyName, const char* namespaze, const char* className, const char* methodName, int argsCount) {
        if (!il2cpp_domain_get) {
            return nullptr;
        }
        Il2CppDomain* domain = il2cpp_domain_get();
        if (!domain) return nullptr;

        static const Il2CppImage* s_lastImage = nullptr;
        static std::string s_lastAssemblyName = "";

        const Il2CppImage* image = nullptr;
        if (s_lastImage && s_lastAssemblyName == (assemblyName ? assemblyName : "")) {
            image = s_lastImage;
        } else {
            image = FindImage(domain, assemblyName);
            if (image) {
                s_lastImage = image;
                s_lastAssemblyName = (assemblyName ? assemblyName : "");
            }
        }

        Il2CppClass* klass = nullptr;
        if (image) {
            klass = FindClass(image, namespaze, className);
        }

        // If not found in primary image, search all loaded images across the domain
        if (!klass && il2cpp_domain_get_assemblies && il2cpp_assembly_get_image) {
            size_t count = 0;
            const Il2CppAssembly** assemblies = il2cpp_domain_get_assemblies(domain, &count);
            if (assemblies) {
                for (size_t i = 0; i < count; i++) {
                    const Il2CppImage* img = il2cpp_assembly_get_image(assemblies[i]);
                    if (!img || img == image) continue;
                    klass = FindClass(img, namespaze, className);
                    if (klass) {
                        image = img;
                        break;
                    }
                }
            }
        }

        if (!klass) {
            return nullptr;
        }

        const MethodInfo* method = FindMethod(klass, methodName, argsCount);
        if (!method) {
            return nullptr;
        }

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
