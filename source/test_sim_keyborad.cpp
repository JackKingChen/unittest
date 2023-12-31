#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include "unittest.h"

#define DEBUG_MT 0
/* horizontal offset between each fingers and the cursor */
#define PINCH_TO_ZOOM_FINGERS_OFFSET 200
#define ROTATION_FINGERS_OFFSET      50
#define PINCH_FACTOR                 0.5

/* If you modify this, make sure to report modifications in player event_manager.hpp */
#define MULTITOUCH_MODE_ZOOM         1
#define MULTITOUCH_MODE_ROTATION     2

// Android MotionEvent action type
#define ACTION_DOWN         0
#define ACTION_UP           1
#define ACTION_MOVE         2
#define ACTION_POINTER_DOWN 5
#define ACTION_POINTER_UP   6

#define BUFSIZE         256
#define MAX_NB_INPUT    22
#define NDEBUG 1
static void input_mt_sync(int uinp_fd, struct input_event* event)
{
    event->type = EV_SYN;
    event->code = SYN_MT_REPORT;
    event->value = 0;
    write(uinp_fd, event, sizeof(*event));
}

static void input_sync(int uinp_fd, struct input_event* event)
{
    event->type = EV_SYN;
    event->code = SYN_REPORT;
    event->value = 0;
    write(uinp_fd, event, sizeof(*event));
}

static void abs_mt_position_x(int uinp_fd, struct input_event* event, int value)
{
    event->type = EV_ABS;
    event->code = ABS_MT_POSITION_X;
    event->value = value;
    write(uinp_fd, event, sizeof(*event));
}

static void abs_mt_position_y(int uinp_fd, struct input_event* event, int value)
{
    event->type = EV_ABS;
    event->code = ABS_MT_POSITION_Y;
    event->value = value;
    write(uinp_fd, event, sizeof(*event));
}

static void btn_touch(int uinp_fd, struct input_event* event, int value)
{
    event->type = EV_KEY;
    event->code = BTN_TOUCH;
    event->value = value;
    write(uinp_fd, event, sizeof(*event));
}

static void abs_mt_pressure(int uinp_fd, struct input_event* event, int value)
{
    event->type = EV_ABS;
    event->code = ABS_MT_PRESSURE;
    event->value = value;
    write(uinp_fd, event, sizeof(*event));
}

static void wheel_event(int uinp_fd, struct input_event* event, int code, int value)
{
    event->type = EV_REL;
    event->code = code;
    event->value = value;
    write(uinp_fd, event, sizeof(*event));
}
//getevent -i
//cat /proc/bus/input/devices
int virtual_Keyboard_input_device_init()
{
    int uinp_fd_kb = -1;
    struct uinput_user_dev uinp_kb;
    int retval = 0;

    if (!(uinp_fd_kb = open("/dev/uinput", O_WRONLY | O_NDELAY))) {
        fprintf(stderr, "Unable to open /dev/uinput !\n");
        return -1;
    }
    memset(&uinp_kb, 0, sizeof(uinp_kb));
    strcpy(uinp_kb.name, "zixun-keyinputDev");

    uinp_kb.id.bustype = BUS_VIRTUAL;
    uinp_kb.id.vendor  = 1;
    uinp_kb.id.product = 1;
    uinp_kb.id.version = 4;
    ioctl(uinp_fd_kb, UI_SET_EVBIT, EV_KEY);
    //ioctl(uinp_fd_kb, UI_SET_EVBIT, EV_ABS);
    for (int i = 0; i < 256; i++) {
        ioctl(uinp_fd_kb, UI_SET_KEYBIT, i);
    }
    //ioctl(uinp_fd_kb, UI_SET_EVBIT, EV_REL);
    write(uinp_fd_kb, &uinp_kb, sizeof(uinp_kb));
    if (ioctl(uinp_fd_kb, UI_DEV_CREATE)) {
        fprintf(stderr, "Unable to create uinput device...\n");
        return -1;
    }
    return uinp_fd_kb;
}

int uinp_fd_kb = -1;

UNITTEST(get_uinput_name)
{
    int lfd;
    char name[256] = { 0 };
    char input_dev_name[128] = { 0 };

    int i = 273;
    sprintf(input_dev_name, "/dev/input/event%d", i);
    lfd = open(input_dev_name, O_RDONLY);
    if (lfd < 0)
        printf("open failed:errno=%d",errno);
    if (ioctl(lfd, EVIOCGNAME(sizeof(name)), name) < 0) {
        printf("ioctl failed:errno=%d", errno);
        close(lfd);
    }
    printf("name: %s\n", name);
}

UNITTEST(uinput_init)
{
    char input_dev_name[128] = { 0 };

    uinp_fd_kb = virtual_Keyboard_input_device_init();//open("/dev/uinput", O_WRONLY);
    if (uinp_fd_kb <= 0) {
        printf("open uinput failed\n");
    }
}

UNITTEST(uinput_key_down)
{
    struct input_event event;

    if (uinp_fd_kb <= 0) {
        printf("open uinput failed\n");
        return;
    }

    if (argc < 1) {
        printf("usage: uinput_key_down keycode");
        return;
    }

    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);

    event.type = EV_KEY;
    event.code = atoi(argv[0]);
    event.value = 1;
    printf("#code=%d,errno=%d\n", event.code, errno);
    int retval = write(uinp_fd_kb, &event, sizeof(event));
    printf("retval=%d,code=%d,errno=%d\n", retval,event.code, errno);
    input_sync(uinp_fd_kb, &event);
}

UNITTEST(uinput_key_up)
{
    struct input_event event;

    if (uinp_fd_kb <= 0) {
        printf("open uinput failed\n");
    }

    if (argc < 1) {
        printf("usage: uinput_key_down keycode");
        return;
    }

    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);

    event.type = EV_KEY;
    event.code = atoi(argv[0]);
    event.value = 0;
    printf("#code=%d,errno=%d\n", event.code, errno);
    int retval = write(uinp_fd_kb, &event, sizeof(event));
    printf("retval=%d,code=%d,errno=%d\n", retval, event.code, errno);
    input_sync(uinp_fd_kb, &event);
}

UNITTEST(kt)
{
    for (int i = 1; i < 256; i++) {
        struct input_event event;
        event.type = EV_KEY;
        event.code = i;
        event.value = 1;
        printf("#code=%d,errno=%d\n", event.code, errno);
        int retval = write(uinp_fd_kb, &event, sizeof(event));
        printf("retval=%d,code=%d,errno=%d\n", retval, event.code, errno);
        input_sync(uinp_fd_kb, &event);
        usleep(1000);
        event.value = 0;

        struct input_event event1;
        event1.type = EV_KEY;
        event1.code = i;
        event1.value = 0;

        printf("#code=%d,errno=%d\n", event1.code, errno);
        retval = write(uinp_fd_kb, &event1, sizeof(event1));
        printf("retval=%d,code=%d,errno=%d\n", retval, event1.code, errno);
        input_sync(uinp_fd_kb, &event1);
    }
}
