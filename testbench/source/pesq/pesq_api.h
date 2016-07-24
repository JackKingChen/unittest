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
#ifndef __PESQ_API_H__
#define __PESQ_API_H__

#include "host.h"

/************************************************************************/
/*                                                                      */
/************************************************************************/

#include "pesq_type.h"
#include "pesq_dsp.h"

/************************************************************************/
/*  for file I/O                                                        */
/************************************************************************/
int pesq_load_data(PESQ_t *pesq,WaveFile &ref,WaveFile &deg);
int pesq_free_data(PESQ_t *pesq);

/************************************************************************/
/*  for filter                                                          */
/************************************************************************/
void pesq_Filter_IIR(PESQ_t *pesq, float * data, long Nsamples );
void pesq_Filter_FFT(PESQ_t *pesq, float * data, long Nsamples, int, const double [][2] );
void pesq_Filter_VAD(PESQ_t *pesq, PESQ_Wave_t * pinfo, float * data, float * VAD, float * logVAD);
void pesq_Filter_DC (float * data, long Nsamples,long dn);


void pesq_Filter_CrudeAlign(PESQ_t *pesq,PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info, PESQ_Result_t * err_info,
                            long Utt_id, float * ftmp);
void pesq_Filter_TimeAlign(PESQ_t *pesq,PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info, PESQ_Result_t * err_info,
                           long Utt_id, float * ftmp);

void pesq_Filter_SplitAlign(PESQ_t *pesq, PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info,PESQ_Result_t * err_info,
                            float * ftmp,
                            long Utt_Start, long Utt_SpeechStart, long Utt_SpeechEnd, long Utt_End,
                            long Utt_DelayEst, float Utt_DelayConf,
                            long * Best_ED1, long * Best_D1, float * Best_DC1,
                            long * Best_ED2, long * Best_D2, float * Best_DC2,
                            long * Best_BP);

/************************************************************************/
/*                                                                      */
/************************************************************************/
int pesq_init(PESQ_t *pesq,int sample_rate);
int pesq_free(PESQ_t *pesq);

int pesq_measure(PESQ_t *pesq,PESQ_Wave_t *ref_info, PESQ_Wave_t *deg_info,PESQ_Result_t *err_info);

/************************************************************************/
/*                                                                      */
/************************************************************************/
int  pesq_id_searchwindows(PESQ_t *pesq, PESQ_Wave_t  * ref_info, PESQ_Wave_t * deg_info,
                           PESQ_Result_t* err_info);

void pesq_id_utterances(PESQ_t *pesq,PESQ_Wave_t  * ref_info, PESQ_Wave_t * deg_info,
                        PESQ_Result_t* err_info);

void pesq_utterance_split(PESQ_t *pesq, PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info,
                          PESQ_Result_t * err_info, float * ftmp);

void pesq_utterance_locate(PESQ_t *pesq, PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info,
                           PESQ_Result_t * err_info, float * ftmp);


void pesq_psychoacoustic_model(PESQ_t *pesq,PESQ_Wave_t * ref_info, PESQ_Wave_t * deg_info,
                               PESQ_Result_t * err_info, float * ftmp);


/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif /*__PESQ_API_H__*/
