/*******************************************************************
*
*    DESCRIPTION:Copyright(c) 2014-2024 KaoPu Network Technology Co,.Ltd
*
*    AUTHOR: chenxianfeng
*
*    HISTORY:
*
*    DATE:2014-10-13
*
*******************************************************************/



#include "compress.h"

#include <string>
#include <list>

#include <Windows.h>
#include <Wincrypt.h>

#include <tchar.h>

#include "osProcess.h"

#include "ttinfo/ttinfo.h"

#include "error/tterrorcode.h"


int  unzip(const wchar_t* fileName,const wchar_t* dstDir)
{
    OSProcess newProcess;
    wchar_t   szCmdLine[1024]           = { 0 };
    wchar_t   szTtInstallPath[MAX_PATH] = { 0 };
    int       exitCode;
    int       retval = 0;

    std::wstring strErrMsg;

    retval = TTInfo::getTtInstallPath(szTtInstallPath);
    if (retval != 0)
        return retval;

    wsprintf(szCmdLine, _T("%s\\Tools\\7z.exe x \"%s\" -aos -o\"%s\""),szTtInstallPath,fileName,dstDir);

    if (!newProcess.create(szCmdLine,NULL,true))
    {
        return TT_ERROR(0,GetLastError());
    }

    if (!newProcess.wait(&exitCode))
    {
        return TT_ERROR(0,GetLastError());
    }

    newProcess.readAllOutput(strErrMsg);

    if (strErrMsg.find(_T("Archives with Errors")) != std::wstring::npos)
    {
        return TT_ERROR(TET_OTHER,TEC_UNZIP_FAILED);
    }

    return 0;
}

int  unzip2(const wchar_t* fileName,const wchar_t* dstDir)
{
    OSProcess newProcess;
    wchar_t   szCmdLine[1024]           = { 0 };
    wchar_t   szTtInstallPath[MAX_PATH] = { 0 };
    int       exitCode;
    int       retval = 0;

    std::wstring strErrMsg;

    retval = TTInfo::getTtInstallPath(szTtInstallPath);
    if (retval != 0)
        return retval;

    wsprintf(szCmdLine, _T("%s\\7z\\7z.exe x \"%s\" -aos -o\"%s\""),szTtInstallPath,fileName,dstDir);

    if (!newProcess.create(szCmdLine,NULL,true))
    {
        return TT_ERROR(0,GetLastError());
    }

    if (!newProcess.wait(&exitCode))
    {
        return TT_ERROR(0,GetLastError());
    }

    newProcess.readAllOutput(strErrMsg);

    if (strErrMsg.find(_T("Archives with Errors")) != std::wstring::npos)
    {
        return TT_ERROR(TET_OTHER,TEC_UNZIP_FAILED);
    }

    return 0;
}


int zip(const std::wstring& dstFile, std::list<std::wstring>& srcFileList)
{
    OSProcess newProcess;
    wchar_t   szCmdLine[1024]           = { 0 };
    wchar_t   szTtInstallPath[MAX_PATH] = { 0 };
    int       exitCode;
    int       retval = 0;

    std::wstring strErrMsg;
    std::wstring strFiles;

    std::list<std::wstring>::iterator it;

    for (it=srcFileList.begin(); it!=srcFileList.end(); it++)
    {
        strFiles = strFiles + _T(" \"") + *it + _T("\" ");
    }

    retval = TTInfo::getTtInstallPath(szTtInstallPath);
    if (retval != 0)
        return retval;

    wsprintf(szCmdLine, _T("%s\\Tools\\7z.exe a \"%s\" %s -mx0"),szTtInstallPath,dstFile.c_str(),strFiles.c_str());
    
    if (!newProcess.create(szCmdLine,NULL,true))
    {
        return TT_ERROR(0,GetLastError());
    }

    if (!newProcess.wait(&exitCode))
    {
        return TT_ERROR(0,GetLastError());
    }

    newProcess.readAllOutput(strErrMsg);

    if (strErrMsg.find(_T("Errors")) != std::wstring::npos)
    {
        return TT_ERROR(TET_OTHER,TEC_UNZIP_FAILED);
    }

    return 0;
}

int zip2(const std::wstring& dstFile, std::list<std::wstring>& srcFileList)
{
    OSProcess newProcess;
    wchar_t   szCmdLine[1024]           = { 0 };
    wchar_t   szTtInstallPath[MAX_PATH] = { 0 };
    int       exitCode;
    int       retval = 0;

    std::wstring strErrMsg;
    std::wstring strFiles;

    std::list<std::wstring>::iterator it;

    for (it=srcFileList.begin(); it!=srcFileList.end(); it++)
    {
        strFiles = strFiles + _T(" \"") + *it + _T("\" ");
    }

    retval = TTInfo::getTtInstallPath(szTtInstallPath);
    if (retval != 0)
        return retval;

     wsprintf(szCmdLine, _T("%s\\7z\\7z.exe a \"%s\" %s -mx0"),szTtInstallPath,dstFile.c_str(),strFiles.c_str());

    if (!newProcess.create(szCmdLine,NULL,true))
    {
        return TT_ERROR(0,GetLastError());
    }

    if (!newProcess.wait(&exitCode))
    {
        return TT_ERROR(0,GetLastError());
    }

    newProcess.readAllOutput(strErrMsg);

    if (strErrMsg.find(_T("Errors")) != std::wstring::npos)
    {
        return TT_ERROR(TET_OTHER,TEC_UNZIP_FAILED);
    }

    return 0;
}

int  getMd5(const wchar_t* szFile,wchar_t* szMd5)
{
    HCRYPTPROV hProv=NULL;
    HCRYPTPROV hHash=NULL;
    HANDLE     hFile=CreateFile(szFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);

    if (hFile==INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        return TT_ERROR(0,GetLastError());
    }

    if(CryptAcquireContext(&hProv,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT)==FALSE)
    {
        return TT_ERROR(0,GetLastError());
    }

    if(CryptCreateHash(hProv,CALG_MD5,0,0,&hHash)==FALSE)
    {
        return FALSE;
    }
    DWORD dwFileSize=GetFileSize(hFile,0);
    if (dwFileSize==0xFFFFFFFF)
    {
        return TT_ERROR(0,GetLastError());
    }

    byte* lpReadFileBuffer=new byte[dwFileSize];
    DWORD lpReadNumberOfBytes;

    if (ReadFile(hFile,lpReadFileBuffer,dwFileSize,&lpReadNumberOfBytes,NULL)==0)
    {
        delete[] lpReadFileBuffer;

        return TT_ERROR(0,GetLastError());
    }
    if(CryptHashData(hHash,lpReadFileBuffer,lpReadNumberOfBytes,0)==FALSE)
    {
        delete[] lpReadFileBuffer;

        return TT_ERROR(0,GetLastError());
    }

    delete[] lpReadFileBuffer;
    CloseHandle(hFile);

    BYTE *pbHash;
    DWORD dwHashLen=/*sizeof(DWORD)*/16;
    if (!CryptGetHashParam(hHash,HP_HASHVAL,NULL,&dwHashLen,0))
    {
        return TT_ERROR(0,GetLastError());
    }

    pbHash=(byte*)malloc(dwHashLen);
    CryptGetHashParam(hHash,HP_HASHVAL,pbHash,&dwHashLen,0);
    if(CryptGetHashParam(hHash,HP_HASHVAL,pbHash,&dwHashLen,0))//获得md5值 
    {
        for(DWORD i=0;i<dwHashLen && i<=16;i++)         //输出md5值 
        {
            wchar_t strTmp[32]= { 0 };
            wsprintf(strTmp,_T("%02x"),pbHash[i]);
            szMd5[2*i] = strTmp[0];
            szMd5[2*i+1] = strTmp[1];
        }
    }

    if(CryptDestroyHash(hHash)==FALSE)
    {
        return TT_ERROR(0,GetLastError());
    }
    if(CryptReleaseContext(hProv,0)==FALSE)
    {
        return TT_ERROR(0,GetLastError());
    }

    return 0;
}

