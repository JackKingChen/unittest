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


#ifndef __HOST_CSVFILE_H__
#define __HOST_CSVFILE_H__

/************************************************************************/
/*                                                                      */
/************************************************************************/

namespace host
{


    /*
    * class of CSVFile
    */
    class CSVFile:public OSFile
    {
    public:
        CSVFile();
        ~CSVFile();
    public:
        enum
        {
            MAX_COL = 128,
            MAX_ROW = 0
        };

    public:
        bool Open(const char *path);
        bool Close();
        bool Create(const char *path,const char *column[],size_t columnNR);

        bool Write(int value,  size_t col);
        bool Write(int value[],size_t nr);
        bool Flush();

        bool Read(int value,  size_t col);
        bool Read(int value[],size_t nr);
        bool Next();

    private:
        size_t  cache_column;
        size_t  cache_count;
        int     cache_value[MAX_COL];

    };

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};

#endif /*__HOST_CSVFILE_H__*/
