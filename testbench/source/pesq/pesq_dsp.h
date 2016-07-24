/*******************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2014-03-29
*
*
*******************************************************************/

#ifndef __PESQ_DSP_H__
#define __PESQ_DSP_H__

/************************************************************************/
/*                                                                      */
/************************************************************************/
typedef struct
{
    unsigned long   FFTSwap;
    unsigned long   FFTLog2N;
    unsigned long * FFTButter;
    unsigned long * FFTBitSwap;
    float         * FFTPhi;
}PESQ_FFT_t;

/************************************************************************/
/*                                                                      */
/************************************************************************/
/*
* IIR API
*/
void pesq_IIR_Filt(const float * h,
                   long    Nsos,
                   float * z,
                   float * x,
                   long    Nx,
                   float * y);

void pesq_IIR_Sos(float * x,
                  long    Nx,
                  float   b0,
                  float   b1,
                  float   b2,
                  float   a1,
                  float   a2,
                  float  *tz1,
                  float  *tz2);

/*
* FFT API
*/
int  pesq_FFT_Init(PESQ_FFT_t *fft,unsigned long N);
void pesq_FFT_Free(PESQ_FFT_t *fft);
void pesq_FFT_Forword(PESQ_FFT_t *fft,float * x, unsigned long N);
void pesq_FFT_Inverse(PESQ_FFT_t *fft,float * x, unsigned long N);


void pesq_RealFFT_Forword(float * x, unsigned long N);
void pesq_RealFFT_Inverse(float * x, unsigned long N);
int  pesq_RealFFT_Correct(float * x1, unsigned long n1,
                          float * x2, unsigned long n2,
                          float * y);

/************************************************************************/
/*                                                                      */
/************************************************************************/
static __inline long pesq_dsp_nextpow2(unsigned long X)
{
    unsigned long C = 1;

    while( (C < ULONG_MAX) && (C < X) )
        C <<= 1;

    return C;
}

static __inline int  pesq_dsp_ispow2(unsigned long X)
{
    unsigned long C = 1;
    while( (C < ULONG_MAX) && (C < X) )
        C <<= 1;

    return (C == X);
}

static __inline int  pesq_dsp_intlog2(unsigned long X)
{
    return (int)floor( log( 1.0 * X ) / log( 2.0 ) + 0.5 );
}

static __inline double pesq_dsp_powof (const float * const x, long start_sample, long stop_sample, long divisor) 
{
    double  power = 0;
    long    i;

    assert (start_sample >= 0);
    assert (start_sample < stop_sample);

    for (i = start_sample; i < stop_sample; i++)
    {
        float h = x [i];
        power += h * h;
    }
    power /= divisor;
    return power;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*__PESQ_DSP_H__*/
