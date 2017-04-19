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

bool cmp(pair<int, int> p, pair<int, int> q) {
	return p.second < q.second;
}

bool cmp2(pair<int, int> p, pair<int, int> q) {
	return p.second > q.second;
}

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num, char * filename)
{
	vector<pair<int, int>> servers;
	vector<int> answer;
	return_time();
	MCF.clear();
	MCF.buildGraph(topo);

    //todo change time limit
	int time_max[4] = {5, 40, 25, 86};


	MCF.getTotalCost(DirectNode, 0);
	printf("direct cost is %d \n", all_cost);
    cout << endl;

    cout << "begin move1, num of servers = " << best_answer.size() << endl;
    cout << "begin move1 at " << return_time() << " second" << endl;
    cout << "begin move1 at " << all_cost << " cost" << endl;
    cout << endl;

	servers = best_answer;
	for (vector<pair<int, int>>::iterator it = servers.begin(); it != servers.end(); ++it) {
        if (return_time() > time_max[0])
            break;
		bool flag = true;
		int cost_flow = MCF.edge[MCF.server2edge[it->first] ^ 1].flow;
		while (flag && positionPrice[it->first] != Level[0].first) {
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

    cout << "begin delete, num of servers = " << best_answer.size() << endl;
	cout << "begin delete at " << return_time() << " second" << endl;
	cout << "begin delete at " << all_cost << " cost" << endl;
	cout << endl;

    bool flag = false;
	servers = best_answer;
	for (vector<pair<int, int>>::iterator it = servers.begin(); it != servers.end(); ++it)
		it->second = MCF.edge[MCF.server2edge[it->first] ^ 1].flow;
	sort(servers.begin(), servers.end(), cmp2);
    vector<int> delete_answer;
	for (vector<pair<int, int>>::iterator it = servers.begin(); it != servers.end(); ++it) {
        if (find(delete_answer.begin(), delete_answer.end(), it->first) != delete_answer.end())
            continue;
        if (flag) {
            flag = false;
            answer.clear();
            for (vector<pair<int, int>>::iterator it = servers.begin(); it != servers.end(); ++it) {
                if (find(delete_answer.begin(), delete_answer.end(), it->first) == delete_answer.end())
                    answer.push_back(it->first);
            }
//            MCF.getTotalCost(answer, 0);
//            printf("num of servers = %lu, cost = %d\n", best_answer.size(), all_cost);
        }
		for (int i = MCF.G[it->first]; i; i = MCF.edge[i].next) {
			if (i % 2 == 1)
				continue;
			if (MCF.edge[i].to == MCF.vSink || MCF.edge[i].to == MCF.vSource)
				continue;
			if (find(answer.begin(), answer.end(), MCF.edge[i].to) == answer.end())
				continue;
			int cost_flow = MCF.edge[MCF.server2edge[MCF.edge[i].to] ^ 1].flow;
			if (MCF.edge[i].flow + MCF.edge[i ^ 1].flow >= cost_flow) {
                if (MCF.edge[MCF.server2edge[it->first] ^ 1].flow + MCF.edge[MCF.server2edge[MCF.edge[i].to] ^ 1].flow  <= Level[num_level - 1].first) {
                    MCF.edge[MCF.server2edge[it->first] ^ 1].flow += MCF.edge[MCF.server2edge[MCF.edge[i].to] ^ 1].flow;
                    delete_answer.push_back(MCF.edge[i].to);
                    flag = true;
                }
			}
		}
	}
    MCF.getTotalCost(answer, 0);

    cout << "begin delete, num of servers = " << best_answer.size() << endl;
    cout << "begin delete at " << return_time() << " second" << endl;
    cout << "begin delete at " << all_cost << " cost" << endl;
    cout << endl;

	int num_sround_min = 5;
	int num_sround;
	pair<int, int> tmp_first;
	pair<int, int> tmp_back;
	while (num_sround_min >= 1) {
        if (return_time() > time_max[1])
            break;
		servers = best_answer;
        for (vector<pair<int, int>>::iterator it = servers.begin(); it != servers.end(); ++it)
            it->second = MCF.edge[MCF.server2edge[it->first] ^ 1].flow;
		sort(servers.begin(), servers.end(), cmp);
		tmp_back = servers.back();
		while (servers.front() != tmp_back) {
            if (return_time() > time_max[1])
                break;
			tmp_first = servers.front();
			servers.erase(servers.begin());
			answer.clear();
			for (vector<pair<int, int>>::iterator it = servers.begin(); it != servers.end(); ++it) {
				answer.push_back(it->first);
			}
			num_sround = 0;
			for (int i = MCF.G[tmp_first.first]; i; i = MCF.edge[i].next) {
                if (return_time() > time_max[1])
                    break;
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
			//printf("num of servers = %lu, cost = %d\n", best_answer.size(), all_cost);
		}
		--num_sround_min;
	}

    cout << "begin move2, num of servers = " << best_answer.size() << endl;
    cout << "begin move2 at " << return_time() << " second" << endl;
    cout << "begin move2 at " << all_cost << " cost" << endl;
    cout << endl;

	vector<int> cheaper;
	vector<int>::iterator itc;

	vector<pair<int,int>>::iterator ita;
	servers= best_answer;
	int old;
	int current_edge;
	answer.clear();
	for (ita = servers.begin(); ita != servers.end(); ++ita){
		answer.push_back(ita->first);
	}

	for (ita = servers.begin(); ita != servers.end();){
        if (return_time() > time_max[2])
            break;
		cheaper.clear();
		old = ita->first;
		current_edge = MCF.G[old];
		while (current_edge){
            if (return_time() > time_max[2])
                break;
			if (current_edge %2 == 1 || MCF.edge[current_edge].to == MCF.vSource ||MCF.edge[current_edge].to == MCF.vSink){
				current_edge = MCF.edge[current_edge].next;
				continue;
			}
			if (positionPrice[MCF.edge[current_edge].to] < positionPrice[old])
				if (find(answer.begin(),answer.end(),MCF.edge[current_edge].to) == answer.end())
					cheaper.push_back(MCF.edge[current_edge].to);
			current_edge = MCF.edge[current_edge].next;
		}
		for (itc = cheaper.begin(); itc != cheaper.end(); ++itc){
            if (return_time() > time_max[2])
                break;
			ita->first = *itc;
			MCF.getTotalCost(servers);
		}
		servers = best_answer;
		*(answer.begin() + (ita - servers.begin())) = ita->first;
		if (ita->first == old)
			++ita;
	}

	cout << "begin add delete, num of servers = " << best_answer.size() << endl;
    cout << "begin add delete at " << return_time() << " second" << endl;
    cout << "begin add delete at " << all_cost << " cost" << endl;
    cout << endl;

	servers = best_answer;
	answer.clear();
	for (vector<pair<int, int>>::iterator it = servers.begin(); it != servers.end(); ++it) {
		answer.push_back(it->first);
	}
	vector<int> answer1;
	vector<int>::iterator itc2;
	vector<int> good;
	good = ReplaceNodes;
	for (itc = good.begin(); itc != good.end(); ++itc){
		if (return_time() > time_max[3])
			break;
		if (find(answer.begin(),answer.end(),*itc) == answer.end()){
			answer.push_back(*itc);
			for (itc2 = answer.begin(); itc2 != answer.end()-1; ){
				if (return_time() > time_max[3])
					break;
				answer1 = answer;
				answer1.erase(answer1.begin() + (itc2 - answer.begin()));
				if (MCF.getTotalCost(answer1,0)){
					answer = answer1;
					continue;
				}
				++itc2;
			}
		}
	}

    printf("begin push, num of servers = %lu\n", best_answer.size());
    cout << "begin push at " << return_time() << " second" << endl;
    cout << "begin push at " << all_cost << " cost" << endl;
    cout << endl;

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

//	for (ita = best_answer.begin(); ita != best_answer.end(); ++ita){
//		cout << "{" << ita->first << "," << ita->second << "}";
//	}
//	cout << endl;

	MCF.getTotalCost(best_answer);
	MCF.printAllPath();
	sprintf(temp, "%d\n\n", MCF.flowCnt);
	strcat(temp, out);

// 需要输出的内容
	char * topo_file = (char *) temp;

#ifdef PRINT_COST
    printf("server num = %lu\n",best_answer.size());
	printf("all cost = %d\n",all_cost);
#endif
	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);
#ifdef PRINT_TIME
	printf("now time is %d\n",return_time());
#endif
}
