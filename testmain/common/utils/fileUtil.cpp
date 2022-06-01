
#include "fileUtil.h"


bool IsDirectory(LPCTSTR pstrPath)
{
    if (NULL == pstrPath)
    {
        return FALSE;
    }

    /*ȥ��·��ĩβ�ķ�б��*/
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

    /*�жϸ�·���Ƿ���Ŀ¼*/
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

    /*���ԴĿ¼�Ƿ��ǺϷ�Ŀ¼*/
    if (!IsDirectory(pstrSrcFolder))
    {
        return FALSE;
    }

    /*���ļ�����ת��ΪCString���ͣ���ȷ��Ŀ¼��·��ĩβΪ��б��*/
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

    /*�����Ŀ¼�����ƣ�ֱ�ӷ��ظ��Ƴɹ�*/
    if (0 == _tcscmp(strSrcFolder, strDstFolder))
    {
        return TRUE;
    }

    /*���Ŀ��Ŀ¼������,�򴴽�Ŀ¼*/
    if (!IsDirectory(strDstFolder))
    {
        if (!CreateDirectory(strDstFolder, NULL))
        {
            /*����Ŀ��Ŀ¼ʧ��*/
            return FALSE;
        }
    }

    /*����ԴĿ¼�в����ļ���ͨ���*/
    CString strWildcard(strSrcFolder);
    strWildcard += _T("*.*");

    /*���ļ����ң��鿴ԴĿ¼���Ƿ����ƥ����ļ�*/
    /*����FindFile�󣬱������FindNextFile���ܻ�ò����ļ�����Ϣ*/
    CFileFind finder;
    BOOL bWorking = finder.FindFile(strWildcard);

    while (bWorking)
    {
        /*������һ���ļ�*/
        bWorking = finder.FindNextFile();

        /*������ǰĿ¼��.������һ��Ŀ¼��..��*/
        if (finder.IsDots())
        {
            continue;
        }

        /*�õ���ǰҪ���Ƶ�Դ�ļ���·��*/
        CString strSrcFile = finder.GetFilePath();

        /*�õ�Ҫ���Ƶ�Ŀ���ļ���·��*/
        CString strDstFile(strDstFolder);
        strDstFile += finder.GetFileName();

        /*�жϵ�ǰ�ļ��Ƿ���Ŀ¼,*/
        /*�����Ŀ¼���ݹ���ø���Ŀ¼,*/
        /*����ֱ�Ӹ����ļ�*/
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
                ASSERT(0);//��Ҫ���������
                //finder.Close();
                //return FALSE;
            }
        }

    } /*while (bWorking)*/

    /*�ر��ļ�����*/
    finder.Close();

    return TRUE;
}
