/*/*******************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2013-04-07
*
*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <termios.h> 
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#include "host.h"

namespace host
{
    /************************************************************************/
    /* class of OSTTY                                                       */
    /************************************************************************/
    /* terminal standout start/normal ESC sequence */
#define  VT100_SOs       "\033[7m"
#define  VT100_SOn       "\033[0m"
    /* Clear-end-of-line and Clear-end-of-screen ESC sequence */
#define  VT100_Ceol      "\033[0K"
#define  VT100_Ceos      "\033[0J"
    /* Cursor motion arbitrary destination ESC sequence */
#define  VT100_CMrc      "\033[%d;%dH"
    /* Cursor motion up and down ESC sequence */
#define  VT100_CMup      "\033[A"
#define  VT100_CMdown    "\n"
    /* Cursor motion left and right ESC sequence */
#define  VT100_CMleft    "\033[%dD"
#define  VT100_CMright   "\033[%dC"

#define  VT100_COLOR_WHITE  "\033[0m"
#define  VT100_COLOR_RED    "\033[1;31m"
#define  VT100_COLOR_GREEN  "\033[1;32m"
#define  VT100_COLOR_YELLOW "\033[1;33m"

    static  void tty_config(struct termios *oldIO)
    {
        struct termios   newIO;

        /*
        * setup signal
        */
        signal(SIGINT,  SIG_IGN);
        signal(SIGTERM, SIG_IGN);
        signal(SIGKILL, SIG_IGN);
        signal(SIGABRT, SIG_IGN);

        /*
        * setup io
        */
        tcgetattr(0, oldIO);

        newIO = *oldIO;
        newIO.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
        newIO.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);

        //newIO.c_cflag &= ~(CSIZE|PARENB);
        //newIO.c_cflag |= CS8;

        newIO.c_cc[VMIN]  = 0;
        newIO.c_cc[VTIME] = 0;

        tcsetattr(0, TCSANOW, &newIO);
    }

    static  void tty_restore(struct termios *oldIO)
    {
        /*
        * restore tty
        */
        tcsetattr(0, TCSANOW, oldIO);

        /*
        * restore signal
        */
        signal(SIGINT,  SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGKILL, SIG_DFL);
        signal(SIGABRT, SIG_DFL);
    }

    static int tty_poll(int fd,int timeout)
    {
        struct pollfd pfd[1];

        pfd[0].fd     = fd;
        pfd[0].events = POLLIN;

        while (1) 
        {
            int n = poll(pfd, 1, timeout);
            if (n >= 0)
                return n;
            /* E.g. strace causes poll to return this */
            if (errno == EINTR)
                continue;
            /* Kernel is very low on memory. Retry. */
            /* I doubt many callers would handle this correctly! */
            if (errno == ENOMEM)
                continue;
            return 0;
        }
        return 0;
    }

    static int tty_convert(char* buff,int len)
    {
        /*
        * type
        */
        struct esc_cmds 
        {
            const char seq[8];
            OSTTY::Key  val;
        };

        static const struct esc_cmds esccmds[] = 
        {
            {"OA"  , OSTTY::TTY_VK_UP      },   // cursor key Up
            {"OB"  , OSTTY::TTY_VK_DOWN    },   // cursor key Down
            {"OC"  , OSTTY::TTY_VK_RIGHT   },   // Cursor Key Right
            {"OD"  , OSTTY::TTY_VK_LEFT    },   // cursor key Left
            {"OH"  , OSTTY::TTY_VK_HOME    },   // Cursor Key Home
            {"OF"  , OSTTY::TTY_VK_END     },   // Cursor Key End
            {"[A"  , OSTTY::TTY_VK_UP      },   // cursor key Up
            {"[B"  , OSTTY::TTY_VK_DOWN    },   // cursor key Down
            {"[C"  , OSTTY::TTY_VK_RIGHT   },   // Cursor Key Right
            {"[D"  , OSTTY::TTY_VK_LEFT    },   // cursor key Left
            {"[H"  , OSTTY::TTY_VK_HOME    },   // Cursor Key Home
            {"[F"  , OSTTY::TTY_VK_END     },   // Cursor Key End
            {"[1~" , OSTTY::TTY_VK_HOME    },   // Cursor Key Home
            {"[2~" , OSTTY::TTY_VK_INSERT  },   // Cursor Key Insert
            {"[3~" , OSTTY::TTY_VK_DELETE  },   // Cursor Key Delete
            {"[4~" , OSTTY::TTY_VK_END     },   // Cursor Key End
            {"[5~" , OSTTY::TTY_VK_PAGEUP  },   // Cursor Key Page Up
            {"[6~" , OSTTY::TTY_VK_PAGEDOWN},   // Cursor Key Page Down
            {"OP"  , OSTTY::TTY_VK_FUN1    },   // Function Key F1
            {"OQ"  , OSTTY::TTY_VK_FUN2    },   // Function Key F2
            {"OR"  , OSTTY::TTY_VK_FUN3    },   // Function Key F3
            {"OS"  , OSTTY::TTY_VK_FUN4    },   // Function Key F4
            // careful: these have no terminating NUL!
            {"[11~", OSTTY::TTY_VK_FUN1    },   // Function Key F1
            {"[12~", OSTTY::TTY_VK_FUN2    },   // Function Key F2
            {"[13~", OSTTY::TTY_VK_FUN3    },   // Function Key F3
            {"[14~", OSTTY::TTY_VK_FUN4    },   // Function Key F4
            {"[15~", OSTTY::TTY_VK_FUN5    },   // Function Key F5
            {"[17~", OSTTY::TTY_VK_FUN6    },   // Function Key F6
            {"[18~", OSTTY::TTY_VK_FUN7    },   // Function Key F7
            {"[19~", OSTTY::TTY_VK_FUN8    },   // Function Key F8
            {"[20~", OSTTY::TTY_VK_FUN9    },   // Function Key F9
            {"[21~", OSTTY::TTY_VK_FUN10   },   // Function Key F10
            {"[23~", OSTTY::TTY_VK_FUN11   },   // Function Key F11
            {"[24~", OSTTY::TTY_VK_FUN12   },   // Function Key F12
        };

        const struct esc_cmds *eindex;

        /*
        * convert esc key
        */
        if(buff[0] == 27 && len>0)
        {
            for (eindex = esccmds; eindex < &esccmds[sizeof(esccmds)/sizeof(esccmds[0])]; eindex++) 
            {
                int n = strlen(eindex->seq);
                if (strncmp(eindex->seq, buff + 1, n) != 0)
                    continue;
                else
                {
                    buff[0]   = eindex->val;
                    buff[len] = 0;
                    memmove(buff+1,buff+1+n,len-n);

                    return len-n;
                }
            }
        }

        return len;
    }

    int OSTTY::read(unsigned char *buff,int len)
    {
        struct termios oldIO;
        int retval;
        int total;
        int fd;

        fd = fileno(stdin);

        /* setup tty */
        tty_config(&oldIO);

        /*wait input*/
        total  = 0;
        retval = tty_poll(fd,1000);
        if(retval>0)
        {
            /*read*/
            retval = ::read(fd, buff,len);
            if(retval>0)
            {
                /* This is an ESC char. Is this Esc sequence?
                * Could be bare Esc key. See if there are any
                *  more chars to read after the ESC. This would
                * be a Function or Cursor Key sequence.
                */
                total = retval;
                if(buff[0] == 27)
                {
                    while (tty_poll(fd,10) > 0 && total <= (len - 8))
                    {
                        /* read the rest of the ESC string*/
                        retval = ::read(fd, buff + total, len - total);
                        if (retval > 0)
                            total += retval;
                    }
                }
                retval = total;
            }
        }

        /* restore tty */
        tty_restore(&oldIO);

        retval = tty_convert((char*)buff,retval);

        return retval;
    }


    int OSTTY::draw(const char * prompt,int prompt_len,const char * line,int pos)
    {
        char cmd[8];

        fputs("\r",  stdout);
        fputs(prompt,stdout);
        fputs(VT100_Ceol,  stdout);
        fputs(line,  stdout);

        sprintf(cmd,VT100_CMright,prompt_len+pos);
        fputs("\r",stdout);
        fputs(cmd, stdout);

        fflush(stdout);
        return 0;
    }

    bool OSTTY::color(Color color)
    {
        switch(color)
        {
        case RED   :std::cout<<VT100_COLOR_RED;   break;
        case GREEN :std::cout<<VT100_COLOR_GREEN; break;
        case YELLOW:std::cout<<VT100_COLOR_YELLOW;break;
        case WHITE :
        default:
            std::cout<<VT100_COLOR_WHITE;break;
        }

        return true;
    }

    /************************************************************************/
    /* class of OSCon                                                       */
    /************************************************************************/
    int OSCon::complete_path(char *word)
    {
        DIR           *dir;
        struct dirent *dirent;
        char           path[CONSOLE_MAX_PATH];
        int            rootnr;
        char           root[CONSOLE_MAX_PATH];
        int            keynr;
        char           key[CONSOLE_MAX_PATH];

        /*
        * try complete path/file
        */
        root[0] = '\0';
        if(input_line[input_pos] != ' ' || input_pos==input_len)
        {
            int i=input_pos;
            while(i>0 && input_line[i-1] != ' ')
            {i--;}

            rootnr = input_pos-i;
            memcpy(root,input_line + i,rootnr);
            root[rootnr] = '\0';
        }
        /*set as current dir*/
        if(root[0] == '\0')
        {
            strcpy(root,"./");
            rootnr = 2;
        }

        /*split path and key*/
        while(rootnr>0 && root[rootnr-1] != '/')
        {rootnr--;}

        strcpy(key,root+rootnr);
        keynr = strlen(key);
        root[rootnr] = '\0';

        /*list path/file in root*/
        if(rootnr>0)
        {
            int  pathmax;
            int  pathlen;
            char str[CONSOLE_MAX_FILE][CONSOLE_MAX_PATH];
            int  strnr;

            /*open dir*/
            dir = opendir(root);
            if (!dir)
                return 0;

            /*list all*/
            pathmax  = 0;
            pathlen  = 0;
            strnr    = 0;
            while ((dirent = readdir(dir)) != NULL && strnr<CONSOLE_MAX_FILE)
            {
                /*skip .*/
                if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0)
                    continue;

                /*check the same*/
                if(strcmp(key,dirent->d_name)==0)
                {
                    /*found and end search*/
                    if(input_line[input_pos-1]!=' ')
                        insert_char(' ');
                    /*close and return*/
                    closedir(dir);
                    return 0;
                }
                /*check the similar*/
                if(strncmp(key,dirent->d_name,keynr)==0)
                {
                    /*save similar*/
                    strncpy(str[strnr],dirent->d_name,CONSOLE_MAX_PATH);
                    if(dirent->d_type ==  DT_DIR)
                        strcat(str[strnr],"/");
                    else
                        strcat(str[strnr]," ");
                    strnr++;

                    /*save max len of file name*/
                    pathlen = strlen(dirent->d_name);
                    pathmax = std::max(pathmax,pathlen);
                }
            }

            /*close dir*/
            closedir(dir);

            /*is this only one similar?*/
            if(strnr>0)
            {
                /*delete key*/
                delete_word(keynr);

                /*find the most similar cmd*/
                str_same(str,strnr,word);

                /*insert the left*/
                insert_word(word,-1);

                if(strnr==1)
                    return 0;
            }

            if(pathmax>0)
            {
                int  col;
                char fmt[32];
                int  i;

                /*setup format*/
                pathmax = rootnr+pathmax+2;
                sprintf(fmt,"%%-%ds",pathmax);

                /*setup column*/
                col = CONSOLE_MAX_WW/pathmax;
                if(col<=0)
                    col = 1;

                /*show similar if no match found*/
                printf("\n");
                for (i=0;i<strnr;i++)
                {
                    if(i!=0 && i%col==0)
                        printf("\n");

                    /*check the similar*/
                    if(key[0]=='\0'  || strncmp(key,str[i],keynr)==0)
                    {
                        snprintf(path,CONSOLE_MAX_PATH,"%s%s",root,str[i]);
                        printf(fmt,path);
                    }
                }

                if(i>0)
                    printf("\n");
            }
        }
        return 0;
    }


    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};
