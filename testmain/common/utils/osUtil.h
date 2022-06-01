
#ifndef OS_UTILS_H
#define OS_UTILS_H

bool createProcessAsync(LPTSTR lpszCmd, BOOL bIsHide = TRUE, LPCTSTR lpszCurrentDir = NULL);
bool createProcessSync(LPTSTR lpszCmd, wchar_t *cResult = NULL,BOOL bIsHide = TRUE, LPCTSTR lpszCurrentDir = NULL);
bool createProcess(LPTSTR lpszCmd, DWORD &dwExitCode, wchar_t *cResult = NULL, DWORD dwMSTime = INFINITE, BOOL bIsHide = TRUE, LPCTSTR lpszCurrentDir = NULL);


bool getServiceStatus(LPWSTR serviceName, SERVICE_STATUS& status);
bool openService(LPWSTR serviceName, SERVICE_STATUS& status);
bool stopService(LPWSTR serviceName);

bool closeProcess(LPCTSTR lpszName, LPTSTR lpszPointPath, LPCTSTR lpzsIgnoreName);

bool isDirectory(const wchar_t* filename);
bool deleteDirectory(const wchar_t* szDirName);

DWORD GetIniFileString(LPCTSTR lpAppName,LPCTSTR lpKeyName,LPCTSTR lpDefault,LPTSTR lpReturnedString,DWORD nSize,LPCTSTR lpFileName);
INT GetIniFileInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,INT nDefault,LPCTSTR lpFileName);
DWORD GetIniFileSection(LPCTSTR lpAppName,LPTSTR lpReturnedString,DWORD nSize,LPCTSTR lpFileName);
#endif

