#include "Channel.h"

struct Channel* channelInit(int fd, int events, handleFunc readHandler, handleFunc writeHandler, void* arg)
{
	struct Channel* channel = (struct Channel*)malloc(sizeof(struct Channel));
	if (channel == nullptr) {
		return nullptr; // �ڴ����ʧ��
	}
	channel->fd = fd;
	channel->events = events;
	channel->readHandler = readHandler;
	channel->writeHandler = writeHandler;
	channel->arg = arg;
	return channel;
}

void writeEventEnable(struct Channel* channel, bool flag)
{
	if (flag) {
		channel->events |= WriteEvent; // ����д�¼�
	} else {
		channel->events &= ~WriteEvent; // ����д�¼�
	}
}

bool isWriteEventEnable(struct Channel* channel)
{
	return channel->events & WriteEvent; // ����Ƿ�����д�¼�
}
