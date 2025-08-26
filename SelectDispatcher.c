#include "Dispatcher.h"
#include <sys/select.h>
#include <stdio.h>

#define Max 1024 // ����¼���

struct SelectData {
	fd_set readSet;
	fd_set writeSet;
};

static void* selectInit();

static int selectAdd(struct Channel* channel, struct EventLoop* evLoop);

static int selectRemove(struct Channel* channel, struct EventLoop* evLoop);

static int selectModify(struct Channel* channel, struct EventLoop* evLoop);

static int selectDispatch(struct EventLoop* evLoop, int timeout);

static int selectClear(struct EventLoop* evLoop);

static void setFDset(struct Channel* channel, struct SelectData* data);		// ��������
static void clearFDset(struct Channel* channel, struct SelectData* data);	// ��������


struct Dispatcher SelectDispatcher = {
	selectInit,
	selectAdd,
	selectRemove,
	selectModify,
	selectDispatch,
	selectClear
};

static void* selectInit()
{
	struct selectData* data = (struct selectData*)malloc(sizeof(struct selectData));
	FD_ZERO(&data->readSet);
	FD_ZERO(&data->writeSet);
	return data;
}

static void setFDset(struct Channel* channel, struct SelectData* data)
{
	if (channel->events & ReadEvent) {
		FD_SET(channel->fd, &data->readSet);
	}
	if (channel->events & WriteEvent) {
		FD_SET(channel->fd, &data->writeSet);
	}
}

static int clearFDset(struct Channel* channel, struct SelectData* data)
{
	if (channel->events & ReadEvent) {
		FD_CLR(channel->fd, &data->readSet);
	}
	if (channel->events & WriteEvent) {
		FD_CLR(channel->fd, &data->writeSet);
	}
}

static int selectAdd(struct Channel* channel, struct EventLoop* evLoop)
{
	struct selectData* data = (struct selectData*)evLoop->dispatcherData;
	if (channel->fd >= Max)
	{
		return -1;
	}
	setFDset(channel, data);
	return 0;
}

static int selectRemove(struct Channel* channel, struct EventLoop* evLoop)
{
	struct selectData* data = (struct selectData*)evLoop->dispatcherData;
	clearFDset(channel, data);
	return 0;
}

static int pollModify(struct Channel* channel, struct EventLoop* evLoop)
{
	struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
	setFDset(channel, data);
	clearFDset(channel, data);
	return 0;
}

static int selectDispatch(struct EventLoop* evLoop, int timeout)
{
	struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
	struct timeval val;
	val.tv_sec = timeout;
	val.tv_usec = 0;
	fd_set rdtmp = data->readSet;
	fd_set wrtmp = data->writeSet;
	int count = select(Max,&rdtmp,&wrtmp,NULL,&val);
	if (count == -1)
	{
		perror("select");
		exit(0);
	}
	for (int i = 0; i < Max; ++i)
	{
		if (FD_ISSET(i,&rdtmp))
		{
			// �ɶ��¼�
			eventActivate(evLoop, i, ReadEvent);
		}
		if (FD_ISSET(i, &wrtmp)) {
			// ��д�¼�
			eventActivate(evLoop, i, WriteEvent);
		}
	}
	return 0;
}

static int selectClear(struct EventLoop* evLoop)
{
	struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
	free(data);
}