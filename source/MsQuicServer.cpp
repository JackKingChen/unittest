
#include <msquic.h>
#include <stdio.h>
#include <stdlib.h>

#include "MsQuicServer.h"

static const QUIC_BUFFER Alpn = { sizeof("sample") - 1, (uint8_t*)"sample" };

static const uint64_t IdleTimeoutMs = 10000;

const QUIC_API_TABLE* MsQuicServer::s_msQuicApi = nullptr;

MsQuicServer::MsQuicServer(const std::string& name, unsigned short port, const QUIC_REGISTRATION_CONFIG& regConfig)
    : m_svrName(name),m_udpPort(port),m_regConfig(regConfig)
{
}

MsQuicServer::~MsQuicServer(void)
{
    exit();
}

bool  MsQuicServer::serverLoadConfiguration(const char* Cert,const char* KeyFile)
{
    QUIC_SETTINGS Settings {0};

    Settings.IdleTimeoutMs = IdleTimeoutMs;
    Settings.IsSet.IdleTimeoutMs = TRUE;

    Settings.ServerResumptionLevel = QUIC_SERVER_RESUME_AND_ZERORTT;
    Settings.IsSet.ServerResumptionLevel = TRUE;
    //
    // Configures the server's settings to allow for the peer to open a single
    // bidirectional stream. By default connections are not configured to allow
    // any streams from the peer.
    //
    Settings.PeerBidiStreamCount = 2;
    Settings.IsSet.PeerBidiStreamCount = TRUE;

    QUIC_CREDENTIAL_CONFIG CredConfig;
    QUIC_CERTIFICATE_FILE CertFile;
    CertFile.CertificateFile = Cert;
    CertFile.PrivateKeyFile  = KeyFile;

    memset(&CredConfig,0,sizeof(QUIC_CREDENTIAL_CONFIG));
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

bool  MsQuicServer::init(const std::string& strCertFile, const std::string& strKeyFile)
{
    QUIC_STATUS Status = QUIC_STATUS_SUCCESS;

    if (QUIC_FAILED(Status = MsQuicOpen(&s_msQuicApi))) {
        printf("MsQuicOpen failed, 0x%x!\n", Status);
        return false;
    }
    if (QUIC_FAILED(Status = s_msQuicApi->RegistrationOpen(&m_regConfig, &m_registration))) {
        printf("RegistrationOpen failed, 0x%x!\n", Status);
        MsQuicClose(s_msQuicApi);
        s_msQuicApi = nullptr;
        return false;
    }

    if (!serverLoadConfiguration(strCertFile.c_str(),strKeyFile.c_str())) {
        return false;
    }

    return true;
}

void  MsQuicServer::exit(void)
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

QUIC_STATUS MsQuicServer::serverConnectionCallback(HQUIC connection,void* contex,QUIC_CONNECTION_EVENT* Event)
{
    MsQuicServer*pThis = (MsQuicServer*)contex;

    switch (Event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED:
        printf("[conn][%p] Connected\n", connection);
        s_msQuicApi->ConnectionSendResumptionTicket(connection, QUIC_SEND_RESUMPTION_FLAG_NONE, 0, NULL);
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        printf("[conn][%p] Shut down by transport, 0x%x\n", connection, Event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status);
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        printf("[conn][%p] Shut down by peer, 0x%llu\n", connection, (unsigned long long)Event->SHUTDOWN_INITIATED_BY_PEER.ErrorCode);
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        printf("[conn][%p] All done\n", connection);
        s_msQuicApi->ConnectionClose(connection);
        break;
    case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
        printf("[strm][%p] Peer started\n", Event->PEER_STREAM_STARTED.Stream);
        //s_msQuicApi->SetCallbackHandler(Event->PEER_STREAM_STARTED.Stream, (void*)serverStreamCallback, Connection);
        return pThis->onPeerStreamStarted(connection,Event->PEER_STREAM_STARTED.Stream);

        break;
    case QUIC_CONNECTION_EVENT_RESUMED:
        printf("[conn][%p] Connection resumed!\n", connection);
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS MsQuicServer::serverListenCallback(HQUIC listener,void* contex,QUIC_LISTENER_EVENT* event)
{
    MsQuicServer* pThis = (MsQuicServer*)contex;

    QUIC_STATUS Status = QUIC_STATUS_NOT_SUPPORTED;
    switch (event->Type) {
    case QUIC_LISTENER_EVENT_NEW_CONNECTION:
        //
        // A new connection is being attempted by a client. For the handshake to
        // proceed, the server must provide a configuration for QUIC to use. The
        // app MUST set the callback handler before returning.
        //
        pThis->s_msQuicApi->SetCallbackHandler(event->NEW_CONNECTION.Connection, (void*)serverConnectionCallback, pThis);
        Status = pThis->s_msQuicApi->ConnectionSetConfiguration(event->NEW_CONNECTION.Connection, pThis->m_configuration);
        break;
    default:
        break;
    }
    return Status;
}

QUIC_STATUS MsQuicServer::onPeerStreamStarted(HQUIC connection, HQUIC stream)
{
    std::lock_guard<std::mutex> guard(m_streamMutex);

    /*FIXME: stream will delete itself when it closed,it may not safe*/
    MsQuicStream* pStream = new MsQuicStream(s_msQuicApi,connection);

    pStream->open(stream,m_eventcallback);

    m_streamEventQueue.push(pStream);

    m_streamCondi.notify_all();

    return QUIC_STATUS_SUCCESS;
}

MsQuicStream* MsQuicServer::accept(const func_stream_event& callback)
{
    MsQuicStream* stream = nullptr;

    m_eventcallback = callback;

    std::unique_lock<std::mutex> lock(m_streamMutex);
    while (m_streamEventQueue.empty()) {
        m_streamCondi.wait(lock);
    }

    stream = m_streamEventQueue.front();
    m_streamEventQueue.pop();

    return stream;
}

void  MsQuicServer::destoryStream(const MsQuicStream* stream)
{
    delete stream;
}

void  MsQuicServer::start(void)
{
    m_asyncFuture = std::async(std::launch::async,[this] {
        QUIC_STATUS     status;
        HQUIC Listener = nullptr;

        QUIC_ADDR Address = {};
        QuicAddrSetFamily(&Address, QUIC_ADDRESS_FAMILY_UNSPEC);
        QuicAddrSetPort(&Address, m_udpPort);

        if (QUIC_FAILED(status = s_msQuicApi->ListenerOpen(m_registration, serverListenCallback, this, &Listener))) {
            printf("ListenerOpen failed, 0x%x!\n", status);
            return false;
        }

        if (QUIC_FAILED(status = s_msQuicApi->ListenerStart(Listener, &Alpn, 1, &Address))) {
            printf("ListenerStart failed, 0x%x!\n", status);
            s_msQuicApi->ListenerClose(Listener);
            return false;
        }

        while (m_bKeepRunning) {
            usleep(5000);
        }

        if (Listener != nullptr) {
            s_msQuicApi->ListenerClose(Listener);
        }
        return  true;
    });

}

void  MsQuicServer::stop(void)
{
    m_bKeepRunning = false;
    m_asyncFuture.get();
}
