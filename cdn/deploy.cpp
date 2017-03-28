#include "deploy.h"
#include <stdio.h>
#include <string.h>
#include <map>

using namespace std;

typedef struct _edge{
    int cab;
    int cost;
    int flow;
    int t_flow;
}edge;

typedef struct _customer{
    int no_network;
    int demand;
}customer;

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    char out[55000] = "";
    char temp[55000] = "";
    int a,b,c,d,i,j,count;
    int num_network, num_edge, num_custom,cost_server;
    sscanf(topo[0],"%d%d%d",&num_network,&num_edge,&num_custom);
    sscanf(topo[2], "%d", &cost_server);
    map<int,edge> edges[num_network];
    for (i = 4; i < 4 + num_edge; ++i){
        sscanf(topo[i],"%d%d%d%d",&a,&b,&c,&d);
        edges[a][b] = {c,d,0,0};
        edges[b][a] = {c,d,0,0};
    }
    customer customers[num_custom];
    int answer[num_custom];
    sprintf(out,"%d\n\n",num_custom);
    for (i = line_num-num_custom; i < line_num; ++i) {
        sscanf(topo[i],"%d%d%d",&a,&b,&c);
        customers[a] = {b,c};
        answer[a] = b;
    }
    //删除无用边
    do {
        count = 0;
        for (i = 0; i < num_network; ++i) {
            if (edges[i].size() == 1) {
                for (j = 0; j < num_custom; ++j) {
                    if (customers[j].no_network == i)
                        break;
                }
                if (j == num_custom) {
                    edges[edges[i].begin()->first].erase(i);
                    edges[i].erase(edges[i].begin());
                    ++count;
                }
            }
        }
    } while (count > 0);

    map<int,edge>::iterator it;
//    //输出所有边（测试）
//    for (i = 0; i < num_network; ++i) {
//        for (it = edges[i].begin(); it != edges[i].end(); ++it) {
//            printf("%d %d %d %d %d %d\n", i, it->first, it->second.cab, it->second.cost, it->second.flow, it->second.t_flow);
//        }
//    }
    for (i = 0; i < num_custom; ++i){
        if (answer[i] != customers[i].no_network)
            continue;
        for (j = 0; j < num_custom; ++j){
            if (i == j)
                continue;
            it = edges[customers[i].no_network].find(customers[j].no_network);
            if (it != edges[customers[i].no_network].end() && it->second.cab >= min(customers[i].demand, customers[j].demand)){
                if (customers[i].demand < customers[j].demand){
                    if (customers[i].demand * it->second.cost <= cost_server){
                        answer[i] = customers[j].no_network;
                        break;
                    }
                } else {
                    if (customers[j].demand * it->second.cost <= cost_server) {
                        answer[j] = customers[i].no_network;
                        break;
                    }
                }
            }
        }
    }
    for (i = 0; i < num_custom; ++i){
        if (answer[i] == customers[i].no_network){
            sprintf(temp,"%d %d %d\n",answer[i],i,customers[i].demand);
            strcat(out,temp);
        } else {
            sprintf(temp,"%d %d %d %d\n",answer[i],customers[i].no_network,i,customers[i].demand);
            strcat(out,temp);
        }
    }
	// 需要输出的内容
	char * topo_file = (char *)out;

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);

}
