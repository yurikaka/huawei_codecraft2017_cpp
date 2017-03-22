#include "deploy.h"
#include <stdio.h>
#include <string.h>

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    char out[55000] = "";
    char temp[55000] = "";
    int a,b,c;
    int num_network, num_edge, num_custom;
    sscanf(topo[0],"%d%d%d",&num_network,&num_edge,&num_custom);
    sprintf(out,"%d\n\n",num_custom);
    for (int i = line_num-num_custom; i < line_num; ++i) {
        sscanf(topo[i],"%d%d%d",&a,&b,&c);
        sprintf(temp,"%d %d %d\n",b,a,c);
        strcat(out,temp);
    }
	// 需要输出的内容
	char * topo_file = (char *)out;

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);

}
