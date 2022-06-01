
#pragma once;

#include <Windows.h>

BOOL DeleteDirectory(LPCTSTR lpszDirectoryPath);
bool IsDirectory(LPCTSTR pstrPath);

BOOL CopyFolder(LPCTSTR pstrSrcFolder, LPCTSTR pstrDstFolder, const std::vector<CString> &ignoreFiles);
