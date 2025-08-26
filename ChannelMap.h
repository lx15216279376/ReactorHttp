#pragma once

struct ChannelMap
{
	int size;          // ��¼ָ��ָ�������Ĵ�С
	struct Channel** list; // ָ��Channelָ�������ָ��
};

// ��ʼ��ChannelMap
struct ChannelMap* channelMapInit(int initialSize);

// ���ChannelMap
void channelMapClear(struct ChannelMap* map);

// ��ChannelMap����ӻ����Channel
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize);