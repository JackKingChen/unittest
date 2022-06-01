
#include "fileUtil.h"


bool IsDirectory(LPCTSTR pstrPath)
{
    if (NULL == pstrPath)
    {
        return FALSE;
    }

    /*去除路径末尾的反斜杠*/
    CString strPath = pstrPath;
    if (strPath.Right(1) == _T('\\'))
    {
        strPath.Delete(strPath.GetLength()-1);
    }

    CFileFind finder;
    BOOL bRet = finder.FindFile(strPath);
    if (!bRet)
    {
        return FALSE;
    }

    /*判断该路径是否是目录*/
    finder.FindNextFile();
    bRet = finder.IsDirectory();
    finder.Close();
    return bRet;
}

BOOL CopyFolder(LPCTSTR pstrSrcFolder, LPCTSTR pstrDstFolder, const std::vector<CString> &ignoreFiles)
{
    if ((NULL == pstrSrcFolder) || (NULL == pstrDstFolder))
    {
        return FALSE;
    }

    /*检查源目录是否是合法目录*/
    if (!IsDirectory(pstrSrcFolder))
    {
        return FALSE;
    }

    /*把文件名称转换为CString类型，并确认目录的路径末尾为反斜杠*/
    CString strSrcFolder(pstrSrcFolder);
    if (strSrcFolder.Right(1) != _T('\\'))
    {
        strSrcFolder += _T("\\");
    }
    CString strDstFolder(pstrDstFolder);
    if (strDstFolder.Right(1) != _T("\\"))
    {
        strDstFolder += _T("\\");
    }

    /*如果是目录自身复制，直接返回复制成功*/
    if (0 == _tcscmp(strSrcFolder, strDstFolder))
    {
        return TRUE;
    }

    /*如果目的目录不存在,则创建目录*/
    if (!IsDirectory(strDstFolder))
    {
        if (!CreateDirectory(strDstFolder, NULL))
        {
            /*创建目的目录失败*/
            return FALSE;
        }
    }

    /*创建源目录中查找文件的通配符*/
    CString strWildcard(strSrcFolder);
    strWildcard += _T("*.*");

    /*打开文件查找，查看源目录中是否存在匹配的文件*/
    /*调用FindFile后，必须调用FindNextFile才能获得查找文件的信息*/
    CFileFind finder;
    BOOL bWorking = finder.FindFile(strWildcard);

    while (bWorking)
    {
        /*查找下一个文件*/
        bWorking = finder.FindNextFile();

        /*跳过当前目录“.”和上一级目录“..”*/
        if (finder.IsDots())
        {
            continue;
        }

        /*得到当前要复制的源文件的路径*/
        CString strSrcFile = finder.GetFilePath();

        /*得到要复制的目标文件的路径*/
        CString strDstFile(strDstFolder);
        strDstFile += finder.GetFileName();

        /*判断当前文件是否是目录,*/
        /*如果是目录，递归调用复制目录,*/
        /*否则，直接复制文件*/
        if (finder.IsDirectory())
        {
            if (!CopyFolder(strSrcFile, strDstFile, ignoreFiles))
            {
                finder.Close();
                return FALSE;
            }
        }
        else
        {
            BOOL bIgnore = FALSE;
            for (UINT i=0; i<ignoreFiles.size(); ++i)
            {
                if(strSrcFile.Find(ignoreFiles[i]) != -1)
                {
                    bIgnore = TRUE;
                    break;
                }
            }

            if (!bIgnore 
                && !CopyFile(strSrcFile, strDstFile, FALSE))
            {
                ASSERT(0);//需要继续处理的
                //finder.Close();
                //return FALSE;
            }
        }

    } /*while (bWorking)*/

    /*关闭文件查找*/
    finder.Close();

    return TRUE;
}
