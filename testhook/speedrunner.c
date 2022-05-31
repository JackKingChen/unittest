#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>

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

typedef struct _MAPINFO{
	long dwBegin;
	long dwEnd;
	char pToken[5];
	long dwOffset;
	char pImage[256];
}MAPINFO,*PMAPINFO;

typedef struct _SPEEDINFO{
	long speedaddr;
	pid_t pid;
	float speed;
	int isArm;
	int isInject;
	long injectArgAddr;
	char path[100];
}SPEEDINFO,*PSPEEDINFO;

#define SPEEDOFFSET_ARM 0x6004
#define SPEEDOFFSET_X86 0x3024
#define INJECTARGOFFSET_ARM 0x500C
#define INJECTARGOFFSET_X86 0x4080

typedef void* (*defhoudini_hookDlopen)(const char * pathname, int mode, int*);
static defhoudini_hookDlopen houdini_hookDlopen = NULL;
#define HOUDINIHOOKDLOPEN "_ZN7houdini10hookDlopenEPKciPb"

typedef void* (*defhoudini_hookDlsym)(int, void* handle, const char* symbo);
static defhoudini_hookDlsym houdini_hookDlsym = NULL;
#define HOUDINIHOOKDLSYM "_ZN7houdini9hookDlsymEbPvPKc"

typedef int (*defhoudini_hookJniOnload)(int, void *funcPtr, void *vm, void *reserved);
static defhoudini_hookJniOnload houdini_hookJniOnload = NULL;
#define HOUDINIHOOKJNIONLOAD "_ZN7houdini13hookJniOnloadEbPvS0_S0_"

typedef int (*defInject)(void* arg1, void* arg2);
static defInject inject = NULL;

static int loadhoudini()
{
//	if(houdini_hookDlopen==NULL || houdini_hookDlsym==NULL)
//	{
//		void* handle = dlopen("/system/lib/libdvm.so", RTLD_LAZY);
//		if (handle == NULL)
//		{
//			LOGD("loadhoudini dlopen libdvm.so failed!");
//			return 1;
//		}
//		houdini_hookDlopen = (defhoudini_hookDlopen)dlsym(handle, HOUDINIHOOKDLOPEN);
//		if (houdini_hookDlopen == NULL)
//		{
//			LOGD("loadhoudini dlsym %s failed!",HOUDINIHOOKDLOPEN);
//			return 2;
//		}
//		houdini_hookDlsym = (defhoudini_hookDlsym)dlsym(handle, HOUDINIHOOKDLSYM);
//		if (houdini_hookDlsym == NULL)
//		{
//			LOGD("loadhoudini dlsym %s failed!",HOUDINIHOOKDLSYM);
//			return 3;
//		}
//		houdini_hookJniOnload = (defhoudini_hookDlsym)dlsym(handle, HOUDINIHOOKJNIONLOAD);
//		if (houdini_hookJniOnload == NULL)
//		{
//			LOGD("loadhoudini dlsym %s failed!",HOUDINIHOOKJNIONLOAD);
//			return 4;
//		}
//	}
	return 0;
}

static void getAppType(PSPEEDINFO pSpeedInfo)
{
//    FILE *fp;
//    char filename[32];
//
//    if (pSpeedInfo->pid < 0) {
//        /* self process */
//        snprintf(filename, sizeof(filename), "/proc/self/maps");
//    } else {
//        snprintf(filename, sizeof(filename), "/proc/%d/maps", pSpeedInfo->pid);
//    }
//
//    char line[1024];
//    int iFind = 0;
//    fp = fopen(filename, "r");
//    if (fp != NULL) {
//    	MAPINFO mi={0};
//		while (fgets(line, sizeof(line), fp)) {
//			sscanf(line,"%x-%x %[a-z-] %x %*x:%*x %*d %s",&mi.dwBegin,&mi.dwEnd,mi.pToken,&mi.dwOffset,mi.pImage);
//			//LOGD("%x-%x %s %s",mi.dwBegin,mi.dwEnd,mi.pToken,mi.pImage);
//			if(strstr(mi.pImage,"/system/lib/arm/"))
//			{
//				LOGD("%x-%x %s %s",mi.dwBegin,mi.dwEnd,mi.pToken,mi.pImage);
//				pSpeedInfo->isArm = 1;
//				break;
//			}
//		}
//		fclose(fp);
//    }
//    else
//    {
//    	LOGD("get_module_base fopen failed!");
//    }

	pSpeedInfo->isArm = 0;
}

static int getInjectArgAddr(PSPEEDINFO pSpeedInfo)
{
    FILE *fp;
    char filename[200];
    char line[1024];
    int injectFind=0;
    char injectName[200];
    snprintf(filename, sizeof(filename), "/proc/self/maps");
    fp = fopen(filename, "r");
    if(pSpeedInfo->isArm==1)
    {
    	snprintf(filename, sizeof(filename), "%s/libJniTool.so", pSpeedInfo->path);
    	pSpeedInfo->injectArgAddr = INJECTARGOFFSET_ARM;
    }
    else
    {
    	snprintf(filename, sizeof(filename), "%s/libJniToolx86.so", pSpeedInfo->path);
    	pSpeedInfo->injectArgAddr = INJECTARGOFFSET_X86;
    }
    if (fp != NULL) {
    	MAPINFO mi={0};
		while (fgets(line, sizeof(line), fp)) {
			sscanf(line,"%x-%x %[a-z-] %x %*x:%*x %*d %s",&mi.dwBegin,&mi.dwEnd,mi.pToken,&mi.dwOffset,mi.pImage);
			//LOGD("%x-%x %s %s",mi.dwBegin,mi.dwEnd,mi.pToken,mi.pImage);
			if(strstr(mi.pImage,filename))
			{
				LOGD("%x-%x %s %s",mi.dwBegin,mi.dwEnd,mi.pToken,mi.pImage);
				pSpeedInfo->injectArgAddr += mi.dwBegin;
				injectFind = 1;
				break;
			}
		}
		fclose(fp);
    }
    else
    {
    	LOGD("get_module_base fopen failed!");
    }

    return (injectFind==1)?0:-1;
}

static int get_module_base(PSPEEDINFO pSpeedInfo)
{
    FILE *fp;
    char filename[200];

    if (pSpeedInfo->pid < 0) {
        /* self process */
        snprintf(filename, sizeof(filename), "/proc/self/maps");
    } else {
        snprintf(filename, sizeof(filename), "/proc/%d/maps", pSpeedInfo->pid);
    }

    char line[1024];
    int speedFind=0;
    char injectName[200];
    fp = fopen(filename, "r");
    if(pSpeedInfo->isArm==1)
    {
    	snprintf(filename, sizeof(filename), "%s/libspeed.so", pSpeedInfo->path);
    	pSpeedInfo->speedaddr = SPEEDOFFSET_ARM;
    }
    else
    {
    	snprintf(filename, sizeof(filename), "%s/libspeedx86.so", pSpeedInfo->path);
    	pSpeedInfo->speedaddr = SPEEDOFFSET_X86;
    }
    if (fp != NULL) {
    	MAPINFO mi={0};
		while (fgets(line, sizeof(line), fp)) {
			sscanf(line,"%x-%x %[a-z-] %x %*x:%*x %*d %s",&mi.dwBegin,&mi.dwEnd,mi.pToken,&mi.dwOffset,mi.pImage);
			//LOGD("%x-%x %s %s",mi.dwBegin,mi.dwEnd,mi.pToken,mi.pImage);
			if(strstr(mi.pImage,filename))
			{
				LOGD("%x-%x %s %s",mi.dwBegin,mi.dwEnd,mi.pToken,mi.pImage);
				pSpeedInfo->speedaddr += mi.dwBegin;
				speedFind = 1;
				break;
			}
		}
		fclose(fp);
    }
    else
    {
    	LOGD("get_module_base fopen failed!");
    }

    return (speedFind==1)?0:-1;
}

static int changeSpeed(PSPEEDINFO pSpeedInfo)
{
	if(pSpeedInfo==NULL)
	{
		return 1;
	}
	char filename[32];
	snprintf(filename, sizeof(filename), "/proc/%d/mem", pSpeedInfo->pid);
	int file=open(filename,O_RDWR,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if(file<0)
	{
		perror("open file error!!!\n");
		return 2;
	}
	//先设置speed
	lseek(file,pSpeedInfo->speedaddr,SEEK_SET);
	if(write(file,&pSpeedInfo->speed,sizeof(pSpeedInfo->speed))<0)
	{
		perror("write file error!!!\n");
		close(file);
		return 3;
	}
	close(file);
	return 0;
}

typedef struct _INJECTARG
{
	pid_t target_pid;
	char library_path[200];
	char function_name[30];
	char param[100];
}InjectArg,*PInjectArg;

static InjectArg injectArg = {0};

static int MyInject(PSPEEDINFO pSpeedInfo)
{
	if(pSpeedInfo==NULL)
	{
		return 1;
	}
	if(loadhoudini()!=0)
	{
		LOGD("main loadhoudini failed!");
		return 2;
	}
	int temp=0;
	char filename[256];
	if(pSpeedInfo->isArm==0)
	{
		//x86
		snprintf(filename, sizeof(filename), "%s/libJniToolx86.so", pSpeedInfo->path);
	}
	else
	{
		//arm
		snprintf(filename, sizeof(filename), "%s/libJniTool.so", pSpeedInfo->path);
	}
//	void* handle = houdini_hookDlopen(filename,RTLD_LAZY,&temp);
	void* handle = dlopen(filename,RTLD_LAZY);
	if(handle==NULL)
	{
		LOGD("MyInject houdini_hookDlopen %d failed",pSpeedInfo->isArm);
		return 3;
	}
//	inject = (defInject)houdini_hookDlsym(temp,handle,"Inject");
	inject = (defInject)dlsym(handle,"Inject");
	if(inject==NULL)
	{
		LOGD("MyInject houdini_hookDlsym %d Inject failed",pSpeedInfo->isArm);
		return 4;
	}
	if(getInjectArgAddr(pSpeedInfo)!=0)
	{
		LOGD("MyInject houdini_hookDlsym %d getInjectArgAddr failed",pSpeedInfo->isArm);
		return 5;
	}
	LOGD("MyInject houdini_hookDlsym %d inject=%x",pSpeedInfo->isArm,inject);
	if(pSpeedInfo->isArm==0)
	{
		//x86
		snprintf(filename, sizeof(filename), "%s/libspeedx86.so", pSpeedInfo->path);
	}
	else
	{
		//arm
		snprintf(filename, sizeof(filename), "%s/libspeed.so", pSpeedInfo->path);
	}
	injectArg.target_pid = pSpeedInfo->pid;
	strcpy(injectArg.library_path,filename);
	strcpy(injectArg.function_name,"speed");
	strcpy(injectArg.param,pSpeedInfo->path);
	memcpy(pSpeedInfo->injectArgAddr,&injectArg,sizeof(injectArg));
	//if(houdini_hookJniOnload(temp,inject,NULL,NULL)<0)
	if(inject(NULL,NULL)<0)
	{
		LOGD("MyInject inject failed");
		return 6;
	}
	return 0;
}

//1.要加速的进程id
//2.变速倍数
//3.目录
int main(int argc, char** argv) {
	if(argc!=4)
	{
		LOGD("ERROR: invalid argument %d\n",argc);
		return 1;
	}
	SPEEDINFO speedInfo = {0,0,0.0f,0,0,0};
	speedInfo.pid = atoi(argv[1]);
	if (speedInfo.pid <=0) {
		LOGD("Can't find the process pid=%s\n",argv[1]);
		return 2;
	}
	if(argv[3]==NULL && strlen(argv[3])>=100)
	{
		LOGD("ERROR: invalid catgory\n");
		return 3;
	}
	strcpy(speedInfo.path,argv[3]);
	getAppType(&speedInfo);
	if(get_module_base(&speedInfo)!=0)
	{
		if(MyInject(&speedInfo)!=0)
		{
			LOGD("ERROR: MyInject Failed\n");
			return 4;
		}
		if(get_module_base(&speedInfo)!=0)
		{
			LOGD("Can't find the module\n");
			return 5;
		}
	}
	speedInfo.speed = atof(argv[2]);
	if(speedInfo.speed<=0.0f)
	{
		LOGD("speed value %s is error\n",argv[2]);
		return 6;
	}
	if(changeSpeed(&speedInfo)!=0)
	{
		LOGD("changeSpeed failed\n");
		return 7;
	}

	return 0;
}
