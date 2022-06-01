#include <stdio.h>
#include <stdlib.h>
#include <asm/user.h>
#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <elf.h>
#include <jni.h>
#include <android/log.h>

#if defined(__i386__)
#define pt_regs         user_regs_struct
#endif

#define ENABLE_DEBUG 1

#if ENABLE_DEBUG
#define  LOG_TAG "JniTool"
#define  LOGD(fmt, args...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG, fmt, ##args)
#define DEBUG_PRINT(format,args...) \
    LOGD(format, ##args)
#else
#define DEBUG_PRINT(format,args...)
#endif

#define CPSR_T_MASK     ( 1u << 5 )

#if defined(__i386__)
const char *libc_path = "/system/lib/libc.so";
#else
const char *libc_path = "/system/lib/arm/libc.so";
#endif
const char *linker_path = "/system/bin/linker";


int ExecuteCommand(char *pCmd);
int IsInjected(int dwPid,char *pLibPath);


int ptrace_readdata(pid_t pid,  uint8_t *src, uint8_t *buf, size_t size)
{
    uint32_t i, j, remain;
    uint8_t *laddr;

    union u {
        long val;
        char chars[sizeof(long)];
    } d;

    j = size / 4;
    remain = size % 4;

    laddr = buf;

    for (i = 0; i < j; i ++) {
        d.val = ptrace(PTRACE_PEEKTEXT, pid, src, 0);
        memcpy(laddr, d.chars, 4);
        src += 4;
        laddr += 4;
    }

    if (remain > 0) {
        d.val = ptrace(PTRACE_PEEKTEXT, pid, src, 0);
        memcpy(laddr, d.chars, remain);
    }

    return 0;
}

int ptrace_writedata(pid_t pid, uint8_t *dest, uint8_t *data, size_t size)
{
    uint32_t i, j, remain;
    uint8_t *laddr;

    union u {
        long val;
        char chars[sizeof(long)];
    } d;

    j = size / 4;
    remain = size % 4;

    laddr = data;

    for (i = 0; i < j; i ++) {
        memcpy(d.chars, laddr, 4);
        ptrace(PTRACE_POKETEXT, pid, dest, (void *)d.val);

        dest  += 4;
        laddr += 4;
    }

    if (remain > 0) {
        d.val = ptrace(PTRACE_PEEKTEXT, pid, dest, 0);
        for (i = 0; i < remain; i ++) {
            d.chars[i] = *laddr ++;
        }

        ptrace(PTRACE_POKETEXT, pid, dest, (void *)d.val);
    }

    return 0;
}

#if defined(__arm__)
int ptrace_call(pid_t pid, uint32_t addr, long *params, uint32_t num_params, struct pt_regs* regs)
{
    uint32_t i;
    for (i = 0; i < num_params && i < 4; i ++) {
        regs->uregs[i] = params[i];
    }

    //
    // push remained params onto stack
    //
    if (i < num_params) {
        regs->ARM_sp -= (num_params - i) * sizeof(long) ;
        ptrace_writedata(pid, (void *)regs->ARM_sp, (uint8_t *)&params[i], (num_params - i) * sizeof(long));
    }

    regs->ARM_pc = addr;
    if (regs->ARM_pc & 1) {
        /* thumb */
        regs->ARM_pc &= (~1u);
        regs->ARM_cpsr |= CPSR_T_MASK;
    } else {
        /* arm */
        regs->ARM_cpsr &= ~CPSR_T_MASK;
    }

    regs->ARM_lr = 0;

    if (ptrace_setregs(pid, regs) == -1
            || ptrace_continue(pid) == -1) {
    	DEBUG_PRINT("error\n");
        return -1;
    }

    int stat = 0;
    waitpid(pid, &stat, WUNTRACED);
    while (stat != 0xb7f) {
        if (ptrace_continue(pid) == -1) {
        	DEBUG_PRINT("error\n");
            return -1;
        }
        waitpid(pid, &stat, WUNTRACED);
    }

    return 0;
}

#elif defined(__i386__)
long ptrace_call(pid_t pid, uint32_t addr, long *params, uint32_t num_params, struct user_regs_struct * regs)
{
    regs->esp -= (num_params) * sizeof(long) ;
    ptrace_writedata(pid, (void *)regs->esp, (uint8_t *)params, (num_params) * sizeof(long));

    long tmp_addr = 0x00;
    regs->esp -= sizeof(long);
    ptrace_writedata(pid, regs->esp, (char *)&tmp_addr, sizeof(tmp_addr));

    regs->eip = addr;

    if (ptrace_setregs(pid, regs) == -1
            || ptrace_continue( pid) == -1) {
    	DEBUG_PRINT("error\n");
        return -1;
    }

    int stat = 0;
    waitpid(pid, &stat, WUNTRACED);
    while (stat != 0xb7f) {
        if (ptrace_continue(pid) == -1) {
        	DEBUG_PRINT("error\n");
            return -1;
        }
        waitpid(pid, &stat, WUNTRACED);
    }

    return 0;
}
#else
#error "Not supported"
#endif

int ptrace_getregs(pid_t pid, struct pt_regs * regs)
{
    if (ptrace(PTRACE_GETREGS, pid, NULL, regs) < 0) {
        perror("ptrace_getregs: Can not get register values");
        return -1;
    }

    return 0;
}

int ptrace_setregs(pid_t pid, struct pt_regs * regs)
{
    if (ptrace(PTRACE_SETREGS, pid, NULL, regs) < 0) {
        perror("ptrace_setregs: Can not set register values");
        return -1;
    }

    return 0;
}

int ptrace_continue(pid_t pid)
{
    if (ptrace(PTRACE_CONT, pid, NULL, 0) < 0) {
        perror("ptrace_cont");
        return -1;
    }

    return 0;
}

int ptrace_attach(pid_t pid)
{
    if (ptrace(PTRACE_ATTACH, pid, NULL, 0) < 0) {
        perror("ptrace_attach");
        return -1;
    }

    int status = 0;
    waitpid(pid, &status , WUNTRACED);

    return 0;
}

int ptrace_detach(pid_t pid)
{
    if (ptrace(PTRACE_DETACH, pid, NULL, 0) < 0) {
        perror("ptrace_detach");
        return -1;
    }

    return 0;
}

void* get_module_base(pid_t pid, const char* module_name)
{
    FILE *fp;
    long addr = 0;
    char *pch;
    char filename[32];
    char line[1024];

    if (pid < 0) {
        /* self process */
        snprintf(filename, sizeof(filename), "/proc/self/maps", pid);
    } else {
        snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    }

    fp = fopen(filename, "r");

    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                pch = strtok( line, "-" );
                addr = strtoul( pch, NULL, 16 );

                if (addr == 0x8000)
                    addr = 0;

                break;
            }
        }

        fclose(fp) ;
    }

    return (void *)addr;
}

void* get_remote_addr(pid_t target_pid, const char* module_name, void* local_addr)
{
    void* local_handle, *remote_handle;

    local_handle = get_module_base(-1, module_name);
    remote_handle = get_module_base(target_pid, module_name);

    DEBUG_PRINT("[+] get_remote_addr: local[%x], remote[%x]\n", local_handle, remote_handle);

    void * ret_addr = (void *)((uint32_t)local_addr + (uint32_t)remote_handle - (uint32_t)local_handle);

//2016-1-18 10:34:41 ������Ҫƫ��+2��������
//#if defined(__i386__)
//    if (!strcmp(module_name, libc_path)) {
//        ret_addr += 2;
//    }
//#endif
    return ret_addr;
}

int find_pid_of(const char *process_name)
{
    int id;
    pid_t pid = -1;
    DIR* dir;
    FILE *fp;
    char filename[32];
    char cmdline[256];

    struct dirent * entry;

    if (process_name == NULL)
        return -1;

    dir = opendir("/proc");
    if (dir == NULL)
        return -1;

    while((entry = readdir(dir)) != NULL) {
        id = atoi(entry->d_name);
        if (id != 0) {
            sprintf(filename, "/proc/%d/cmdline", id);
            fp = fopen(filename, "r");
            if (fp) {
                fgets(cmdline, sizeof(cmdline), fp);
                fclose(fp);

                if (strcmp(process_name, cmdline) == 0) {
                    /* process found */
                    pid = id;
                    break;
                }
            }
        }
    }

    closedir(dir);
    return pid;
}

long ptrace_retval(struct pt_regs * regs)
{
#if defined(__arm__)
    return regs->ARM_r0;
#elif defined(__i386__)
    return regs->eax;
#else
#error "Not supported"
#endif
}

long ptrace_ip(struct pt_regs * regs)
{
#if defined(__arm__)
    return regs->ARM_pc;
#elif defined(__i386__)
    return regs->eip;
#else
#error "Not supported"
#endif
}

int ptrace_call_wrapper(pid_t target_pid, const char * func_name, void * func_addr, long * parameters, int param_num, struct pt_regs * regs)
{
    DEBUG_PRINT("[+] Calling %s in target process.\n", func_name);
    if (ptrace_call(target_pid, (uint32_t)func_addr, parameters, param_num, regs) == -1)
        return -1;

    if (ptrace_getregs(target_pid, regs) == -1)
        return -1;
    DEBUG_PRINT("[+] Target process returned from %s, return value=%x, pc=%x \n",
            func_name, ptrace_retval(regs), ptrace_ip(regs));
    return 0;
}

int inject_remote_process(pid_t target_pid, const char *library_path, const char *function_name, const char *param, size_t param_size)
{
    int nRet = -1;
    void *mmap_addr, *dlopen_addr, *dlsym_addr, *dlclose_addr, *dlerror_addr;
    void *local_handle, *remote_handle, *dlhandle;
    uint8_t *map_base = 0;
    uint8_t *dlopen_param1_ptr, *dlsym_param2_ptr, *saved_r0_pc_ptr, *inject_param_ptr, *remote_code_ptr, *local_code_ptr;

    struct pt_regs regs = {0}, original_regs = {0};
    extern uint32_t _dlopen_addr_s, _dlopen_param1_s, _dlopen_param2_s, _dlsym_addr_s, \
        _dlsym_param2_s, _dlclose_addr_s, _inject_start_s, _inject_end_s, _inject_function_param_s, \
        _saved_cpsr_s, _saved_r0_pc_s;

    uint32_t code_length;
    long parameters[10];


    if(IsInjected(target_pid,library_path) == 1){
    	nRet = 0;
    	goto exit;
    }
    ExecuteCommand("setenforce 0");

    DEBUG_PRINT("[+] Injecting process: %d\n", target_pid);

    if (ptrace_attach(target_pid) == -1)
        goto exit;

    if (ptrace_getregs(target_pid, &regs) == -1)
        goto exit;


    /* save original registers */
    memcpy(&original_regs, &regs, sizeof(regs));

    //PrintData(&original_regs,sizeof(regs));

    mmap_addr = get_remote_addr(target_pid, libc_path, (void *)mmap);
    DEBUG_PRINT("[+] Remote mmap address: %x\n", mmap_addr);

    /* call mmap */
    parameters[0] = 0;  // addr
    parameters[1] = 0x4000; // size
    parameters[2] = PROT_READ | PROT_WRITE | PROT_EXEC;  // prot
    parameters[3] =  MAP_ANONYMOUS | MAP_PRIVATE; // flags
    parameters[4] = 0; //fd
    parameters[5] = 0; //offset

    if (ptrace_call_wrapper(target_pid, "mmap", mmap_addr, parameters, 6, &regs) == -1)
        goto exit;

    map_base = (uint8_t *)ptrace_retval(&regs);

    if(map_base == 0){
    	nRet = -2;
    	goto exit;
	}

    dlopen_addr = get_remote_addr( target_pid, linker_path, (void*)dlopen );
    dlsym_addr = get_remote_addr( target_pid, linker_path, (void*)dlsym );
    dlclose_addr = get_remote_addr( target_pid, linker_path, (void*)dlclose );
    dlerror_addr = get_remote_addr( target_pid, linker_path, (void*)dlerror );

    DEBUG_PRINT("[+] Get imports: dlopen: %x, dlsym: %x, dlclose: %x, dlerror: %x\n",
            dlopen_addr, dlsym_addr, dlclose_addr, dlerror_addr);

    DEBUG_PRINT("library path = %s\n", library_path);
    ptrace_writedata(target_pid, map_base, library_path, strlen(library_path) + 1);

    parameters[0] = map_base;
    parameters[1] = RTLD_NOW| RTLD_GLOBAL;

    if (ptrace_call_wrapper(target_pid, "dlopen", dlopen_addr, parameters, 2, &regs) == -1)
        goto exit;

    void * sohandle = ptrace_retval(&regs);

    if(sohandle == 0){
		if (ptrace_call_wrapper(target_pid, "dlerror", dlerror_addr, parameters, 0, &regs) == -1)
			goto exit;
		char *pError = ptrace_retval(&regs);
		DEBUG_PRINT("dlerror %s\n", pError);
    	nRet = -3;
    	goto exit;
    }

#define FUNCTION_NAME_ADDR_OFFSET       0x100
    ptrace_writedata(target_pid, map_base + FUNCTION_NAME_ADDR_OFFSET, function_name, strlen(function_name) + 1);
    parameters[0] = sohandle;
    parameters[1] = map_base + FUNCTION_NAME_ADDR_OFFSET;

    if (ptrace_call_wrapper(target_pid, "dlsym", dlsym_addr, parameters, 2, &regs) == -1)
        goto exit;

    void * hook_entry_addr = ptrace_retval(&regs);
    DEBUG_PRINT("%s = %p\n", function_name,hook_entry_addr);

    if(hook_entry_addr == 0){
		nRet = -4;
		goto exit;
	}

#define FUNCTION_PARAM_ADDR_OFFSET      0x200
    ptrace_writedata(target_pid, map_base + FUNCTION_PARAM_ADDR_OFFSET, param, strlen(param) + 1);
    parameters[0] = map_base + FUNCTION_PARAM_ADDR_OFFSET;

    if (ptrace_call_wrapper(target_pid, function_name, hook_entry_addr, parameters, 1, &regs) == -1)
        goto exit;

    nRet = ptrace_retval(&regs);
    DEBUG_PRINT("Inject Success!!! nRet = %d\n",nRet);
//    DEBUG_PRINT("Press enter to dlclose and detach\n");
//    getchar();
//    parameters[0] = sohandle;
//
//    if (ptrace_call_wrapper(target_pid, "dlclose", dlclose, parameters, 1, &regs) == -1)
//        goto exit;

    /* restore */
    //PrintData(&original_regs,sizeof(regs));

exit:
	ptrace_setregs(target_pid, &original_regs);
	ptrace_detach(target_pid);
    return nRet;
}
//
//char *pselinux_mnt = 0;
//
//int init_selinuxmnt(){
//	int nRet = 0;
//	//struct statfs fs = {0};
//	char pBuffer[1024] = {0};
//	int pRet = 0;
//
//	if(!pselinux_mnt){
//		while(1){
//			nRet = statfs("/sys/fs/selinux",&pRet);
//			DEBUG_PRINT("statfs %d\n",nRet);
//			if(nRet >= 0)
//				break;
//			if(__errno() != 4){
//				FILE *fp = fopen("/proc/filesystems","r");
//				if(fp){
//					while(fgets(pBuffer,1023,fp)){
//						printf("1 %s\n",pBuffer);
//						if(strstr(pBuffer,"selinuxfs")){
//							fclose(fp);
//							fp = fopen("/proc/mounts","r");
//							if(fp){
//								while(fgets(pBuffer,1023,fp)){
//									printf("2 %s\n",pBuffer);
//									char *pTmp = strchr(pBuffer,0x20);
//									if(!pTmp)
//										break;
//									char *pMode = pTmp + 1;
//									char *pTmp2 = strchr(pMode,0x20);
//									if(!pTmp2)
//										break;
//									char *pTmp3 = pTmp2 + 1;
//									char pStr[] = "selinuxfs";
//									int nLen = 10;
//									if(strncmp(pTmp3,pStr,10) == 0){
//										*pTmp2 = 0;
//										pselinux_mnt = strdup(pMode);
//									}
//								}
//								fclose(fp);
//							}
//						}
//
//					}
//				}
//				break;
//			}else{
//				if(!nRet && pRet == 0xf97cff8c){
//					pselinux_mnt = strdup("/sys/fs/selinux");
//				}
//			}
//		}
//	}
//
//	return 0;
//}
//int selinux_getenforce(){
//	int nRet = 0;
//	char pFile[0x1000] = {0};
//	char pBuffer[0x20] = {0};
//	if(pselinux_mnt){
//		snprintf(pFile,0x1000,"%s/enforce",pselinux_mnt);
//		int hFile = open(pFile,0);
//		if(hFile >= 0){
//			int nRead = read(hFile,pBuffer,0x13);
//			close(hFile);
//			int nTmp = 0;
//			if(nRead >= 0 && sscanf(pBuffer,"%d",&nTmp) == 1)
//				nRet = nTmp;
//		}
//
//	}else{
//		nRet = -1;
//	}
//
//	return nRet;
//}
//int selinux_setenforce(unsigned int nCode){
//	int nRet = 0;
//	char pFile[0x1000] = {0};
//	char pBuffer[0x20] = {0};
//	if(pselinux_mnt){
//		snprintf(pFile,0x1000,"%s/enforce",pselinux_mnt);
//		int hFile = open(pFile,1);
//		if(hFile >= 0){
//			snprintf(pBuffer,0x14,"%d",nCode);
//			char *p = pBuffer;
//			unsigned int nResult = 0;
//			do{
//				unsigned int nTmp = *(unsigned int*)p;
//				p += 4;
//				nResult = ~nTmp & (nTmp - 0x1010101) & 0x80808080;
//			}while(!nResult);
//			if(!(nResult & 0x8080))
//				nResult >>= 16;
//			if((nResult & 0x8080 & 0xffff) == 0){
//				p += 2;
//			}
//			nRet = write(hFile,pBuffer,p - pBuffer);
//			close(hFile);
//			nRet = nRet >> 31;
//		}
//
//	}else{
//		nRet = -1;
//	}
//
//	return nRet;
//}
int __attribute__ ((constructor)) IgnoreSignal(){
	bsd_signal(4,0);
	bsd_signal(6,0);
	bsd_signal(7,0);
	bsd_signal(8,0);
	bsd_signal(11,0);
	bsd_signal(16,0);
	return bsd_signal(13,0);
}

int ExecuteCommand(char *pCmd){
	//LOG("RootExec Entry %s\n",pCmd);
	int bRet = 0;
	FILE *fp = NULL;

	//char pBuffer[MAX_BUFFER_SIZE] = {0};
	fp = popen(pCmd,"r");

	if(fp == NULL){
		//LOG("popen %s failed\n",pCmd);
		return bRet;
	}
	//char *pBuffer = (char*)malloc(MAX_BUFFER_SIZE);
	//memset(pBuffer,0,MAX_BUFFER_SIZE);

	//fread(pBuffer,MAX_BUFFER_SIZE,1,fp);
	//fgets(pBuffer,sizeof(pBuffer),fp);

	//LOG("%s\n",pBuffer);

	pclose(fp);

	//return pBuffer;
	bRet = 1;
	return bRet;
}
__attribute__ ((visibility ("default")))
int IsInjected(int dwPid,char *pLibPath){
	int bRet = 0;

	if(!dwPid || !pLibPath)
		return bRet;

	char *pLibName = strrchr(pLibPath,'/');

	if(!pLibName)
		return bRet;

	pLibName++;
	FILE *fp = NULL;

	char pBuffer[4096] = {0};
	char pCmd[256] = {0};
	char pImageName[256] = {0};

	sprintf(pCmd,"/proc/%d/maps",dwPid);

	fp = fopen(pCmd,"r");

	if(fp == NULL){
		DEBUG_PRINT("fopen %s failed\n",pCmd);
		return bRet;
	}
	bRet = 0;

	while (fgets(pBuffer, sizeof(pBuffer), fp)) {
		if(strstr(pBuffer,pLibName)){
			bRet = 1;
		}
	}

	fclose(fp);
	DEBUG_PRINT("IsInjected %d\n",bRet);
	return bRet;
}

typedef struct _INJECTARG
{
	pid_t target_pid;
	char library_path[200];
	char function_name[30];
	char param[100];
}InjectArg,*PInjectArg;

InjectArg g_injectArg = {0};

__attribute__ ((visibility ("default")))
int Inject(PInjectArg arg1,PInjectArg arg2){
	DEBUG_PRINT("Inject come in");
	if(g_injectArg.target_pid==0)
	{
		DEBUG_PRINT("Inject invalid arguments");
		return -1;
	}
	int nRet = -1;
	pid_t target_pid = g_injectArg.target_pid;
	char *library_path = g_injectArg.library_path;
	char *function_name = g_injectArg.function_name;
	char *param = g_injectArg.param;
	if(IsInjected(target_pid,library_path) == 1){
		nRet = 0;
	}else{
		nRet = inject_remote_process(target_pid, library_path, function_name, param, strlen(param));
	}
	return nRet;
}
//
//__attribute__ ((visibility ("default")))
//jboolean Java_com_gamehelper_tool_JniTool_IsInjected( JNIEnv* env,jobject obj,jint jPid,jstring jLibPath){
//	unsigned char bRet = 0;
//
//	int dwPid = jPid;
//	const char *pLibPath = (*env)->GetStringUTFChars(env,jLibPath,0);
//
//	if(IsInjected(dwPid,pLibPath) == 1){
//		bRet = 1;
//	}
//	return bRet;
//}
//__attribute__ ((visibility ("default")))
//jint Java_com_gamehelper_tool_JniTool_Inject( JNIEnv* env,jobject obj,jint jPid,jstring jLibPath,jstring jEntry,jstring jParam){
//
//	int bRet = 0;
//	int dwPid = jPid;
//	const char *pLibPath = (*env)->GetStringUTFChars(env,jLibPath,0);
//	const char *pEntry = (*env)->GetStringUTFChars(env,jEntry,0);
//	const char *pParam = (*env)->GetStringUTFChars(env,jParam,0);
//
//	if(IsInjected(dwPid,pLibPath) == 0)
//		bRet = inject_remote_process(dwPid,pLibPath,pEntry,pParam,strlen(pParam));
//
//	return bRet;
//
//}