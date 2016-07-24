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

#include "pesq_api.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/
#ifndef PESQ_TWOPI
#define PESQ_TWOPI   6.283185307179586f
#endif

/************************************************************************/
/*                                                                      */
/************************************************************************/

int  pesq_FFT_Init(PESQ_FFT_t *fft,unsigned long N)
{
    unsigned long   C, L, K;
    float           Theta;
    float         * PFFTPhi;

    if( (fft->FFTSwap != N) && (fft->FFTSwap != 0) )
        pesq_FFT_Free(fft);

    if( fft->FFTSwap == N )
    {
        return 0;
    }
    else
    {
        C = N;
        for( fft->FFTLog2N = 0; C > 1; C >>= 1 )
            fft->FFTLog2N++;

        C = 1;
        C <<= fft->FFTLog2N;
        if( N == C )
            fft->FFTSwap = N;

        fft->FFTButter = (unsigned long *) malloc( sizeof(unsigned long) * (N >> 1) );
        fft->FFTBitSwap = (unsigned long *) malloc( sizeof(unsigned long) * N );
        fft->FFTPhi = (float *) malloc( 2 * sizeof(float) * (N >> 1) );

        PFFTPhi = fft->FFTPhi;
        for( C = 0; C < (N >> 1); C++ )
        {
            Theta = (float)((PESQ_TWOPI * C) / N);
            (*(PFFTPhi++)) = (float) cos( Theta );
            (*(PFFTPhi++)) = (float) sin( Theta );
        }

        fft->FFTButter[0] = 0;
        L = 1;
        K = N >> 2;
        while( K >= 1 )
        {
            for( C = 0; C < L; C++ )
                fft->FFTButter[C+L] = fft->FFTButter[C] + K;
            L <<= 1;
            K >>= 1;
        }
    }

    return 0;
}

void pesq_FFT_Free(PESQ_FFT_t *fft)
{
    if( fft->FFTSwap != 0 )
    {
        free( fft->FFTButter );
        free( fft->FFTBitSwap );
        free( fft->FFTPhi );
        fft->FFTSwap = 0;
    }
    memset(fft,0,sizeof(PESQ_FFT_t));
}

void pesq_FFT_Forword(PESQ_FFT_t *fft,float * x, unsigned long N)
{
    unsigned long   Cycle, C, S, NC;
    unsigned long   Step    = N >> 1;
    unsigned long   K1, K2;
    register float  R1, I1, R2, I2;
    float           ReFFTPhi, ImFFTPhi;

    if( N > 1 )
    {
        pesq_FFT_Init(fft, N );

        for( Cycle = 1; Cycle < N; Cycle <<= 1, Step >>= 1 )
        {
            K1 = 0;
            K2 = Step << 1;

            for( C = 0; C < Cycle; C++ )
            {
                NC = fft->FFTButter[C] << 1;
                ReFFTPhi = fft->FFTPhi[NC];
                ImFFTPhi = fft->FFTPhi[NC+1];
                for( S = 0; S < Step; S++ )
                {
                    R1 = x[K1];
                    I1 = x[K1+1];
                    R2 = x[K2];
                    I2 = x[K2+1];

                    x[K1++] = R1 + ReFFTPhi * R2 + ImFFTPhi * I2;
                    x[K1++] = I1 - ImFFTPhi * R2 + ReFFTPhi * I2;
                    x[K2++] = R1 - ReFFTPhi * R2 - ImFFTPhi * I2;
                    x[K2++] = I1 + ImFFTPhi * R2 - ReFFTPhi * I2;
                }
                K1 = K2;
                K2 = K1 + (Step << 1);
            }
        }

        NC = N >> 1;
        for( C = 0; C < NC; C++ )
        {
            fft->FFTBitSwap[C] = fft->FFTButter[C] << 1;
            fft->FFTBitSwap[C+NC] = 1 + fft->FFTBitSwap[C];
        }
        for( C = 0; C < N; C++ )
        {
            if( (S = fft->FFTBitSwap[C]) != C )
            {
                fft->FFTBitSwap[S] = S;
                K1 = C << 1;
                K2 = S << 1;
                R1 = x[K1];
                x[K1++] = x[K2];
                x[K2++] = R1;
                R1 = x[K1];
                x[K1] = x[K2];
                x[K2] = R1;
            }
        }
    }
}

void pesq_FFT_Inverse(PESQ_FFT_t *fft,float * x, unsigned long N)
{
    unsigned long   Cycle, C, S, NC;
    unsigned long   Step    = N >> 1;
    unsigned long   K1, K2;
    register float  R1, I1, R2, I2;
    float           ReFFTPhi, ImFFTPhi;

    if( N > 1 )
    {
        pesq_FFT_Init(fft, N );

        for( Cycle = 1; Cycle < N; Cycle <<= 1, Step >>= 1 )
        {
            K1 = 0;
            K2 = Step << 1;

            for( C = 0; C < Cycle; C++ )
            {
                NC = fft->FFTButter[C] << 1;
                ReFFTPhi = fft->FFTPhi[NC];
                ImFFTPhi = fft->FFTPhi[NC+1];
                for( S = 0; S < Step; S++ )
                {
                    R1 = x[K1];
                    I1 = x[K1+1];
                    R2 = x[K2];
                    I2 = x[K2+1];

                    x[K1++] = R1 + ReFFTPhi * R2 - ImFFTPhi * I2;
                    x[K1++] = I1 + ImFFTPhi * R2 + ReFFTPhi * I2;
                    x[K2++] = R1 - ReFFTPhi * R2 + ImFFTPhi * I2;
                    x[K2++] = I1 - ImFFTPhi * R2 - ReFFTPhi * I2;
                }
                K1 = K2;
                K2 = K1 + (Step << 1);
            }
        }

        NC = N >> 1;
        for( C = 0; C < NC; C++ )
        {
            fft->FFTBitSwap[C] = fft->FFTButter[C] << 1;
            fft->FFTBitSwap[C+NC] = 1 + fft->FFTBitSwap[C];
        }
        for( C = 0; C < N; C++ )
            if( (S = fft->FFTBitSwap[C]) != C )
            {
                fft->FFTBitSwap[S] = S;
                K1 = C << 1;
                K2 = S << 1;
                R1 = x[K1];
                x[K1++] = x[K2];
                x[K2++] = R1;
                R1 = x[K1];
                x[K1] = x[K2];
                x[K2] = R1;
            }

            NC = N << 1;
            for( C = 0; C < NC; )
                x[C++] /= N;
    }
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void pesq_RealFFT_Forword(float *x, unsigned long N) 
{
    PESQ_FFT_t    fft;
    float        *y;
    unsigned long i;

    y = (float *) malloc (2 * N * sizeof (float));

    for (i = 0; i < N; i++) 
    {
        y [2 * i] = x [i];
        y [2 * i + 1] = 0.0f;
    }

    memset(&fft,0,sizeof(fft));

    pesq_FFT_Init(&fft, N);
    pesq_FFT_Forword(&fft,y, N);
    pesq_FFT_Free(&fft);

    for (i = 0; i <= N / 2; i++) 
    {
        x [2 * i] = y [2 * i];
        x [2 * i + 1] = y [2 * i + 1];
    }

    free (y);
}

void pesq_RealFFT_Inverse(float *x, unsigned long N)
{
    PESQ_FFT_t    fft;
    float        *y;
    unsigned long i;

    y = (float *) malloc (2 * N * sizeof (float));

    for (i = 0; i <= N / 2; i++) 
    {
        y [2 * i] = x [2 * i];
        y [2 * i + 1] = x [2 * i + 1];
    }

    for (i = N / 2 + 1; i < N; i++) 
    {
        int j = N - i;
        y [2 * i] = x [2 * j];
        y [2 * i + 1] = -x [2 * j + 1];
    }    

    memset(&fft,0,sizeof(fft));

    pesq_FFT_Init(&fft, N);
    pesq_FFT_Inverse(&fft,y, N);
    pesq_FFT_Free(&fft);

    for (i = 0; i < N; i++) 
    {
        x [i] = y [2 * i];
    }

    free (y);
}

int pesq_RealFFT_Correct(float * x1, unsigned long n1,
                         float * x2, unsigned long n2,
                         float * y)
{
    register float  r1, i1;
    float         * tmp1;
    float         * tmp2;
    long            C, D, Nx, Ny;

    Nx = pesq_dsp_nextpow2( std::max(n1, n2) );
    tmp1 = (float *) malloc(sizeof(float) * (2 * Nx + 2));
    tmp2 = (float *) malloc(sizeof(float) * (2 * Nx + 2));

    for( C = n1 - 1; C >= 0; C-- )
    {
        tmp1[C] = *(x1++);
    }
    for( C = n1; C < 2 * Nx; C++ )
        tmp1[C] = 0.0;

    pesq_RealFFT_Forword( tmp1, 2*Nx );

    for( C = 0; C < (long) n2; C++ )
    {
        tmp2[C] = x2[C];
    }
    for( C = n2; C < 2 * Nx; C++ )
        tmp2[C] = 0.0;

    pesq_RealFFT_Forword( tmp2, 2*Nx );

    for( C = 0; C <= Nx; C++ )
    {
        D = C << 1; r1 = tmp1[D]; i1 = tmp1[1 + D];
        tmp1[D] = r1 * tmp2[D] - i1 * tmp2[1 + D];
        tmp1[1 + D] = r1 * tmp2[1 + D] + i1 * tmp2[D];
    }

    pesq_RealFFT_Inverse( tmp1, 2*Nx );
    Ny = n1 + n2 - 1;
    for( C = 0; C < Ny; C++ )
        y[C] = tmp1[C];

    free( tmp1 );
    free( tmp2 );

    return Ny;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void pesq_IIR_Sos(float * x,
                  long    Nx,
                  float   b0,
                  float   b1,
                  float   b2,
                  float   a1,
                  float   a2,
                  float  *tz1,
                  float  *tz2)
{
    register float z0;
    register float z1;
    register float z2;

    if( tz1 == NULL ) z1 = 0.0f; else z1 = *tz1;
    if( tz2 == NULL ) z2 = 0.0f; else z2 = *tz2;

    if( (a1 != 0.0f) || (a2 != 0.0f) )
    {
        if( (b1 != 0.0f) || (b2 != 0.0f) )
        {
            while( (Nx) > 0 )
            {
                Nx--;
                z0 = (*x) - a1 * z1 - a2 * z2;
                *(x++) = b0 * z0 + b1 * z1 + b2 * z2;
                z2 = z1;
                z1 = z0;
            }
        }
        else
        {
            if( b0 != 1.0f )
            {
                while( (Nx) > 0 )
                {
                    Nx--;
                    z0 = (*x) - a1 * z1 - a2 * z2;
                    *(x++) = b0 * z0;
                    z2 = z1;
                    z1 = z0;
                }
            }
            else
            {
                while( (Nx) > 0 )
                {
                    Nx--;
                    z0 = (*x) - a1 * z1 - a2 * z2;
                    *(x++) = z0;
                    z2 = z1;
                    z1 = z0;
                }
            }
        }
    }
    else
    {
        if( (b1 != 0.0f) || (b2 != 0.0f) )
        {
            while( (Nx) > 0 )
            {
                Nx--;
                z0 = (*x);
                *(x++) = b0 * z0 + b1 * z1 + b2 * z2;
                z2 = z1;
                z1 = z0;
            }
        }
        else
        {
            if( b0 != 1.0f )
            {
                while( (Nx) > 0 )
                {
                    Nx--;
                    *x = b0 * (*x);
                    x++;
                }
            }
        }
    }

    if( tz1 != NULL ) (*tz1) = z1;
    if( tz2 != NULL ) (*tz2) = z2;
}

void pesq_IIR_Filt(const float * h,
                   long    Nsos,
                   float * z,
                   float * x,
                   long    Nx,
                   float * y)
{
    long C;

    if( y == NULL )
        y = x;
    else
    {
        for( C = 0; C < Nx; C++ )
            y[C] = x[C];
    }


    for( C = 0; C < Nsos; C++ )
    {
        if( z != NULL )
        {
            pesq_IIR_Sos( y, Nx, h[0], h[1], h[2], h[3], h[4], z, z+1 );
            z += 2;
        }
        else
            pesq_IIR_Sos( y, Nx, h[0], h[1], h[2], h[3], h[4], NULL, NULL );
        h += 5;
    }
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
