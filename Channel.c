#include "Channel.h"

struct Channel* channelInit(int fd, int events, handleFunc readHandler, handleFunc writeHandler, void* arg)
{
	struct Channel* channel = (struct Channel*)malloc(sizeof(struct Channel));
	if (channel == nullptr) {
		return nullptr; // 内存分配失败
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
		channel->events |= WriteEvent; // 启用写事件
	} else {
		channel->events &= ~WriteEvent; // 禁用写事件
	}
}

bool isWriteEventEnable(struct Channel* channel)
{
	return channel->events & WriteEvent; // 检查是否启用写事件
}
