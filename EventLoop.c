#include "EventLoop.h"
#include <assert.h>

struct EventLoop* eventLoopInit()
{
	return eventLoopInitEx(NULL);
}

// 写本地消息到socketpair[0]，用于唤醒事件循环
void taskWakeup(struct EventLoop* evLoop)
{
	const char* msg = "我是海贼王？？？";
	write(evLoop->socketpair[0], msg, strlen(msg));

}

// 读取本地消息，从而清空socketpair[1]的读缓冲区
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
	evLoop->dispatcherData = evLoop->dispatcher->init(); // 初始化分发器数据

	evLoop->head = NULL; // 初始化任务队列头
	evLoop->tail = NULL; // 初始化任务队列尾

	evLoop->channelMap = channelMapInit(128); // 初始化Channel映射

	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, evLoop->socketpair); // 创建socket对
	if(ret == -1)
	{
		perror("socketpair");
		exit(0);
	}
	struct Channel* channel = channelInit(evLoop->socketpair[1], ReadEvent,readLocalMessage,NULL,evLoop);
	eventLoopAddTask(evLoop, channel, ADD); // 将socketpair[1]添加到事件循环中
	
	return evLoop;
}

int eventLoopRun(struct EventLoop* evLoop)
{
	assert(evLoop != NULL);
	// 取出事件分发和检测模型
	struct Dispatcher* dispatcher = evLoop->dispatcher;
	// 比较线程ID是否正常
	if (evLoop->threadID != pthread_self())
	{
		return -1;
	}

	// 循环进行事件处理
	while (!evLoop->isQuit)
	{
		dispatcher->dispatch(evLoop, 2);
		eventLoopProcessTask(evLoop); // 处理任务队列中的任务
	}
	return 0;
}

int eventActivate(struct EventLoop* evLoop, int fd, int event)
{
	if (fd < 0 || evLoop == NULL)
	{
		return -1;
	}

	// 取出channel
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
	// 加锁，保护共享资源（任务队列）
	pthread_mutex_lock(&evLoop->mutex);
	// 创建新的任务节点
	struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
	node->channel = channel;
	node->type = type;
	node->next = NULL;
	// 将任务节点添加到任务队列
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

	// 处理节点
	/*
	* 细节：
	*	1. 对于链表节点的添加：可能是当前线程添加的节点，也可能是其他线程（主线程）添加的节点
	*		1). 修改fd的事件，当前子线程发起，当前子线程处理
	*		2). 添加新的fd,主线程发起，当前子线程处理
	*	2. 不能让主线程处理任务队列，需要当前的子线程处理
	*/
	if (evLoop->threadID == pthread_self())
	{
		// 当前子线程
		eventLoopProcessTask(evLoop); // 处理任务队列中的任务
	}
	else
	{
		// 主线程 -- 告诉子线程处理任务队列中的任务
		// 1. 子线程在工作，2. 子线程阻塞在等待事件上
		taskWakeup(evLoop); // 唤醒子线程，让子线程处理任务队列中的任务
	}
	return 0;
}

int eventLoopProcessTask(struct EventLoop* evLoop)
{
	pthread_mutex_lock(&evLoop->mutex);
	// 处理任务队列中的任务
	struct CannelElement* head = evLoop->head;
	while(head != NULL)
	{
		struct ChannelElement* channel = head->channel;
		if(head->type == ADD)
		{
			// 添加
			eventLoopAdd(evLoop, channel);
		}
		else if(head->type == DELETE)
		{
			// 删除
			eventLoopRemove(evLoop, channel);
		}
		else if(head->type == MODIFY)
		{
			// 修改
			eventLoopModify(evLoop, channel);
		}
		struct ChannelElement* temp = head;
		head = head->next;
		free(tmp);
	}
	evLoop->head = evLoop->tail = NULL; // 清空任务队列
	pthread_mutex_unlock(&evLoop->mutex);
	return 0;
}

int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel)
{
	int fd = channel->fd;
	struct ChannelMap* channelMap = evLoop->channelMap;
	if(fd >= channelMap->size)
	{
		// 扩展channelMap
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
	// 删除channel和fd的对应关系
	evLoop->channelMap->list[channel->fd] = NULL;
	// 关闭fd
	close(channel->fd);
	// 释放channel
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
