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

#include "host.h"
#include "pesq_api.h"


/************************************************************************/
/*                                                                      */
/************************************************************************/
static int io_read(PESQ_t *pesq,PESQ_Wave_t * info,WaveFile &in)
{
    short    frame[1024*16];
    size_t   count;
    size_t   total;
    float   *buffer;

    info->Nsamples = (2 * PESQ_SEARCHBUFFER * pesq->downsample) + in.Wav_Samples();
    info->dataSize = info->Nsamples + (PESQ_DATAPADDING_MSECS  * (pesq->samplerate  / 1000));
    info->data     = (float*)malloc(info->dataSize * sizeof(float) );
    info->VAD      = (float*)malloc(info-> Nsamples * sizeof(float) / pesq->downsample );
    info->logVAD   = (float*)malloc(info-> Nsamples * sizeof(float) / pesq->downsample );

    assert(info-> data != NULL);
    assert(info-> VAD != NULL);
    assert(info-> logVAD != NULL);

    buffer = info-> data;
    for(int i = 0;i<info->dataSize; i++)
        *(buffer++) = 0.0f;

    buffer = info-> data+(PESQ_SEARCHBUFFER*pesq->downsample);
    total  = 0;

    in.Seek(0);
    while((count = in.Read(frame,NULL,TABLESIZE(frame)))>0)
    {
        for(size_t i=0;i<count;i++)
        {
            *(buffer++) = (float)(frame[i]);
        }
        total+=count;
    }

    assert(in.Wav_Samples()==total);

    return 0;
}

static int io_free(PESQ_t *pesq,PESQ_Wave_t * info)
{
    if(info->data)
        free(info->data);
    if(info->VAD)
        free(info->VAD);
    if(info->logVAD)
        free(info->logVAD);

    info->data   = NULL;
    info->VAD    = NULL;
    info->logVAD = NULL;
    return 0;
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
int pesq_load_data(PESQ_t *pesq,WaveFile &ref,WaveFile &deg)
{
    PESQ_Wave_t *ref_info = &pesq->ref_info;
    PESQ_Wave_t *deg_info = &pesq->deg_info;
    size_t       temsize;

    if(io_read(pesq,ref_info,ref)<0)
        return -1;
    if(io_read(pesq,deg_info,deg)<0)
        return -1;

    /*alloc temp buffer*/
    temsize  = std::max(ref_info->Nsamples,deg_info->Nsamples);
    temsize += PESQ_DATAPADDING_MSECS  * (pesq->samplerate / 1000);
    temsize  = std::max(temsize,(size_t)(12 * pesq->alignNfft));
    temsize *= sizeof(float);
    pesq->run_temp = (float *)malloc(temsize+512);

    /*check*/
    if((ref_info-> Nsamples - 2 * PESQ_SEARCHBUFFER * pesq->downsample < pesq->samplerate / 4) ||
        (deg_info-> Nsamples - 2 * PESQ_SEARCHBUFFER * pesq->downsample < pesq->samplerate / 4))
    {
        return -1;
    }

    return 0;
}

int pesq_free_data(PESQ_t *pesq)
{
    PESQ_Wave_t *ref_info = &pesq->ref_info;
    PESQ_Wave_t *deg_info = &pesq->deg_info;

    io_free(pesq,ref_info);
    io_free(pesq,deg_info);

    if(pesq->run_temp)
        free(pesq->run_temp);
    pesq->run_temp = NULL;

    return 0;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
