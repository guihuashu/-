#include <QApplication>
#include <iostream>
#include <MediaEncode.h>
#include <QThread>
#include <thread>
#include <videoAudioTst.h>
#include <ffmopr.h>

static const char *outUrl = "rtmp://hdlcontrol.com/live/stream";
static MediaEncode mediaEncode;
static OutMedia outMedia;
static QtVideoCap cap;





void init_ffmpeg()
{
}

void sendPkt()
{
    while(1)
    {
        AVPacket *aPkt;
        AVPacket *vPkt;
        //aPkt = mediaEncode.gain_aPkt();
        vPkt = mediaEncode.gain_vPkt();
        /* 必须要这样, 否则音频卡顿 */
        if (!aPkt && !vPkt) {
            continue;
        }
#if 0
        if (aPkt) {
            outMedia.send_aPkt(aPkt);
        }
#endif
        if (vPkt) {
            CUR;
            outMedia.send_vPkt(vPkt);
            CUR;
        }
        //av_usleep(1000 * 20);
    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    CUR;
    // 视频测试
    initFFM();
    cap.init();
    if (!mediaEncode.init_vEncode(&cap)){
        qWarning()<<"ERR: init_vEncode";
        return -1;
    }
    outMedia.init_outMedia(outUrl, "flv");
    /* 增加音频流和视频流 */
    //outMedia.addStream(mediaEncode.aEncodeCtx);
    outMedia.addStream(mediaEncode._vEncodeCtx);
    outMedia.dump_outMediaFmt();
    outMedia.write_headerInfo();
    std::thread th(sendPkt);
    th.detach();


    return a.exec();
}
