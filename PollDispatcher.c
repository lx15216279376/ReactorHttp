#include "Dispatcher.h"
#include <sys/poll.h>


#define Max 1024 // ����¼���

struct PollData {
	int maxfd; // ����ļ�������
	struct pollfd fds[Max]; // pollfd����
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
	data->maxfd = 0; // ��ʼ������ļ�������Ϊ0
	for (int i = 0; i < Max; i++) {
		data->fds[i].fd = -1; // ��ʼ�������ļ�������Ϊ-1
		data->fds[i].events = 0; // ��ʼ���¼�Ϊ0
		data->fds[i].revents = 0; // ��ʼ�������¼�Ϊ0
	}
	return data;
}


static int pollAdd(struct Channel* channel, struct EventLoop* evLoop)
{
	struct PollData* data = (struct PollData*)evLoop->dispatcherData;
	int events = 0;
	if (channel->events & ReadEvent) {
		events |= POLLIN; // �ɶ��¼�
	}
	if (channel->events & WriteEvent) {
		events |= POLLOUT; // ��д�¼�
	}
	int i = 0;
	for (; i < Max; ++i)
	{
		if(data->fds[i].fd == -1) {
			data->fds[i].fd = channel->fd; // �����ļ�������
			data->fds[i].events = events; // �����¼�
			if (i > data->maxfd) {
				data->maxfd = i; // ��������ļ�������
			}
			break; // �ҵ�����λ�ú��˳�ѭ��
		}
	}
	if (i >= Max) {
		return -1; // û�пռ�����µ��ļ�������
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
			data->fds[i].fd = -1;// �����ļ�������
			data->fds[i].events = 0;// �����¼�
			data->fds[i].revents = 0; // ��������¼�
			break; 
		}
	}
	if (i >= Max) {
		return -1; // û�пռ�����µ��ļ�������
	}
	return 0;
}

static int pollModify(struct Channel* channel, struct EventLoop* evLoop)
{
	struct PollData* data = (struct PollData*)evLoop->dispatcherData;
	int events = 0;
	if (channel->events & ReadEvent) {
		events |= POLLIN; // �ɶ��¼�
	}
	if (channel->events & WriteEvent) {
		events |= POLLOUT; // ��д�¼�
	}
	int i = 0;
	for (; i < Max; ++i)
	{
		if (data->fds[i].fd == channel->fd)
			data->fds[i].events = events; // �����¼�
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
			// �ɶ��¼�
			eventActivate(evLoop, data->fds[i].fd, ReadEvent);
		}
		if (data->fds[i].revents & EPOLLOUT) {
			// ��д�¼�
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