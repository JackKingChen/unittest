
#pragma once;

#include <string>
#include <list>
#include <vector>

std::wstring&   string_ReplaceAll(std::wstring& str,const std::wstring& old_value,const std::wstring&   new_value);
std::string&    string_ReplaceAll(std::string& str,const std::string& old_value,const std::string&   new_value);

std::wstring stringToWString(const std::string &str);
std::string  wstringToString(const std::wstring &wstr);

bool stringSplit(const std::string& strSrc,char split, std::vector<std::string>& strListDst);
bool stringSplit(const std::wstring& strSrc,wchar_t split, std::vector<std::wstring>& strListDst);

std::string& trim(std::string &s);
std::wstring& trim(std::wstring &s);

std::string urlEncode(const std::string& szToEncode);
std::string urlDecode(const std::string& szToDecode);

bool isContainsChinese(const std::wstring& str);
bool isContainsBackspace(const std::wstring& str);
bool isTheSamePath(const std::wstring& strPath1, const std::wstring& strPath2);
bool isInSubPath(const std::wstring& strSubPath, const std::wstring& strPath);
bool isInDir(const std::wstring& strFilePath, const std::wstring& strDir);

std::wstring getFileNameFromPath(const std::wstring& strFilePath);
std::wstring getFileDirFromPath(const std::wstring& strFilePath);

