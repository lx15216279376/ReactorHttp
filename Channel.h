#pragma once
#include <stdbool.h>

// ���庯��ָ������
typedef int(*handleFunc)(void* arg);

// �����ļ��������Ķ�д�¼�����ṹ��
enum FDEvent
{
	Timeout = 0x01, // ��ʱ�¼�
	ReadEvent = 0x02, // �ɶ��¼�
	WriteEvent = 0x04, // ��д�¼�
};

struct Channel
{
	// �ļ�������
	int fd;
	// �¼�
	int events;
	// �ص�����
	handleFunc readHandler;
	handleFunc writeHandler;
	// �ص������Ĳ���
	void* arg;
};

// ����һ���µ�Channel
struct Channel* channelInit(int fd, int events, handleFunc readHandler, handleFunc writeHandler, void* arg);

//�޸�Channel���¼������or�����д�¼�
void writeEventEnable(struct Channel* channel, bool flag);

// �ж��Ƿ���Ҫ����ļ���������д�¼�
bool isWriteEventEnable(struct Channel* channel);