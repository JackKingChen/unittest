#ifndef NDVIDEOSESSION_H
#define NDVIDEOSESSION_H

#include <future>

class NdVideoSession
{
public:
    NdVideoSession();
    ~NdVideoSession();
    bool init(int width, int height, float frame_rate, float time_unit, int max_payload_size, int video_sock);
    bool set_socket(int sock);
    bool start();
    bool stop();
private:
    bool                        m_startFlags;
    bool                        m_initFlags;
    bool                        m_startStatus;
    int                         m_width;
    int                         m_height;
    float                       m_frameRate;
    float                       m_timeUnit;
    int                         m_maxPayloadSize;
    std::future<void>           m_videoHandle;
    bool                        m_runFlags;
    int                         m_videoSock;
};


#endif // NDVIDEOSESSION_H