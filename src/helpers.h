#pragma once
#include "globals.h"

Il2CppObject* Invoke(const MethodInfo* method, void* obj, void** params);
bool IsAlive(void* obj);
Il2CppImage* FindImage(const char* name);
Vector3 GetPosition(void* transform);
void InitDirectCalls();
void* GetTransformOf(void* component);
void* GetMainCamera();
Vector3 WorldToScreen(void* camera, Vector3 wp);
int GetScreenWidth();
int GetScreenHeight();
bool SafeGetPosition(void* transform, Vector3& out);
