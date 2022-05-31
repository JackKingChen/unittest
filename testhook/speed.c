#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <jni.h>
#include <pthread.h>
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>

#define JNI_DEBUG 1

#ifdef JNI_DEBUG
#ifndef LOG_TAG
#define LOG_TAG "JNI_DEBUG"
#endif
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(3, LOG_TAG, __VA_ARGS__)
#endif

typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned char  BYTE;
typedef long LONG;
typedef WORD* PWORD;
typedef DWORD* PDWORD;
typedef BYTE* PBYTE;

static int (*old_gettimeofday)(struct timeval* tv, struct timezone *tz) = NULL;
static int (*old_clock_gettime)(clockid_t which_clock, struct timespec * tp) = NULL;

//自定义变速时间结构体
typedef struct _SPEEDTIME
{
	int64_t speedTime;		//保存变速后的时间值
	int64_t oriTime;		//保存原来的时间值
}SPEEDTIME,*PSPEEDTIME;
float g_speed = 1.0f;	//时间变速倍数，必须>0

//1秒=1000000LL微妙
#define SECTOUSEC 1000000LL
static SPEEDTIME g_timeofday = {0LL,0LL};

//计算变速后的时间
#define CALCSPEEDTIME(diff,curr,time,convert,sec,sec2)	if(time.speedTime==0LL)	\
		{	\
			time.speedTime = time.oriTime = curr;	\
		}	\
		else	\
		{	\
			diff = curr - time.oriTime;\
			diff *= g_speed;	\
			time.speedTime += diff;	\
			time.oriTime = curr;	\
			sec = time.speedTime / convert;	\
			sec2 = time.speedTime % convert; \
		}

static int new_gettimeofday(struct timeval* tv, struct timezone *tz)
{
	int64_t diff,curr;
	int iret = old_gettimeofday(tv,tz); //调用原来的时间函数
	if(iret!=0)
	{
		return iret;
	}

	curr = tv->tv_sec*SECTOUSEC + tv->tv_usec;	//计算当前微妙级的时间
	CALCSPEEDTIME(diff,curr,g_timeofday,SECTOUSEC,tv->tv_sec,tv->tv_usec)
	return iret;
}

//1秒=1000000000LL纳秒
#define SECTONSEC 1000000000LL
static SPEEDTIME g_clock_realtime = {0LL,0LL};
static SPEEDTIME g_clock_monotonic = {0LL,0LL};

static int new_clock_gettime(clockid_t clk_id, struct timespec *ptp)
{
	int64_t diff,curr;
	int iret = old_clock_gettime(clk_id,ptp);	//调用原来的时间函数
	if(iret!=0)
	{
		return iret;
	}
	if(clk_id==CLOCK_REALTIME || clk_id==CLOCK_MONOTONIC)
	{
		curr = ptp->tv_sec*SECTONSEC + ptp->tv_nsec;	//计算当前纳秒级的时间
		switch(clk_id)
		{
		case CLOCK_REALTIME:
			{
				CALCSPEEDTIME(diff,curr,g_clock_realtime,SECTONSEC,ptp->tv_sec,ptp->tv_nsec)
			}
			break;
		case CLOCK_MONOTONIC:
			{
				CALCSPEEDTIME(diff,curr,g_clock_monotonic,SECTONSEC,ptp->tv_sec,ptp->tv_nsec)
			}
			break;
		default:
			break;
		}
	}
	return iret;
}

typedef void (*DefMSHookFunction)(void *symbol, void *replace, void **result);
DefMSHookFunction MSHookFunction=NULL;

int speed(const char* path)
{
	if(path!=NULL)
	{
		if(MSHookFunction==NULL)
		{
			char filename[256];
#ifdef __i386__
			snprintf(filename, sizeof(filename), "%s/libsubstratex86.so", path);
#else
			snprintf(filename, sizeof(filename), "%s/libsubstrate.so", path);
#endif
			void* handle = dlopen(filename, RTLD_LAZY);
			if (handle == NULL)
			{
				LOGD("Failed to open libaray libsubstrate error:%s", dlerror());
				return -1;
			}

			MSHookFunction = dlsym(handle, "MSHookFunction");
			if(MSHookFunction == NULL)
			{
				LOGD("Failed to get func MSHookFunction error:%s", dlerror());
				return -1;
			}
			LOGD("JNI_OnLoad MSHookFunction=%x",MSHookFunction);
	    	//pthread_t id;
			//pthread_create(&id, NULL, (void *) Dump, NULL);
		}
		if(MSHookFunction!=NULL)
		{
			MSHookFunction(&gettimeofday, &new_gettimeofday, &old_gettimeofday);
			MSHookFunction(&clock_gettime, &new_clock_gettime, &old_clock_gettime);
#ifdef __i386__
			LOGD("JNI_OnLoad old_gettimeofday=%x,old_clock_gettime=%x 386",old_gettimeofday,old_clock_gettime);
#else
			LOGD("JNI_OnLoad old_gettimeofday=%x,old_clock_gettime=%x arm",old_gettimeofday,old_clock_gettime);
#endif
			return 0;
		}
	}

	return -1;
}

//static int g_thread = 0;
//
//jint JNI_OnLoad(JavaVM* vm, void* reserved) {
//    JNIEnv* env;
//
//    if((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_4) != JNI_OK)
//    {
//    	return -1;
//    }
//
//    if(g_thread==0)
//    {
//
////    	pthread_t id;
////		int ret=pthread_create(&id, NULL, (void *) speed, 1);
////		if(ret!=0)
////		{
////			LOGD ("Create pthread error!\n");
////		}
////		else
////		{
////			g_thread = 1;
////		}
////    	LOGD("JNI_OnLoad come in");
//    	if(speed(1)==0)
//    	{
//    		return -1;
//    	}
//		g_thread = 1;
//    }
//    return JNI_VERSION_1_4;
//}
