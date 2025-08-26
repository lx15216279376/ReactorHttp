#include "ChannelMap.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct ChannelMap* channelMapInit(int initialSize)
{
	struct ChannelMap* map = (struct ChannelMap*)malloc(sizeof(struct ChannelMap));
	map->size = initialSize;
	map->list = (struct Channel**)malloc(sizeof(struct Channel*) * initialSize);
	return map;
}

void channelMapClear(struct ChannelMap* map)
{
	if (map != NULL)
	{
		for (int i = 0; i < map->size; ++i)
		{
			if (map->list[i] != NULL)
			{
				free(map->list[i]);
			}
		}
		free(map->list);
		map->list = NULL;
	}
	map->size = 0;
}

bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize)
{
	if (map->size < newSize)
	{
		int curSize = map->size;
		while(curSize < newSize)
		{
			curSize*=2; 
		}
		// 扩容realloc
		struct Channel** temp=realloc(map->list, curSize * unitSize);
		if (temp == NULL)
		{
			return false; // 内存分配失败
		}
		map->list = temp;	 // 更新指向的数组
		memset(&map->list[map->size], 0, (curSize - map->size) * unitSize); // 清空新分配的空间
		map->size = curSize; // 更新大小
	}
	return true;
}
