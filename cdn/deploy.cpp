#include "deploy.h"
#include <stdio.h>
#include <string.h>
#include <map>

using namespace std;

typedef struct _edge{
    int cab;
    int cost;
}edge;

typedef struct _customer{
    int no_network;
    int demand;
}customer;

/*
int main() {
    MinCostFlow<int,int> MCF;
    for(int i = 0; i < 8; i++){
        MCF.addV();
    }
    MCF.addEdge(0,1,5,0);
    MCF.addEdge(0,2,2,0);
    MCF.addEdge(2,3,7,2);
    MCF.addEdge(1,2,3,1);
    MCF.addEdge(1,3,3,4);
    MCF.addEdge(3,6,1,8);
    MCF.addEdge(3,5,7,5);
    MCF.addEdge(3,4,5,2);
    MCF.addEdge(4,5,3,1);
    MCF.addEdge(6,7,1,0);
    MCF.addEdge(5,7,4,0);
    MCF.addEdge(4,7,2,0);

    auto result = MCF.minCostFlow(0,7);
    cout << "Flow   " << result.first <<"Cost  " <<result.second << endl;
	return 0;
}*/
//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    char out[55000] = "";
    char temp[55000] = "";
    int a,b,c,d;
    int num_network, num_edge, num_custom,cost_server;
    sscanf(topo[0],"%d%d%d",&num_network,&num_edge,&num_custom);
    sscanf(topo[2], "%d", &cost_server);
    map<int,edge> edges[num_network];
    for (int i = 4; i < 3 + num_edge; ++i){
        sscanf(topo[i],"%d%d%d%d",&a,&b,&c,&d);
        edges[a][b] = {c,d};
        edges[b][a] = {c,d};
    }
    customer customers[num_custom];
    int answer[num_custom];
    sprintf(out,"%d\n\n",num_custom);
    for (int i = line_num-num_custom; i < line_num; ++i) {
        sscanf(topo[i],"%d%d%d",&a,&b,&c);
        customers[a] = {b,c};
        answer[a] = b;
    }
    map<int,edge>::iterator it;
    for (int i = 0; i < num_custom; ++i){
        if (answer[i] != customers[i].no_network)
            continue;
        for (int j = 0; j < num_custom; ++j){
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
    for (int i = 0; i < num_custom; ++i){
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
