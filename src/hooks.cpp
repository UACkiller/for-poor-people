#include "hooks.h"
#include "helpers.h"
#include "entity.h"

typedef void (__cdecl *fn_CVMethod)(void* self, const MethodInfo* method);
static fn_CVMethod g_OrigUpdate = nullptr;
static fn_CVMethod g_OrigOnPreRender = nullptr;
static const MethodInfo* g_HookedMethod = nullptr;
static const MethodInfo* g_HookedPreRender = nullptr;

volatile float g_ZoomOverride = 0;
float g_ZoomDefault = 12.0f;
bool  g_ZoomHooked = false;
static void* g_ZoomCamera = nullptr;

static void ApplyZoom() {
    float zoom = g_ZoomOverride;
    if (zoom < 1.0f) zoom = 1.0f;
    if (zoom > 100.0f) zoom = 100.0f;
    if (zoom > 0.0f && zoom != g_ZoomDefault && g_ZoomCamera && g_R.camera_set_orthoSize) {
        void* p[] = { &zoom };
        Il2CppException* exc = nullptr;
        il2cpp.runtime_invoke(g_R.camera_set_orthoSize, g_ZoomCamera, p, &exc);
    }
    if (zoom > 0.0f && zoom != g_ZoomDefault && g_ZoomCamera && g_R.camera_set_rect) {
        struct { float x, y, w, h; } fullRect = {0.f, 0.f, 1.f, 1.f};
        void* rp[] = { &fullRect };
        Il2CppException* exc2 = nullptr;
        il2cpp.runtime_invoke(g_R.camera_set_rect, g_ZoomCamera, rp, &exc2);
    }
}

static void __cdecl HookedUpdate(void* self, const MethodInfo* method) {
    __try {
        if (g_OrigUpdate) g_OrigUpdate(self, method);
        ApplyZoom();
        void* cam = GetMainCamera();
        if (!cam) return;
        int sw = GetScreenWidth(), sh = GetScreenHeight();
        static DWORD last = 0;
        DWORD now = GetTickCount();
        if (now - last >= SCAN_INTERVAL_MS) {
            last = now;
            DiscoverEntities();
        }
        UpdatePositions(cam, sw, sh);
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }
}

static void __cdecl HookedOnPreRender(void* self, const MethodInfo* method) {
    __try {
        if (g_OrigOnPreRender) g_OrigOnPreRender(self, method);
        ApplyZoom();
    } __except(EXCEPTION_EXECUTE_HANDLER) { return; }
}

static void HookMethod(const MethodInfo* mi, void* hook, void** origOut) {
    *origOut = mi->methodPointer;
    DWORD old;
    VirtualProtect((void*)&mi->methodPointer, sizeof(void*), PAGE_READWRITE, &old);
    *(void**)&mi->methodPointer = hook;
    VirtualProtect((void*)&mi->methodPointer, sizeof(void*), old, &old);
}

void InstallCameraViewHook() {
    if (!g_R.cameraViewClass) return;
    g_HookedMethod = il2cpp.class_get_method_from_name(g_R.cameraViewClass, "Update", 0);
    if (g_HookedMethod)
        HookMethod(g_HookedMethod, (void*)&HookedUpdate, (void**)&g_OrigUpdate);

    g_HookedPreRender = il2cpp.class_get_method_from_name(g_R.cameraViewClass, "OnPreRender", 0);
    if (g_HookedPreRender)
        HookMethod(g_HookedPreRender, (void*)&HookedOnPreRender, (void**)&g_OrigOnPreRender);

    if (g_R.camera_getMain && g_R.camera_get_orthoSize) {
        void* cam = (void*)Invoke(g_R.camera_getMain, nullptr, nullptr);
        if (cam) {
            Il2CppObject* b = Invoke(g_R.camera_get_orthoSize, cam, nullptr);
            if (b) g_ZoomDefault = *(float*)il2cpp.object_unbox(b);
            g_ZoomCamera = cam;
        }
    }
    g_ZoomOverride = g_ZoomDefault;
    g_ZoomHooked = true;
}

void ShutdownCameraViewHook() {
    if (g_OrigUpdate && g_HookedMethod) {
        DWORD old;
        VirtualProtect((void*)&g_HookedMethod->methodPointer, sizeof(void*), PAGE_READWRITE, &old);
        *(void**)&g_HookedMethod->methodPointer = g_OrigUpdate;
        VirtualProtect((void*)&g_HookedMethod->methodPointer, sizeof(void*), old, &old);
        g_OrigUpdate = nullptr;
        g_HookedMethod = nullptr;
    }
    if (g_OrigOnPreRender && g_HookedPreRender) {
        DWORD old;
        VirtualProtect((void*)&g_HookedPreRender->methodPointer, sizeof(void*), PAGE_READWRITE, &old);
        *(void**)&g_HookedPreRender->methodPointer = g_OrigOnPreRender;
        VirtualProtect((void*)&g_HookedPreRender->methodPointer, sizeof(void*), old, &old);
        g_OrigOnPreRender = nullptr;
        g_HookedPreRender = nullptr;
    }
}

void DisableFogOfWar() {
    Il2CppClass* objClass = il2cpp.class_from_name(g_R.coreImage, "UnityEngine", "Object");
    if (!objClass) return;
    const MethodInfo* destroyMethod = il2cpp.class_get_method_from_name(objClass, "Destroy", 1);
    if (!destroyMethod)
        destroyMethod = il2cpp.class_get_method_from_name(objClass, "DestroyImmediate", 1);
    if (!destroyMethod) return;

    auto hookRenderNoop = [](Il2CppClass* klass, const char* name, const char* methodName, int args) {
        if (!klass) return;
        const MethodInfo* m = il2cpp.class_get_method_from_name(klass, methodName, args);
        if (!m) return;
        static void* noopFn = nullptr;
        if (!noopFn) {
            noopFn = VirtualAlloc(nullptr, 16, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            unsigned char code[] = {0xC3};
            memcpy(noopFn, code, sizeof(code));
        }
        DWORD old;
        VirtualProtect((void*)&m->methodPointer, sizeof(void*), PAGE_READWRITE, &old);
        *(void**)&m->methodPointer = noopFn;
        VirtualProtect((void*)&m->methodPointer, sizeof(void*), old, &old);
    };

    auto hookNoopRet = [](Il2CppClass* klass, const char* methodName) {
        if (!klass) return;
        const MethodInfo* m = il2cpp.class_get_method_from_name(klass, methodName, 2);
        if (!m) m = il2cpp.class_get_method_from_name(klass, methodName, 0);
        if (!m) return;
        static void* noopFunc = nullptr;
        if (!noopFunc) {
            noopFunc = VirtualAlloc(nullptr, 16, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            if (noopFunc) {
                unsigned char code[] = {0xC2, 0x00, 0x00};
                memcpy(noopFunc, code, sizeof(code));
            }
        }
        if (noopFunc) {
            DWORD old;
            VirtualProtect((void*)&m->methodPointer, sizeof(void*), PAGE_READWRITE, &old);
            *(void**)&m->methodPointer = noopFunc;
            VirtualProtect((void*)&m->methodPointer, sizeof(void*), old, &old);
        }
    };

    hookRenderNoop(il2cpp.class_from_name(g_R.csharpImage, "", ".bbf"), "ToxicRenderer", "Render", 1);
    hookRenderNoop(il2cpp.class_from_name(g_R.csharpImage, "", ".btx"), "FrostRenderer", "Render", 1);
    hookRenderNoop(il2cpp.class_from_name(g_R.csharpImage, "", ".bty"), "NightRenderer", "Render", 1);
    hookRenderNoop(il2cpp.class_from_name(g_R.csharpImage, "", ".n"), "FrostEffect", "OnRenderImage", 2);
    hookRenderNoop(il2cpp.class_from_name(g_R.csharpImage, "", ".bsi"), "NightRenderer", "OnRenderImage", 2);
    hookRenderNoop(il2cpp.class_from_name(g_R.csharpImage, "Game.PostProcessing", "Toxic"), "Toxic", "OnRenderImage", 2);
    hookNoopRet(il2cpp.class_from_name(g_R.csharpImage, "", ".bsd"), "OnRenderImage");

    void* cam = (void*)Invoke(g_R.camera_getMain, nullptr, nullptr);
    if (!cam) return;

    if (g_R.transform_get_childCount && g_R.transform_getChild &&
        g_R.component_getTransform && g_R.component_getGameObject &&
        g_R.gameObject_setActive && destroyMethod) {
        void* camTr = (void*)Invoke(g_R.component_getTransform, cam, nullptr);
        if (camTr) {
            Il2CppObject* cObj = Invoke(g_R.transform_get_childCount, camTr, nullptr);
            if (cObj) {
                int childCount = *(int*)il2cpp.object_unbox(cObj);
                for (int i = 0; i < childCount; i++) {
                    void* childArgs[] = { &i };
                    void* childTr = (void*)Invoke(g_R.transform_getChild, camTr, childArgs);
                    if (!childTr) continue;
                    void* childGo = (void*)Invoke(g_R.component_getGameObject, childTr, nullptr);
                    if (!childGo) continue;
                    int bFalse = 0;
                    void* actArgs[] = { &bFalse };
                    Invoke(g_R.gameObject_setActive, childGo, actArgs);
                    void* dArgs[] = { childGo };
                    Invoke(destroyMethod, nullptr, dArgs);
                }
            }
        }
    }

    Il2CppClass* behaviourClass = il2cpp.class_from_name(g_R.coreImage, "UnityEngine", "Behaviour");
    const MethodInfo* setEnabled = behaviourClass ?
        il2cpp.class_get_method_from_name(behaviourClass, "set_enabled", 1) : nullptr;

    struct Target { const char* name; const char* ns; };
    Target targets[] = {
        {".n", ""},
        {".bsd", ""},
        {nullptr, nullptr}
    };
    for (int i = 0; targets[i].name; i++) {
        Il2CppClass* klass = il2cpp.class_from_name(g_R.csharpImage, targets[i].ns, targets[i].name);
        if (!klass) continue;
        Il2CppType* type = il2cpp.class_get_type(klass);
        if (!type) continue;
        Il2CppObject* typeObj = il2cpp.type_get_object(type);
        if (!typeObj) continue;
        Il2CppObject* arrObj = nullptr;
        if (g_fotArgs == 2) {
            int b = 1; void* p[] = { typeObj, &b };
            arrObj = Invoke(g_fot, nullptr, p);
        } else {
            void* p[] = { typeObj };
            arrObj = Invoke(g_fot, nullptr, p);
        }
        if (!arrObj) continue;
        int len = *(int*)((char*)arrObj + 0xC);
        for (int j = 0; j < len; j++) {
            void* obj = *(void**)((char*)arrObj + 0x10 + j * 4);
            if (!obj) continue;
            if (setEnabled) {
                int bFalse = 0;
                void* pa[] = { &bFalse };
                Invoke(setEnabled, obj, pa);
            }
            void* pa[] = { obj };
            Invoke(destroyMethod, nullptr, pa);
        }
    }
}
