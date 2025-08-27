#pragma once
#include <stdbool.h>
#include <Dispatcher.h>
#include <pthread.h>

// epollDispatcher����һ��Դ�ļ��ж����ȫ�ֱ�������ʾʹ��epoll���¼��ַ������ڵ�ǰ
// �ļ���ʹ�ã���Ҫextern�ؼ����������Ĵ��ڡ�
extern struct Dispatcher EpollDispatcher;
extern struct Dispatcher PollDispatcher;
extern struct Dispatcher SelectDispatcher;


// ����������еĽڵ�
enum ElemType{ADD,DELETE,MODIFY};
struct ChannelElement
{
	int type;	// �������ͣ�1��ʾ��ӣ�2��ʾɾ����3��ʾ�޸�
	struct channel* channel;
	struct ChannelElement* next;
};

struct Eventloop
{
	bool isQuit; // �Ƿ��˳��¼�ѭ��
	struct Dispatcher* dispatcher; // �¼��ַ���
	void* dispatcherData; // �ַ�������
	struct ChannelMap* channelMap; // ����Channel��ӳ��

	// �������
	struct ChannelElement* head; // ������е�ͷָ��
	struct ChannelElement* tail; // ������е�βָ��

	// �߳�id��name
	pthread_t threadID;
	char threadName[32];
	pthread_mutex_t mutex; // �������������������

	int socketpair[2]; // ���ڻ����¼�ѭ����socket��
};

// ��ʼ���¼�ѭ��
struct EventLoop* eventLoopInit();
struct EventLoop* eventLoopInitEx(const char* threadName);

// �����¼�ѭ��
int eventLoopRun(struct EventLoop* evLoop);

// ��������ļ�������
int eventActivate(struct EventLoop* evLoop, int fd, int event);

// ��������������
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel,int type);

// ������������е�����
int eventLoopProcessTask(struct EventLoop* evLoop);

// �������,��������ӵ�dispatcher��Ӧ�ļ�⼯����
int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel);

// ɾ�����񣬽������dispatcher��Ӧ�ļ�⼯����ɾ��
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel);

// �ͷ�channel����ɾ������ĺ�������
int deatroyChannel(struct EventLoop* evLoop,struct Channel* channel);

// �޸�����
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel);
