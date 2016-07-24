/********************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2012-04-21
*
*******************************************************************/

/*for stander*/
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*for unittest*/
#include "unittest.h"

/*for local*/
#include "host.h"

using namespace host;

/************************************************************************/
/*                                                                      */
/************************************************************************/
UNITTEST(CSVFile)
{
    CSVFile csv;

    /*
    * test write
    */
    const char *column[]=
    {
        "type-0","type-1","type-2","type-3",
    };
    EXPECT_TRUE(csv.Create("./result/csvfile.csv",column,TABLESIZE(column)));

    for (int i=0;i<1234;i++)
    {
        int value[16];
        for (size_t k=0;k<TABLESIZE(column);k++)
            value[k] = std::rand()%1000;
        
        QUIET_EXPECT_TRUE(csv.Write(value,TABLESIZE(column)));
    }
    csv.Close();


    /*
    * test 
    */
    EXPECT_TRUE(csv.Create("./result/csvfile2.csv",column,TABLESIZE(column)));

    for (int i=0;i<1000;i++)
    {
        float temp  = (float)i;
        int   value = (int)(cos(temp)*100);
        int   col   = i%TABLESIZE(column);

        QUIET_EXPECT_TRUE(csv.Write(value,col));
    }
    csv.Close();
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
