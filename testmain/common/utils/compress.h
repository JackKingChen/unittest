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

#ifndef COMPRESS_H_H_H
#define COMPRESS_H_H_H

#include <string>
#include <list>

int  unzip(const wchar_t* fileName,const wchar_t* dstDir);
int  unzip2(const wchar_t* fileName,const wchar_t* dstDir);
int  zip(const std::wstring& dstFile, std::list<std::wstring>& srcFileList);
int  zip2(const std::wstring& dstFile, std::list<std::wstring>& srcFileList);
int  getMd5(const wchar_t* szFile,wchar_t* szMd5);


#endif /*COMPRESS_H_H_H*/
