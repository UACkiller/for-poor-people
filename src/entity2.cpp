#include "entity.h"
#include "helpers.h"

void DiscoverEntities() {
    if (!g_fot) {
        Il2CppClass* objClass = il2cpp.class_from_name(g_R.coreImage, "UnityEngine", "Object");
        if (!objClass) return;
        g_fot = il2cpp.class_get_method_from_name(objClass, "FindObjectsOfType", 2);
        if (g_fot) g_fotArgs = 2;
        else {
            g_fot = il2cpp.class_get_method_from_name(objClass, "FindObjectsOfType", 1);
            if (g_fot) g_fotArgs = 1;
        }
    }
    if (!g_fot) return;
    Il2CppType* type = il2cpp.class_get_type(g_R.baseEntityClass);
    if (!type) return;
    Il2CppObject* typeObj = il2cpp.type_get_object(type);
    if (!typeObj) return;
    Il2CppObject* arrObj = nullptr;
    if (g_fotArgs == 2) {
        int f = 0; void* p[] = { typeObj, &f };
        arrObj = Invoke(g_fot, nullptr, p);
    } else {
        void* p[] = { typeObj };
        arrObj = Invoke(g_fot, nullptr, p);
    }
    if (!arrObj) return;
    int len = *(int*)((char*)arrObj + 0xC);
    if (len <= 0 || len > 5000) return;

    std::vector<Entity> newList;
    for (int i = 0; i < len; i++) {
        void* ent = *(void**)((char*)arrObj + 0x10 + i * 4);
        if (!ent || !IsAlive(ent)) continue;
        void* tr = nullptr;
        if (g_R.be_getTransform) tr = (void*)Invoke(g_R.be_getTransform, ent, nullptr);
        if (!tr) tr = GetTransformOf(ent);
        if (!tr) continue;
        bool isLocal = false;
        if (g_R.be_getIsLocalPlayer) {
            Il2CppObject* b = Invoke(g_R.be_getIsLocalPlayer, ent, nullptr);
            if (b) isLocal = *(bool*)il2cpp.object_unbox(b);
        }
        int eType = -1;
        if (g_R.be_getEntityType) {
            Il2CppObject* t = Invoke(g_R.be_getEntityType, ent, nullptr);
            if (t) eType = *(int*)il2cpp.object_unbox(t);
        }
        bool isPlayerClass = (eType == 3);
        uint64_t pid = 0;
        if (g_R.be_getId) {
            Il2CppObject* idObj = Invoke(g_R.be_getId, ent, nullptr);
            if (idObj) pid = *(uint64_t*)il2cpp.object_unbox(idObj);
        }
        bool isEnemy = false;
        if (g_R.be_getIsEnemy) {
            Il2CppObject* b = Invoke(g_R.be_getIsEnemy, ent, nullptr);
            if (b) isEnemy = *(bool*)il2cpp.object_unbox(b);
        }
        Entity e = {};
        e.object = ent; e.transform = tr; e.isLocal = isLocal;
        e.isPlayer = isPlayerClass; e.isEnemy = isEnemy;
        e.entityType = eType; e.playerId = pid;
        newList.push_back(e);
    }

    g_Entities = std::move(newList);
}

void UpdatePositions(void* camera, int scrW, int scrH) {
    if (g_Entities.empty()) return;
    Vector3 localPos = {0,0,0};
    bool hasLocal = false;
    for (auto& e : g_Entities) {
        if (e.isLocal && e.transform) {
            if (SafeGetPosition(e.transform, localPos)) { hasLocal = true; break; }
        }
    }
    if (!hasLocal && camera) {
        void* ct = GetTransformOf(camera);
        if (ct) { SafeGetPosition(ct, localPos); hasLocal = true; }
    }
    for (auto& e : g_Entities) {
        if (!e.transform) { e.onScreen = false; continue; }
        Vector3 wp;
        if (!SafeGetPosition(e.transform, wp)) { e.object = nullptr; e.onScreen = false; continue; }
        e.worldPos = wp;
        e.distance = hasLocal ? Vec3Dist(wp, localPos) : 0;
        if (camera) {
            Vector3 sp = WorldToScreen(camera, wp);
            e.screenPos.x = sp.x;
            e.screenPos.y = (float)scrH - sp.y;
            e.onScreen = (sp.z > 0);
        } else e.onScreen = false;
    }
    g_Entities.erase(
        std::remove_if(g_Entities.begin(), g_Entities.end(),
            [](const Entity& e) { return !e.object; }),
        g_Entities.end());
}
