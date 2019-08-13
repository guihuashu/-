#ifndef PKTLIST_H
#define PKTLIST_H

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavcodec/mediacodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/opt.h>
    #include <libavutil/dict.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavutil/time.h>
}
#include <iostream>
#include <list>
#include <ffmopr.h>
#include <debug.h>

class PktList{
public:
    PktList(unsigned int listSize);
    void pushBackPkt(AVPacket *pkt);     // 新包压入链表尾部, 超出长度将从前面删除
    AVPacket *fontPkt();                 // 取出第一个包
    unsigned int size();
public:
    std::list<AVPacket *> _pktList;
    std::mutex _mutex;
    unsigned int _listSize;
};


#endif // PKTLIST_H
