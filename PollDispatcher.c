#include "Dispatcher.h"
#include <sys/poll.h>


#define Max 1024 // 最大事件数

struct PollData {
	int maxfd; // 最大文件描述符
	struct pollfd fds[Max]; // pollfd数组
};

static void* pollInit();

static int pollAdd(struct Channel* channel, struct EventLoop* evLoop);

static int pollRemove(struct Channel* channel, struct EventLoop* evLoop);

static int pollModify(struct Channel* channel, struct EventLoop* evLoop);

static int pollDispatch(struct EventLoop* evLoop, int timeout);

static int pollClear(struct EventLoop* evLoop);


struct Dispatcher PollDispatcher = {
	pollInit,
	pollAdd,
	pollRemove,
	pollModify,
	pollDispatch,
	pollClear
};

static void* pollInit()
{
	struct PollData* data = (struct PollData*)malloc(sizeof(struct PollData));
	data->maxfd = 0; // 初始化最大文件描述符为0
	for (int i = 0; i < Max; i++) {
		data->fds[i].fd = -1; // 初始化所有文件描述符为-1
		data->fds[i].events = 0; // 初始化事件为0
		data->fds[i].revents = 0; // 初始化返回事件为0
	}
	return data;
}


static int pollAdd(struct Channel* channel, struct EventLoop* evLoop)
{
	struct PollData* data = (struct PollData*)evLoop->dispatcherData;
	int events = 0;
	if (channel->events & ReadEvent) {
		events |= POLLIN; // 可读事件
	}
	if (channel->events & WriteEvent) {
		events |= POLLOUT; // 可写事件
	}
	int i = 0;
	for (; i < Max; ++i)
	{
		if(data->fds[i].fd == -1) {
			data->fds[i].fd = channel->fd; // 设置文件描述符
			data->fds[i].events = events; // 设置事件
			if (i > data->maxfd) {
				data->maxfd = i; // 更新最大文件描述符
			}
			break; // 找到空闲位置后退出循环
		}
	}
	if (i >= Max) {
		return -1; // 没有空间添加新的文件描述符
	}
	return 0;
}

static int pollRemove(struct Channel* channel, struct EventLoop* evLoop)
{
	struct PollData* data = (struct PollData*)evLoop->dispatcherData;

	int i = 0;
	for (; i < Max; ++i)
	{
		if (data->fds[i].fd == channel->fd){
			data->fds[i].fd = -1;// 设置文件描述符
			data->fds[i].events = 0;// 设置事件
			data->fds[i].revents = 0; // 清除返回事件
			break; 
		}
	}
	if (i >= Max) {
		return -1; // 没有空间添加新的文件描述符
	}
	return 0;
}

static int pollModify(struct Channel* channel, struct EventLoop* evLoop)
{
	struct PollData* data = (struct PollData*)evLoop->dispatcherData;
	int events = 0;
	if (channel->events & ReadEvent) {
		events |= POLLIN; // 可读事件
	}
	if (channel->events & WriteEvent) {
		events |= POLLOUT; // 可写事件
	}
	int i = 0;
	for (; i < Max; ++i)
	{
		if (data->fds[i].fd == channel->fd)
			data->fds[i].events = events; // 设置事件
			break;
		}
	}
	return 0;
}

static int pollDispatch(struct EventLoop* evLoop, int timeout)
{
	struct PollData* data = (struct PollData*)evLoop->dispatcherData;
	int count = poll(data->fds, data->maxfd+1, timeout*1000);
	if (count == -1)
	{
		perror("poll");
		exit(0);
	}
	for (int i = 0; i <= data->maxfd; ++i)
	{
		if (data->fds[i].fd == -1)
		{
			continue;
		}
		if (data->fds[i].revents & EPOLLIN) {
			// 可读事件
			eventActivate(evLoop, data->fds[i].fd, ReadEvent);
		}
		if (data->fds[i].revents & EPOLLOUT) {
			// 可写事件
			eventActivate(evLoop, data->fds[i].fd, WriteEvent);
		}
	}
	return 0;
}

static int epollClear(struct EventLoop* evLoop)
{
	struct PollData* data = (struct EpollData*)evLoop->dispatcherData;
	free(data);
}