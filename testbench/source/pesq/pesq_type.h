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

#ifndef __PESQ_TYPE_H__
#define __PESQ_TYPE_H__

/************************************************************************/
/*                                                                      */
/************************************************************************/
#define PESQ_TARGET_AVG_POWER   1E7
#define PESQ_LINIIR             60
#define PESQ_MAXNUTTERANCES     50
#define PESQ_WHOLE_SIGNAL       -1
#define PESQ_LSMJ               20
#define PESQ_LFBANK             35
#define PESQ_DATAPADDING_MSECS  320
#define PESQ_SEARCHBUFFER       75

#define PESQ_EPS                (1E-12)
#define PESQ_MINSPEECHLGTH      4

#define PESQ_JOINSPEECHLGTH     50
#define PESQ_MINUTTLENGTH       50

#define PESQ_SATDB              90.31
#define PESQ_FIXDB              -32.0

#define PESQ_TWOPI              6.28318530717959
#define PESQ_Nfmax              512

#define PESQ_Sp_8k              2.764344e-5
#define PESQ_Sl_8k              1.866055e-1

#define PESQ_Sp_16k             6.910853e-006
#define PESQ_Sl_16k             1.866055e-001

#define PESQ_Dz                 0.312
#define PESQ_gamma              0.001
#define PESQ_Tl                 10000.0f
#define PESQ_Ts                 10000000.0f
#define PESQ_Tt                 0.02f
#define PESQ_Tn                 0.01f

#define PESQ_D_POW_F            2
#define PESQ_D_POW_S            6
#define PESQ_D_POW_T            2

#define PESQ_A_POW_F            1
#define PESQ_A_POW_S            6
#define PESQ_A_POW_T            2


#define PESQ_D_WEIGHT            0.1
#define PESQ_A_WEIGHT            0.0309

#define PESQ_DOWNSAMPLE_08K_BAND 32
#define PESQ_DOWNSAMPLE_16K_BAND 64

#define PESQ_ALIGN_NFFT_08K      512
#define PESQ_ALIGN_NFFT_16K      1024

#define PESQ_IN_IIR_NSOS_08K     8L
#define PESQ_IN_IIR_NSOS_16K     12L

#define PESQ_NB_MODE             0
#define PESQ_WB_MODE             1

/************************************************************************/
/*                                                                      */
/************************************************************************/
typedef struct
{
    long    Nsamples;
    long    apply_swap;
    long    input_filter;
    long    dataSize;
    float * data;
    float * VAD;
    float * logVAD;
} PESQ_Wave_t;

typedef struct
{
    long    mode;
    long    Nutterances;
    long    Largest_uttsize;
    long    Nsurf_samples;

    long    Crude_DelayEst;
    float   Crude_DelayConf;
    long    UttSearch_Start[PESQ_MAXNUTTERANCES];
    long    UttSearch_End[PESQ_MAXNUTTERANCES];
    long    Utt_DelayEst[PESQ_MAXNUTTERANCES];
    long    Utt_Delay[PESQ_MAXNUTTERANCES];
    float   Utt_DelayConf[PESQ_MAXNUTTERANCES];
    long    Utt_Start[PESQ_MAXNUTTERANCES];
    long    Utt_End[PESQ_MAXNUTTERANCES];

    float   pesq_mos;
    float   mapped_mos;

} PESQ_Result_t;

typedef struct
{
    int            samplerate;
    int            downsample;
    int            bands;
    int            alignNfft;

    float         *run_temp;
    PESQ_Wave_t    ref_info;
    PESQ_Wave_t    deg_info;
    PESQ_Result_t  err_info;

    /**/
    long           InIIR_Nsos;
    const float   *InIIR_Hsos;
} PESQ_t;

/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*__PESQ_TYPE_H__*/
