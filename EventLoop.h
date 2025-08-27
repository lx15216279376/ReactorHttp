#pragma once
#include <stdbool.h>
#include <Dispatcher.h>
#include <pthread.h>

// epollDispatcher是另一个源文件中定义的全局变量，表示使用epoll的事件分发器，在当前
// 文件中使用，需要extern关键字声明它的存在。
extern struct Dispatcher EpollDispatcher;
extern struct Dispatcher PollDispatcher;
extern struct Dispatcher SelectDispatcher;


// 定义任务队列的节点
enum ElemType{ADD,DELETE,MODIFY};
struct ChannelElement
{
	int type;	// 任务类型，1表示添加，2表示删除，3表示修改
	struct channel* channel;
	struct ChannelElement* next;
};

struct Eventloop
{
	bool isQuit; // 是否退出事件循环
	struct Dispatcher* dispatcher; // 事件分发器
	void* dispatcherData; // 分发器数据
	struct ChannelMap* channelMap; // 管理Channel的映射

	// 任务队列
	struct ChannelElement* head; // 任务队列的头指针
	struct ChannelElement* tail; // 任务队列的尾指针

	// 线程id，name
	pthread_t threadID;
	char threadName[32];
	pthread_mutex_t mutex; // 互斥锁，保护任务队列

	int socketpair[2]; // 用于唤醒事件循环的socket对
};

// 初始化事件循环
struct EventLoop* eventLoopInit();
struct EventLoop* eventLoopInitEx(const char* threadName);

// 启动事件循环
int eventLoopRun(struct EventLoop* evLoop);

// 处理激活的文件描述符
int eventActivate(struct EventLoop* evLoop, int fd, int event);

// 添加任务到任务队列
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel,int type);

// 处理任务队列中的任务
int eventLoopProcessTask(struct EventLoop* evLoop);

// 添加任务,将任务添加到dispatcher对应的检测集合中
int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel);

// 删除任务，将任务从dispatcher对应的检测集合中删除
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel);

// 释放channel，即删除任务的后续处理
int deatroyChannel(struct EventLoop* evLoop,struct Channel* channel);

// 修改任务
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel);
