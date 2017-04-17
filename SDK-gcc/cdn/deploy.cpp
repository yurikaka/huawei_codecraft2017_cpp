#include "deploy.h"

#define PRINT_COST
#define PRINT_TIME

char out[550000];
char temp[550000];
int all_cost = INT32_MAX;
int current_cost;

int min_node_cost = INT32_MAX;

vector <pair<int,int>> best_answer;
vector <int> DirectNode;

extern const int NODEMAX, EDGEMAX, INF;

unordered_map<int,pair<int,int>> Level;
int positionPrice[NODEMAX + 10];

MCMF MCF;

/*************************************************/

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

bool cmp(pair<int, int> p, pair<int, int> q) {
	return p.second < q.second;
}

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num, char * filename)
{
	vector<pair<int, int>> servers;
	vector<int> answer;
	return_time();
	MCF.clear();
	MCF.buildGraph(topo);

	MCF.getTotalCost(DirectNode, 0);
	printf("direct cost is %d \n", all_cost);

	servers = best_answer;
	for (vector<pair<int, int>>::iterator it = servers.begin(); it != servers.end(); ++it) {
		bool flag = true;
		int cost_flow = MCF.edge[MCF.server2edge[it->first] ^ 1].flow;
		while (flag && positionPrice[it->first] != min_node_cost) {
			flag = false;
			for (int i = MCF.G[it->first]; i; i = MCF.edge[i].next) {
				if (i % 2 == 1)
					continue;
				if (MCF.edge[i].to == MCF.vSink || MCF.edge[i].to == MCF.vSource)
					continue;
				if (MCF.edge[i].flow + MCF.edge[i ^ 1].flow >= cost_flow) {
					if (MCF.edge[i].cost * cost_flow <= positionPrice[it->first] - positionPrice[MCF.edge[i].to]) {
						*it = pair<int, int>(MCF.edge[i].to, it->second);
						flag = true;
						break;
					}
				}
			}
		}
	}
	answer.clear();
	for (vector<pair<int, int>>::iterator it = servers.begin(); it != servers.end(); ++it) {
		if (find(answer.begin(), answer.end(), it->first) == answer.end())
			answer.push_back(it->first);
	}
	MCF.getTotalCost(answer, 0);
	printf("cost = %d\n", all_cost);

	int num_sround_min = 5;
	int num_sround;
	pair<int, int> tmp_first;
	pair<int, int> tmp_back;
	while (num_sround_min >= 1) {
		servers = best_answer;
		sort(servers.begin(), servers.end(), cmp);
		tmp_back = servers.back();
		while (servers.front() != tmp_back) {
			tmp_first = servers.front();
			servers.erase(servers.begin());
			answer.clear();
			for (vector<pair<int, int>>::iterator it = servers.begin(); it != servers.end(); ++it) {
				answer.push_back(it->first);
			}
			num_sround = 0;
			for (int i = MCF.G[tmp_first.first]; i; i = MCF.edge[i].next) {
				if (i % 2 == 1)
					continue;
				if (MCF.edge[i].to == MCF.vSink || MCF.edge[i].to == MCF.vSource)
					continue;
				if (find(answer.begin(), answer.end(), MCF.edge[i].to) != answer.end())
					++num_sround;
			}
			if (num_sround >= num_sround_min) {
				MCF.getTotalCost(answer, 0);
				if (current_cost == -1)
					servers.push_back(tmp_first);
			} else {
				servers.push_back(tmp_first);
			}
			//printf("num of servers = %lu, cost = %d\n", answer.size(), all_cost);
		}
		--num_sround_min;
	}

	servers = best_answer;
	answer.clear();
	for (vector<pair<int, int>>::iterator it = servers.begin(); it != servers.end(); ++it) {
		answer.push_back(it->first);
	}
	double min_use = 0.00;
	do {
		MCF.getTotalCost(answer, min_use);
		min_use += 0.01;
	} while (current_cost != -1);
	printf("cost = %d\n", all_cost);

	MCF.printAllPath();
	sprintf(temp, "%d\n\n", MCF.flowCnt);
	strcat(temp, out);

// 需要输出的内容
	char * topo_file = (char *) temp;

#ifdef PRINT_COST
    printf("server num = %d\n",best_answer.size());
	printf("all cost = %d\n",all_cost);
#endif
	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);
#ifdef PRINT_TIME
	printf("now time is %d\n",return_time());
#endif
}
