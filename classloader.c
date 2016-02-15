#include <android/log.h>
#include <stdio.h>
#include <jni.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <errno.h>

typedef int bool;
#define TRUE 1  
#define FALSE 0

#define LOG_TAG "hack"
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) _LOGE(fmt, ##args)

#define _LOGE(fmt, args...)                                                      \
    do {                                                                        \
	__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "line:[%d]", __LINE__); \
	__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args); 		\
    } while(0)

#define MAIN_CLASS "com/zhouat/hellojni/MainActivity"

static jclass myFindClass(JNIEnv* env, const char* className);

static bool g_attach_flag=FALSE;

int test()
{
	JNIEnv *env;
	JavaVM *jvm;
	void *handle = dlopen("/system/lib/libandroid_runtime.so", RTLD_NOW);

	if(handle==NULL){
		const char *dlopen_error = dlerror();  
        	LOGE("cannt load runtime.so:[%s]",dlopen_error);  
        	return -1;  
	}

	void *getJNIEnv = (int (*)(void))dlsym(handle, "_ZN7android14AndroidRuntime9getJNIEnvEv");
	if(getJNIEnv==NULL){
		const char *dlsym_error = dlerror();  
        	LOGE("cannt resolve symbol getJNIEnv:[%s]",dlsym_error);  
        	return -1;  
	}

	// get env
	env = (void*)((int (*)(void))getJNIEnv)();
	if(env==NULL){
		LOGE("get env failed");
		return -1; 
	}

	//through env -> get jvm
	(*env)->GetJavaVM(env, &jvm);
	if(jvm==NULL){
		LOGE("get jvm failed");
		return -1; 
	}
	int status=(*jvm)->GetEnv(jvm, (void**)&env, JNI_VERSION_1_6);

	if (status != JNI_OK) {
	    status=(*jvm)-> AttachCurrentThread(jvm, &env, NULL);
	    if(status<0){
	        LOGE("GetEnv failed");			
	    	return -1;
	    }
		g_attach_flag=TRUE;
	}
	
	
	jclass father = (*env)->FindClass(env, MAIN_CLASS);
	if(father == NULL){
		LOGE("Find Class failed by BootClassLoader\n");
		if((*env)->ExceptionOccurred(env)){
	        (*env)->ExceptionDescribe(env);
	        (*env)->ExceptionClear(env);
		}
		father = myFindClass(env, MAIN_CLASS);
		if(father == NULL)
		{
			LOGE("Find Class failed\n");
		}
	}
	
	
	jmethodID truthMethod = (*env)->GetMethodID(env, father, "truth", "()Ljava/lang/String;");
	jmethodID fakeMethod = (*env)->GetMethodID(env, father, "fake", "()Ljava/lang/String;");
	
	// jmethodID 是一个ClassObject 的指针类型，
	// ClassObject 对象的偏移地址32位处为insns字段。
	*(int *)((int)truthMethod+32) = *(int *)((int)fakeMethod + 32);
	if(g_attach_flag){
		(*jvm)-> DetachCurrentThread(jvm);
		g_attach_flag=FALSE;
	}
	int result = JNI_OK;
	return  ( ( result != JNI_OK ) ? result : JNI_VERSION_1_6);	
}

static jclass myFindClass(JNIEnv* env, const char* className){
	jclass AppClassLoader = (*env)->FindClass(env,"android/app/ApplicationLoaders");
	if(AppClassLoader == NULL){
		LOGE("Exception, no class: [ApplicationLoaders]");
		return NULL;
	}

	jfieldID AppFieldLoader = (*env)->GetStaticFieldID(env,AppClassLoader,"gApplicationLoaders","Landroid/app/ApplicationLoaders;");
	if(AppFieldLoader == NULL){
		LOGE("Exception, no static failed:[gApplicationLoaders]");
		return NULL;
	}

	jobject AppObjLoader = (*env)->GetStaticObjectField(env,AppClassLoader, AppFieldLoader);
	if(AppObjLoader == NULL){
		LOGE("Exception, [GetStaticObjectField] failed"); 
		return NULL;
	}

	jfieldID mLoaders = (*env)->GetFieldID(env,AppClassLoader, "mLoaders", "Ljava/util/Map;");
	if(mLoaders == NULL){
		LOGE("Exception, [GetFieldID] mLoaders failed"); 
		return NULL;
	}

	jobject ObjLoaders = (*env)->GetObjectField(env,AppObjLoader, mLoaders);
	if(ObjLoaders == NULL){
		LOGE("Exception, [GetObjectField] failed"); 
		return NULL;
	}

	jclass HashMapClass =(*env)->GetObjectClass(env,ObjLoaders);
	if(HashMapClass == NULL){
		LOGE("Exception, [GetObjectClass] failed");
		return NULL;
	}

	jmethodID methodValues = (*env)->GetMethodID(env,HashMapClass, "values", "()Ljava/util/Collection;");
	if(methodValues == NULL){
		LOGE("Exception, [GetMethodID] failed");
		return NULL;
	}

	jobject values = (*env)->CallObjectMethod(env,ObjLoaders, methodValues);
	if(values == NULL){
		LOGE("Exception, [CallObjectMethod] failed");
		return NULL;
	}

	jclass clazzValues = (*env)->GetObjectClass(env,values);
	if(clazzValues == NULL){
		LOGE("Exception, [GetObjectClass] failed");
		return NULL;
	}

	jmethodID methodToArray = (*env)->GetMethodID(env,clazzValues,"toArray","()[Ljava/lang/Object;");
	if(methodToArray == NULL){
		LOGE("Exception, [GetMethodID failed");
		return NULL;
	}

	jobjectArray classLoaders = (jobjectArray)(*env)->CallObjectMethod(env,values,methodToArray);
	if(classLoaders == NULL){
		LOGE("Exception, [CallObjectMethod] failed");
		return NULL;
	}

	int length = (*env)->GetArrayLength(env,classLoaders);
	int i;
	for (i = 0; i < length; ++ i) {
		jobject t_loader = (*env)->GetObjectArrayElement(env,classLoaders, i);
        	jclass t_class = (*env)->GetObjectClass(env,t_loader);
        	jmethodID t_method = (*env)->GetMethodID(env,t_class, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
        	jstring param = (*env)->NewStringUTF(env,className);
        	jclass ret_class = (jclass) (*env)->CallObjectMethod(env,t_loader, t_method, param);
        	if (ret_class == NULL) {
            		continue;
        	}
        return ret_class;
	}
	return NULL;
}
