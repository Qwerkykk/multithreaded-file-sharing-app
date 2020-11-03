#ifndef SYSPRO_PROJECT3_CLIENT_POOL_H
#define SYSPRO_PROJECT3_CLIENT_POOL_H

#include "struct.h"

void initializePool(Pool_t* pool, const Arguments* arguments);

void obtain(Pool_t* pool,PoolItem* item);

void place(Pool_t* pool,PoolItem* item);

void clientListToPool(Pool_t* pool, ClientListNode* clientList);

#endif
