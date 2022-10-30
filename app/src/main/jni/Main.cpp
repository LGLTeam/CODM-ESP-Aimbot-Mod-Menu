#include "Includes.h"
#include "CODM.h"

bool titleValid = false;


extern "C" {
JNIEXPORT jstring
JNICALL
Java_com_gotoubun_Floating_Title(JNIEnv *env, jobject thiz) {
    titleValid = true;
    return env->NewStringUTF(("LOL DONT USE THIS PRJECT NOOB"));
}
}
void native_onSendConfig(JNIEnv *env, jobject thiz, jstring s, jstring v) {
    const char *config = env->GetStringUTFChars(s, 0);
    const char *value = env->GetStringUTFChars(v, 0);

    Config[config] = (u_long) strtoul(value, 0, 0);
}

jclass (*orig_FindClass)(JNIEnv *env, const char *name);
int Register1(JNIEnv *env) {
    JNINativeMethod methods[] = {{"onSendConfig", "(Ljava/lang/String;Ljava/lang/String;)V", (void *) native_onSendConfig},
                                 {"onCanvasDraw", "(Landroid/graphics/Canvas;IIF)V",         (void *) native_onCanvasDraw}};


    jclass clazz = env->FindClass("com/gotoubun/Floating");
    if (!clazz)
        return -1;

    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return -1;

    return 0;
}

int Register2(JNIEnv *env) {
    JNINativeMethod methods[] = {{"Init",  "(Landroid/content/Context;)V",                   (void *) native_Init}};

    jclass clazz = env->FindClass("com/gotoubun/Launcher");
    if (!clazz)
        return -1;

    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return -1;

    return 0;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (Register1(env) != 0)
        return -1;
    if (Register2(env) != 0)
        return -1;
    
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM *vm, void *reserved) {
}
