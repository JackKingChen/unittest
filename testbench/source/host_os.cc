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
    /* class of OSFile                                                      */
    /************************************************************************/
    OSFile::OSFile(const char *path,const char * mode)
    {
        writeable = false;
        handle    = NULL;
        if(path!=NULL)
            open(path,mode);
    }

    OSFile::~OSFile()
    {
        close();
    }

    bool OSFile::open(const char *path,const char * mode)
    {
        assert(path!=NULL);
        assert(mode!=NULL);

        close();

        char         tmp[512];
        const char * src = path;
        char       * dst = tmp;
        while(*src!='\0')
        {
#if defined(OS_WINDOWS)
            if(*src=='/')
                *dst = '\\';
#else /*Linux*/
            if(*src=='\\')
                *dst = '/';
#endif
            else
                *dst = *src;
            src++;
            dst++;
        }
        *dst='\0';
        path = tmp;

        if(strchr(mode,'w') || strchr(mode,'+'))
            writeable = true;
        else
            writeable = false;

        handle = fopen(path,mode);
        return is_open();
    }

    bool OSFile::close(void)
    {
        if(handle)
            fclose(handle);
        handle    = NULL;
        writeable = false;

        return true;
    }

    size_t OSFile::seek(seektype type,int pos)
    {
        if(!is_open())
            return 0;

        switch(type)
        {
        case OSFile::beg:
            return fseek(handle,pos,SEEK_SET);
        case OSFile::end:
            return fseek(handle,0,  SEEK_END);
        case OSFile::cur:
        default:
            return fseek(handle,pos,SEEK_CUR);
        }
        return 0;
    }

    size_t OSFile::seek (int pos)
    {
        return seek(OSFile::cur,pos);
    }

    size_t OSFile::read (void* buff,size_t size)
    {
        if(!is_open())
            return 0;
        return fread(buff,1,size,handle);
    }

    size_t OSFile::write(void* buff,size_t size)
    {
        if(!is_open())
            return 0;
        return fwrite(buff,1,size,handle);
    }
    bool OSFile::getline(char* str,size_t max_size)
    {
        if(!is_open())
            return 0;

        return (fgets(str,max_size,handle)!=NULL);
    }
    size_t OSFile::putline(const char* fmt,...)
    {
        if(!is_open())
            return 0;

        size_t  ret;
        va_list va;
        va_start(va,fmt);
        ret = vfprintf(handle,fmt,va);
        va_end(va);
        return ret;
    }
    size_t OSFile::total(void)
    {
        size_t offset;
        size_t length;

        if(!is_open())
            return 0;

        offset = ftell(handle);
        fseek(handle,0,SEEK_END);
        length = ftell(handle);

        return length;
    }

    size_t OSFile::tell(void)
    {
        if(!is_open())
            return 0;

        return ftell(handle);
    }
    void OSFile::flush(void)
    {
        if(!is_open())
            return ;

        fflush(handle);
    }

    bool OSFile::exist(const char * path)
    {
        return access(path,0)!=-1;
    }
    bool OSFile::path_dir(const char * full_path,char *dir_path)
    {
        assert(full_path!=NULL);
        assert(dir_path!=NULL);
        
        const char *end;
        const char *src;

        end = strrchr(full_path,'\\');
        if(!end)
            end = strrchr(full_path,'/');
        
        if(!end)
        {
            dir_path[0] = '\0';
            return false;
        }

        src = full_path;
        while(src != end)
            *dir_path++ = *src++;
        *dir_path = '\0';

        return true;
    }

    bool OSFile::path_file(const char * full_path,char *file_path)
    {
        assert(full_path!=NULL);
        assert(file_path!=NULL);

        const char *begin;
        const char *end;

        begin = strrchr(full_path,'\\');
        if(!begin)
            begin = strrchr(full_path,'/');
        if(!begin)
        {
            file_path[0] = '\0';
            return false;
        }
        end = strrchr(begin,'.');
        if(end)
        {
            const char *src = begin;
            while(src++ != end-1)
                *file_path++ = *src;
            *file_path = '\0';
        }
        else
        {
            strcpy(file_path,end+1);
        }
        return true;
    }
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
}; /*host*/
/************************************************************************/
/*                                                                      */
/************************************************************************/
