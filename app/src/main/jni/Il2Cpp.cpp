//
// Fixed by musk.
//

#include "Il2Cpp.h"
// ========================================================================================================================================== //
#define IL2CPP__TAG "Il2CppSdk"
#define IL2CPP_LOGI(...) __android_log_print(ANDROID_LOG_INFO,IL2CPP__TAG,__VA_ARGS__)
#define IL2CPP_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,IL2CPP__TAG,__VA_ARGS__)
#define IL2CPP_LOGW(...) __android_log_print(ANDROID_LOG_WARN,IL2CPP__TAG,__VA_ARGS__)
#define IL2CPP_LOGE(...) __android_log_print(ANDROID_LOG_ERROR,IL2CPP__TAG,__VA_ARGS__)
// ========================================================================================================================================== //
map<string, size_t> m_cacheFields;
map<string, void *> m_cacheMethods;
map<string, void *> m_cacheClasses;
// ========================================================================================================================================== //
namespace {
    const void *(*il2cpp_assembly_get_image)(const void *assembly);

    void *(*il2cpp_domain_get)();

    void **(*il2cpp_domain_get_assemblies)(const void *domain, size_t *size);

    const char *(*il2cpp_image_get_name)(void *image);

    void *(*il2cpp_class_from_name)(const void *image, const char *namespaze, const char *name);

    void *(*il2cpp_class_get_field_from_name)(void *klass, const char *name);

    void *(*il2cpp_class_get_method_from_name)(void *klass, const char *name, int argsCount);

    size_t (*il2cpp_field_get_offset)(void *field);

    void (*il2cpp_field_static_get_value)(void *field, void *value);

    void (*il2cpp_field_static_set_value)(void *field, void *value);

    void *(*il2cpp_array_new)(void *elementTypeInfo, size_t length);

    char *(*il2cpp_type_get_name)(void *type);

    void* (*il2cpp_method_get_param)(void *method, uint32_t index);

    void* (*il2cpp_class_get_methods)(void *klass, void* *iter);

    const char* (*il2cpp_method_get_name)(void *method);

    const char *(*il2cpp_class_get_name)(void *klass);

    void *(*il2cpp_class_get_nested_types)(void *, void **);

    void *(*il2cpp_object_new)(void *);
}
// ========================================================================================================================================== //
vector<string> split_string(string str, string token) {
    vector<string> result;
    while (str.size()) {
        int index = str.find(token);
        if (index != string::npos) {
            result.push_back(str.substr(0, index));
            str = str.substr(index + token.size());
            if (str.size() == 0)
                result.push_back(str);
        } else {
            result.push_back(str);
            str = "";
        }
    }
    return result;
}
// ========================================================================================================================================== //
int not_found_export = 0;

void *get_export_function(const char *lib, const char *name)
{
    void *handle = dlopen(lib, 4);
    if(handle) {
        void *fn = dlsym(handle, name);
        if (fn) {
            return fn;
        }
    }
    not_found_export++;
    return 0;
}
// ========================================================================================================================================== //
#define GAME_LIB_ENGINE "libil2cpp.so"
uintptr_t lib_addr = 0;

uintptr_t Il2CppBase(){
    if(lib_addr)
    {
        return lib_addr;
    }

    char line[512];

    FILE *f = fopen("/proc/self/maps", "r");

    if (!f)
        return 0;

    while (fgets(line, sizeof line, f)) {
        uintptr_t base;
        char tmp[64];
        sscanf(line, "%" PRIXPTR "-%*" PRIXPTR " %*s %*s %*s %*s %s", &base, tmp);
        if (strstr(tmp, GAME_LIB_ENGINE)) {
            fclose(f);
            lib_addr = base;
            return base;
        }
    }
    fclose(f);
    return 0;
}
// ========================================================================================================================================== //
typedef unsigned short UTF16;
typedef wchar_t UTF32;
typedef char UTF8;

int is_surrogate(UTF16 uc) {
    return (uc - 0xd800u) < 2048u;
}

int is_high_surrogate(UTF16 uc) {
    return (uc & 0xfffffc00) == 0xd800;
}

int is_low_surrogate(UTF16 uc) {
    return (uc & 0xfffffc00) == 0xdc00;
}

UTF32 surrogate_to_utf32(UTF16 high, UTF16 low) {
    return (high << 10) + low - 0x35fdc00;
}

const char* utf16_to_utf8(const UTF16* source, size_t len) {
    std::u16string s(source, source + len);
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.to_bytes(s).c_str();
}

const wchar_t* utf16_to_utf32(const UTF16* source, size_t len) {
    UTF32* output = new UTF32[len + 1];

    for (int i = 0; i < len; i++) {
        const UTF16 uc = source[i];
        if (!is_surrogate(uc)) {
            output[i] = uc;
        }
        else {
            if (is_high_surrogate(uc) && is_low_surrogate(source[i]))
                output[i] = surrogate_to_utf32(uc, source[i]);
            else
                output[i] = L'?';
        }
    }

    output[len] = L'\0';
    return output;
}
// ========================================================================================================================================== //
const char* Il2CppString::CString() {
    return utf16_to_utf8(&this->start_char, this->length);
}
// ========================================================================================================================================== //
const wchar_t* Il2CppString::WCString() {
    return utf16_to_utf32(&this->start_char, this->length);
}
// ========================================================================================================================================== //
int Il2Cpp::Attach(const char *libname) {
    if(!libname) return -1;

    il2cpp_assembly_get_image = (const void *(*)(const void *)) get_export_function(libname, "il2cpp_assembly_get_image");

    il2cpp_domain_get = (void *(*)()) get_export_function(libname, "il2cpp_domain_get");

    il2cpp_domain_get_assemblies = (void **(*)(const void* , size_t*)) get_export_function(libname, "il2cpp_domain_get_assemblies");

    il2cpp_image_get_name = (const char *(*)(void *)) get_export_function(libname, "il2cpp_image_get_name");

    il2cpp_class_from_name = (void* (*)(const void*, const char*, const char *)) get_export_function(libname, "il2cpp_class_from_name");

    il2cpp_class_get_field_from_name = (void* (*)(void*, const char *)) get_export_function(libname, "il2cpp_class_get_field_from_name");;

    il2cpp_class_get_method_from_name = (void* (*)(void *, const char*, int)) get_export_function(libname, "il2cpp_class_get_method_from_name");;

    il2cpp_field_get_offset = (size_t (*)(void *)) get_export_function(libname, "il2cpp_field_get_offset");;

    il2cpp_field_static_get_value = (void (*)(void*, void *)) get_export_function(libname, "il2cpp_field_static_get_value");;

    il2cpp_field_static_set_value = (void (*)(void*, void *)) get_export_function(libname, "il2cpp_field_static_set_value");;

    il2cpp_array_new = (void *(*)(void*, size_t)) get_export_function(libname, "il2cpp_array_new");;

    il2cpp_type_get_name = (char *(*)(void *)) get_export_function(libname, "il2cpp_type_get_name");;

    il2cpp_method_get_param = (void *(*)(void *, uint32_t)) get_export_function(libname, "il2cpp_method_get_param");;

    il2cpp_class_get_methods = (void *(*)(void *, void **)) get_export_function(libname, "il2cpp_class_get_methods");;

    il2cpp_method_get_name = (const char *(*)(void *)) get_export_function(libname, "il2cpp_method_get_name");;

    il2cpp_class_get_name = (const char *(*)(void *)) get_export_function(libname, "il2cpp_class_get_name");

    il2cpp_class_get_nested_types = (void *(*)(void *, void **)) get_export_function(libname, "il2cpp_class_get_nested_types");

    il2cpp_object_new = (void *(*)(void *)) get_export_function(libname, "il2cpp_object_new");

    if(not_found_export)
    {
        return -1;
    }
    return 0;

}
// ========================================================================================================================================== //
void *Il2Cpp::GetImage(const char *image) {
    size_t size;
    void **assemblies = il2cpp_domain_get_assemblies(il2cpp_domain_get(), &size);
    for(int i = 0; i < size; ++i)
    {
        void *img = (void *)il2cpp_assembly_get_image(assemblies[i]);

        const char *img_name = il2cpp_image_get_name(img);

        if(strcmp(img_name, image) == 0)
        {
            return img;
        }
    }
    return 0;
}
// ========================================================================================================================================== //
void Il2Cpp::GetStaticFieldValue(const char *image, const char *namespaze, const char *clazz, const char *name, void *output) {
    void *img = GetImage(image);
    if(!img) {
        IL2CPP_LOGI("Can't find image %s!", image);
        return;
    }
    void *klass = GetClass(image, namespaze, clazz);
    if(!klass) {
        IL2CPP_LOGI("Can't find class %s for field %s!", clazz, name);
        return;
    }

    void *field = il2cpp_class_get_field_from_name(klass, name);
    if(!field) {
        IL2CPP_LOGI("Can't find field %s in class %s!", name, clazz);
        return;
    }

    il2cpp_field_static_get_value(field, output);
}
// ========================================================================================================================================== //
void Il2Cpp::SetStaticFieldValue(const char *image, const char *namespaze, const char *clazz, const char *name, void* value) {
    void *img = GetImage(image);
    if(!img) {
        IL2CPP_LOGI("Can't find image %s!", image);
        return;
    }
    void *klass = GetClass(image, namespaze, clazz);
    if(!klass) {
        IL2CPP_LOGI("Can't find class %s for field %s!", clazz, name);
        return;
    }

    void *field = il2cpp_class_get_field_from_name(klass, name);
    if(!field) {
        IL2CPP_LOGI("Can't find field %s in class %s!", name, clazz);
        return;
    }

    il2cpp_field_static_set_value(field, value);
}
// ========================================================================================================================================== //
void *Il2Cpp::GetClass(const char *image, const char *namespaze, const char *clazz) {
    string _sig = image;
    _sig += namespaze;
    _sig += clazz;

    if(m_cacheClasses.count(_sig) > 0)
    {
        return m_cacheClasses[_sig];
    }

    void *img = GetImage(image);
    if(!img) {
        IL2CPP_LOGI("Can't find image %s!", image);
        return 0;
    }

    vector<string> classes = split_string(clazz, ".");

    void *klass = il2cpp_class_from_name(img, namespaze, classes[0].c_str());
    if(!klass) {
        IL2CPP_LOGI("Can't find class %s!", clazz);
        return 0;
    }

    if(classes.size() > 1)
    {
        void *iter = 0;
        void *nest = il2cpp_class_get_nested_types(klass, &iter);
        while(nest)
        {
            const char *name = il2cpp_class_get_name(nest);
            if(strcmp(name, classes[1].c_str()) == 0)
            {
                return nest;
            }

            nest = il2cpp_class_get_nested_types(klass, &iter);
        }
        IL2CPP_LOGI("Can't find subclass %s in class %s!", classes[1].c_str(), classes[0].c_str());
        return 0;
    }

    return klass;
}
// ========================================================================================================================================== //
void *Il2Cpp::NewClassObject(const char *image, const char *namespaze, const char *clazz) {
    void *img = GetImage(image);
    if(!img) {
        IL2CPP_LOGI("Can't find image %s!", image);
        return 0;
    }

    void *klass = GetClass(image, namespaze, clazz);
    if(!klass) {
        IL2CPP_LOGI("Can't find class %s!", clazz);
        return 0;
    }

    void *obj = il2cpp_object_new(klass);
    if(!obj)
    {
        IL2CPP_LOGI("Can't create object for %s", clazz);
        return 0;
    }

    return obj;
}
// ========================================================================================================================================== //
void *Il2Cpp::GetMethodOffset(const char *image, const char *namespaze, const char *clazz, const char *name, int argsCount) {
    string _sig = image;
    _sig += namespaze;
    _sig += clazz;
    _sig += name;
    _sig += to_string(argsCount);

    if(m_cacheMethods.count(_sig) > 0)
    {
        return m_cacheMethods[_sig];
    }

    void *img = GetImage(image);
    if(!img) {
        IL2CPP_LOGI("Can't find image %s!", image);
        return 0;
    }

    void *klass = GetClass(image, namespaze, clazz);
    if(!klass) {
        IL2CPP_LOGI("Can't find class %s for method %s!", clazz, name);
        return 0;
    }

    void **method = (void**)il2cpp_class_get_method_from_name(klass, name, argsCount);
    if(!method) {
        IL2CPP_LOGI("Can't find method %s in class %s!", name, clazz);
        return 0;
    }

    m_cacheMethods[_sig] = *method;

    return *method;
}
// ========================================================================================================================================== //
void *Il2Cpp::GetMethodOffset(const char *image, const char *namespaze, const char *clazz, const char *name, char** args, int argsCount) {
    string _sig = image;
    _sig += namespaze;
    _sig += clazz;
    _sig += name;
    for (int i = 0; i < argsCount; i++) {
        _sig += args[i];
    }
    _sig += to_string(argsCount);

    if(m_cacheMethods.count(_sig) > 0)
    {
        return m_cacheMethods[_sig];
    }

    void *img = GetImage(image);
    if(!img) {
        IL2CPP_LOGI("Can't find image %s!", image);
        return 0;
    }

    void *klass = GetClass(image, namespaze, clazz);
    if(!klass) {
        IL2CPP_LOGI("Can't find class %s for method %s!", clazz, name);
        return 0;
    }

    void *iter = 0;

    int score = 0;

    void **method = (void**) il2cpp_class_get_methods(klass, &iter);
    while(method) {
        const char *fname = il2cpp_method_get_name(method);
        if(strcmp(fname, name) == 0) {
            for (int i = 0; i < argsCount; i++) {
                void *arg = il2cpp_method_get_param(method, i);
                if (arg) {
                    const char *tname = il2cpp_type_get_name(arg);
                    if (strcmp(tname, args[i]) == 0) {
                        score++;
                    } else {
                        IL2CPP_LOGI("Argument at index %d didn't matched requested argument!\r\n\tRequested: %s\r\n\tActual: %s\r\nSkipping function...", i, args[i], tname);
                        score = 0;
goto skip;
                    }
                }
            }
        }
        skip:

        if(score == argsCount)
        {
            IL2CPP_LOGI("Found matched function!");

            auto result = *method;
            m_cacheMethods[_sig] = result;

            return result;
        }

        method = (void **) il2cpp_class_get_methods(klass, &iter);
    }
    IL2CPP_LOGI("Cannot find function %s in class %s!", name, clazz);
    return 0;
}
// ========================================================================================================================================== //
uintptr_t Il2Cpp::GetFieldOffset(const char *image, const char *namespaze, const char *clazz, const char *name) {
    string _sig = image;
    _sig += namespaze;
    _sig += clazz;
    _sig += name;

    if(m_cacheFields.count(_sig) > 0)
    {
        return m_cacheFields[_sig];
    }

    void *img = GetImage(image);
    if(!img) {
        IL2CPP_LOGI("Can't find image %s!", image);
        return -1;
    }
    void *klass = GetClass(image, namespaze, clazz);
    if(!klass) {
        IL2CPP_LOGI("Can't find class %s for field %s!", clazz, name);
        return -1;
    }

    void *field = il2cpp_class_get_field_from_name(klass, name);
    if(!field) {
        IL2CPP_LOGI("Can't find field %s in class %s!", name, clazz);
        return -1;
    }

    auto result = il2cpp_field_get_offset(field);
    m_cacheFields[_sig] = result;

    return result;
}
// ========================================================================================================================================== //
bool Il2Cpp::IsAssembliesLoaded() {
    size_t size;
    void **assemblies = il2cpp_domain_get_assemblies(il2cpp_domain_get(), &size);

    return size != 0 && assemblies != 0;
}
// ========================================================================================================================================== //
void* Il2Cpp::CreateArray(const char *image, const char *namespaze, const char *clazz, size_t length) {
    void *img = GetImage(image);
    if(!img) {
        IL2CPP_LOGI("Can't find image %s!", image);
        return 0;
    }
    void *klass = GetClass(image, namespaze, clazz);
    if(!klass) {
        IL2CPP_LOGI("Can't find class %s!", clazz);
        return 0;
    }

    return il2cpp_array_new(klass, length);
}
// ========================================================================================================================================== //
