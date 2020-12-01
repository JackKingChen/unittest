
#include "MsQuicClient.h"

#define LOGE(format,...)  printf("MsQuicClient:Error:" format,##__VA_ARGS__)
#define LOGI(format,...)  printf("MsQuicClient:INFO:" format,##__VA_ARGS__)

MsQuicClient::MsQuicClient(const std::string& name) : m_name(name)
{
    QUIC_STATUS status = QUIC_STATUS_SUCCESS;

    /*initializes the MsQuic library and returns a the API function table.*/
    if (QUIC_FAILED(status = MsQuicOpen(&m_msQuic)))
    {
        LOGE("MsQuicOpen failed, 0x%x!\n", status);
        return;
    }

    /*register connections*/
    if (QUIC_FAILED(status = m_msQuic->RegistrationOpen(&m_regConfig, &m_registration))) {
        LOGE("RegistrationOpen failed, 0x%x!\n", status);
        return;
    }

    /*client default config*/
    QUIC_SETTINGS Settings {0};
    Settings.IdleTimeoutMs = 10000;
    Settings.IsSet.IdleTimeoutMs = true;

    QUIC_CREDENTIAL_CONFIG CredConfig;
    memset(&CredConfig, 0, sizeof(CredConfig));
    CredConfig.Type   = QUIC_CREDENTIAL_TYPE_NONE;
    CredConfig.Flags  = QUIC_CREDENTIAL_FLAG_CLIENT;
    /*unsecuret*/
    CredConfig.Flags |= QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION;

    QUIC_BUFFER Alpn = { sizeof("sample") - 1, (uint8_t*)"sample" };
    if (QUIC_FAILED(status = m_msQuic->ConfigurationOpen(m_registration, &Alpn, 1, &Settings, sizeof(Settings), nullptr, &m_configration))) {
        LOGE("ConfigurationOpen failed, 0x%x!\n", status);
        return;
    }

    if (QUIC_FAILED(status = m_msQuic->ConfigurationLoadCredential(m_configration, &CredConfig))) {
        LOGE("ConfigurationLoadCredential failed, 0x%x!\n", status);
        return;
    }
}
MsQuicClient::~MsQuicClient(void)
{
    if (m_msQuic)
    {
        if (m_connection)
            m_msQuic->ConnectionClose(m_connection);
        if (m_configration)
            m_msQuic->ConfigurationClose(m_configration);
        if (m_registration)
            m_msQuic->RegistrationClose(m_registration);

        MsQuicClose(m_msQuic);

        m_connection   = nullptr;
        m_configration = nullptr;
        m_registration = nullptr;
        m_msQuic       = nullptr;
        
    }    
}

QUIC_STATUS MsQuicClient::clientConnectionCallback(HQUIC connection,void* Context ,QUIC_CONNECTION_EVENT* event)
{
    MsQuicClient* pThis = (MsQuicClient*)Context;

    switch (event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED:
        LOGI("[conn][%p] connected\n", connection);
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        LOGI("[conn][%p] Shut down by transport, 0x%x\n", connection, event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status);
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        LOGI("[conn][%p] Shut down by peer, 0x%llu\n", connection, (unsigned long long)event->SHUTDOWN_INITIATED_BY_PEER.ErrorCode);
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        LOGI("[conn][%p] All done\n", connection);
        if (!event->SHUTDOWN_COMPLETE.AppCloseInProgress) {
            pThis->m_msQuic->ConnectionClose(connection);
        }
        break;
    case QUIC_CONNECTION_EVENT_RESUMPTION_TICKET_RECEIVED:
        LOGI("[conn][%p] Resumption ticket received (%u bytes):\n", connection, event->RESUMPTION_TICKET_RECEIVED.ResumptionTicketLength);
        for (uint32_t i = 0; i < event->RESUMPTION_TICKET_RECEIVED.ResumptionTicketLength; i++) {
            printf("%.2X", (uint8_t)event->RESUMPTION_TICKET_RECEIVED.ResumptionTicket[i]);
        }
        printf("\n");
        break;
    default:
        break;
    }

    if (!pThis->m_bConnected)
    {
        if (event->Type == QUIC_CONNECTION_EVENT_CONNECTED)
        {
            pThis->m_bConnected = true;
            pThis->m_condConn.notify_all();
        }
        else
        {
            pThis->m_bConnectedFailed = true;
        }
    }

    if (pThis->m_connectCallback)
        return pThis->m_connectCallback(event);

    return QUIC_STATUS_SUCCESS;
}

bool MsQuicClient::connect(const char* target, unsigned short port, const func_connect_event& callback)
{
    QUIC_STATUS Status = QUIC_STATUS_SUCCESS;

    m_connectCallback = callback;

    if (QUIC_FAILED(Status = m_msQuic->ConnectionOpen(m_registration, clientConnectionCallback, this, &m_connection))) {
        printf("ConnectionOpen failed, 0x%x!\n", Status);
        return false;
    }

    LOGI("[m_conn][%p] connected\n", m_connection);

    if (QUIC_FAILED(Status = m_msQuic->ConnectionStart(m_connection, m_configration, QUIC_ADDRESS_FAMILY_UNSPEC, target, port))) {
        printf("ConnectionStart failed, 0x%x!\n", Status);
        m_msQuic->ConnectionClose(m_connection);
        m_connection = nullptr;
        return false;
    }

    return true;
}

bool MsQuicClient::waitForConnected(int timeout)
{
    std::cv_status retval = std::cv_status::no_timeout;

    std::unique_lock<std::mutex> lock(m_connMutex);
    while (!m_bConnected && !m_bConnectedFailed) {
        if (timeout>=0)
            retval = m_condConn.wait_for(lock,std::chrono::milliseconds(timeout));
        else
            m_condConn.wait(lock);
    }

    if (retval == std::cv_status::timeout)
        return false;

    return m_bConnectedFailed;
}

MsQuicStream* MsQuicClient::createStream(const func_stream_event& callback)
{
    if (m_connection)
    {
        MsQuicStream* pStream = new MsQuicStream(m_msQuic,m_connection);

        if (pStream->open(nullptr,callback))
            return pStream;

        delete pStream;
    }
    else
        printf("not connected\n");

    return nullptr;
}
void  MsQuicClient::destoryStream(const MsQuicStream* stream)
{
    delete stream;
}
