
#include "unittest.h"

#include <VideoToolbox/VideoToolbox.h>
#include <CoreVideo/CoreVideo.h>
#include <CoreMedia/CoreMedia.h>

#if 0
UNITTEST(videotoolbx)
{
    dispatch_sync(cEncodeQueue, ^{

            frameID = 0;

            int width = 480,height = 640;

            //1.调用VTCompressionSessionCreate创建编码session
            //参数1：NULL 分配器,设置NULL为默认分配
            //参数2：width
            //参数3：height
            //参数4：编码类型,如kCMVideoCodecType_H264
            //参数5：NULL encoderSpecification: 编码规范。设置NULL由videoToolbox自己选择
            //参数6：NULL sourceImageBufferAttributes: 源像素缓冲区属性.设置NULL不让videToolbox创建,而自己创建
            //参数7：NULL compressedDataAllocator: 压缩数据分配器.设置NULL,默认的分配
            //参数8：回调  当VTCompressionSessionEncodeFrame被调用压缩一次后会被异步调用.注:当你设置NULL的时候,你需要调用VTCompressionSessionEncodeFrameWithOutputHandler方法进行压缩帧处理,支持iOS9.0以上
            //参数9：outputCallbackRefCon: 回调客户定义的参考值
            //参数10：compressionSessionOut: 编码会话变量
            OSStatus status = VTCompressionSessionCreate(NULL, width, height, kCMVideoCodecType_H264, NULL, NULL, NULL, didCompressH264, (__bridge void *)(self), &cEncodeingSession);

            NSLog(@"H264:VTCompressionSessionCreate:%d",(int)status);

            if (status != 0) {

                NSLog(@"H264:Unable to create a H264 session");
                return ;
            }

            //设置实时编码输出（避免延迟）
            VTSessionSetProperty(cEncodeingSession, kVTCompressionPropertyKey_RealTime, kCFBooleanTrue);
            VTSessionSetProperty(cEncodeingSession, kVTCompressionPropertyKey_ProfileLevel,kVTProfileLevel_H264_Baseline_AutoLevel);

            //是否产生B帧(因为B帧在解码时并不是必要的,是可以抛弃B帧的)
            VTSessionSetProperty(cEncodeingSession, kVTCompressionPropertyKey_AllowFrameReordering, kCFBooleanFalse);

            //设置关键帧（GOPsize）间隔，GOP太小的话图像会模糊
            int frameInterval = 10;
            CFNumberRef frameIntervalRaf = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &frameInterval);
            VTSessionSetProperty(cEncodeingSession, kVTCompressionPropertyKey_MaxKeyFrameInterval, frameIntervalRaf);

            //设置期望帧率，不是实际帧率
            int fps = 10;
            CFNumberRef fpsRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &fps);
            VTSessionSetProperty(cEncodeingSession, kVTCompressionPropertyKey_ExpectedFrameRate, fpsRef);

            //码率的理解：码率大了话就会非常清晰，但同时文件也会比较大。码率小的话，图像有时会模糊，但也勉强能看
            //码率计算公式，参考印象笔记
            //设置码率、上限、单位是bps
            int bitRate = width * height * 3 * 4 * 8;
            CFNumberRef bitRateRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &bitRate);
            VTSessionSetProperty(cEncodeingSession, kVTCompressionPropertyKey_AverageBitRate, bitRateRef);

            //设置码率，均值，单位是byte
            int bigRateLimit = width * height * 3 * 4;
            CFNumberRef bitRateLimitRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &bigRateLimit);
            VTSessionSetProperty(cEncodeingSession, kVTCompressionPropertyKey_DataRateLimits, bitRateLimitRef);
            //开始编码=    VTCompressionSessionPrepareToEncodeFrames(cEncodeingSession);
        });
}
#endif

