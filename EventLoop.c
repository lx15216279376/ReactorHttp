#include "EventLoop.h"
#include <assert.h>

struct EventLoop* eventLoopInit()
{
	return eventLoopInitEx(NULL);
}

// д������Ϣ��socketpair[0]�����ڻ����¼�ѭ��
void taskWakeup(struct EventLoop* evLoop)
{
	const char* msg = "���Ǻ�����������";
	write(evLoop->socketpair[0], msg, strlen(msg));

}

// ��ȡ������Ϣ���Ӷ����socketpair[1]�Ķ�������
int readLocalMessage(void* arg)
{
	struct EventLoop* evLoop = (struct EventLoop*)arg;
	char buf[256] = {0};
	read(evLoop->socketpair[1], buf, sizeof(buf));
	return 0;
}

struct EventLoop* eventLoopInitEx(const char* threadName)
{
	struct EventLoop* evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
	evLoop->isQuit = false;
	evLoop->threadID = pthread_self();
	pthread_mutex_init(&evLoop->mutex, NULL);
	strcpy(evLoop->threadName, threadName == NULL ? "MainThread" : threadName);
	evLoop->dispatcher = &EpollDispatcher;
	evLoop->dispatcherData = evLoop->dispatcher->init(); // ��ʼ���ַ�������

	evLoop->head = NULL; // ��ʼ���������ͷ
	evLoop->tail = NULL; // ��ʼ���������β

	evLoop->channelMap = channelMapInit(128); // ��ʼ��Channelӳ��

	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, evLoop->socketpair); // ����socket��
	if(ret == -1)
	{
		perror("socketpair");
		exit(0);
	}
	struct Channel* channel = channelInit(evLoop->socketpair[1], ReadEvent,readLocalMessage,NULL,evLoop);
	eventLoopAddTask(evLoop, channel, ADD); // ��socketpair[1]��ӵ��¼�ѭ����
	
	return evLoop;
}

int eventLoopRun(struct EventLoop* evLoop)
{
	assert(evLoop != NULL);
	// ȡ���¼��ַ��ͼ��ģ��
	struct Dispatcher* dispatcher = evLoop->dispatcher;
	// �Ƚ��߳�ID�Ƿ�����
	if (evLoop->threadID != pthread_self())
	{
		return -1;
	}

	// ѭ�������¼�����
	while (!evLoop->isQuit)
	{
		dispatcher->dispatch(evLoop, 2);
		eventLoopProcessTask(evLoop); // ������������е�����
	}
	return 0;
}

int eventActivate(struct EventLoop* evLoop, int fd, int event)
{
	if (fd < 0 || evLoop == NULL)
	{
		return -1;
	}

	// ȡ��channel
	struct Channel* channel = evLoop->channelMap->list[fd];
	assert(channel->fd == fd);
	if (event & ReadEvent && channel->readCallback())
	{
		channel->readCallback(channel->arg);
	}
	if (event & WriteEvent && channel->writeCallback())
	{
		channel->writeCallback(channel->arg);
	}
	return 0;
}

int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type)
{
	// ����������������Դ��������У�
	pthread_mutex_lock(&evLoop->mutex);
	// �����µ�����ڵ�
	struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
	node->channel = channel;
	node->type = type;
	node->next = NULL;
	// ������ڵ���ӵ��������
	if (evLoop->head == NULL)
	{
		evLoop->head = node;
		evLoop->tail = node;
	}
	else
	{
		evLoop->tail->next = node;
		evLoop->tail = node;
	}
	pthread_mutex_unlock(&evLoop->mutex);

	// ����ڵ�
	/*
	* ϸ�ڣ�
	*	1. ��������ڵ����ӣ������ǵ�ǰ�߳���ӵĽڵ㣬Ҳ�����������̣߳����̣߳���ӵĽڵ�
	*		1). �޸�fd���¼�����ǰ���̷߳��𣬵�ǰ���̴߳���
	*		2). ����µ�fd,���̷߳��𣬵�ǰ���̴߳���
	*	2. ���������̴߳���������У���Ҫ��ǰ�����̴߳���
	*/
	if (evLoop->threadID == pthread_self())
	{
		// ��ǰ���߳�
		eventLoopProcessTask(evLoop); // ������������е�����
	}
	else
	{
		// ���߳� -- �������̴߳�����������е�����
		// 1. ���߳��ڹ�����2. ���߳������ڵȴ��¼���
		taskWakeup(evLoop); // �������̣߳������̴߳�����������е�����
	}
	return 0;
}

int eventLoopProcessTask(struct EventLoop* evLoop)
{
	pthread_mutex_lock(&evLoop->mutex);
	// ������������е�����
	struct CannelElement* head = evLoop->head;
	while(head != NULL)
	{
		struct ChannelElement* channel = head->channel;
		if(head->type == ADD)
		{
			// ���
			eventLoopAdd(evLoop, channel);
		}
		else if(head->type == DELETE)
		{
			// ɾ��
			eventLoopRemove(evLoop, channel);
		}
		else if(head->type == MODIFY)
		{
			// �޸�
			eventLoopModify(evLoop, channel);
		}
		struct ChannelElement* temp = head;
		head = head->next;
		free(tmp);
	}
	evLoop->head = evLoop->tail = NULL; // ����������
	pthread_mutex_unlock(&evLoop->mutex);
	return 0;
}

int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel)
{
	int fd = channel->fd;
	struct ChannelMap* channelMap = evLoop->channelMap;
	if(fd >= channelMap->size)
	{
		// ��չchannelMap
		if(!makeMapRoom(channelMap,fd,sizeof(struct Channel*)))
		{
			return -1;
		}
	}

	if(channelMap->list[fd] == NULL)
	{
		channelMap->list[fd] = channel;
		evLoop->dispatcher->add(channel, evLoop);
	}
	return 0;
}

int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel)
{
	int fd = channel->fd;
	struct ChannelMap* channelMap = evLoop->channelMap;
	if (fd >= channelMap->size)
	{
		return -1;
	}

	int ret = evLoop->dispatcher->remove(channel, evLoop);
	return ret;
}

int deatroyChannel(struct EventLoop* evLoop,struct Channel* channel)
{
	// ɾ��channel��fd�Ķ�Ӧ��ϵ
	evLoop->channelMap->list[channel->fd] = NULL;
	// �ر�fd
	close(channel->fd);
	// �ͷ�channel
	free(channel);
	return 0;
}

int eventLoopModify(struct EventLoop* evLoop,struct Channel* channel)
{
	int fd = channel->fd;
	struct ChannelMap* channelMap = evLoop->channelMap;
	if (fd >= channelMap->size)
	{
		return -1;
	}

	int ret = evLoop->dispatcher->modify(channel, evLoop);
	return 0;
}
