#ifndef FRAME_LIST_H
#define FRAME_LIST_H

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

class FrameList{
public:
    FrameList(unsigned int listSize);
    void pushBackFrame(AVFrame *frame);     // 新包压入链表尾部, 超出长度将从前面删除
    AVFrame *fontFrame();                 // 取出第一个包
    unsigned int size();
public:
    std::list<AVFrame *> _frameList;
    std::mutex _mutex;
    unsigned int _listSize;
};


#endif // FRAME_LIST_H
