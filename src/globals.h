#pragma once
#include "../include/il2cpp.h"
#include "../kiero/minhook/include/MinHook.h"
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

#define SCAN_INTERVAL_MS 1000

struct Vector2 { float x, y; };
struct Vector3 { float x, y, z; };

static float Vec3Dist(const Vector3& a, const Vector3& b) {
    float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

struct Entity {
    void*   object;
    void*   transform;
    Vector3 worldPos;
    Vector2 screenPos;
    bool    onScreen;
    bool    isLocal;
    bool    isPlayer;
    bool    isEnemy;
    float   distance;
    int     entityType;
    uint64_t playerId;
};

struct Resolved {
    Il2CppImage* coreImage   = nullptr;
    Il2CppImage* csharpImage = nullptr;
    Il2CppClass* cameraClass    = nullptr;
    Il2CppClass* transformClass = nullptr;
    Il2CppClass* componentClass = nullptr;
    Il2CppClass* screenClass    = nullptr;
    Il2CppClass* gameObjectClass = nullptr;
    Il2CppClass* baseEntityClass    = nullptr;
    Il2CppClass* basePlayerClass    = nullptr;
    Il2CppClass* cameraViewClass    = nullptr;
    const MethodInfo* transform_getPosition  = nullptr;
    const MethodInfo* component_getTransform = nullptr;
    const MethodInfo* camera_getMain         = nullptr;
    const MethodInfo* camera_w2s             = nullptr;
    const MethodInfo* screen_getWidth        = nullptr;
    const MethodInfo* screen_getHeight       = nullptr;
    const MethodInfo* be_getTransform        = nullptr;
    const MethodInfo* be_getIsLocalPlayer    = nullptr;
    const MethodInfo* be_getId               = nullptr;
    const MethodInfo* be_getEntityType       = nullptr;
    const MethodInfo* be_getIsEnemy          = nullptr;
    const MethodInfo* bp_getName             = nullptr;
    const MethodInfo* camera_get_orthoSize   = nullptr;
    const MethodInfo* camera_set_orthoSize   = nullptr;
    const MethodInfo* camera_get_rect        = nullptr;
    const MethodInfo* camera_set_rect        = nullptr;
    const MethodInfo* transform_get_childCount = nullptr;
    const MethodInfo* transform_getChild     = nullptr;
    const MethodInfo* component_getGameObject = nullptr;
    const MethodInfo* gameObject_setActive   = nullptr;
    size_t cachedPtrOffset = 0;
    bool   hasCachedPtr    = false;
};

typedef void  (__cdecl *fn_get_position)(Vector3* ret, void* self, const MethodInfo*);
typedef void* (__cdecl *fn_get_transform)(void* self, const MethodInfo*);
typedef void* (__cdecl *fn_camera_main)(const MethodInfo*);
typedef void  (__cdecl *fn_w2s)(Vector3* ret, void* self, Vector3 pos, const MethodInfo*);
typedef int   (__cdecl *fn_screen_int)(const MethodInfo*);

extern IL2CPP_API       il2cpp;
extern HMODULE          g_hModule;
extern volatile bool    g_Running;

extern std::vector<Entity> g_Entities;

extern Resolved g_R;

extern fn_get_position   g_fp_getPos;
extern fn_get_transform  g_fp_getTr;
extern fn_camera_main    g_fp_camMain;
extern fn_w2s            g_fp_w2s;
extern fn_screen_int     g_fp_scrW;
extern fn_screen_int     g_fp_scrH;
extern bool g_directCalls;

extern const MethodInfo* g_fot;
extern int g_fotArgs;

extern volatile float g_ZoomOverride;
extern float g_ZoomDefault;
extern bool  g_ZoomHooked;
