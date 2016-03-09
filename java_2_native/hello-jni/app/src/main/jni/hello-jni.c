#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jni.h>
#include <assert.h>
#include <android/log.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "zhouat", __VA_ARGS__)
#define father_clazz "com/example/hellojni/HelloJni"
#define target_fun "test"
#define target_fun_sign  "()Ljava/lang/String;"

jstring native_test(JNIEnv* env, jobject thiz)
{
    return (*env)->NewStringUTF(env, "fuck...");
}

static JNINativeMethod gMethods[] = {
        {target_fun, target_fun_sign, (void*)native_test},//绑定
};

/*
* 为某一个类注册本地方法
*/
static int registerNativeMethods(JNIEnv* env
        , const char* className
        , JNINativeMethod* gMethods, int numMethods) {
    jclass clazz;
    clazz = (*env)->FindClass(env, className);
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}


/*
* 为所有类注册本地方法
*/
static int registerNatives(JNIEnv* env) {
    return registerNativeMethods(env, father_clazz, gMethods,
                                 sizeof(gMethods) / sizeof(gMethods[0]));
}

#define SET_METHOD_FLAG(method, flag) \
    do { (method)->accessFlags |= (flag); } while (0)

enum {
    ACC_PUBLIC       = 0x00000001,       // class, field, method, ic
    ACC_PRIVATE      = 0x00000002,       // field, method, ic
    ACC_PROTECTED    = 0x00000004,       // field, method, ic
    ACC_STATIC       = 0x00000008,       // field, method, ic
    ACC_FINAL        = 0x00000010,       // class, field, method, ic
    ACC_SYNCHRONIZED = 0x00000020,       // method (only allowed on natives)
    ACC_SUPER        = 0x00000020,       // class (not used in Dalvik)
    ACC_VOLATILE     = 0x00000040,       // field
    ACC_BRIDGE       = 0x00000040,       // method (1.5)
    ACC_TRANSIENT    = 0x00000080,       // field
    ACC_VARARGS      = 0x00000080,       // method (1.5)
    ACC_NATIVE       = 0x00000100,       // method
    ACC_INTERFACE    = 0x00000200,       // class, ic
    ACC_ABSTRACT     = 0x00000400,       // class, method, ic
    ACC_STRICT       = 0x00000800,       // method
    ACC_SYNTHETIC    = 0x00001000,       // field, method, ic
    ACC_ANNOTATION   = 0x00002000,       // class, ic (1.5)
    ACC_ENUM         = 0x00004000,       // class, field, ic (1.5)
    ACC_CONSTRUCTOR  = 0x00010000,       // method (Dalvik only)
    ACC_DECLARED_SYNCHRONIZED =
    0x00020000,       // method (Dalvik only)
    ACC_CLASS_MASK =
    (ACC_PUBLIC | ACC_FINAL | ACC_INTERFACE | ACC_ABSTRACT
     | ACC_SYNTHETIC | ACC_ANNOTATION | ACC_ENUM),
    ACC_INNER_CLASS_MASK =
    (ACC_CLASS_MASK | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC),
    ACC_FIELD_MASK =
    (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
     | ACC_VOLATILE | ACC_TRANSIENT | ACC_SYNTHETIC | ACC_ENUM),
    ACC_METHOD_MASK =
    (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
     | ACC_SYNCHRONIZED | ACC_BRIDGE | ACC_VARARGS | ACC_NATIVE
     | ACC_ABSTRACT | ACC_STRICT | ACC_SYNTHETIC | ACC_CONSTRUCTOR
     | ACC_DECLARED_SYNCHRONIZED),
};

typedef unsigned int u4;
typedef unsigned short u2;
typedef int bool;

typedef struct DexProto {
    const void* dexFile;     /* file the idx refers to */
    u4 protoIdx;                /* index into proto_ids table of dexFile */
} DexProto;

typedef  struct Method Method;
typedef void (*DalvikBridgeFunc)(const u4* args, void* pResult,
                                 const Method* method, struct Thread* self);
typedef void (*DalvikNativeFunc)(const u4* args, void* pResult);

struct Method {
    void 		 *clazz;
    u4             accessFlags;
    u2             methodIndex;
    u2              registersSize;  /* ins + locals */
    u2              outsSize;
    u2              insSize;
    const char*     name;
    DexProto        prototype;
    const char*     shorty;
    const u2*       insns;
    int             jniArgInfo;
    DalvikBridgeFunc nativeFunc;
    const void* registerMap;
    bool            inProfile;
};

jboolean g_bool_fun_is_static=JNI_FALSE;

Method* get_method(JNIEnv *env, const char *szclz, const char *function, const char* fun_sign) {
    jclass clazz = (*env)->FindClass(env,szclz);
    jmethodID method = (*env)->GetMethodID(env,clazz,function,fun_sign);
    if(method==NULL){
        g_bool_fun_is_static=JNI_TRUE;
        (*env)->ExceptionClear(env);
        method = (*env)->GetStaticMethodID(env,clazz,function,fun_sign);
    }
    return method;
}

void Hook(JNIEnv * env) {
    Method *method = NULL;
    method=get_method(env, father_clazz , target_fun, target_fun_sign);
    if(g_bool_fun_is_static==JNI_TRUE){
        method->registersSize=0;
    } else{
        method->registersSize=1;
    }
    SET_METHOD_FLAG(method, ACC_NATIVE);
    method->jniArgInfo = 0x80000000;
    registerNatives(env);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    assert(env != NULL);

    Hook(env);

    result = JNI_VERSION_1_4;

    return result;
}