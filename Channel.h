#pragma once
#include <stdbool.h>

// 定义函数指针类型
typedef int(*handleFunc)(void* arg);

// 定义文件描述符的读写事件处理结构体
enum FDEvent
{
	Timeout = 0x01, // 超时事件
	ReadEvent = 0x02, // 可读事件
	WriteEvent = 0x04, // 可写事件
};

struct Channel
{
	// 文件描述符
	int fd;
	// 事件
	int events;
	// 回调函数
	handleFunc readHandler;
	handleFunc writeHandler;
	// 回调函数的参数
	void* arg;
};

// 创建一个新的Channel
struct Channel* channelInit(int fd, int events, handleFunc readHandler, handleFunc writeHandler, void* arg);

//修改Channel的事件，检测or不检测写事件
void writeEventEnable(struct Channel* channel, bool flag);

// 判断是否需要检测文件描述符的写事件
bool isWriteEventEnable(struct Channel* channel);