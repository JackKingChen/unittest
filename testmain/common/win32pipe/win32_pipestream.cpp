
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

#include "win32_pipestream.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "model.h"

/* The official documentation states that the name of a given named
 * pipe cannot be more than 256 characters long.
 */


#define INVALIDATE_PIPE_NAME      "PIPE-UNKNOWN"

Win32PipeStream::Win32PipeStream(const char* pipename) :
    m_pipe(INVALID_HANDLE_VALUE)
{
    sprintf(m_pipeName, "\\\\.\\pipe\\zhangyu-%s", pipename);
    m_pipeName[MAX_PATH-1] = '\0';
}

Win32PipeStream::Win32PipeStream(HANDLE pipe) :
    m_pipe(pipe)
{
}

Win32PipeStream::~Win32PipeStream()
{
    if (m_pipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_pipe);
        m_pipe = INVALID_HANDLE_VALUE;
    }
}

Win32PipeStream *Win32PipeStream::accept()
{
    Win32PipeStream*  clientStream;
    HANDLE pipe;

    pipe = ::CreateNamedPipeA(
                m_pipeName,                // pipe name
                PIPE_ACCESS_DUPLEX,  // read-write access
                PIPE_TYPE_BYTE |     // byte-oriented writes
                PIPE_READMODE_BYTE | // byte-oriented reads
                PIPE_WAIT,           // blocking operations
                PIPE_UNLIMITED_INSTANCES, // no limit on clients
                1024*64,                // input buffer size
                1024*64,                // output buffer size
                0,                   // client time-out
                NULL);               // default security attributes

    if (pipe == INVALID_HANDLE_VALUE)
    {
        TT_LOG_ERR(_T("[Win32Pipe]: CreateNamedPipe failed:%d\n"),GetLastError());
        return NULL;
    }

    // Stupid Win32 API design: If a client is already connected, then
    // ConnectNamedPipe will return 0, and GetLastError() will return
    // ERROR_PIPE_CONNECTED. This is not an error! It just means that the
    // function didn't have to wait.
    //
    if (::ConnectNamedPipe(pipe, NULL) == 0 && GetLastError() != ERROR_PIPE_CONNECTED)
    {
        TT_LOG_ERR(_T("[Win32Pipe]: ConnectNamedPipe failed: %d\n"),GetLastError());
        return NULL;
    }

    clientStream = new Win32PipeStream(pipe);
    return clientStream;
}

int Win32PipeStream::connect(void)
{
    HANDLE pipe;
    int    tries = 10;
    int    retval= 0;

    /* We're going to loop in order to wait for the pipe server to
     * be setup properly.
     */
    for (; tries > 0; tries--)
    {
        pipe = ::CreateFileA(
                    m_pipeName,                          // pipe name
                    GENERIC_READ | GENERIC_WRITE,  // read & write
                    0,                             // no sharing
                    NULL,                          // default security attrs
                    OPEN_EXISTING,                 // open existing pipe
                    0,                             // default attributes
                    NULL);                         // no template file

        /* If we have a valid pipe handle, break from the loop */
        if (pipe != INVALID_HANDLE_VALUE)
        {
            break;
        }

        /* We can get here if the pipe is busy, i.e. if the server hasn't
         * create a new pipe instance to service our request. In which case
         * GetLastError() will return ERROR_PIPE_BUSY.
         *
         * If so, then use WaitNamedPipe() to wait for a decent time
         * to try again.
         */
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            /* Not ERROR_PIPE_BUSY */
            retval = GetLastError();
            //if (retval != 2)
                //printf("[Win32Pipe]: CreateFile failed: %d\n", retval);
            errno = EINVAL;

            return retval;
        }

        /* Wait for 5 seconds */
        if ( !WaitNamedPipeA(m_pipeName, 5000) )
        {
            retval = GetLastError();
            //printf("[Win32Pipe]: WaitNamedPipe failed Failure: %d\n", retval);
            errno = EINVAL;

            return retval;
        }
    }

    if (tries>0)
        m_pipe = pipe;

    return retval;
}

int Win32PipeStream::writefully(void *buf, size_t size)
{
    if (m_pipe == INVALID_HANDLE_VALUE)
        return ERROR_INVALID_HANDLE;

    size_t res = size;
    int retval = 0;

    while (res > 0)
    {
        DWORD  written;
        if (! ::WriteFile(m_pipe, (const char *)buf + (size - res), res, &written, NULL)) {
            retval =  (int)GetLastError();
            TT_LOG_ERR(_T("[Win32Pipe]: writefully failed Failure: %d\n"),retval);
            break;
        }
        res -= written;
    }
    return retval;
}

int Win32PipeStream::readfully(void *buf, size_t len)
{
    int retval = 0;

    if (m_pipe == INVALID_HANDLE_VALUE)
        return ERROR_INVALID_HANDLE;

    if (!buf)
    {
        return ERROR_INVALID_HANDLE;  // do not allow NULL buf in that implementation
    }

    size_t res = len;
    while (res > 0) {
        DWORD  readcount = 0;
        if (! ::ReadFile(m_pipe, (char *)buf + (len - res), res, &readcount, NULL) || readcount == 0)
        {
            errno = (int)GetLastError();
            retval = errno;
            TT_LOG_ERR(_T("[Win32Pipe]: readfully failed Failure: %d\n"),retval);
            return retval;
        }
        res -= readcount;
    }

    return retval;
}
