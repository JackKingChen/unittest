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


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <limits.h>

/*local*/
#include "unittest.h"
#include "loadtest.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
LoadTest::LoadTest()
{
    test_basic_tick = 10*1000; /*10ms*/
    test_result_us  = 0;
    test_result_mips= 0;

    /*
    * set as highest 
    */
    OSThread::SetPriority(OSThread::PriorityMax);
}

LoadTest::~LoadTest()
{ 
    /*
    * set as normal 
    */
    OSThread::SetPriority(OSThread::PriorityMin);
}

int LoadTest::Run(unsigned long result[],int loop)
{
    OSTime begin;
    OSTime ending;
    int i;
    int n;
    for (i=0,n=0;n<loop;i++)
    {
        begin.GetTime();

        Code();

        ending.GetTime();
        ending-= begin;

        /*must be <10ms*/
        if(ending.tv_sec==0 && ending.tv_usec < 10*1000*1000)
        {
            /*not possible to greater than 1s*/
            result[n] = ending.tv_usec;
            if(result[n]<(unsigned long)test_basic_tick)
                n++;
        }
        if((i>loop*2) && (n<loop/2))
            return -EFAULT;
    }

    return 0;
}

int LoadTest::RunAvg(int loop,int systick)
{
    unsigned long *result;
    double  total;
    double  average;
    double  below_average;
    int     below_average_nr;

    /*set system tick*/
    if(systick>0)
        test_basic_tick = systick;

    /*get more than twice*/
    loop  *=2;
    result = (unsigned long *)malloc(sizeof(unsigned long)*loop+16);

    /*get data*/
    Run(result,loop);

    /*
    * get average
    */
    total  = 0;
    average= 0;
    for (int i=0;i<loop;i++)
    {
        total+=result[i];
    }
    average=total/loop;

    /*
    * get fast
    */
    below_average_nr = 0;
    below_average    = 0;

    for (int i=0;i<loop;i++)
    {
        if(result[i]<average)
        {
            below_average+=result[i];
            below_average_nr++;
        }
    }
    below_average=below_average/below_average_nr;

    /*to time*/
    test_result_us   = below_average;

    /*to mips*/
    test_result_mips = OS::GetMips()*test_result_us/(1000*1000);

    /**/
    free(result);
    return 0;
}

int LoadTest::RunMin(int loop,int systick)
{
    unsigned long *result;
    double  mininum;

    /*set system tick*/
    if(systick>0)
        test_basic_tick = systick;

    /*get more than twice*/
    loop  *=2;
    result = (unsigned long *)malloc(sizeof(unsigned long)*loop+16);

    /*get data*/
    if(Run(result,loop)<0)
        return -EFAULT;

    /*
    * get average
    */
    mininum = INT_MAX;
    for (int i=0;i<loop;i++)
    {
        if(result[i]<mininum)
            mininum = result[i];
    }

    /*to time*/
    test_result_us   = mininum;

    /*to mips*/
    test_result_mips = OS::GetMips()*test_result_us/(1000*1000);

    /**/
    free(result);
    return 0;
}

int LoadTest::RunMax(int loop,int systick)
{
    unsigned long *result;
    double  mininum;

    /*set system tick*/
    if(systick>0)
        test_basic_tick = systick;

    /*get more than twice*/
    loop  *=2;
    result = (unsigned long *)malloc(sizeof(unsigned long)*loop+16);

    /*get data*/
    if(Run(result,loop)<0)
        return -EFAULT;

    /*
    * get average
    */
    mininum = 0;
    for (int i=0;i<loop;i++)
    {
        if(result[i]>mininum)
            mininum = result[i];
    }

    /*to time*/
    test_result_us   = mininum;

    /*to mips*/
    test_result_mips = OS::GetMips()*test_result_us/(1000*1000);

    /**/
    free(result);
    return 0;
}

double LoadTest::GetMips(void)
{

    return test_result_mips;
}

int   LoadTest::GetTime(void)
{

    return (int)test_result_us;
}

double LoadTest::RunAvgMHz(int loop)
{
    RunAvg(loop);
    return GetMips();
}
double LoadTest::RunMinMHz(int loop)
{
    RunMin(loop);
    return GetMips();
}
double LoadTest::RunMaxMHz(int loop)
{
    RunMax(loop);
    return GetMips();
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
