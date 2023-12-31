
#include <msquic.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include "CaeQuicServer.h"

#include <algorithm>

static const QUIC_BUFFER Alpn = { sizeof("sample") - 1, (uint8_t*)"sample" };
static const uint64_t IdleTimeoutMs = 10000;
const QUIC_API_TABLE* s_msQuicApi = nullptr;

CaeQuicServer::CaeQuicServer(const std::string& name, unsigned short port)
    : m_svrName(name), m_udpPort(port)
{
    m_regConfig.AppName = m_svrName.c_str();
    m_regConfig.ExecutionProfile = QUIC_EXECUTION_PROFILE_LOW_LATENCY;
}

CaeQuicServer::~CaeQuicServer(void)
{
    this->exit();
}

bool  CaeQuicServer::serverLoadConfiguration(const char* Cert, const char* KeyFile)
{
    QUIC_SETTINGS Settings{ 0 };

    Settings.IdleTimeoutMs = IdleTimeoutMs;
    Settings.IsSet.IdleTimeoutMs = TRUE;

    Settings.ServerResumptionLevel = QUIC_SERVER_RESUME_AND_ZERORTT;
    Settings.IsSet.ServerResumptionLevel = TRUE;

    Settings.PeerBidiStreamCount = 2;
    Settings.IsSet.PeerBidiStreamCount = TRUE;

    QUIC_CREDENTIAL_CONFIG CredConfig;
    QUIC_CERTIFICATE_FILE CertFile;
    CertFile.CertificateFile = Cert;
    CertFile.PrivateKeyFile = KeyFile;

    memset(&CredConfig, 0, sizeof(QUIC_CREDENTIAL_CONFIG));
    CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
    CredConfig.CertificateFile = &CertFile;

    QUIC_STATUS Status = QUIC_STATUS_SUCCESS;
    if (QUIC_FAILED(Status = s_msQuicApi->ConfigurationOpen(m_registration, &Alpn, 1, &Settings, sizeof(Settings), nullptr, &m_configuration))) {
        printf("ConfigurationOpen failed, 0x%x!\n", Status);
        return false;
    }

    if (QUIC_FAILED(Status = s_msQuicApi->ConfigurationLoadCredential(m_configuration, &CredConfig))) {
        printf("ConfigurationLoadCredential failed, 0x%x!\n", Status);
        return false;
    }

    return true;
}

bool  CaeQuicServer::init(const std::string& strCertFile, const std::string& strKeyFile)
{
    QUIC_STATUS Status = QUIC_STATUS_SUCCESS;

    if (QUIC_FAILED(Status = MsQuicOpen2(&s_msQuicApi))) {
        printf("MsQuicOpen failed, 0x%x!\n", Status);
        return false;
    }
    if (QUIC_FAILED(Status = s_msQuicApi->RegistrationOpen(&m_regConfig, &m_registration))) {
        printf("RegistrationOpen failed, 0x%x!\n", Status);
        MsQuicClose(s_msQuicApi);
        s_msQuicApi = nullptr;
        return false;
    }

    if (!serverLoadConfiguration(strCertFile.c_str(), strKeyFile.c_str())) {
        return false;
    }

    return true;
}

void  CaeQuicServer::exit(void)
{
    if (s_msQuicApi != nullptr) {
        if (m_configuration != nullptr) {
            s_msQuicApi->ConfigurationClose(m_configuration);
            m_configuration = nullptr;
        }
        if (m_registration != nullptr) {
            s_msQuicApi->RegistrationClose(m_registration);
            m_registration = nullptr;
        }
        MsQuicClose(s_msQuicApi);
        s_msQuicApi = nullptr;
    }
}

QUIC_STATUS CaeQuicServer::serverConnectionCallback(HQUIC connection, void* contex, QUIC_CONNECTION_EVENT * event)
{
    CaeQuicServer* pThis = (CaeQuicServer*)contex;
    return pThis->onConnectionCallback(connection, event);
}

QUIC_STATUS CaeQuicServer::serverListenCallback(HQUIC listener, void* contex, QUIC_LISTENER_EVENT * event)
{
    CaeQuicServer* pThis = (CaeQuicServer*)contex;

    QUIC_STATUS Status = QUIC_STATUS_NOT_SUPPORTED;
    switch (event->Type) {
    case QUIC_LISTENER_EVENT_NEW_CONNECTION:
        s_msQuicApi->SetCallbackHandler(event->NEW_CONNECTION.Connection, (void*)serverConnectionCallback, pThis);
        Status = s_msQuicApi->ConnectionSetConfiguration(event->NEW_CONNECTION.Connection, pThis->m_configuration);
        break;
    default:
        break;
    }
    return Status;
}

bool  CaeQuicServer::start(const std::function<void(HQUIC)>& funcConnCb)
{
    QUIC_STATUS     status;

    QUIC_ADDR Address = {};
    QuicAddrSetFamily(&Address, QUIC_ADDRESS_FAMILY_UNSPEC);
    QuicAddrSetPort(&Address, m_udpPort);

    m_connectedCallback = funcConnCb;

    if (QUIC_FAILED(status = s_msQuicApi->ListenerOpen(m_registration, serverListenCallback, this, &m_listener))) {
        printf("ListenerOpen failed, 0x%x!\n", status);
        return false;
    }

    if (QUIC_FAILED(status = s_msQuicApi->ListenerStart(m_listener, &Alpn, 1, &Address))) {
        printf("ListenerStart failed, 0x%x!\n", status);
        s_msQuicApi->ListenerClose(m_listener);
        m_listener = nullptr;
        return false;
    }

    return  true;
}

void  CaeQuicServer::stop(void)
{
    if (m_listener != nullptr) {
        s_msQuicApi->ListenerClose(m_listener);
        m_listener = nullptr;
    }
}

QUIC_STATUS CaeQuicServer::onConnectionCallback(HQUIC connection, QUIC_CONNECTION_EVENT* event)
{
    switch (event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED:
        printf("[conn][%p] Connected\n", connection);
        s_msQuicApi->ConnectionSendResumptionTicket(connection, QUIC_SEND_RESUMPTION_FLAG_NONE, 0, NULL);
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        if (event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status == QUIC_STATUS_CONNECTION_IDLE) {
            printf("[conn][%p] Successfully shut down on idle.\n", connection);
        }
        else {
            printf("[conn][%p] Shut down by transport, 0x%x\n", connection, event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status);
        }
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        printf("[conn][%p] Shut down by peer, 0x%llu\n", connection, (unsigned long long)event->SHUTDOWN_INITIATED_BY_PEER.ErrorCode);
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        printf("[conn][%p] All done\n", connection);
        s_msQuicApi->ConnectionClose(connection);
        break;
    case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
        printf("[strm][%p] Peer started\n", event->PEER_STREAM_STARTED.Stream);
        if (m_connectedCallback) {
            m_connectedCallback(event->PEER_STREAM_STARTED.Stream);
        }
        break;
    case QUIC_CONNECTION_EVENT_RESUMED:
        printf("[conn][%p] Connection resumed!\n", connection);
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}


