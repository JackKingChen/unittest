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

#include "host.h"

/************************************************************************/
/*  define                                                              */
/************************************************************************/
/*
* wave head
*/
#define RIFF_NAME           "RIFF"
#define WAVE_NAME           "WAVE"
#define FORMAT_NAME         "fmt "
#define FACT_NAME           "fact"
#define DATA_NAME           "data"

typedef struct
{
    char         name[4];
    unsigned int size;
    char         type[4];
}riff_chunk_t;

typedef struct
{
    char         name[4];
    unsigned int size;
}format_head_t;

typedef struct
{   
    unsigned short int  format;
    unsigned short int  channels;
    unsigned int        sample_rate;
    unsigned int        byte_rate;
    unsigned short int  align;
    unsigned short int  bit_per_sample;
    char                reverse[256];
}format_chunk_t;

typedef struct
{
    char         name[4];
    char         codec_name[4];
    unsigned int size;
}fact_chunk_t;

typedef struct
{
    char         name[4];
    unsigned int size;
}data_chunk_t;

/*codec table*/
typedef struct
{
    int         codec_ptype;
    int         codec_ptime;
}wave_codec_table_t;

/************************************************************************/
/*  static function                                                     */
/************************************************************************/
/*
* wave codec table
*/
static wave_codec_table_t wave_ptype_table[] = 
{
    {WAVE_PTYPE_PCMU,        20},
    {WAVE_PTYPE_PCMA,        20},
    {WAVE_PTYPE_G722,        20},
    {WAVE_PTYPE_G723,        30},
    {WAVE_PTYPE_G729,        20},
    {WAVE_PTYPE_GSM,         20},
    {WAVE_PTYPE_G726_16,     20},
    {WAVE_PTYPE_G726_24,     20},
    {WAVE_PTYPE_G726_32,     20},
    {WAVE_PTYPE_G726_40,     20},
};

/************************************************************************/
/*  debug function                                                      */
/************************************************************************/
#ifdef WAVE_DEBUG
static void debug_print_riff_chunk(riff_chunk_t* riff)
{
    char name[16];
    char type[16];

    memset(name, 0, sizeof(name));
    memcpy(name, riff->name, sizeof(riff->name));

    memset(type, 0, sizeof(type));
    memcpy(type, riff->type, sizeof(riff->type));

    printf("RIFF CHUNK size:  %d\n", sizeof(riff_chunk_t));
    printf("RIFF name %s\n",name);
    printf("RIFF type %s\n", type);
    printf("RIFF (file size - riff head(8)) = %d\n", riff->size);
}

static void debug_print_format_head(format_head_t* format_head)
{
    char name[16];

    memset(name, 0, sizeof(name));
    memcpy(name, format_head->name, sizeof(format_head->name));

    printf("FORMAT HEAD size:  %d\n", sizeof(format_head_t));
    printf("FORMAT name %s\n",name);
    printf("FORMAT CHUNK size %d\n", format_head->size);
}


static void debug_print_format_chunk(format_chunk_t* format)
{
    printf("FORMAT format %d (pcm=1, alaw=6 ulaw=7)\n", format->format);
    printf("FORMAT channel %d\n", format->channels);
    printf("FORMAT channel %d\n", format->channels);
    printf("FORMAT sample_rate %d\n", format->sample_rate);
    printf("FORMAT byte_rate %d\n", format->byte_rate);
    printf("FORMAT align  %d\n", format->align);
    printf("FORMAT bit per sample %d\n", format->bit_per_sample);
}


static void debug_print_fact_chunk(fact_chunk_t* fact)
{
    char name[16];

    memset(name, 0, sizeof(name));
    memcpy(name, fact->name, sizeof(fact->name));

    printf("FACT CHUNK size: %d\n", sizeof(fact_chunk_t));
    printf("FACT CHUNK name: %s\n", name);
    printf("FACT id   %d\n", fact->id);
    printf("FACT size is %d\n", fact->size);
}


static void debug_print_data_chunk(data_chunk_t* data)
{
    char name[16];

    memset(name, 0, sizeof(name));
    memcpy(name, data->name, sizeof(data->name));

    printf("DATA CHUNK size: %d\n", sizeof(data_chunk_t));
    printf("DATA CHUNK name: %s\n", name);
    printf("DATA size is %d\n", data->size);
}

static void wav_debug(const wave_t *wave_info)
{
    riff_chunk_t      riff;
    format_head_t     format_head;
    format_chunk_t    format;
    fact_chunk_t      fact;
    data_chunk_t      data;

    wav_write_head(wave_info, &riff, &format_head, &format, &fact, &data);
    if(wav_check_head(&riff, &format_head, &format, &fact, &data))
        return;

    debug_print_riff_chunk(&riff);
    debug_print_format_head(&format_head);
    debug_print_format_chunk(&format);
    debug_print_fact_chunk(&fact);
    debug_print_data_chunk(&data);

    return;
}

#else
#define wav_debug(wave_info)

#endif/*WAVE_DEBUG*/


/************************************************************************/
/*  control function                                                    */
/************************************************************************/
static int wav_read_head(FILE* fp, 
                         riff_chunk_t*   riff, 
                         format_head_t*  format_head,
                         format_chunk_t* format, 
                         fact_chunk_t*   fact, 
                         data_chunk_t*   data)
{
    int size;

    size = fread(riff,1, sizeof(riff_chunk_t),fp);
    if(size != sizeof(riff_chunk_t))
        return -1;

    size = fread(format_head,1,sizeof(format_head_t),fp);

    assert(size == sizeof(format_head_t));
    assert(format_head->size <= sizeof(format_chunk_t));

    size = fread(format,1,format_head->size,fp);
    assert(size == (int) format_head->size);

    if(format->format != WAVE_PTYPE_PCM)
    {
        size = fread(fact,1,sizeof(fact_chunk_t),fp);
        assert(size == sizeof(fact_chunk_t));
    }

    size = fread(data,1,sizeof(data_chunk_t),fp);
    assert(size == sizeof(data_chunk_t));

    return 0;
}


static int wav_check_head(riff_chunk_t*   riff,
                          format_head_t*  format_head,
                          format_chunk_t* format,
                          fact_chunk_t*   fact,
                          data_chunk_t*   data)
{ 

    assert( strncmp(riff->name, RIFF_NAME, sizeof(RIFF_NAME) - 1) ==0 );
    assert( strncmp(format_head->name, FORMAT_NAME, sizeof(FORMAT_NAME) - 1) == 0);

    if(format->format != WAVE_PTYPE_PCM)
    {
        assert( strncmp(fact->name, FACT_NAME, sizeof(FACT_NAME) - 1) == 0);
    }

    assert( strncmp(data->name, DATA_NAME, sizeof(DATA_NAME) - 1) == 0);

    assert(format->bit_per_sample <= 64);
    assert(format->sample_rate <= 192000);
    assert(format->channels <= 6);

    return 0;
}


static int wav_get_format_chunk_size(int codec_type)
{
    switch(codec_type)
    {
    case WAVE_PTYPE_PCM:
        return 18;
    case WAVE_PTYPE_PCMA:
    case WAVE_PTYPE_PCMU:
        return 18;
    default:
        assert(false);
        return 18;
    }

    return 0;
}


static int wav_fill_riff_chunk(int file_size, riff_chunk_t* riff)
{

    memset(riff, 0, sizeof(riff_chunk_t));
    strncpy(riff->name, RIFF_NAME, sizeof(riff->name));
    /*total size - 8*/
    riff->size = file_size - 8;
    strncpy(riff->type, WAVE_NAME, sizeof(riff->type));

    return 0;
}


static int wav_fill_format_head(int codec_type, format_head_t* format_head)
{

    memset(format_head, 0, sizeof(format_head_t));
    strncpy(format_head->name, FORMAT_NAME, sizeof(format_head->name));
    format_head->size = wav_get_format_chunk_size(codec_type);

    return 0;
}


static int wav_fill_format_chunk(const wave_t *wave_info, format_chunk_t* format)
{

    memset(format, 0, sizeof(format_chunk_t));
    format->format          = (unsigned short int)wave_info->codec_type;
    format->channels        = (unsigned short int)wave_info->channels;
    format->sample_rate     = (unsigned int)wave_info->sample_rate;
    format->byte_rate       = (unsigned int)wave_info->byte_rate;
    format->align           = wave_info->channels*wave_info->precision/8;
    format->bit_per_sample  = (unsigned short int)wave_info->precision;

    return 0;
}


static int wav_fill_fact_chunk(fact_chunk_t* fact)
{

    memset(fact, 0, sizeof(fact_chunk_t));
    strncpy(fact->name, FACT_NAME, sizeof(fact->name));
    //strncpy(fact->codec_name, "PCMU", sizeof(fact->codec_name));
    fact->size = 0x00530700;

    return 0;
}


static int wav_fill_data_chunk(int data_payload, data_chunk_t* data)
{

    memset(data, 0, sizeof(data_chunk_t));
    strncpy(data->name, DATA_NAME, sizeof(data->name));
    data->size = data_payload;/*total size - header_size*/

    return 0;
}

static int wav_write_head(const wave_t * wave_info,   riff_chunk_t*   riff, 
                          format_head_t* format_head, format_chunk_t* format, 
                          fact_chunk_t*  fact,        data_chunk_t*   data)
{
    int retval;

    /*fill riff_chunk_t*/
    retval = wav_fill_riff_chunk(wave_info->file_size, riff);
    if(retval<0)
        return retval;

    /*fill format_head_t*/
    retval = wav_fill_format_head(wave_info->codec_type, format_head);
    if(retval<0)
        return retval;

    /*fill format_chunk_t*/
    retval = wav_fill_format_chunk(wave_info, format);
    if(retval<0)
        return retval;

    /*fill fact_chunk_t*/
    retval = wav_fill_fact_chunk(fact);
    if(retval<0)
        return retval;

    /*fill data_chunk_t*/
    return wav_fill_data_chunk(wave_info->data_payload, data);
}


/************************************************************************/
/*  external function                                                   */
/************************************************************************/
int wave_write_header(FILE *fp, wave_t *wave_info)
{
    riff_chunk_t      riff;
    format_head_t     format_head;
    format_chunk_t    format;
    fact_chunk_t      fact;
    data_chunk_t      data;
    int               hdrsiz;
    int               align;

    if(fp ==NULL || wave_info == NULL)
        return -EINVAL;

    /*convert wave info to var*/
    wav_write_head(wave_info, &riff, &format_head, &format, &fact, &data);
    wav_check_head(&riff, &format_head, &format, &fact, &data);

    fseek(fp, 0, SEEK_SET);

    /*
    * read wave head info to file
    */

    hdrsiz = sizeof(riff_chunk_t);
    if(fwrite(&riff,1, sizeof(riff_chunk_t),fp) != sizeof(riff_chunk_t))
    {
        assert(false);
        return -EFAULT;
    }

    hdrsiz += sizeof(format_head_t);
    if(fwrite(&format_head,1, sizeof(format_head_t),fp) != sizeof(format_head_t))
    {
        assert(false);
        return -EFAULT;
    }

    hdrsiz += format_head.size;
    if(fwrite(&format,1, format_head.size,fp) != format_head.size)
    {
        assert(false);
        return -EFAULT;
    }

    if(format.format != WAVE_PTYPE_PCM)
    {
        hdrsiz += sizeof(fact_chunk_t);
        if(fwrite(&fact,1, sizeof(fact_chunk_t),fp) != sizeof(fact_chunk_t))
        {
            assert(false);
            return -EFAULT;
        }
    }

    hdrsiz += sizeof(data_chunk_t);

    /*
    * align to 
    */
    data.size  = wave_info->file_size-hdrsiz;
    align     = (wave_info->precision/8)*wave_info->channels;
    data.size = (data.size/align)*align;
    if(fwrite(&data,1, sizeof(data_chunk_t),fp) != sizeof(data_chunk_t))
    {
        assert(false);
        return -EFAULT;
    }

    wave_info->head_size    = hdrsiz;
    wave_info->data_payload = data.size;
    return 0;
}


int wave_read_header(FILE *fp, wave_t* wave_info)
{
    riff_chunk_t      riff;
    format_head_t     format_head;
    format_chunk_t    format;
    fact_chunk_t      fact;
    data_chunk_t      data;
    size_t            total;

    if(fp ==NULL || wave_info == NULL)
        return -EINVAL;

    /*get total file length*/
    fseek(fp,0,SEEK_END);
    total = ftell(fp);
    fseek(fp,0,SEEK_SET);

    /*
    * read file head info
    */
    if(wav_read_head(fp, &riff, &format_head, &format, &fact, &data) < 0)
        return -EFAULT;

    /*
    * check if head info is right
    */
    if(wav_check_head(&riff, &format_head, &format, &fact, &data) < 0)
    {
        assert(false);
        return -EFAULT;
    }

    assert(data.size <=  total);

    /*
    * start to copy
    */
    wave_info->file_size    = total;
    wave_info->codec_type   = format.format;
    wave_info->channels     = format.channels;
    wave_info->sample_rate  = format.sample_rate;
    wave_info->byte_rate    = format.byte_rate;
    wave_info->precision    = format.bit_per_sample;
    wave_info->data_payload = data.size;
    wave_info->head_size    = sizeof(riff_chunk_t) 
        + sizeof(format_head_t) 
        + format_head.size 
        //+ sizeof(fact_chunk_t) 
        + sizeof(data_chunk_t); 

    if(format.format != WAVE_PTYPE_PCM)
        wave_info->head_size += sizeof(fact_chunk_t) 

        wav_debug(wave_info);

    return 0;
}

int wave_get_ptime(int wave_ptype)
{
    int i;

    for(i=0; i < (int)(sizeof(wave_ptype_table)/sizeof(wave_ptype_table[0])); i++)
    {
        if(wave_ptype != wave_ptype_table[i].codec_ptype)
        {
            continue;
        }

        return wave_ptype_table[i].codec_ptime;
    }

    return WAVE_UNKNOWN_PTIME;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/

