#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "lib_io.h"
#include <stdio.h>
#include <string.h>
#include <map>
#include <limits.h>
#include <queue>
#include <memory.h>
#include <unordered_set>

using namespace std;


typedef struct _edge{
    int cab;
    int cost;
    int flow;
    int t_flow;
}edge;

typedef struct _customer_server{
    int no_network_usage;
    int demand_usage_t;
}customer_server;

typedef struct _cost_flow{
    int cost;
    int flow;
}cost_flow;

void deploy_server(char * graph[MAX_EDGE_NUM], int edge_num, char * filename);


#endif
