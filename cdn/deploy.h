#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "lib_io.h"
#include <string.h>
#include <map>
#include <iostream>

using namespace std;

typedef struct _level{
    int output;
    int cost;
}level;

typedef struct _customer{
    int node;
    int demand;
}customer;

typedef struct _edge{
    int cab;
    int price;
}edge;

void deploy_server(char * graph[MAX_EDGE_NUM], int edge_num, char * filename);

	

#endif
