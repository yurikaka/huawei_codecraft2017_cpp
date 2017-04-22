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

//todo  bfs对地价进行统计  同时对平均cost进行统计
/**************************新加变量****************************/
int out_degree[NODEMAX] = {0};//出度
int out_flow[NODEMAX] = {0};//出边流量
double out_percent[NODEMAX] = {0};
int node_actual_flow[NODEMAX] = {0};//todo 每次跑solve的时候统计，每次跑完后统计策略后需要清除
int source_server_flow[NODEMAX] = {0};

int total_lost_flow = 0;//总的缺失费用
unordered_map<int,int>consumer_lost_flow;//每个消费节点的缺失费用
unordered_map<int,int>consumer_needed_flow;//每个消费节点的需求


unordered_set<int> deletedNodes;//删除过的点不会在加进来

unordered_set<int>consumerNodes;

int node_selected_cnt[NODEMAX] = {0};//每个点被选择的次数
int node_level[NODEMAX] = {0};

/**************************动态规划分组背包****************************/
//todo 动态规划分组背包  很关键的近似解
//有多少个点就有多少个组，bandwidth是容量(即总需求)，cost是价格，每组等级个点
//先统计  每个点的价值 buildGraph 时统计 价值包括链路成本(链路成本用加权平均)
int node_average_chain_cost[NODEMAX];
int node_cost[NODEMAX];
int current_price = 0;

pair<int,int> group[NODEMAX][12];

typedef vector<pair<int,int>> vpii;

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



vector<int> combine(vector<int>A,vector<int>B){
    set<int>result_set;
    for(auto it = A.begin(); it != A.end(); ++it){
        result_set.insert(*it);
    }
    for(auto it = B.begin(); it != B.end(); ++it){
        result_set.insert(*it);
    }
    return vector<int>(result_set.begin(),result_set.end());
}

vector<pair<int,int>> combinePairAnswer(vector<pair<int,int>>A,vector<pair<int,int>>B){
    for(auto it = B.begin(); it != B.end(); ++it){
        A.push_back(*it);
    }
    return A;
}

vector<int> interSection(vector<int>A,vector<int>B){
    set<int> result;
    vector <int> answer;
    for(auto it = A.begin(); it != A.end(); ++it){
        result.insert(*it);
    }
    for(auto it = B.begin(); it != B.end(); ++it){
        if(result.count(*it)) answer.push_back(*it);
    }
    return answer;
}

vector<int> A_minus_B(vector<int> A, vector<int> B){
    vector<int>result;
    unordered_set<int> tmpset;
    for(int i = 0; i < B.size(); ++i){
        tmpset.insert(B[i]);
    }
    for(int i = 0; i < A.size(); ++i){
        if(tmpset.count(A[i]))continue;
        result.push_back(A[i]);
    }
    return result;
}

void printVector(vector<int>v){
    sort(v.begin(),v.end());
    for(int i = 0;  i< v.size(); ++i){
        cout << v[i] << endl;
    }
}

void printNodeActualFlow(vector<int>nodes){
    for(int i = 0; i < nodes.size(); ++i){
        cout<<"node: "<< nodes[i]  << "    actual flow" << node_actual_flow[nodes[i]] << "  ability  "<< out_flow[nodes[i]] << endl;
    }
}


void BFSOverLap(vector<int> nodes){
    //todo 过滤掉消费点，过滤掉已有服务器，过滤掉已经删除的点
    //todo 对需要求得这些节点分别进行bfs一层，然后求交集，然后根据某种算法排序，选取一个最小的相交，求交集的元素包括自己本身，先遍历一层
    //todo  这是添加节点的算法 也可以是对原有服务器升级！！！
    //todo 先bfs简单遍历一层

//    unordered_set<int>serverset(servers.begin(),servers.end()); todo 后续要加上这部分
//    if(serverset.count(i) || consumerNodes.count(i) || deletedNodes.count(i)) continue;

    unordered_map<int,int> nodemap;

    unordered_set<int> result;
    for(int i = 0; i < nodes.size(); ++i){
        for(int _i = MCF.G[nodes[i]]; _i ; _i = MCF.edge[_i].next){
            if(_i&1)continue;
            int v = MCF.edge[_i].to;
            if (nodemap.count(v))++nodemap[v];
            else{
                nodemap[v] = 1;
            }
        }
    }
    vpii tmp;
    for(auto it = nodemap.begin(); it != nodemap.end(); ++it){
        tmp.push_back(make_pair(it->second,it->first));
    }
    sort(tmp.begin(),tmp.end());
    reverse(tmp.begin(),tmp.end());
    int cnt = 0;

    for(auto it = tmp.begin();it != tmp.end(); ++it){
        ++cnt;
        if(cnt == 10)break;
        cout << "count "<< it->first << "   node   "<< it->second << endl;
    }

}

vector<int> notEnoughConsumers(){
    vector<int> result;
    for(auto it = consumer_lost_flow.begin(); it!= consumer_lost_flow.end(); ++it){
        if(it->second > 0) {
            result.push_back(it->first);
        }
    }
    return result;
}


vector<int> printConsumerLost( ){
//    非零点
    int notzero_cnt = 0;
    int tmpTotalLost = 0;
    vector<pair<int,int>>tmp_lost;
    for(auto it = consumer_lost_flow.begin(); it!= consumer_lost_flow.end(); ++it){
        if(it->second > 0) {
            ++notzero_cnt;
            tmp_lost.push_back(make_pair(it->second,it->first));
        }
    }
    sort(tmp_lost.begin(),tmp_lost.end());
    reverse(tmp_lost.begin(),tmp_lost.end());
    for(auto it = tmp_lost.begin(); it != tmp_lost.end(); ++it){
        cout<<"consumer lost" << it->first << " node  "<< it->second<<endl;
    }
//    cout<< "still need: " << total_lost_flow << endl;
//    cout<<" still needed consumer count " << notzero_cnt << "total cousumer count" << consumer_lost_flow.size() << endl;
    vector<int> answer;
    int len = tmp_lost.size();
    for(int i = 0; i < len;++i){
        tmpTotalLost += consumer_needed_flow[tmp_lost[i].second];
        answer.push_back(tmp_lost[i].second);
        if(tmpTotalLost >= total_lost_flow) break;
    }
    return answer;
}

vector<pair<int,int>>getPairAnswerFromVector(vector<int> nodes){
    vector<pair<int,int>> result;
    for(int i = 0; i < nodes.size(); ++i){
        int tmplevel = 0;
        int minimum_ability = 0;
        if(consumerNodes.count(nodes[i])){
//            todo 这里可以做很多文章
            minimum_ability = consumer_needed_flow[nodes[i]];

        }else{
//            todo 这里可以做很多文章
            minimum_ability = min(Level[num_level - 1].first,out_flow[nodes[i]]);
        }
        for(int level = 0; level < num_level; ++level){
            if(minimum_ability <= Level[level].first){

                tmplevel = level;
                break;
            }
        }
        result.push_back(make_pair(nodes[i],tmplevel));
    }
    return result;
};


vector<pair<int,int>>adjustNodes(vector<pair<int,int>> nodes){
//    todo 需要策略去做这些事情
    //todo 如果服务器等级很低，即性价比比较低，同时服务器的实际输出比它的能高，同时出度还没有满足，那么我们升级服务器，当然不能升到最高级

};

vector<pair<int,int>> deleteLowEffiecientServers(vector<pair<int,int>> servers){

    for(auto it = servers.begin(); it != servers.end(); ++it){
        int v = it->first;
        float ability = min(out_flow[v],Level[node_level[v]].first);
        if( ability/node_actual_flow[v] >= 1.5){
            deletedNodes.insert(v);
        }
        if(node_level[v] == 0){
            deletedNodes.insert(v);
        }
    }
    vector<pair<int,int>> result;
    for(auto it = servers.begin(); it != servers.end(); ++it){
        int v = it->first;
        if(deletedNodes.count(v))continue;
        result.push_back(*it);
    }
    return result;
};

vpii addHotPotentialServersFromConsumer(vpii servers,vector<int>nodes){
    unordered_set<int> heihei;
    unordered_set<int> intersection;
    for(auto it = servers.begin(); it != servers.end(); ++it){
        heihei.insert(it->first);
    }
    for(int i = 0; i < nodes.size(); ++i){
        if(heihei.count(nodes[i])) {
            intersection.insert(nodes[i]);
            continue;
        }
        node_level[nodes[i]] = 0;
        servers.push_back(make_pair(nodes[i],0));
    }
    for(auto it = servers.begin(); it != servers.end(); ++it){
        if(intersection.count(it->first)) {
            it->second += 1;
            node_level[it->first] = it->second;
        }
    }
    return servers;
};
vector<pair<int,int>> addHotPotentialServersFromOutOfConsumer(vector<pair<int,int>>servers){
//    todo 选择不在deletedNodeSet 里面同时加起来又大于总需求的

};


//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num, char * filename)
{

    return_time();
	MCF.clear();
	MCF.buildGraph(topo);


    vpii dp_answer = MCF.DPForAnswer();

    MCF.getTotalCost(dp_answer);
    MCF.statistic();

    dp_answer = deleteLowEffiecientServers(dp_answer);//去除低效的点

    vector<int> dp_vector;
    for(int i = 0; i < dp_answer.size(); ++i){
        dp_vector.push_back(dp_answer[i].first);
    }



    vector<int>c_answer = printConsumerLost();
    vector<int>c_intersect_dp = A_minus_B(c_answer,dp_vector);
    cout <<"fuck" <<c_intersect_dp.size() << endl;
    vpii c_pair_answer = getPairAnswerFromVector(c_intersect_dp);
    vpii myAnswer = combinePairAnswer(c_pair_answer,dp_answer);

    int minimumLost = INT32_MAX;
    int price = 0;
    int cnt = 10;
    vpii myResult;
    while(--cnt) {
        MCF.getTotalCost(myAnswer);
        MCF.statistic();


        if (total_lost_flow < minimumLost){
            price = current_cost;
            minimumLost = total_lost_flow;
            vector<int> tmp_consumer = notEnoughConsumers();
            myResult = addHotPotentialServersFromConsumer(myAnswer,tmp_consumer);

        }



        myAnswer = deleteLowEffiecientServers(myAnswer);
        vector<int> tmpMyAnser;

        for(auto it = myAnswer.begin(); it != myAnswer.end(); ++it){
            tmpMyAnser.push_back(it->first);
        }
        c_answer = printConsumerLost();
        c_answer = A_minus_B(c_answer,tmpMyAnser);
        c_pair_answer = getPairAnswerFromVector(c_answer);

        myAnswer = combinePairAnswer(myAnswer,c_pair_answer);

    }




    MCF.getTotalCost(myResult);
    MCF.statistic();
    for (auto it = myResult.begin(); it != myResult.end(); ++it) {
        if(it->second == 0)continue;
        while(it->second != 0 && node_actual_flow[it->first] <= Level[it->second - 1].first) {
            it->second -= 1;
        }
    }
    MCF.getTotalCost(myResult);
    cout <<" minimumLost " <<total_lost_flow <<" price " <<current_cost << " my size  "<<myResult.size() << endl;
    if(MCF.isFeasibleFlow()) {
        for(int i = 0; i < myResult.size(); ++i){
            node_level[myResult[i].first] = myResult[i].second;
        }
        cout<< "feasible" << current_cost << endl;
    }
	MCF.printAllPath();
	sprintf(temp, "%d\n\n", MCF.flowCnt);
	strcat(temp, out);

// 需要输出的内容
	char * topo_file = (char *) temp;

#ifdef PRINT_COST
    printf("server num = %lld\n",myResult.size());
	printf("all cost = %d\n",all_cost);
#endif
	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(topo_file, filename);
#ifdef PRINT_TIME
	printf("now time is %d\n",return_time());
#endif
}
