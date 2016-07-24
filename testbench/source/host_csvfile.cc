/* ******************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2013-09-16
*
*
****************************************************************** */

/************************************************************************/
/*Include                                                              */
/************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "host.h"

/************************************************************************/
/*Define                                                               */
/************************************************************************/
namespace host
{
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    CSVFile::CSVFile()
    {

    }
    CSVFile::~CSVFile()
    {

    }
    bool CSVFile::Open(const char *path)
    {
        
        return false;
    }

    bool CSVFile::Close()
    {
        /*just close it*/
        return OSFile::close();
    }

    bool CSVFile::Create(const char *path,const char *column[],size_t columnNR)
    {
        /*create file*/
        if(!OSFile::open(path,"w+"))
            return false;

        /*write column name*/
        assert(columnNR<MAX_COL);

        for (size_t i=0;i<columnNR;i++)
        {
            OSFile::putline("%s,",column[i]);
        }
        OSFile::putline("\n");

        /*reset cache*/
        cache_column= columnNR;
        cache_count = 0;
        return true;
    }

    bool CSVFile::Write(int value,  size_t col)
    {
        if(col>cache_column)
            return false;
        cache_value[cache_count] = value;
        cache_count++;

        if(cache_count>=cache_column)
            return Flush();
        return true;
    }

    bool CSVFile::Write(int value[],size_t nr)
    {
        char   buf[4069];
        size_t pos;

        if(!value || nr!=cache_column)
            return false;

        pos = 0;
        for (size_t i=0;i<nr && pos<sizeof(buf);i++)
        {
            pos+=sprintf(buf+pos,"%d,",value[i]);
        }
        strcat(buf+pos,"\n");
        pos++;

        return (OSFile::write(buf,pos) == pos);
    }

    bool CSVFile::Flush()
    {
        /*just write cache*/
        if(Write(cache_value,cache_count))
        {
            cache_count = 0;
            return true;
        }

        return false;
    }

    bool CSVFile::Read(int value,  size_t col)
    {
        
        return true;
    }

    bool CSVFile::Read(int value[],size_t nr)
    {

        return true;
    }

    bool CSVFile::Next()
    {

        return true;
    }

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};
/************************************************************************/
/*                                                                      */
/************************************************************************/

