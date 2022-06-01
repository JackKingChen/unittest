
#ifndef _TT_ERROR_CODE_H_H_
#define _TT_ERROR_CODE_H_H_

/*
*   |<- 31 ->|<- 30~27 ->|<- 26~16 ->|<- 15~0 ->|
*      恒为1    模拟器类型     错误类型     错误码
*/

#define TT_EMULATOR_TYPE                         (8)

/*
*   Error Type
*/
#define TET_EMULATOR_INSTALL                     (1)        /* 模拟器安装 */
#define TET_EMULATOR_UNINTALL                    (2)        /* 模拟器卸载 */
#define TET_EMULATOR_START                       (3)        /* 模拟器启动 */
#define TET_EMULATOR_STOP                        (4)        /* 模拟器关闭 */

#define TET_MULTI_EMULATOR                       (5)        /* 模拟器多开 */

#define TET_EMULATOR_CFG                         (6)        /* 模拟器配置 */

#define TET_APK_INSTALL                          (7)        /* 应用安装 */
#define TET_APK_UNINSTALL                        (8)        /* 应用卸载 */
#define TET_APK_START                            (9)        /* 应用启动 */
#define TET_APK_STOP                             (10)        /*应用关闭 */

#define TET_EMULATOR_DATA                        (11)       /* 数据转移 */

#define TET_HOST_ENV                             (12)        /* 主机环境问题 */

#define TET_SCRIPT                               (13)        /* 脚本文件 */

#define TET_EXTERN                               (14)        /* 外部控制错误 */

#define TET_OTHER                                (0x7FF)


#define TT_ERROR(type,id)                        ((1<<31)|(TT_EMULATOR_TYPE<<27)|(type<<16)|id)

#define TT_ERROR_TYPE(nErrorID)                  (nErrorID & 0x0EFF0000)
#define TT_ERROR_CODE(nErrorID)                  (nErrorID & 0x0000FFFF)


/************************************************************************/
/*                                                                      */
/************************************************************************/

#define TEC_UNDEFINED                            (0xFFFF)

/*file error (0x0001~0x0010)*/
#define TEC_FILE_NO_EXIST                        (0x0001)
#define TEC_FILE_BROKEN                          (0x0002)

/*disk error (0x0011~0x0020)*/
#define TEC_DISK_FULL                            (0x0011)

#define TEC_UNZIP_FAILED                         (0x0020)

/*host environment error(0x0100~0x00200)*/
#define TEC_HOST_COLORDEPTH                      (0x0100)

/*script error (0x0201~0x0300)*/
#define TEC_SCRIPT_NOT_EXIST                     (0x0201)  /*脚本文件不存在*/
#define TEC_SCRIPT_BROKEN                        (0x0202)  /*脚本文件损坏*/
#define TEC_SCRIPT_NOT_MATCH                     (0x0203)  /*脚本文件与当前应用不匹配*/

/*extern control error (0x0301~0x0400)*/
#define TEC_MSG_FORMAT_WRONG                     (0x0301)

/*emulator error (0x1000~0x2000)*/

#define TEC_EMU_NOT_EXIST                        (0x1000)

/************************************************************************/
/*                                                                      */
/************************************************************************/
/*
* install/uninstall errorcode
*/
/*
*
0 Setup was successfully run to completion or the /HELP or /? command line parameter was used.

1 Setup failed to initialize.

2 The user clicked Cancel in the wizard before the actual installation started, or chose "No" on the opening "This will install..." message box.

3 A fatal error occurred while preparing to move to the next installation phase (for example, from displaying the pre-installation wizard pages to the actual installation process). This should never happen except under the most unusual of circumstances, such as running out of memory or Windows resources.

4 A fatal error occurred during the actual installation process.

Note: Errors that cause an Abort-Retry-Ignore box to be displayed are not fatal errors. If the user chooses Abort at such a message box, exit code 5 will be returned.

5 The user clicked Cancel during the actual installation process, or chose Abort at an Abort-Retry-Ignore box.

6 The Setup process was forcefully terminated by the debugger (Run | Terminate was used in the IDE).

7 The Preparing to Install stage determined that Setup cannot proceed with installation. (First introduced in Inno Setup 5.4.1.)

8 The Preparing to Install stage determined that Setup cannot proceed with installation, and that the system needs to be restarted in order to correct the problem. (First introduced in Inno Setup 5.4.1.)
*/

/*0~8 is innosetup install error code*/

/*0~11031 win32 error*/

#define TEC_HAS_INSTALLED                        (12000)   /*已经安装*/
#define TEC_HAS_UNINSTALLED                      (12000)   /*已经卸载*/

#define TSC_NEED_REBOOT                          (8)    /*需要重启*/

#define TSC_HAS_INSTALLED                        (12000)   /*已经安装*/
#define TSC_HAS_UNINSTALLED                      (12000)   /*已经卸载*/


/*
*   Error Code
*/
#define TT_ERROR_INSTALL_NEED_REBOOT             TT_ERROR(TET_EMULATOR_INSTALL,TSC_NEED_REBOOT)
#define TT_ERROR_INSTALL_HAS_DONE                TT_ERROR(TET_EMULATOR_INSTALL,TSC_HAS_INSTALLED)

#define TT_ERROR_UNINSTALL_NEED_REBOOT           TT_ERROR(TET_EMULATOR_UNINTALL,TSC_NEED_REBOOT)
#define TT_ERROR_UNINSTALL_HAS_DONE              TT_ERROR(TET_EMULATOR_INSTALL,TSC_HAS_INSTALLED)

/************************************************************************/
/*                                                                      */
/************************************************************************/
/*
*   Start Error Code
*/
#define TEC_START_VMCOM_ERROR                    (1)        /*虚拟机组件未注册成功*/
#define TEC_START_NO_ADAPT_FIND                  (2)        /*虚拟网卡未安装*/
#define TEC_START_VMCOMPAIBILITY_ERROR           (3)        /*接口错误,一般程序兼容性问,大部分设置为vista sp2兼容性OK*/

#define TEC_START_VBOX_NOT_INSTALL               (4)        /*未安装VBox*/
#define TEC_START_TTVBOX_NOT_INSTALL             (5)        /*未安装或者不是我们的VBox版本*/

#define TEC_START_SET_ADAPT_FAILED               (6)        /*设置adapt失败*/
#define TEC_START_SET_DHCP_FAILED                (7)        /*设置DHCP失败*/
#define TEC_START_VM_NOT_FIND                    (8)        /*虚拟机未找到*/
#define TEC_START_VM_LOAD_FAILED                 (9)        /*vm 注册失败*/
#define TEC_START_VM_UNLOAD_FAILED               (10)       /*vm 注销失败*/
#define TEC_START_VM_CFGFILE_BROKEN              (11)       /*vm 配置文件丢失或者损坏*/
#define TEC_START_VM_START_VM_FAILED             (12)       /*vm start 失败*/
#define TEC_START_VM_GET_VMIP_FAILED             (13)       /*vm getip failed*/
#define TEC_START_VM_VT_NOT_OPEN                 (14)       /*vt 未开*/
#define TEC_START_VM_VT_GET_FAILED               (15)       /*vt 未开*/
#define TEC_START_VM_COLORDEPTH_SET_ERROR        (16)       /*系统颜色修改失败*/
#define TEC_START_VM_CFG_PDMDEVICE_FAILED        (17)       /*PDM 设备配置失败*/
#define TEC_START_VM_GET_PDMDEVICE_ENABLE        (18)       /*获取PDM配置失败*/
#define TEC_START_VBOX_NEED_FIXED                (19)       /*vbox 注册信息错误或者文件丢失或者服务未注册，此时需要修复*/
#define TEC_START_NETPORT_ADD_FAILED             (20)       /**/
#define TEC_START_NETPORT_DEL_FAILED             (21)       /**/
#define TEC_START_DISK_FULL                      (22)       /*磁盘已满*/

#define TEC_START_OPENGL_INIT_FAILED             (64)       /*opengl 初始化失败*/
#define TEC_START_OPENGL_CONNECT_FAILED          (65)       /*opengl 连接失败*/

#define TEC_START_VMSVC_ACCESS_ERROR             (96)        /*虚拟机服务无法访问*/

#define TEC_HOST_NEED_REBOOT                     (96)
#define TEC_GUEST_NEED_REBOOT                    (97)


/*
*   TianTian Start Error Code
*/
#define TT_ERROR_START_VMCOM                     TT_ERROR(TET_EMULATOR_START,TEC_START_VMCOM_ERROR)        /*虚拟机组件未注册成功*/
#define TT_ERROR_START_NO_ADAPT_FIND             TT_ERROR(TET_EMULATOR_START,TEC_START_NO_ADAPT_FIND)        /*虚拟网卡未安装*/
#define TT_ERROR_START_VMCOMPAIBILITY            TT_ERROR(TET_EMULATOR_START,TEC_START_VMCOMPAIBILITY_ERROR)/*接口错误,一般程序兼容性问,大部分设置为vista sp2兼容性OK*/

#define TT_ERROR_START_VBOX_NOT_INSTALL          TT_ERROR(TET_EMULATOR_START,TEC_START_VBOX_NOT_INSTALL)   /*未安装VBox*/
#define TT_ERROR_START_TTVBOX_NOT_INSTALL        TT_ERROR(TET_EMULATOR_START,TEC_START_TTVBOX_NOT_INSTALL)  /*未安装或者不是我们的VBox版本*/

#define TT_ERROR_START_SET_ADAPT                 TT_ERROR(TET_EMULATOR_START,TEC_START_SET_ADAPT_FAILED)     /*设置adapt失败*/
#define TT_ERROR_START_SET_DHCP                  TT_ERROR(TET_EMULATOR_START,TEC_START_SET_DHCP_FAILED)      /*设置DHCP错误*/

#define TT_ERROR_VM_NOT_FIND                     TT_ERROR(TET_EMULATOR_START,TEC_START_VM_NOT_FIND)        /*虚拟机未找到*/

#define TT_ERROR_START_VM_LOAD                   TT_ERROR(TET_EMULATOR_START,TEC_START_VM_LOAD_FAILED)       /*vm 注册失败*/
#define TT_ERROR_START_VM_UNLOAD                 TT_ERROR(TET_EMULATOR_START,TEC_START_VM_UNLOAD_FAILED)     /*vm 注销失败*/
#define TT_ERROR_START_VM_CFGFILE_BROKEN         TT_ERROR(TET_EMULATOR_START,TEC_START_VM_CFGFILE_BROKEN)    /*vm 配置文件丢失或者损坏*/
#define TT_ERROR_START_VM_START_VM               TT_ERROR(TET_EMULATOR_START,TEC_START_VM_START_VM_FAILED)   /*vm start 失败*/
#define TT_ERROR_START_VM_GET_VMIP               TT_ERROR(TET_EMULATOR_START,TEC_START_VM_GET_VMIP_FAILED)    /*vm getip failed*/
#define TT_ERROR_START_VM_VT_NOT_OPEN            TT_ERROR(TET_EMULATOR_START,TEC_START_VM_VT_NOT_OPEN)        /*vt 未开*/
#define TT_ERROR_START_VM_VT_GET                 TT_ERROR(TET_EMULATOR_START,TEC_START_VM_VT_GET_FAILED)       /*vt 未开*/
#define TT_ERROR_START_VM_COLORDEPTH_SET         TT_ERROR(TET_EMULATOR_START,TEC_START_VM_COLORDEPTH_SET_ERROR)        /*系统颜色修改失败*/
#define TT_ERROR_START_VM_CFG_PDMDEVICE          TT_ERROR(TET_EMULATOR_START,TEC_START_VM_CFG_PDMDEVICE_FAILED)        /*PDM 设备配置失败*/
#define TT_ERROR_START_VM_GET_PDMDEVICE_ENABLE   TT_ERROR(TET_EMULATOR_START,TEC_START_VM_GET_PDMDEVICE_ENABLE)        /*获取PDM配置失败*/
#define TT_ERROR_START_VBOX_NEED_FIXED           TT_ERROR(TET_EMULATOR_START,TEC_START_VBOX_NEED_FIXED)           /*vbox 注册信息错误或者文件丢失或者服务未注册，此时需要修复*/
#define TT_ERROR_START_NETPORT_ADD               TT_ERROR(TET_EMULATOR_START,TEC_START_NETPORT_ADD_FAILED)        /*vbox 注册信息错误或者文件丢失或者服务未注册，此时需要修复*/
#define TT_ERROR_START_NETPORT_DEL               TT_ERROR(TET_EMULATOR_START,TEC_START_NETPORT_DEL_FAILED)        /*vbox 注册信息错误或者文件丢失或者服务未注册，此时需要修复*/
#define TT_ERROR_START_DISK_FULL                 TT_ERROR(TET_EMULATOR_START,TEC_START_DISK_FULL)                /*磁盘已满*/

#define TT_ERROR_START_OPENGL_INIT               TT_ERROR(TET_EMULATOR_START,TEC_START_OPENGL_INIT_FAILED)           /*opengl 初始化失败*/
#define TT_ERROR_START_OPENGL_CONNECT            TT_ERROR(TET_EMULATOR_START,TEC_START_OPENGL_CONNECT_FAILED)        /*opengl 连接失败*/


#define TT_ERROR_HOST_NEED_REBOOT                TT_ERROR(TET_EMULATOR_START,TEC_START_OPENGL_INIT_FAILED)           /*opengl 初始化失败*/
#define TT_ERROR_GUEST_NEED_REBOOT               TT_ERROR(TET_EMULATOR_START,TEC_START_OPENGL_CONNECT_FAILED)        /*opengl 连接失败*/


/************************************************************************/
/*                                                                      */
/************************************************************************/


#endif /*_TT_ERROR_CODE_H_H_*/
