#include "pktlist.h"

PktList::PktList(unsigned int listSize):
    _listSize(listSize)
{
    _pktList.clear();
}


// 新包压入链表尾部, 超出长度将从前面删除
void PktList::pushBackPkt(AVPacket *pkt)
{
    if (!pkt){
        qWarning()<<"pkt is NULL";
        return;
    }

    _mutex.lock();
    while (_pktList.size() >= _listSize){   // 链表满了
        CUR;
        if (_pktList.size() < 1)
            break;
        // 从前面删除包
        AVPacket *pkt = _pktList.front();  // 取出第一个包
        _pktList.pop_front();              // 删除第一个节点
        FFmOpr::freePkt(&pkt);             // 释放包
    }
    _pktList.push_back(pkt);    // 包压入链表尾部
    qInfo()<<"_list->size() = "<<_pktList.size();
    _mutex.unlock();
}

// 取出第一个包
AVPacket *PktList::fontPkt()
{
    if (_pktList.size() < 1) {
        return NULL;
    }
    _mutex.lock();
    AVPacket *pkt = _pktList.back();	// 取出第一包
    _pktList.pop_back();				// 删除第一个节点
    _mutex.unlock();
    return pkt;
}

unsigned int PktList::size()
{
    return _pktList.size();
}

