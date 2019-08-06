#include <videoAudioTst.h>





void rtmp_audio_video()
{	//注册所有的编解码器, 封装器, 网络协议

    CUR;
    const char *outUrl = "rtmp://hdlcontrol.com/live/stream";
    MediaEncode *mediaEncode;
	OutMedia outMedia;
	AVPacket *aPkt;
	AVPacket *vPkt;
    CUR;
    // 视频测试
    mediaEncode = new MediaEncode();
    //std::thread vth(mediaEncode->init_vEncode());
//    if (!mediaEncode->init_vEncode()){
//        qWarning()<<"ERR: init_vEncode";
//        return;
//    }
    CUR;
#if 0
    if (!mediaEncode.init_avEncode()) {
		qDebug() << "ERR:init_audio";
		exit(0);
	}
#endif
    if (!outMedia.init_outMedia(string(outUrl), string("flv"))) {
        qWarning()<<"ERR: init_outMedia"<<endl;
        return;
    }
    CUR;
    /* 增加音频流和视频流 */
    //outMedia.addStream(mediaEncode.aEncodeCtx);
    outMedia.addStream(mediaEncode->_vEncodeCtx);
	outMedia.dump_outMediaFmt();
	outMedia.write_headerInfo();
    CUR;
	while(1)
	{
        //aPkt = mediaEncode.gain_aPkt();
        vPkt = mediaEncode->gain_vPkt();
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
