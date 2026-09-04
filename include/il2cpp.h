#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>

// ====================================================
// IL2CPP opaque types
// ====================================================
struct Il2CppDomain;
struct Il2CppAssembly;
struct Il2CppImage;
struct Il2CppClass;
struct Il2CppType;
struct Il2CppObject;
struct Il2CppString;
struct Il2CppArray;
struct Il2CppException;
struct FieldInfo;

// MethodInfo layout (first field = native function pointer)
struct MethodInfo {
    void* methodPointer;
    void* invoker_method;
    const char* name;
    Il2CppClass* klass;
    Il2CppType* return_type;
    // ... more fields we don't need
};

// Il2CppArray layout on x86:
//   +0  Il2CppObject (8 bytes: vtable + monitor)
//   +8  Il2CppArrayBounds* bounds (4 bytes)
//  +12  uint32_t max_length (4 bytes)
//  +16  data[0] ...
#define IL2CPP_ARRAY_DATA_OFFSET 16

inline void** il2cpp_array_addr(Il2CppArray* arr, size_t idx) {
    return (void**)((char*)arr + IL2CPP_ARRAY_DATA_OFFSET + idx * sizeof(void*));
}

inline uint32_t il2cpp_array_len(Il2CppArray* arr) {
    return *(uint32_t*)((char*)arr + 12);
}

// ====================================================
// IL2CPP API function pointer types
// ====================================================
#define DECL_API(ret, name, ...) typedef ret (*fn_##name)(__VA_ARGS__)

DECL_API(Il2CppDomain*,    il2cpp_domain_get);
DECL_API(Il2CppAssembly**,  il2cpp_domain_get_assemblies,     Il2CppDomain*, size_t*);
DECL_API(Il2CppImage*,      il2cpp_assembly_get_image,        Il2CppAssembly*);
DECL_API(const char*,        il2cpp_image_get_name,            Il2CppImage*);
DECL_API(size_t,             il2cpp_image_get_class_count,     Il2CppImage*);
DECL_API(Il2CppClass*,      il2cpp_image_get_class,           Il2CppImage*, size_t);
DECL_API(const char*,        il2cpp_class_get_name,            Il2CppClass*);
DECL_API(const char*,        il2cpp_class_get_namespace,       Il2CppClass*);
DECL_API(Il2CppClass*,      il2cpp_class_from_name,           Il2CppImage*, const char*, const char*);
DECL_API(const MethodInfo*,  il2cpp_class_get_method_from_name, Il2CppClass*, const char*, int);
DECL_API(const MethodInfo*,  il2cpp_class_get_methods,         Il2CppClass*, void**);
DECL_API(const char*,        il2cpp_method_get_name,           const MethodInfo*);
DECL_API(FieldInfo*,         il2cpp_class_get_field_from_name, Il2CppClass*, const char*);
DECL_API(FieldInfo*,         il2cpp_class_get_fields,          Il2CppClass*, void**);
DECL_API(const char*,        il2cpp_field_get_name,            FieldInfo*);
DECL_API(size_t,             il2cpp_field_get_offset,          FieldInfo*);
DECL_API(void,               il2cpp_field_static_get_value,    FieldInfo*, void*);
DECL_API(Il2CppObject*,     il2cpp_runtime_invoke,            const MethodInfo*, void*, void**, Il2CppException**);
DECL_API(Il2CppObject*,     il2cpp_object_new,                Il2CppClass*);
DECL_API(void*,              il2cpp_object_unbox,              Il2CppObject*);
DECL_API(Il2CppString*,     il2cpp_string_new,                const char*);
DECL_API(uint16_t*,          il2cpp_string_chars,              Il2CppString*);
DECL_API(int32_t,            il2cpp_string_length,             Il2CppString*);
DECL_API(void*,              il2cpp_resolve_icall,             const char*);
DECL_API(Il2CppClass*,      il2cpp_object_get_class,          Il2CppObject*);
DECL_API(Il2CppType*,       il2cpp_class_get_type,            Il2CppClass*);
DECL_API(Il2CppObject*,     il2cpp_type_get_object,           Il2CppType*);
DECL_API(Il2CppClass*,      il2cpp_class_get_parent,          Il2CppClass*);
DECL_API(void*,              il2cpp_thread_attach,             Il2CppDomain*);
DECL_API(void,               il2cpp_thread_detach,             void*);
DECL_API(int32_t,            il2cpp_method_get_param_count,    const MethodInfo*);
DECL_API(const Il2CppType*,  il2cpp_method_get_return_type,    const MethodInfo*);

#undef DECL_API

// ====================================================
// ====================================================
struct IL2CPP_API {
    fn_il2cpp_domain_get                domain_get;
    fn_il2cpp_domain_get_assemblies     domain_get_assemblies;
    fn_il2cpp_assembly_get_image        assembly_get_image;
    fn_il2cpp_image_get_name            image_get_name;
    fn_il2cpp_image_get_class_count     image_get_class_count;
    fn_il2cpp_image_get_class           image_get_class;
    fn_il2cpp_class_get_name            class_get_name;
    fn_il2cpp_class_get_namespace       class_get_namespace;
    fn_il2cpp_class_from_name           class_from_name;
    fn_il2cpp_class_get_method_from_name class_get_method_from_name;
    fn_il2cpp_class_get_methods         class_get_methods;
    fn_il2cpp_method_get_name           method_get_name;
    fn_il2cpp_class_get_field_from_name class_get_field_from_name;
    fn_il2cpp_class_get_fields          class_get_fields;
    fn_il2cpp_field_get_name            field_get_name;
    fn_il2cpp_field_get_offset          field_get_offset;
    fn_il2cpp_field_static_get_value    field_static_get_value;
    fn_il2cpp_runtime_invoke            runtime_invoke;
    fn_il2cpp_object_new                object_new;
    fn_il2cpp_object_unbox              object_unbox;
    fn_il2cpp_string_new                string_new;
    fn_il2cpp_string_chars              string_chars;
    fn_il2cpp_string_length             string_length;
    fn_il2cpp_resolve_icall             resolve_icall;
    fn_il2cpp_object_get_class          object_get_class;
    fn_il2cpp_class_get_type            class_get_type;
    fn_il2cpp_type_get_object           type_get_object;
    fn_il2cpp_class_get_parent          class_get_parent;
    fn_il2cpp_thread_attach             thread_attach;
    fn_il2cpp_thread_detach             thread_detach;
    fn_il2cpp_method_get_param_count    method_get_param_count;

    bool init() {
        char _n[] = {'\x1d','\x3b','\x37','\x3f','\x1b','\x29','\x29','\x3f','\x37','\x38','\x36','\x23','\x74','\x3e','\x36','\x36',0};
        for(int _i=0;_n[_i];_i++) _n[_i]^=0x5A;
        HMODULE ga = GetModuleHandleA(_n);
        if (!ga) return false;

#define R(field, export_name) \
        field = (decltype(field))GetProcAddress(ga, export_name); \
        if (!field) return false

        R(domain_get,               "il2cpp_domain_get");
        R(domain_get_assemblies,    "il2cpp_domain_get_assemblies");
        R(assembly_get_image,       "il2cpp_assembly_get_image");
        R(image_get_name,           "il2cpp_image_get_name");
        R(image_get_class_count,    "il2cpp_image_get_class_count");
        R(image_get_class,          "il2cpp_image_get_class");
        R(class_get_name,           "il2cpp_class_get_name");
        R(class_get_namespace,      "il2cpp_class_get_namespace");
        R(class_from_name,          "il2cpp_class_from_name");
        R(class_get_method_from_name, "il2cpp_class_get_method_from_name");
        R(class_get_methods,        "il2cpp_class_get_methods");
        R(method_get_name,          "il2cpp_method_get_name");
        R(class_get_field_from_name, "il2cpp_class_get_field_from_name");
        R(class_get_fields,         "il2cpp_class_get_fields");
        R(field_get_name,           "il2cpp_field_get_name");
        R(field_get_offset,         "il2cpp_field_get_offset");
        R(field_static_get_value,   "il2cpp_field_static_get_value");
        R(runtime_invoke,           "il2cpp_runtime_invoke");
        R(object_new,               "il2cpp_object_new");
        R(object_unbox,             "il2cpp_object_unbox");
        R(string_new,               "il2cpp_string_new");
        R(string_chars,             "il2cpp_string_chars");
        R(string_length,            "il2cpp_string_length");
        R(resolve_icall,            "il2cpp_resolve_icall");
        R(object_get_class,         "il2cpp_object_get_class");
        R(class_get_type,           "il2cpp_class_get_type");
        R(type_get_object,          "il2cpp_type_get_object");
        R(class_get_parent,         "il2cpp_class_get_parent");
        R(thread_attach,            "il2cpp_thread_attach");
        R(thread_detach,            "il2cpp_thread_detach");
        method_get_param_count = (fn_il2cpp_method_get_param_count)GetProcAddress(ga, "il2cpp_method_get_param_count");

#undef R
        return true;
    }
};
