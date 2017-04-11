#include "deploy.h"
#include <stdio.h>

char out[550000] = "";
char out_tmp[550000] = "";

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
	int network_num,edge_num,customer_num,level_num,i,j,a,b,c,d;
	sscanf(topo[0],"%d%d%d",&network_num,&edge_num,&customer_num);
	level_num = line_num - network_num - edge_num - customer_num -5;
	level levels[level_num];
	for (i = 0; i < level_num; ++i){
		sscanf(topo[i+2],"%d%d%d",&a,&b,&c);
		levels[i] = {b,c};
	}
	int node_cost[network_num];
	for (i = 3 + level_num; i < network_num + level_num + 3; ++i){
		sscanf(topo[i],"%d%d",&a,&b);
		node_cost[a] = b;
	}
	map<int,edge> edges[network_num];
	for (i = 4 + level_num + network_num; i < 4 + level_num + network_num + edge_num; ++i){
		sscanf(topo[i],"%d%d%d%d",&a,&b,&c,&d);
		edges[a][b] = {c,d};
		edges[b][a] = {c,d};
	}
	customer customers[customer_num];
	sprintf(out,"%d\n\n",customer_num);
	for (i = 5 + level_num + network_num + edge_num; i < 5 + level_num + network_num + edge_num + customer_num; ++i){
		sscanf(topo[i],"%d%d%d",&a,&b,&c);
		customers[a] = {b,c};
		for (j = 0; j < level_num; ++j){
			if (levels[j].output >= c)
				break;
		}
		sprintf(out_tmp,"%d %d %d %d\n",b,a,c,j);
		strcat(out,out_tmp);
	}
	// 需要输出的内容
	char * topo_file = (char *) out;

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);

}
