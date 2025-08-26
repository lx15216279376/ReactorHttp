#pragma once

struct ChannelMap
{
	int size;          // 记录指针指向的数组的大小
	struct Channel** list; // 指向Channel指针数组的指针
};

// 初始化ChannelMap
struct ChannelMap* channelMapInit(int initialSize);

// 清空ChannelMap
void channelMapClear(struct ChannelMap* map);

// 在ChannelMap中添加或更新Channel
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize);