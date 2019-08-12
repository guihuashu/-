#include <frameList.h>

FrameList::FrameList(unsigned int listSize):
    _listSize(listSize)
{
    _frameList.clear();
}


// 新包压入链表尾部, 超出长度将从前面删除
void FrameList::pushBackFrame(AVFrame *frame)
{
    if (!frame){
        qWarning()<<"pkt is NULL";
        return;
    }
    _mutex.lock();
    while (_frameList.size() >= _listSize){   // 链表满了
        if (_frameList.size() < 1)
            break;
        // 从前面删除包
        AVFrame *fm = _frameList.front();  // 取出第一个包
        _frameList.pop_front();              // 删除第一个节点
        av_frame_unref(fm);
    }
    _frameList.push_back(frame);    // 包压入链表尾部
    //qInfo()<<"_list->size() = "<<_frameList.size();
    _mutex.unlock();
}

// 取出第一个包
AVFrame *FrameList::fontFrame()
{
    if (_frameList.size() < 1) {
        return NULL;
    }
    _mutex.lock();
    AVFrame *frame = _frameList.front();	// 取出第一包
    _frameList.pop_front();				// 删除第一个节点
    _mutex.unlock();
    return frame;
}

unsigned int FrameList::size()
{
    _mutex.lock();
    return _frameList.size();
    _mutex.unlock();
}

