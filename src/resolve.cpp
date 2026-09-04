#include "resolve.h"
#include "helpers.h"

bool ResolveAll() {
    g_R.coreImage = FindImage("CoreModule");
    if (!g_R.coreImage) g_R.coreImage = FindImage("UnityEngine.dll");
    g_R.csharpImage = FindImage("Assembly-CSharp.dll");
    if (!g_R.coreImage || !g_R.csharpImage) return false;

    g_R.cameraClass = il2cpp.class_from_name(g_R.coreImage, "UnityEngine", "Camera");
    g_R.transformClass = il2cpp.class_from_name(g_R.coreImage, "UnityEngine", "Transform");
    g_R.componentClass = il2cpp.class_from_name(g_R.coreImage, "UnityEngine", "Component");
    g_R.screenClass = il2cpp.class_from_name(g_R.coreImage, "UnityEngine", "Screen");

    g_R.baseEntityClass = il2cpp.class_from_name(g_R.csharpImage, "", "bid");
    g_R.cameraViewClass = il2cpp.class_from_name(g_R.csharpImage, "", "boz");

    g_R.transform_getPosition = il2cpp.class_get_method_from_name(g_R.transformClass, "get_position", 0);
    g_R.component_getTransform = il2cpp.class_get_method_from_name(g_R.componentClass, "get_transform", 0);
    g_R.camera_getMain = il2cpp.class_get_method_from_name(g_R.cameraClass, "get_main", 0);
    g_R.screen_getWidth = il2cpp.class_get_method_from_name(g_R.screenClass, "get_width", 0);
    g_R.screen_getHeight = il2cpp.class_get_method_from_name(g_R.screenClass, "get_height", 0);
    g_R.camera_get_orthoSize = il2cpp.class_get_method_from_name(g_R.cameraClass, "get_orthographicSize", 0);
    g_R.camera_set_orthoSize = il2cpp.class_get_method_from_name(g_R.cameraClass, "set_orthographicSize", 1);
    g_R.camera_get_rect = il2cpp.class_get_method_from_name(g_R.cameraClass, "get_rect", 0);
    g_R.camera_set_rect = il2cpp.class_get_method_from_name(g_R.cameraClass, "set_rect", 1);

    g_R.gameObjectClass = il2cpp.class_from_name(g_R.coreImage, "UnityEngine", "GameObject");
    if (g_R.transformClass) {
        g_R.transform_get_childCount = il2cpp.class_get_method_from_name(g_R.transformClass, "get_childCount", 0);
        g_R.transform_getChild = il2cpp.class_get_method_from_name(g_R.transformClass, "GetChild", 1);
    }
    if (g_R.componentClass) {
        g_R.component_getGameObject = il2cpp.class_get_method_from_name(g_R.componentClass, "get_gameObject", 0);
    }
    if (g_R.gameObjectClass) {
        g_R.gameObject_setActive = il2cpp.class_get_method_from_name(g_R.gameObjectClass, "SetActive", 1);
    }

    if (g_R.cameraClass) {
        g_R.camera_w2s = il2cpp.class_get_method_from_name(g_R.cameraClass, "WorldToScreenPoint", 1);
        if (!g_R.camera_w2s) {
            void* miter = nullptr; const MethodInfo* m;
            while ((m = il2cpp.class_get_methods(g_R.cameraClass, &miter))) {
                if (m->name && strcmp(m->name, "WorldToScreenPoint") == 0 &&
                    il2cpp.method_get_param_count(m) == 2) {
                    g_R.camera_w2s = m; break;
                }
            }
        }
    }

    if (g_R.baseEntityClass) {
        g_R.be_getTransform = il2cpp.class_get_method_from_name(g_R.baseEntityClass, "ipv", 0);
        g_R.be_getIsLocalPlayer = il2cpp.class_get_method_from_name(g_R.baseEntityClass, "iqg", 0);
        g_R.be_getId = il2cpp.class_get_method_from_name(g_R.baseEntityClass, "ipz", 0);
        g_R.be_getEntityType = il2cpp.class_get_method_from_name(g_R.baseEntityClass, "iqb", 0);
    }

    Il2CppClass* objClass = il2cpp.class_from_name(g_R.coreImage, "UnityEngine", "Object");
    if (objClass) {
        FieldInfo* f = il2cpp.class_get_field_from_name(objClass, "m_CachedPtr");
        if (f) { g_R.cachedPtrOffset = il2cpp.field_get_offset(f); g_R.hasCachedPtr = true; }
    }

    return g_R.camera_getMain && g_R.transform_getPosition;
}
