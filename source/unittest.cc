/********************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: JackChen
*
*    HISTORY:
*
*    DATE:2012-04-21
*
*******************************************************************/

/*local*/
#include "unittest.h"

namespace unittest
{
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    bool UnitTest::test_abort = 0;

    /************************************************************************/
    /* special command for run all cases                                    */
    /************************************************************************/
    UNITTEST_HIDE(all)
    {
        OSCon::command_run(this,true);
    }
    UNITTEST_HIDE(case)
    {
        OSCon::command_run(this,false);
    }

    /************************************************************************/
    /* special command for estimate the time be taken by a case             */
    /************************************************************************/
    UNITTEST_HIDE(time)
    {
        if(argc>0)
        {
            OSTime  now;
            OSTime  dif;
            OSTTY::print(OSTTY::YELLOW,"time : %s\n",argv[0]);

            now.GetTime();
            OSCon::command_exe(argv[0],argc-1,argv+1);
            dif.GetTime();
            
            dif-=now;
            OSTTY::print(OSTTY::YELLOW,"time : %d s,%d ms,%d us\n",
                dif.tv_sec,dif.tv_usec/1000,dif.tv_usec%1000);
        }
        else
        {
            OSTTY::print(OSTTY::RED,
                "time:must be set a case!!\n"
                "for example:\n"
                "   time ${case}\n"
                "   time flow\n"
                );
        }
    }

    /************************************************************************/
    /* special command for stress test a case                               */
    /************************************************************************/
    UNITTEST_HIDE(stress)
    {
        int loop;

        if(argc<2 || (loop = atoi(argv[0]))<=0)
        {
            OSTTY::print(OSTTY::RED,
                "stress:must be set loop count!!\n"
                "for example:\n"
                "   stress ${loop}  ${case}\n"
                "   stress 100 flow\n"
                );
        }
        else
        {
            for (int i=0;i<loop;i++)
            {
                OSTTY::print(OSTTY::YELLOW,"stress(%-8d): %s\n",i,argv[1]);

                OSCon::command_exe(argv[1],argc-1,argv+1);
            }
        }
    }
    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
};
/************************************************************************/
/*                                                                      */
/************************************************************************/
