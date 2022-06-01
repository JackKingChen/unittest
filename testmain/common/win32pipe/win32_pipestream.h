
/*******************************************************************
*
*    DESCRIPTION:Copyright(c) 2014-2024 KaoPu Network Technology Co,.Ltd
*
*    AUTHOR: chenxianfeng (ebdsoft@163.com)
*
*    HISTORY:
*
*    DATE:2015-12-02
*
*******************************************************************/

#ifndef WIN32_PIPE_STREAM_H_H_H
#define WIN32_PIPE_STREAM_H_H_H


#include <Windows.h>

class Win32PipeStream
{
public:
    explicit Win32PipeStream(const char* pipename);
    virtual ~Win32PipeStream();

public:
    Win32PipeStream *accept();
    int connect(void);

    int writefully(void *buf, size_t size);
    int readfully(void *buf, size_t len);

private:
    Win32PipeStream(HANDLE pipe);

private:
    HANDLE  m_pipe;
    char    m_pipeName[MAX_PATH];
};

#endif /*WIN32_PIPE_STREAM_H_H_H*/
