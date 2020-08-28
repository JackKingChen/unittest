#ifndef __NDHARDLINK_H__
#define __NDHARDLINK_H__

#include "headers.h"
#include <iostream>
#include <sstream>
#include <string>
#include <functional>



#ifdef _WIN32
    #define close_socket(x) closesocket(x)
#else
    #define close_socket(x) close(x)
#endif // _WIn32

typedef std::function<void(const std::string&, const std::string& , const std::string&)> FUNC_ERR_CALLBACK; 


class NdHardLink : public MIPComponentChain
{
public:
    NdHardLink(const std::string& chainName, FUNC_ERR_CALLBACK callback)
        : MIPComponentChain(chainName)
        , m_errCallback(callback)
    {

    }

    void onThreadExit(bool error, const std::string& errorComponent, const std::string& errorDescription)
    {
        if(!error) {
            m_errCallback(getName(), "", "Success Exit!");
            return;
        }
        m_errCallback(getName(), errorComponent, errorDescription);
    }

private:
    FUNC_ERR_CALLBACK   m_errCallback;
};



#endif // __NDHARDLINK_H__