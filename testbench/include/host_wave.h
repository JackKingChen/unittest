/*************************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR: 
*
*    HISTORY:
*
*    DATE:2013-04-07
*
*************************************************************************/

#ifndef __HOST_WAVE_H__
#define __HOST_WAVE_H__

/*
* unknown type
*/
#define WAVE_UNKNOWN_RATE   -1
#define WAVE_UNKNOWN_PTIME  -1
#define WAVE_UNKNOWN_PTYPE  -1

/*
* wave codec type
*/
#define WAVE_PTYPE_PCM           1
#define WAVE_PTYPE_PCMA          6
#define WAVE_PTYPE_PCMU          7
#define WAVE_PTYPE_G722          9
#define WAVE_PTYPE_G723          4
#define WAVE_PTYPE_G729          18
#define WAVE_PTYPE_GSM           3

#define WAVE_PTYPE_G726_16      103
#define WAVE_PTYPE_G726_24      104
#define WAVE_PTYPE_G726_32      102
#define WAVE_PTYPE_G726_40      105

/*
* wave file type
*/
typedef struct
{
    int codec_type;   /*wave codec type*/
    int channels;     /*use channels*/
    int sample_rate;  /*sample rate*/
    int byte_rate;    /*byte rate*/
    int precision;    /*sample point:8 bit,12 bit,16 bit...*/
    int data_payload; /*file size*/
    int head_size;    /*file head info size*/
    int file_size;
}wave_t;

/************************************************************************/
/*  function declear                                                    */
/************************************************************************/
int  wave_read_header (FILE* fp, wave_t* wave_info);
int  wave_write_header(FILE* fp, wave_t* wave_info);

/************************************************************************/
/*                                                                      */
/************************************************************************/
#endif/*__HOST_WAVE_H__*/
