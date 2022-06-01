
#include "util.h"

#include <Windows.h>
#include <codecvt>

std::wstring&   string_ReplaceAll(std::wstring& str,const std::wstring& old_value,const std::wstring&   new_value)
{
    while(true)
    {
        std::wstring::size_type pos(0);
        if(   (pos=str.find(old_value))!=std::wstring::npos)
            str.replace(pos,old_value.length(),new_value);
        else   break;
    }
    return str;
}

std::string&   string_ReplaceAll(std::string& str,const std::string& old_value,const std::string&   new_value)
{
    while(true)
    {
        std::string::size_type pos(0);
        if(   (pos=str.find(old_value))!=std::string::npos)
            str.replace(pos,old_value.length(),new_value);
        else   break;
    }
    return str;
}

std::wstring stringToWString(const std::string &str)
{
#if 0
    std::wstring wstr(str.length(),L' ');
    std::copy(str.begin(), str.end(), wstr.begin());
    return wstr;
#else
     std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;

     return conv.from_bytes(str);
#endif
}

std::string wstringToString(const std::wstring &wstr)
{
#if 0
    std::string str(wstr.length(), ' ');
    std::copy(wstr.begin(), wstr.end(), str.begin());
    return str;
#else
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;

    return  conv.to_bytes(wstr);
#endif
}

bool stringSplit(const std::string& strSrc,char split, std::vector<std::string>& strListDst)
{
    std::string o_str = strSrc;
    int index = 0;
    do
    {
        std::string strTmp = "";
        index = o_str.find(split);
        if( -1 == index )
        {
            strTmp = o_str.substr( 0, o_str.length() );
            strListDst.push_back( strTmp );
            break;
        }
        strTmp = o_str.substr( 0, index );
        o_str.erase( 0, index+1 );
        strListDst.push_back( strTmp );
    }while(true);

    return true;
}

bool stringSplit(const std::wstring& strSrc,wchar_t split, std::vector<std::wstring>& strListDst)
{
    std::wstring o_str = strSrc;
    int index = 0;
    do
    {
        std::wstring strTmp = L"";
        index = o_str.find(split);
        if( -1 == index )
        {
            strTmp = o_str.substr( 0, o_str.length() );
            strListDst.push_back( strTmp );
            break;
        }
        strTmp = o_str.substr( 0, index );
        o_str.erase( 0, index+1 );
        strListDst.push_back( strTmp );
    }while(true);

    return true;
}

std::string& trim(std::string &s)
{  
    if (s.empty())
    {
        return s;
    }

    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;  
}

std::wstring& trim(std::wstring &s)
{  
    if (s.empty())
    {
        return s;
    }

    s.erase(0,s.find_first_not_of(L" "));
    s.erase(s.find_last_not_of(L" ") + 1);
    return s;  
}

std::string urlEncode(const std::string& szToEncode)
{
    std::string src = szToEncode;
    char hex[] = "0123456789ABCDEF";
    std::string dst;

    for (size_t i = 0; i < src.size(); ++i)
    {
        unsigned char cc = src[i];
        if (isascii(cc))
        {
            if (cc == ' ')
            {
                dst += "%20";
            }
            else
                dst += cc;
        }
        else
        {
            unsigned char c = static_cast<unsigned char>(src[i]);
            dst += '%';
            dst += hex[c / 16];
            dst += hex[c % 16];
        }
    }
    return dst;
}


std::string urlDecode(const std::string& szToDecode)
{
    std::string result;
    int hex = 0;
    for (size_t i = 0; i < szToDecode.length(); ++i)
    {
        switch (szToDecode[i])
        {
        case '+':
            result += ' ';
            break;
        case '%':
            if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2]))
            {
                std::string hexStr = szToDecode.substr(i + 1, 2);
                hex = strtol(hexStr.c_str(), 0, 16);
                if (!((hex >= 48 && hex <= 57) || //0-9
                    (hex >=97 && hex <= 122) ||   //a-z
                    (hex >=65 && hex <= 90) ||    //A-Z
                    hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29
                    || hex == 0x2a || hex == 0x2b|| hex == 0x2c || hex == 0x2d || hex == 0x2e || hex == 0x2f
                    || hex == 0x3A || hex == 0x3B|| hex == 0x3D || hex == 0x3f || hex == 0x40 || hex == 0x5f
                    ))
                {
                    result += char(hex);
                    i += 2;
                }
                else result += '%';
            }else {
                result += '%';
            }
            break;
        default:
            result += szToDecode[i];
            break;
        }
    }
    return result;
}

int Base64Encode( const void* pSrc, const int nSrcLen, BYTE* pbyDest, const int nMaxLen )
{
    const char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    BYTE* p1 = (BYTE*)pSrc;
    ULONG i, v;
    int nIndex = 0;
    for (i = 0; i < nSrcLen; i += 3)
    {
        if ( nIndex + 3 >= nMaxLen )
        {
            break;
        }
        switch ( nSrcLen - i )
        {
        case 1:
            v = (p1[i] << 16);
            pbyDest[nIndex++] = base64_alphabet[v >> 18];
            pbyDest[nIndex++] = base64_alphabet[(v >> 12) & 63];
            pbyDest[nIndex++] = base64_alphabet[64];
            pbyDest[nIndex++] = base64_alphabet[64];
            break;

        case 2:
            v = (p1[i] << 16) | (p1[i + 1] << 8);
            pbyDest[nIndex++] = base64_alphabet[v >> 18];
            pbyDest[nIndex++] = base64_alphabet[(v >> 12) & 63];
            pbyDest[nIndex++] = base64_alphabet[(v >> 6) & 63];
            pbyDest[nIndex++] = base64_alphabet[64];
            break;

        default:
            v = (p1[i] << 16) | (p1[i + 1] << 8) | p1[i + 2];
            pbyDest[nIndex++] = base64_alphabet[v >> 18];
            pbyDest[nIndex++] = base64_alphabet[(v >> 12) & 63];
            pbyDest[nIndex++] = base64_alphabet[(v >> 6) & 63];
            pbyDest[nIndex++] = base64_alphabet[v & 63];
            break;
        }
    }
    if ( pbyDest && nIndex < nMaxLen )
    {
        pbyDest[nIndex] = '\0';
    }
    return (nSrcLen+2)/3*4;
}

int Base64Decode( const BYTE* pbySrc, const int nSrcLen, void* pDest, const int nMaxLen )
{
    const char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    UCHAR *p2 = (UCHAR *)pDest;
    ULONG i, v, n;
    UCHAR base64_table[256];

    for (i = 0; i < sizeof(base64_table); i++)
        base64_table[i] = 255;

    for (i = 0; i < 64; i++)
        base64_table[base64_alphabet[i]] = (char)i;

    for (i = 0, n = 0; i < nSrcLen; i++)
    {
        if (base64_table[pbySrc[i]] == 255)
            break;

        if ((ULONG)(p2 - (UCHAR *)pDest) >= nMaxLen)
            break;

        v = base64_table[pbySrc[i]] | (v << 6);
        n += 6;

        if (n >= 8)
        {
            n -= 8;
            *p2++ = (UCHAR)(v >> n);
        }
    }
    //计算末尾有几个 = 号( Base64转码中 ， =号是Encode时最末不足时候追加的字符) 
    int nEndCount = 0;
    for ( int j = nSrcLen - 1; j >= 0; j-- )
    {
        if ( pbySrc[j] == base64_alphabet[64] )
        {
            nEndCount++;
        }
        else
        {
            break;
        }
    }

    return nSrcLen/4 * 3 - nEndCount;
}

bool isContainsChinese(const std::wstring& str)
{
    int nSize = str.length();

    for (int i=0; i<nSize; i++)
    {
        if (str.at(i)>127)
            return true;
    }

    return false;
}

bool isContainsBackspace(const std::wstring& str)
{
    int nSize = str.length();

    for (int i=0; i<nSize; i++)
    {
        if (str.at(i)==L' ')
            return true;
    }

    return false;
}

bool isTheSamePath(const std::wstring& strPath1, const std::wstring& strPath2)
{
    int n1 = strPath1.length();
    int n2 = strPath2.length();
    int i  = 0;
    int j  = 0;

    while (i<n1&&j<n2)
    {
        while (i<n1 && (strPath1.at(i)==L'\\' || strPath1.at(i)==L'/')) i++;
        while (j<n2 && (strPath2.at(j)==L'\\' || strPath2.at(j)==L'/')) j++;
        if (strPath1.at(i) != strPath2.at(j))
            return false;
        i++;
        j++;
    }

    if (i==n1 && j==n2)
        return true;

    return false;
}

bool isInSubPath(const std::wstring& strSubPath, const std::wstring& strPath)
{
    int n1 = strSubPath.length();
    int n2 = strPath.length();
    int i  = 0;
    int j  = 0;

    while (i<n1&&j<n2)
    {
        while (i<n1 && (strSubPath.at(i)==L'\\' || strSubPath.at(i)==L'/')) i++;
        while (j<n2 && (strPath.at(j)==L'\\' || strPath.at(j)==L'/')) j++;
        if (strSubPath.at(i) != strPath.at(j))
            return false;
        i++;
        j++;
    }

    if (i==n1 && j<n2)
        return true;

    return false;
}

bool isInDir(const std::wstring& strFilePath, const std::wstring& strDir)
{
    std::wstring sDir = getFileDirFromPath(strFilePath);

    return isTheSamePath(sDir,strDir);
}

std::wstring getFileNameFromPath(const std::wstring& strFilePath)
{
    std::wstring strFileName;

    int  i;
    for (i=strFilePath.length()-1; i>0; i--)
    {
        if (strFilePath[i]==L'\\' || strFilePath[i]==L'/')
            break;
    }

    strFileName = strFilePath.substr(i+1,strFilePath.length()-i-1);

    return strFileName;
}

std::wstring getFileDirFromPath(const std::wstring& strFilePath)
{
    std::wstring strFileDir;

    int  i;
    for (i=strFilePath.length()-1; i>0; i--)
    {
        if (strFilePath[i]==L'\\' || strFilePath[i]==L'/')
            break;
    }

    strFileDir = strFilePath.substr(0,i);

    return strFileDir;
}
