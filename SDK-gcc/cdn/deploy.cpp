#include "deploy.h"

#define PRINT_COST
#define PRINT_TIME

char out[550000];
char temp[550000];
int all_cost = INT32_MAX;
int current_cost;

int num_level;

vector <pair<int,int>> best_answer;
vector <int> DirectNode;

extern const int NODEMAX, EDGEMAX, INF;

unordered_map<int,pair<int,int>> Level;
int positionPrice[NODEMAX + 10];

bool push_better;
bool pushable;

int nodesOutFlow[NODEMAX + 10];
vector <int> ReplaceNodes;

int out_degree[NODEMAX] = {0};//出度
int out_flow[NODEMAX] = {0};//出边流量
int node_actual_flow[NODEMAX] = {0};//todo 每次跑solve的时候统计，每次跑完后统计策略后需要清除
int total_lost_flow = 0;//总的确实费用
unordered_map<int,int>consumer_lost_flow;//每个消费节点的缺失费用
unordered_map<int,int>consumer_needed_flow;//每个消费节点的需求
int node_selected_cnt[NODEMAX];//todo 每个点被选择的次数
int node_level[NODEMAX];//每个消费节点的level

int node_average_chain_cost[NODEMAX];//加权平均 建图的时候求一次 为价值做辅助
int node_cost[NODEMAX];//包含地价以及链路成本
unordered_map <int,vector<pair<int,int>>> group;

MCMF MCF;

//返回当前经过的时间
int return_time()
{
	struct timeb rawtime;
	ftime(&rawtime);
	static bool first = true;
	static int ms;
	static unsigned long s;
	if (first) {
		ms = rawtime.millitm;
		s = rawtime.time;
		first = false;
	}
	int out_ms = rawtime.millitm - ms;
	unsigned long out_s = rawtime.time - s;
	if (out_ms < 0)
	{
		out_ms += 1000;
		out_s -= 1;
	}
	return out_s;
}

bool cmp_actual_flow(int p, int q){
    return node_actual_flow[p] > node_actual_flow[q];
}

bool cmp_actual_flow_up(int p, int q){
    return node_actual_flow[p] < node_actual_flow[q];
}

bool cmp_out_degeree(int p, int q){
	return out_degree[p] > out_degree[q];
}

bool cmp_out_flow(int p, int q){
	return out_flow[p] > out_flow[q];
}

bool cmp_lost_flow(int p, int q){
    return consumer_lost_flow[p] > consumer_lost_flow[q];
}

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num, char * filename)
{
	vector<pair<int, int>> servers;
	vector<int> current_answer,rest,all_node;
    int i,last_queshi=0;
	return_time();
	MCF.clear();
	MCF.buildGraph(topo);

    current_answer.clear();
    vector<int>::iterator itm;

	all_node.clear();
	for (i = 0; i < MCF.nodeNumber; ++i){
		all_node.push_back(i);
	}

    int jiaoji = 50;
    if (MCF.nodeNumber < 1000)
        jiaoji = 29;

	sort(all_node.begin(),all_node.end(),cmp_out_degeree);
	vector<int> more1(all_node.begin(),all_node.begin()+jiaoji);
    cout << *(more1.begin()) <<","<< out_degree[*(more1.begin())] << "," << *(more1.begin()+1) <<","<< out_degree[*(more1.begin()+1)] << endl;
	sort(all_node.begin(),all_node.end(),cmp_out_flow);
	vector<int> more2(all_node.begin(),all_node.begin()+jiaoji);

    for (itm = more1.begin(); itm != more1.end(); ++itm){
        if (find(more2.begin(),more2.end(),*itm) != more2.end())
            current_answer.push_back(*itm);
    }
    MCF.getTotalCost(current_answer,0);

    cout << current_answer.size() << endl;
    cout << "also need " << total_lost_flow << " of " << MCF.needSum << endl << endl;

    unordered_map<int,int>::iterator itc;
    vector<int> lost_direct;
//    for (itc = consumer_lost_flow.begin(); itc != consumer_lost_flow.end(); ++itc){
//        if (itc->second != 0)
//            lost_direct.push_back(itc->first);
//    }
//    i = 0;
//    for (itm = lost_direct.begin(); itm != lost_direct.end(); ++itm){
//        current_answer.push_back(*itm);
//        ++i;
////            cout << itc->first << "," << i++ << endl;
//        if (i % 2 == 0) {
//            MCF.getTotalCost(current_answer, 0);
//            if (total_lost_flow <= 0)
//                break;
//        }
//    }
    i = 0;
    for (itc = consumer_lost_flow.begin(); itc != consumer_lost_flow.end(); ++itc){
        if (itc->second != 0) {
            current_answer.push_back(itc->first);
            ++i;
//            cout << itc->first << "," << i++ << endl;
            if (i % 2 == 0) {
                MCF.getTotalCost(current_answer, 0);
                if (total_lost_flow <= 0)
                    break;
            }
        }
    }


//    rest = all_node;
//	for (itm = current_answer.begin(); itm != current_answer.end(); ++itm){
//		rest.erase(find(rest.begin(),rest.end(),*itm));
//	}
//    sort(rest.begin(),rest.end(),cmp_actual_flow_up);







	MCF.getTotalCost(current_answer,0);





    //最后一遍调用用来输出流的
	MCF.printAllPath();
	sprintf(temp, "%d\n\n", MCF.flowCnt);
	strcat(temp, out);

// 需要输出的内容
	char * topo_file = (char *) temp;

#ifdef PRINT_COST
    printf("server num = %lld\n",best_answer.size());
	printf("all cost = %d\n",all_cost);
#endif
	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);
#ifdef PRINT_TIME
	printf("now time is %d\n",return_time());
#endif
}
