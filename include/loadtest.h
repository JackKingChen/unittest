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
#ifndef __LOAD_TEST_H__
#define __LOAD_TEST_H__

#include <stdio.h>
#include <istream>
#include <ostream>
#include <iostream>

#include "unittest.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace unittest
{
    /*
    * class of load-test
    */
    class LoadTest
    {
    public:
        LoadTest();
        ~LoadTest();

    public:

        virtual void Code(void){};

    public:
        int   Run(unsigned long result[],int loop);
        int   RunAvg(int loop,int systick=-1);
        int   RunMin(int loop,int systick=-1);
        int   RunMax(int loop,int systick=-1);

        double GetMips(void);
        int    GetTime(void);

        double RunAvgMHz(int loop);
        double RunMinMHz(int loop);
        double RunMaxMHz(int loop);

    protected:
        int     test_basic_tick;
        double  test_result_us;
        double  test_result_mips;
    };
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*__LOAD_TEST_H__*/
