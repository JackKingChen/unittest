/*******************************************************************
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

#include "host.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace host
{
    /************************************************************************/
    /* class of  OSTTY                                                      */
    /************************************************************************/
    /*
    * static functions
    */
    void OSTTY::print(Color color,const char*fmt,...)
    {
        va_list va;
        va_start(va,fmt);

        OSTTY::color(color);
        vfprintf(stdout,fmt,va);
        OSTTY::color(OSTTY::WHITE);

        va_end(va);
    }

    int  OSTTY::getkey (void)
    {
        while(1)
        {
            unsigned char buff[8];
            if(OSTTY::read(buff,sizeof(buff))>0)
                return static_cast<int>(buff[0]);
        }
        return 0;
    }
    bool OSTTY::hungup(void)
    {
        int key;

        while(getkey(key))
            ;
        return true;
    }
    bool OSTTY::getkey(int &key)
    {
        key = OSTTY::getkey();

        return (key!=27/*esc*/);
    }
    bool OSTTY::getnumber(int &key)
    {
        key = OSTTY::getkey();
        
        if(key>='0' && key<='9')
        {
            key-='0';
            return true;
        }
        else
            return false;
    }

    int  OSTTY::getline(char *line,int len,const char*prompt)
    {
        OSTTY tty(prompt);

        while(tty.getline()>0)
        {
            strncpy(line,tty.input_line,len);
            return tty.input_len;
        }

        return 0;
    }

    /*
    * class functions
    */
    OSTTY::OSTTY(const char*prompt)
    {
        /*
        * setup prompt
        */
        if(!prompt)
            prompt = "$:";

        strcpy(cmd_prompt,prompt);
        cmd_prompt_nr = strlen(cmd_prompt);

        memset(input_line,0,sizeof(input_line));
        input_pos = 0;
        input_len = 0;
        input_eof = 0;

        memset(history,0,sizeof(history));
        history_nr = 0;
        history_wr = 0;
        history_rd = 0;
    }

    OSTTY::~OSTTY()
    {

    }

    int OSTTY::insert_char(const char chr)
    {
        if(input_pos < input_len)
        {
            memmove(input_line + input_pos + 1,
                input_line + input_pos,
                input_len  - input_pos + 1);

            input_line[input_pos] = chr;

            input_pos++;
            input_len++;
        }
        else
        {
            input_line[input_pos+0] = chr;
            input_line[input_pos+1] = '\0';
            input_pos++;
            input_len++;
        }

        return input_len;
    }

    int OSTTY::insert_word(const char *word,int len)
    {
        int i;

        for (i=0;(i<len || len<0) && word[i]!='\0';i++)
        {
            insert_char(word[i]);
        }

        return input_len;
    };

    int OSTTY::delete_char()
    {
        if(input_pos>0)
        {
            memmove(input_line + input_pos - 1 ,
                input_line + input_pos,
                input_len  - input_pos + 1);

            input_pos--;
            input_len--;
        }
        return input_len;
    }

    int OSTTY::delete_word(int nr)
    {
        while(nr-->0)
        {
            delete_char();
        }
        return input_len;
    }

    int OSTTY::input(char chr)
    {
        unsigned char key = (unsigned char)chr;

        if(input_len + 1 > (int)sizeof(input_line)-1)
        {
            /*end input*/
            input_line[input_len] = '\0';
            return 1;
        }

        switch ((OSTTY::Key)key)
        {
        case OSTTY::TTY_VK_RETURN:
        case OSTTY::TTY_VK_NEWLINE:
            {
                /*end input*/
                input_line[input_len] = '\0';
                return 1;
            }

        case OSTTY::TTY_VK_NUL:
            {
                return 0;
            }

        case OSTTY::TTY_VK_TAB:
            {
                /*do auto complete*/
                complete();

                return 0;
            }

        case OSTTY::TTY_VK_HOME:
            {
                input_pos = 0;
                return 0;
            }

        case OSTTY::TTY_VK_END:
            {
                input_pos = input_len;
                return 0;
            }

        case OSTTY::TTY_VK_DELETE:
        case OSTTY::TTY_VK_BACKSPACE:
        case OSTTY::TTY_VK_BACKSPACE_MAC:
            {
                delete_char();
                return 0;
            }
        case OSTTY::TTY_VK_LEFT:
            {
                if(input_pos>0)
                {
                    input_pos--;
                }
                return 0;
            }
        case OSTTY::TTY_VK_RIGHT:
            {
                if(input_pos < input_len)
                {
                    input_pos++;
                }
                return 0;
            }
        case OSTTY::TTY_VK_UP:
        case OSTTY::TTY_VK_DOWN:
            {
                if(history_nr>0)
                {
                    strcpy(input_line,history[history_rd]);
                    input_eof = 0;
                    input_len = strlen(input_line);
                    input_pos = input_len;

                    if(key==OSTTY::TTY_VK_UP)
                    {
                        if(history_rd>0)
                            history_rd--;
                    }
                    else
                    {
                        if(history_rd<history_nr-1)
                            history_rd++;
                    }
                }
                return 0;
            }
        default:
            {
                /*control key is greater than 0!!*/
                if(isprint(chr))
                {
                    insert_char(chr);
                }
                return 0;
            }
        }

        return 0;
    }

    int OSTTY::getline(void)
    {
        char   buff[128];
        int    retval;
        int    i;

do_getline:
        /*
        * show showhelp
        */
        printf("\n%s",cmd_prompt);
        fflush(stdout);

        /*cut off*/
        input_line[0] = 0;
        input_pos     = 0;
        input_len     = 0;
        input_eof     = 0;

        /*loop*/
        while(1)
        {
            /* read tty */
            retval = read((unsigned char*)buff,sizeof(buff)-1);

            if(retval>0)
            {
                /*
                * ctrl+c,then exit,
                * but we just exit while nothing has been input!
                */
                if(retval==1)
                {
                    if(buff[0]==OSTTY::TTY_VK_INTC)
                    {
                        if(input_len > 0)
                            goto do_getline;
                        else
                            return -1;
                    }
                }

                /*input*/
                for (i=0;i<retval && !input_eof;i++)
                {
                    input_eof = input(buff[i]);
                }

                /*eof*/
                if(input_eof)
                    return input_len;

                /*redraw this line*/
                draw(cmd_prompt,cmd_prompt_nr,input_line,input_pos);
            }
        }

        return input_len;
    }

    int OSTTY::complete(void)
    {
        return 0;
    }
    int OSTTY::memorize(const char* cmd)
    {
        if(history_nr<MAX_HIS)
            history_nr++;
        history_wr = (history_wr+1)%MAX_HIS;
        history_rd  = history_wr;

        strcpy(history[history_wr],cmd);
        return 0;
    }

    /************************************************************************/
    /* class of  OSTime                                                     */
    /************************************************************************/

    /************************************************************************/
    /* class of  OSCon                                                */
    /************************************************************************/
    int  OSCon::str_same(char str[][OSCon::CONSOLE_MAX_PATH],int nr,char *same)
    {
        int len = 0;

        while(nr>0)
        {
            int  idx = 0;
            char chr = str[0][len];

            for (idx=0;idx<nr && str[idx][len]!='\0';idx++)
            {
                if(str[idx][len] != chr)
                    break;
            }

            if(idx<nr)
                break;

            same[len++] = chr;
        }

        same[len] = '\0';
        return len;
    }

    void OSCon::str_parse(char* p_line,int n_argc,int* p_argc,const char*  argv[])
    {
        int argc;

        /* for each argument */
        for (argc = 0; argc < (n_argc - 1); argc++)
        {
            /* Skip leading white space */
            while (isspace(*p_line))
            {
                p_line++;
            }

            /* Stop if comment or end of line. */
            if (*p_line == '\0' || *p_line == '#')
            {
                break;
            }

            /* record the start of the argument */
            argv[argc] = p_line;

            /* find the end of the argument */
            while (*p_line != '\0' && !isspace(*p_line) && *p_line != '#')
            {
                p_line++;
            }

            /* null terminate argument */
            if (*p_line == '#')
            {
                *p_line = '\0';
            }
            else if (*p_line != '\0')
            {
                *p_line = '\0';
                p_line++;
            }
        }

        /* null terminate list of arguments */
        argv[argc] = NULL;
        *p_argc = argc;
    }

    int OSCon::complete(void)
    {
        char word[CONSOLE_MAX_PATH];
        int  word_nr;
        int  i;

        /*
        * try complete OSCmd
        */
        /*get first word*/
        for(word_nr=0;word_nr<input_pos;word_nr++)
        {
            word[word_nr] = input_line[word_nr];
            if(word[word_nr]==' ')
                break;
        }
        word[word_nr] = '\0';

        if(word_nr==input_pos)
        {
            /*match in OSCmd list*/
            if(word[0])
            {
                char str[CONSOLE_MAX_CMD][CONSOLE_MAX_PATH];
                int  strnr;

                /*is any one match??*/
                strnr  = 0;
                for(i=0;i<(int)command_list->size();i++)
                {
                    OSCmd *cmd = command_list->at(i);
                    /*the same*/
                    if(strcmp(word,cmd->cmd_name())==0)
                    {
                        if(input_line[input_pos-1]!=' ')
                            insert_char(' ');
                        return 0;
                    }
                    /*the similar*/
                    if(strncmp(word,cmd->cmd_name(),word_nr)==0)
                    {
                        strcpy(str[strnr++],cmd->cmd_name());
                    }
                }
                /*is this only one similar?*/
                if(strnr>0)
                {
                    /*find the most similar cmd*/
                    str_same(str,strnr,word);

                    /*insert the left*/
                    insert_word(word+word_nr,-1);

                    /*only one match??*/
                    if(strnr == 1)
                    {
                        if(input_line[input_pos-1]!=' ')
                            insert_char(' ');
                        return 0;
                    }
                }
            }

            /*show similar*/
            command_help(NULL,word);
        }
        else
        {
            complete_path(word);
        }

        return 0;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    OSCon::OSCon(const char *prompt):OSTTY(prompt)
    {
        command_ref++;
    }
    OSCon::~OSCon()
    {
        command_ref--;
    }

    int  OSCon::run(void)
    {
        char           save[CONSOLE_MAX];
        const char*    argv[CONSOLE_MAX];
        int            argc;
        int            retval;

        if(command_list->size()<=0)
            return -EINVAL;

        history_nr  = 0;
        history_wr  = -1;
        history_rd  = 0;

        /*setup prompt*/
        while (1)
        {
            /* read the next OSCmd */
            retval = getline();
            if(retval<0)
                break;
            if(retval==0)
                continue;

            /*save history*/
            strncpy(save,input_line,CONSOLE_MAX);

            /* str_parse the OSCmd */
            str_parse(input_line, CONSOLE_MAX, &argc, argv);

            /*no args*/
            if(argc<1 || *argv[0]=='\0')
                continue;

            printf("\n");
            fflush(stdout);

            /*show showhelp*/
            if(strcmp(argv[0],"?")==0 || strcmp(argv[0],"help")==0)
            {
                /*show showhelp on argv[1]*/
                command_help(argv[1],NULL);
            }
            /*show history*/
            else if(strcmp(argv[0],">")==0 || strcmp(argv[0],"history")==0)
            {
                int i;
                for (i=0;i<history_nr;i++)
                {
                    printf("%2d %s\n",i,history[i]);
                }
            }
            /* execute it */
            else if(command_exe(argv[0],argc-1,argv+1)<0)
            {
                /*do not save failed OSCmd as history!!*/
                continue;
            }

            /*save history*/
            OSTTY::memorize(save);
        }

        printf("\n");
        fflush(stdout);

        return 0;
    }

    int  OSCon::exe(const char *cmdline,int argc,const char **argv)
    {
        if(command_list->size()<=0 || !cmdline)
            return -EINVAL;

        if(argc>0 && argv!=NULL)
        {
            return command_exe(cmdline,argc,argv);
        }
        else
        {
            char           exe_line[CONSOLE_MAX];
            const char*    exe_argv[CONSOLE_MAX];
            int            exe_argc;

            /* str_parse the OSCmd */
            strncpy(exe_line,cmdline,sizeof(exe_line)-1);
            str_parse(exe_line, CONSOLE_MAX, &exe_argc, exe_argv);

            /*no args*/
            if(exe_argc<1 || *exe_argv[0]=='\0')
                return -EINVAL;

            /*execute and return*/
            return command_exe(exe_argv[0],exe_argc-1,exe_argv+1);
        }
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    std::vector<OSCmd*> *OSCon::command_list(NULL);
    unsigned long              OSCon::command_ref(0);

    void OSCon::command_add(OSCmd *cmd)
    {
        if(!command_list)
            command_list = new std::vector<OSCmd*>();

        command_list->push_back(cmd);
    }

    void OSCon::command_del(OSCmd *cmd)
    {
        if(command_list)
        {
            std::vector<OSCmd*>::iterator p = command_list->begin();
            while(p!=command_list->end())
            {
                if(*p == cmd)
                {
                    command_list->erase(p);
                    break;
                }
                p++;
            }

            /*cleanup if no body exist*/
            if(command_list->size()==0 && command_ref==0)
            {
                delete command_list;
                command_list = NULL;
            }
        }
    }

    int OSCon::command_help(const char *cmdname,const char *cmdtips)
    {
        int len;
        int i;
        int k;
        int n;

        if(cmdname)
        {
            /*find cmd named 'cmdname'*/
            for(i=0;i<(int)command_list->size();i++)
            {
                OSCmd *cmd = command_list->at(i);
                if(strcmp(cmd->cmd_name(),cmdname)==0)
                {
                    printf("help of %s:\n\n%s",cmdname,cmd->cmd_help());
                    return 0;
                }
            }
            /*not found*/
            printf("%s:not found!\n",cmdname);
        }
        else
        {
            /*show all OSCmd*/
            if(cmdtips)
                len = strlen(cmdtips);
            else
                len = 0;

            printf("\n");

            k = 0;
            i = 0;
            for(;i<(int)command_list->size();i++)
            {
                OSCmd *cmd = command_list->at(i);

                if(cmdtips==NULL || strncmp(cmd->cmd_name(),cmdtips,len)==0)
                {
                    if(k!=0 && k%4==0)
                        printf("\n");

                    n = strlen(cmd->cmd_name());

                    /*next*/
                    if(n<32)
                    {
                        printf("%-32s",cmd->cmd_name());
                        k+=1;
                    }
                    else
                    {
                        printf("%s",cmd->cmd_name());
                        k =4;
                    }
                }
            }
            if(k>0)
                printf("\n");
        }
        return -1;
    }

    int OSCon::command_exe(const char *cmdname,int argc,const char **argv)
    {
        int i;

        /*find and run OSCmd*/
        for (i=0;i<(int)command_list->size();i++)
        {
            OSCmd *cmd = command_list->at(i);

            if(strcmp(cmd->cmd_name(),cmdname)==0)
            {
                /*check args number*/
                if(argc<cmd->cmd_argc())
                {
                    printf("%d args are required\n",cmd->cmd_argc());
                    printf("showhelp of %s:\n\n%s",cmdname,cmd->cmd_help());
                    return 0;
                }

                /*do run*/
                return cmd->cmd_action(argc,argv);
            }
        }

        /*not found*/
        printf("%s: not found\n",argv[0]);

        return -1;
    }

    int OSCon::command_run(OSCmd *excutor,bool runHide)
    {
        if(command_list)
        {
            std::vector<OSCmd*>::iterator p = command_list->begin();
            while(p!=command_list->end())
            {
                OSCmd * cmd = *p;

                if(cmd!=excutor && (runHide || !cmd->cmd_hide()))
                {
                    cmd->cmd_action(0,0);
                }
                p++;
            }
        }

        return -1;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/

    OSCmd::OSCmd()
    {
        OSCon::command_add(this);
    };
    OSCmd::~OSCmd()
    {
        OSCon::command_del(this);
    };


    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
}; /*host*/
/************************************************************************/
/*                                                                      */
/************************************************************************/
