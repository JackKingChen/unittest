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

/*for unittest*/
#include "unittest.h"

/*for local*/
#include "host.h"

using namespace host;


/************************************************************************/
/*                                                                      */
/************************************************************************/
UNITTEST(pesq)
{
    double pesqMOS;
    double mapMOS;

    
    EXPECT_TRUE(WaveFile::PESQ("audio_tiny16_g722_pesq_before.wav",
        "audio_tiny16_g722_pesq_decode.wav",pesqMOS,mapMOS));

    printf("PESQ=%f , %f\n",pesqMOS,mapMOS);

    EXPECT_EQ(456863,(int)(mapMOS*100000));

    EXPECT_TRUE(WaveFile::PESQ("audio_tiny8_g729_pesq_before.wav",
        "audio_tiny8_g729_pesq_decode.wav",pesqMOS,mapMOS));
    printf("PESQ=%f , %f\n",pesqMOS,mapMOS);

    EXPECT_EQ(317018,(int)(pesqMOS*100000));
    EXPECT_EQ(307609,(int)(mapMOS*100000));


    EXPECT_TRUE(WaveFile::PESQ("audio_tiny16_g722_pesq_decode.wav",
        "audio_tiny16_g722_pesq_decode.wav",pesqMOS,mapMOS));
    printf("PESQ=%f , %f\n",pesqMOS,mapMOS);

    EXPECT_EQ(464388,(int)(mapMOS*100000));

    OSTTY::getkey();
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
