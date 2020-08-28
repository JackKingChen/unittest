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
#ifndef __HOST_CONSOLE_H__
#define __HOST_CONSOLE_H__

/************************************************************************/
/* basic class of host                                              */
/************************************************************************/
namespace host
{
    class OSTTY;
    class OSCon;
    class OSCmd;

    /*
    * class of OSTTY
    */
    class OSTTY
    {
    public:
        OSTTY(const char*prompt);
        ~OSTTY();

        // Misc. non-Ascii keys that report an escape sequence
        enum Key
        {
            TTY_VK_NUL        ='\0',
            TTY_VK_RETURN     ='\r',
            TTY_VK_NEWLINE    ='\n',          
            TTY_VK_BACKSPACE  ='\b',
            TTY_VK_BACKSPACE_MAC  =127, // macos 127
            TTY_VK_INTC       =3   ,
            TTY_VK_TAB        =9   ,
            TTY_VK_ENTER      =13  ,
            TTY_VK_ESC        =27  ,
            TTY_VK_SPACE      =32  ,
            TTY_VK_UP         =128 ,// cursor key Up
            TTY_VK_DOWN       =129 ,// cursor key Down
            TTY_VK_RIGHT      =130 ,// Cursor Key Right
            TTY_VK_LEFT       =131 ,// cursor key Left
            TTY_VK_HOME       =132 ,// Cursor Key Home
            TTY_VK_END        =133 ,// Cursor Key End
            TTY_VK_INSERT     =134 ,// Cursor Key Insert
            TTY_VK_DELETE     =135 ,// Cursor Key Insert
            TTY_VK_PAGEUP     =136 ,// Cursor Key Page Up
            TTY_VK_PAGEDOWN   =137 ,// Cursor Key Page Down
            TTY_VK_FUN1       =138 ,// Function Key F1
            TTY_VK_FUN2       =139 ,// Function Key F2
            TTY_VK_FUN3       =140 ,// Function Key F3
            TTY_VK_FUN4       =141 ,// Function Key F4
            TTY_VK_FUN5       =142 ,// Function Key F5
            TTY_VK_FUN6       =143 ,// Function Key F6
            TTY_VK_FUN7       =144 ,// Function Key F7
            TTY_VK_FUN8       =145 ,// Function Key F8
            TTY_VK_FUN9       =146 ,// Function Key F9
            TTY_VK_FUN10      =147 ,// Function Key F10
            TTY_VK_FUN11      =148 ,// Function Key F11
            TTY_VK_FUN12      =149 ,// Function Key F12
        };

        enum Color
        {
            WHITE  =0,
            RED    =1,
            GREEN  =2,
            YELLOW =3,
        };
        enum Limites
        {
            MAX_LINE    = 256,
            MAX_HIS     = 10 ,
            MAX_PROMPT  = 32 ,
        };

    public:
        static void print(Color color,const char*fmt,...);
        static bool color(Color color);
        static int  read(unsigned char *buff,int len);
        static int  draw(const char * prompt,int prompt_len,const char * line,int pos);
        static bool hungup(void);
        static int  getkey(void);
        static bool getkey(int &key);
        static int  getline(char *line,int len,const char*prompt="input$");
        static bool getnumber(int &key);

    public:

        virtual int  getline(void);

        virtual int  input(char chr);
        virtual int  insert_char(const char chr);
        virtual int  insert_word(const char *word,int len);
        virtual int  delete_char();
        virtual int  delete_word(int nr);

        /*virtual hook*/
        virtual int  complete(void);
        virtual int  memorize(const char* cmd);

    protected:
        /*input*/
        char input_line[MAX_LINE];
        int  input_pos;
        int  input_len;
        int  input_eof;

        /*OSCmd list*/
        char cmd_prompt[MAX_PROMPT];
        int  cmd_prompt_nr;

        /*history*/
        char history[MAX_HIS][MAX_LINE];
        int  history_nr;
        int  history_wr;
        int  history_rd;
    };

    /*
    * class of OSCmd
    */
    class OSCmd
    {
    public:
        OSCmd();
        ~OSCmd();

    protected:
        friend class OSCon;

        /*OSCmd action*/
        virtual const char*cmd_name(void){return "no body";};
        virtual const char*cmd_help(void){return "nothing";};
        virtual int        cmd_action(int argc, const char* argv[]){return 0;};
        virtual int        cmd_argc(void){return (int)-1;};
        virtual bool       cmd_hide(void){return false;};
    };

    /*
    * class of OSCon
    */
    class OSCon:public OSTTY
    {
    public:
        OSCon(const char *prompt);
        ~OSCon();

        int  run(void);
        int  exe(const char *cmdline,int argc=0,const char **argv=NULL);

    public:
        friend class OSCmd;

    public:
        /*max cmd string length*/
        enum
        {
            CONSOLE_MAX         = OSTTY::MAX_LINE,  /*max basic char*/
            CONSOLE_MAX_WORD    = 128,              /*max length of one word*/
            CONSOLE_MAX_WW      = 128,              /*max OSCon window width*/
            CONSOLE_MAX_WH      = 128,              /*max OSCon window height*/
            CONSOLE_MAX_FILE    = 128,              /*max file/path in one directory*/
            CONSOLE_MAX_PATH    = 128,              /*max file/path in one directory*/
            CONSOLE_MAX_CMD     = 128,
        };

    protected:
        /*
        * helper
        */
        int  str_same (char str[][OSCon::CONSOLE_MAX_PATH],int nr,char *same);
        void str_parse(char* p_line,int n_argc,int* p_argc,const char*  argv[]);

        /*
        * hook
        */
        virtual int  complete_path(char *word);
        virtual int  complete(void);

        /*static data*/
    protected:
        static std::vector<OSCmd*> *command_list;
        static unsigned long        command_ref;
        
        /*static members*/
    protected:
        static void command_add(OSCmd *cmd);
        static void command_del(OSCmd *cmd);

    public:
        static int  command_help(const char *cmdname,const char *cmdtips);
        static int  command_exe (const char *cmdname,int argc=0,const char **argv=NULL);
        static int  command_run (OSCmd *excutor,bool runHide=false);
    };

};/*endof host*/
/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*__HOST_CONSOLE_H__*/
