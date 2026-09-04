#include "helpers.h"

Il2CppObject* Invoke(const MethodInfo* method, void* obj, void** params) {
    if (!method) return nullptr;
    Il2CppException* exc = nullptr;
    Il2CppObject* result = il2cpp.runtime_invoke(method, obj, params, &exc);
    if (exc) return nullptr;
    return result;
}

bool IsAlive(void* obj) {
    if (!obj) return false;
    if (!g_R.hasCachedPtr) return true;
    void* nativePtr = *(void**)((char*)obj + g_R.cachedPtrOffset);
    return nativePtr != nullptr;
}

Il2CppImage* FindImage(const char* name) {
    Il2CppDomain* domain = il2cpp.domain_get();
    size_t count = 0;
    Il2CppAssembly** assemblies = il2cpp.domain_get_assemblies(domain, &count);
    for (size_t i = 0; i < count; i++) {
        Il2CppImage* img = il2cpp.assembly_get_image(assemblies[i]);
        const char* n = il2cpp.image_get_name(img);
        if (n && strstr(n, name)) return img;
    }
    return nullptr;
}

Vector3 GetPosition(void* transform) {
    Vector3 pos = {0,0,0};
    if (!transform || !g_R.transform_getPosition) return pos;
    Il2CppObject* b = Invoke(g_R.transform_getPosition, transform, nullptr);
    if (b) { Vector3* p = (Vector3*)il2cpp.object_unbox(b); if (p) pos = *p; }
    return pos;
}

void InitDirectCalls() {
    if (g_R.transform_getPosition && g_R.transform_getPosition->methodPointer)
        g_fp_getPos = (fn_get_position)g_R.transform_getPosition->methodPointer;
    if (g_R.component_getTransform && g_R.component_getTransform->methodPointer)
        g_fp_getTr = (fn_get_transform)g_R.component_getTransform->methodPointer;
    if (g_R.camera_getMain && g_R.camera_getMain->methodPointer)
        g_fp_camMain = (fn_camera_main)g_R.camera_getMain->methodPointer;
    if (g_R.camera_w2s && g_R.camera_w2s->methodPointer)
        g_fp_w2s = (fn_w2s)g_R.camera_w2s->methodPointer;
    if (g_R.screen_getWidth && g_R.screen_getWidth->methodPointer)
        g_fp_scrW = (fn_screen_int)g_R.screen_getWidth->methodPointer;
    if (g_R.screen_getHeight && g_R.screen_getHeight->methodPointer)
        g_fp_scrH = (fn_screen_int)g_R.screen_getHeight->methodPointer;
    g_directCalls = (g_fp_getPos && g_fp_getTr && g_fp_camMain && g_fp_w2s && g_fp_scrW && g_fp_scrH);
}

void* GetTransformOf(void* component) {
    if (!component) return nullptr;
    if (g_fp_getTr)
        return g_fp_getTr(component, g_R.component_getTransform);
    if (!g_R.component_getTransform) return nullptr;
    return (void*)Invoke(g_R.component_getTransform, component, nullptr);
}

void* GetMainCamera() {
    if (g_fp_camMain)
        return g_fp_camMain(g_R.camera_getMain);
    if (!g_R.camera_getMain) return nullptr;
    return (void*)Invoke(g_R.camera_getMain, nullptr, nullptr);
}

Vector3 WorldToScreen(void* camera, Vector3 wp) {
    Vector3 r = {0,0,0};
    if (!camera) return r;
    if (g_fp_w2s) {
        g_fp_w2s(&r, camera, wp, g_R.camera_w2s);
        return r;
    }
    if (!g_R.camera_w2s) return r;
    void* p[] = { &wp };
    Il2CppObject* b = Invoke(g_R.camera_w2s, camera, p);
    if (b) { Vector3* v = (Vector3*)il2cpp.object_unbox(b); if (v) r = *v; }
    return r;
}

int GetScreenWidth() {
    if (g_fp_scrW) return g_fp_scrW(g_R.screen_getWidth);
    if (!g_R.screen_getWidth) return 1920;
    Il2CppObject* b = Invoke(g_R.screen_getWidth, nullptr, nullptr);
    return b ? *(int*)il2cpp.object_unbox(b) : 1920;
}

int GetScreenHeight() {
    if (g_fp_scrH) return g_fp_scrH(g_R.screen_getHeight);
    if (!g_R.screen_getHeight) return 1080;
    Il2CppObject* b = Invoke(g_R.screen_getHeight, nullptr, nullptr);
    return b ? *(int*)il2cpp.object_unbox(b) : 1080;
}

bool SafeGetPosition(void* transform, Vector3& out) {
    if (!transform) return false;
    if (g_fp_getPos) {
        g_fp_getPos(&out, transform, g_R.transform_getPosition);
        return true;
    }
    if (!g_R.transform_getPosition) return false;
    Il2CppException* exc = nullptr;
    Il2CppObject* b = il2cpp.runtime_invoke(g_R.transform_getPosition, transform, nullptr, &exc);
    if (exc || !b) return false;
    Vector3* p = (Vector3*)il2cpp.object_unbox(b);
    if (!p) return false;
    out = *p;
    return true;
}
