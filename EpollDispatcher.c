#include "Dispatcher.h"
#include <sys/epoll.h>


#define Max 520 // 最大事件数

struct EpollData {
	int epfd; // epoll文件描述符
	struct epoll_event* events; // 事件数组
};

static void* epollInit();

static int epollAdd(struct Channel* channel, struct EventLoop* evLoop);

static int epollRemove(struct Channel* channel, struct EventLoop* evLoop);

static int epollModify(struct Channel* channel, struct EventLoop* evLoop);

static int epollDispatch(struct EventLoop* evLoop, int timeout);

static int epollClear(struct EventLoop* evLoop);

static int epollCtl(struct Channel* channel, struct EventLoop* evLoop, int op);

struct Dispatcher EpollDispatcher = {
	epollInit,
	epollAdd,
	epollRemove,
	epollModify,
	epollDispatch,
	epollClear
};

static void* epollInit()
{
	struct EpollData* data = (struct EpollData*)malloc(sizeof(struct EpollData));
	data->epfd = epoll_create(10);
	if(data->epfd < 0) {
		perror("epoll_create");
		exit(0);
	}
	data->events = (struct epoll_event*)calloc(Max, sizeof(struct epoll_event));
	return data;
}

static int epollCtl(struct Channel* channel, struct EventLoop* evLoop, int op)
{
	struct EpollData* data = (struct EpollData*)evLoop->dispatcherData;
	struct epoll_event ev;
	ev.data.fd = channel->fd; // 设置事件的文件描述符
	int events = 0;
	if (channel->events & ReadEvent) {
		events |= EPOLLIN; // 可读事件
	}
	if (channel->events & WriteEvent) {
		events |= EPOLLOUT; // 可写事件
	}
	ev.events = events;
	int ret = epoll_ctl(data->epfd, op, channel->fd, &ev);
	return ret;
}

static int epollAdd(struct Channel* channel, struct EventLoop* evLoop)
{
	int ret = epollCtl(channel, evLoop,EPOLL_CTL_ADD);
	if (ret == -1)
	{
		perror("epoll_ctl: add");
		exit(0);
	}
	return ret;
}

static int epollRemove(struct Channel* channel, struct EventLoop* evLoop)
{
	int ret = epollCtl(channel, evLoop, EPOLL_CTL_DEL);
	if (ret == -1)
	{
		perror("epoll_ctl: del");
		exit(0);
	}
	return ret;
}

static int epollModify(struct Channel* channel, struct EventLoop* evLoop)
{
	int ret = epollCtl(channel, evLoop, EPOLL_CTL_MOD);
	if (ret == -1)
	{
		perror("epoll_ctl: mod");
		exit(0);
	}
	return ret;
}

static int epollDispatch(struct EventLoop* evLoop, int timeout)
{
	struct EpollData* data = (struct EpollData*)evLoop->dispatcherData;
	int count = epoll_wait(data->epfd, data->events, Max, timeout);
	for (int i = 0; i < count; i++)
	{
		int events = data->events[i].events;
		int fd = data->events[i].data.fd;
		if(events & EPOLLERR || events & EPOLLHUP) {
			// 错误或挂起事件，处理逻辑
			// epollRemove(channel, evLoop);
			continue;
		}
		if(events & EPOLLIN) {
			// 可读事件
			eventActivate(evLoop, fd, ReadEvent);
		}
		if(events & EPOLLOUT) {
			// 可写事件
			eventActivate(evLoop, fd, WriteEvent);
		}
	}
	return 0;
}

static int epollClear(struct EventLoop* evLoop)
{
	struct EpollData* data = (struct EpollData*)evLoop->dispatcherData;
	free(data->events);
	close(data->epfd);
	free(data);
}
