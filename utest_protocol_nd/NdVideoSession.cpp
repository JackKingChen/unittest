#include "NdVideoSession.h"
#include "NdHardLink.h"
#include "headers.h"
#include <iostream>


NdVideoSession::NdVideoSession()
    : m_startFlags(false)
    , m_initFlags(false)
    , m_startStatus(false)
    , m_width(-1)
    , m_height(-1)
    , m_frameRate(30.0f)
    , m_timeUnit(1.0 / 90000.0)
    , m_maxPayloadSize(64000)
    , m_runFlags(false)
    , m_videoSock(-1)
{

}

NdVideoSession::~NdVideoSession()
{
}

bool NdVideoSession::init(int width, int height, float frame_rate, float time_unit, int max_payload_size, int video_sock)
{
    if(m_initFlags)
    {
        std::cout << "[NdVideoSession]" << " already inited" << std::endl; 
        return false;
    }
    bool result      = false;
    int  result_int  = 0;
    m_width          = width;
    m_height         = height;
    m_frameRate      = frame_rate;
    m_timeUnit       = time_unit;
    m_maxPayloadSize = max_payload_size;
    m_runFlags       = false;
    m_videoSock      = video_sock;

    m_initFlags      = true;

    return true;
}

bool NdVideoSession::set_socket(int socket)
{
    // TODO : 
    return false;
}

bool NdVideoSession::start()
{
    printf("Ln:%d[%s] Enter\r\n", __LINE__, __FUNCTION__);
    
    if(m_initFlags)
    {
        m_videoHandle = std::async(std::launch::async, [this]{
            jrtplib::RTPSession rtp_sess;
            jrtplib::RTPSessionParams sess_params;
            sess_params.SetProbationType(jrtplib::RTPSources::NoProbation);
            sess_params.SetOwnTimestampUnit(m_timeUnit);
            sess_params.SetMaximumPacketSize(m_maxPayloadSize + 12);    // account for RTP header

            printf("Ln:%d[%s] Enter\r\n", __LINE__, __FUNCTION__);
            
            jrtplib::RTPAbortDescriptors descriptor;
            int status = descriptor.Init();
            if(status < 0)
            {
                std::cout << "[NdVideoSession] RTPAbortDescriptor : " << jrtplib::RTPGetErrorString(status) << std::endl;
                return;
            }

            jrtplib::RTPTCPTransmissionParams trans_params;
            trans_params.SetCreatedAbortDescriptors(&descriptor);

            status = rtp_sess.Create(sess_params, &trans_params, jrtplib::RTPTransmitter::TCPProto);
            if(status < 0) {
                std::cout << "[NdVideoSession] RTPSession Create : " << jrtplib::RTPGetErrorString(status) << std::endl;
                return;
            }

            
            bool result = false;
            MIPAverageTimer avgTimer(MIPTime(1.0 / m_frameRate));
            std::cout << "MIPAverageTimer OK" << std::endl;


            MIPQuartzCaptureScreen input;
            result = input.init(MIPRAWVIDEOMESSAGE_TYPE_BGRA32, true);
            if(!result){
                std::cout << input.getErrorString() << std::endl;
                return;
            }
            std::cout << "MIPQuartzCaptureScreen OK" << std::endl;

            MIPAVCodecEncoder::initAVCodec();
            MIPAVCodecFrameConverter convert;
            result = convert.init(m_width, m_height, MIPRAWVIDEOMESSAGE_TYPE_YUV420P);
            if(!result) {
                std::cout << convert.getErrorString() << std::endl;
                return;
            }
            std::cout << "MIPAVCodecFrameConverter OK " << std::endl;

            MIPAVCodecEncoder::initAVCodec();
            MIPAVCodecEncoder encoder;
            result = encoder.init(m_width, m_height, m_frameRate, 6000000, MIPAVCodecEncoder::CT_H264);
            if(!result) {
                std::cout << encoder.getErrorString() << std::endl;
                return;
            }
            std::cout << "MIPAVCodecEncoder OK" << std::endl;


            MIPRTPVideoEncoder rtppack;
            result = rtppack.init(m_frameRate, m_maxPayloadSize, MIPRTPVideoEncoder::H264);
            if(!result)
            {
                std::cout << rtppack.getErrorString() << std::endl;
                return;
            }
            rtppack.setPayloadType(103);
            std::cout << "MIPRTPVideoEncoder OK" << std::endl;

            MIPRTPComponent rtp;
            result = rtp.init(&rtp_sess);
            if(!result)
            {
                std::cout << rtp.getErrorString() << std::endl;
                return;
            }
            std::cout << "MIPRTPComponent OK" << std::endl;
#if 0
            MIPDumpFile dumpfile;
            result = dumpfile.init("~/dumpfile");
            if (!result)
            {
                std::cout << "error message : " << dumpfile.getComponentName() << "=>" << dumpfile.getErrorString();
                return;
            }
#endif

            NdHardLink link("mac-screen-video-link", [](const std::string& error_link, const std::string& error_comp, const std::string& error_desc){
                    std::cout << "error link : " << error_link  << "\n"
                              << "\t error_component :" << error_comp << "\n"
                              << "\t error description : " << error_desc << std::endl; 
            });
            
            result = link.setChainStart(&avgTimer);
            if(!result)
            {
                std::cout << link.getErrorString() << std::endl;
                return;
            }
            result = link.addConnection(&avgTimer, &input);
            if(!result)
            {
                std::cout << link.getErrorString() << std::endl;
                return;
            }
            result = link.addConnection(&input, &convert);
            if(!result)
            {
                std::cout << link.getErrorString() << std::endl;
                return;
            }
            result = link.addConnection(&convert, &encoder);
            if(!result)
            {
                std::cout << link.getErrorString() << std::endl;
                return;
            }
            result = link.addConnection(&encoder, &rtppack);
            if(!result)
            {
                std::cout << link.getErrorString() << std::endl;
                return;
            }
            result = link.addConnection(&rtppack, &rtp);
            if(!result)
            {
                std::cout << link.getErrorString() << std::endl;
                return;
            }
            #if 0
            result = link.addConnection(&rtp, &dumpfile);
            if(!result)
            {
                std::cout << link.getErrorString() << std::endl;
                return;
            }
            #endif

            std::cout << "Mac Screen Video Link Ok" << std::endl;
            rtp_sess.AddDestination(jrtplib::RTPTCPAddress(m_videoSock));
            result = link.start();
            if(!result) {
                link.getErrorString();
                return;
            }
            m_runFlags = true;
            while(m_runFlags) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            std::cout << "tear down" << std::endl;
        });

        return true;
    }
    else{
        std::cout << "[NdVideoSession][WARNING]video session already start" << std::endl;
        return false;
    }
}

bool NdVideoSession::stop()
{
    if(m_initFlags && m_runFlags){
        m_runFlags = false;
        m_videoHandle.get();
        std::cout << "[NdVideoSession][Info] stop ok" << std::endl;
        return true;
    }
    return false;
}
