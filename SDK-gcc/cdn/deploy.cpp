#include "deploy.h"

#define PRINT_COST
#define PRINT_TIME

char out[550000];
char temp[550000];
int all_cost = INT32_MAX;
int current_cost;

int num_level;

int mcf_times = 0;
vector <pair<int,int>> best_answer;
vector <int> DirectNode;

extern const int NODEMAX, EDGEMAX, INF;

unordered_map<int,pair<int,int>> Level;

int positionPrice[NODEMAX + 10];

bool push_better;
bool pushable;
bool better;

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


void printOrderServerLevel(){
    vpii result;
    for(int i = 0; i < Level.size(); ++i){
        result.push_back(make_pair(float(Level[i].second) /float(Level[i].first),i));
    }
    sort(result.begin(),result.end());
    for(auto it = result.begin(); it != result.end(); ++it){
//        cout<< "cost  " << it->first << "level  " << it->second;
    }
}

vector<int> getVectorConsumerLost( ){
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
//        cout<<"consumer lost" << it->first << " node  "<< it->second<<endl;
    }

    vector<int> answer;
    int len = tmp_lost.size();
    for(int i = 0; i < len;++i){
        tmpTotalLost += consumer_needed_flow[tmp_lost[i].second];
        answer.push_back(tmp_lost[i].second);
        if(tmpTotalLost >= total_lost_flow) break;
    }
    return answer;
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
//        cout << v[i] << endl;
    }
}

void printNodeActualFlow(vector<int>nodes){
    for(int i = 0; i < nodes.size(); ++i){
//        cout<<"node: "<< nodes[i]  << "    actual flow" << node_actual_flow[nodes[i]] << "  ability  "<< out_flow[nodes[i]] << endl;
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
//        cout << "count "<< it->first << "   node   "<< it->second << endl;
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

vector<pair<int,int>>getPairAnswerFromVector(vector<int> nodes,set<int> anserset){
    vector<pair<int,int>> result;
    for(int i = 0; i < nodes.size(); ++i){
        if(anserset.count(nodes[i]))continue;
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

vpii getAnswerFromConsumerLost(vpii servers){

//    非零点
    set <int> answerSet;
    for(int i = 0; i < servers.size(); ++i){
        answerSet.insert(servers[i].first);
        if(consumerNodes.count(servers[i].first)){
            servers[i].second += 1;
        }
    }
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
//        cout<<"consumer lost" << it->first << " node  "<< it->second<<endl;
    }


    vector<int> answer;
    int len = tmp_lost.size();
    for(int i = 0; i < len;++i){
        tmpTotalLost += consumer_needed_flow[tmp_lost[i].second];
        answer.push_back(tmp_lost[i].second);
        if(tmpTotalLost >= total_lost_flow) break;
    }

    vpii result = getPairAnswerFromVector(answer,answerSet);
    for(int i = 0 ; i < result.size(); ++i){
        servers.push_back(result[i]);
    }
    return servers;
}

vector<pair<int,int>>adjustNodes(vector<pair<int,int>> nodes){
    //todo 需要策略去做这些事情
    // todo 如果服务器等级很低，即性价比比较低，同时服务器的实际输出比它的能高，同时出度还没有满足，那么我们升级服务器，当然不能升到最高级
        for (auto it = nodes.begin(); it != nodes.end(); ++it) {
            if(it->second == 0)continue;
            while(it->second != 0 && node_actual_flow[it->first] <= Level[it->second - 1].first) {
                it->second -= 1;
            }
    // todo 很关键
            node_level[it->first] = it->second < 0 ? 0 : it->second;
        }
        return nodes;
};


//vector<pair<int,int>>adjustNodes(vector<pair<int,int>> nodes){
//    //todo 需要策略去做这些事情
//    //todo 如果服务器等级很低，即性价比比较低，同时服务器的实际输出比它的能高，同时出度还没有满足，那么我们升级服务器，当然不能升到最高级
//    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
//        if(it->second == 0)continue;
//        while(it->second != 0 && node_actual_flow[it->first] <= Level[it->second - 1].first) {
//            it->second -= 1;
//        }
//    }
//    return nodes;
//};

vector<pair<int,int>> addServer(vpii answer,vector<int> server){
    for(auto it = server.begin(); it != server.end(); ++it){
       answer.push_back(make_pair(*it,0));
    }
    return answer;
};


vector<pair<int,int>> upperServer(vpii answer,vector<int> server){
    set<int> tmpSet(server.begin(),server.end());
    for(auto it = answer.begin(); it != answer.end(); ++it){
        if(tmpSet.count(it->first)){
            if(it->second < num_level){
                it->second += 1;
            }
        }
    }
    return answer;
};


vector<pair<int,int>> shengjidayu(vector<pair<int,int>> servers){
    for(auto it = servers.begin(); it != servers.end(); ++it){
        int v = it->first;
        float ability = min(out_flow[v],Level[node_level[v]].first);
        if (node_actual_flow[v] > ability){
            if (it->second < num_level - 1)
                ++it->second;
        }
    }
    return servers;
};

vector<pair<int,int>> deleteLowEffiecientServers(vector<pair<int,int>> servers){

    for(auto it = servers.begin(); it != servers.end(); ++it){
        int v = it->first;
        float ability = min(out_flow[v],Level[node_level[v]].first);
        if( ability/node_actual_flow[v] >= 1.6){
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

//缺失流量的服务器
//vector<int> bfs(vector<int> servers,vector<int> consumers){
//    unordered_map<int,set<int>> consumerMap;
//
//    set<int> mySet(consumers.begin(),consumers.end());
//    for(auto c : consumers){
//        consumerMap[c] = {};
//    }
//    set<int> visited;
//
//    for (auto server: servers){
//        queue<int> Q;
//        Q.push(server);
//        while(!Q.empty()){
//            int tmpv = Q.front();
//            Q.pop();
//            if(visited.count(tmpv))continue;
//            visited.insert(tmpv);
//            for(int i = MCF.G[tmpv]; i ; i = MCF.edge[i].next){
//                if (i & 1) continue;
//                if (MCF.edge[i].flow == 0) continue;
//                if (MCF.edge[i^1].flow == 0)continue;
//                int node = MCF.edge[i].to;
//                if(mySet.count(node))consumerMap[node].insert(server);
//                Q.push(node);
//            }
//        }
//        visited.clear();
//    }
//    unordered_map<int,int> tmpmap;
//    for(auto c : consumers){
////        cout << "cousumer  " << c << ": servers";
//        for(auto it = consumerMap[c].begin(); it != consumerMap[c].end(); ++it){
////            cout << " " << *it << " ";
//            if(tmpmap.count(*it)){
//                tmpmap[*it] += 1;
//            }else{
//                tmpmap[*it] = 1;
//            }
//        }
////        cout << endl;
//    }
//    vpii res;
//    for(auto it = tmpmap.begin(); it != tmpmap.end(); ++it){
//        res.push_back(make_pair(it->second,it->first));
//    }
//    sort(res.begin(),res.end());
//    reverse(res.begin(),res.end());
//    vector<int> resv;
//    for(auto it = res.begin(); it != res.end(); ++it){
//        resv.push_back(it->second);
//    }
//    return resv;
//}

int getLevel(int flow){
    if(flow == 0){
        return -1;
    }
    if(flow > Level[num_level - 1].first)return num_level - 1;
    int level;
    level = 0;
    for(int i = 0; i < num_level; ++i){
        if(flow <= Level[i].first){
            level = i;
            break;
        }
    }
    return level;
}

vpii getEfficientBackEndServers(vpii exclude,int num){
//  todo 也可以通过排序来提高某些满流量的服务器的等级 同时降低其它流量服务器的等级
    vector<int> excludeServers;
    for(int i = 0; i < exclude.size(); ++i){
        excludeServers.push_back(exclude[i].first);
    }

    set<int>exset(excludeServers.begin(),excludeServers.end());
    vector<pair<int ,pair<int,int>>> ret;
    for(int node = 0; node < MCF.nodeNumber; ++node){
        if(exset.count(node)) continue;
        for(int level = 0; level < num_level; ++level){
            int bandwidth = min(Level[level].first,out_flow[node]);
//                模拟实际情况求得cost
            int cost = Level[level].second + positionPrice[node] + bandwidth * node_average_chain_cost[node];
            ret.push_back(make_pair(cost,make_pair(node,level)));
        }
    }
    sort(ret.begin(),ret.end());
    int len = ret.size();
    set<int>quchong;
    vpii result;
    for(int i = 0; i < min(len,num); ++i){
        if(quchong.count(ret[i].second.first))continue;
        result.push_back(ret[i].second);
    }
    for(int i = 0; i < result.size(); ++i){
//        cout<< "back server   " <<result[i].first << " level: " << result[i].second << endl;
    }
    return result;
}

vpii addServersFromConsumer(vpii servers,vector<int>nodes){
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
        node_level[nodes[i]] = getLevel(consumer_needed_flow[nodes[i]] - node_actual_flow[nodes[i]]);
        servers.push_back(make_pair(nodes[i],getLevel(consumer_needed_flow[nodes[i]] - node_actual_flow[nodes[i]])));
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

vector<pair<int,int>> bfs(vector<int> servers,vector<int> consumers){
    unordered_map<int,set<int>> consumerMap;

    set<int> mySet(consumers.begin(),consumers.end());
    for(auto c : consumers){
        consumerMap[c] = {};
    }
    set<int> visited;

    for (auto server: servers){
        queue<int> Q;
        Q.push(server);
        while(!Q.empty()){
            int tmpv = Q.front();
            Q.pop();
            if(visited.count(tmpv))continue;
            visited.insert(tmpv);
            for(int i = MCF.G[tmpv]; i ; i = MCF.edge[i].next){
                if (i & 1) continue;
                if (MCF.edge[i].flow == 0) continue;
                if (MCF.edge[i^1].flow == 0)continue;
                int node = MCF.edge[i].to;
                if(mySet.count(node))consumerMap[node].insert(server);
                Q.push(node);
            }
        }
        visited.clear();
    }
    unordered_map<int,int> tmpmap;
    for(auto c : consumers){
        cout << "cousumer  " << c << ": servers";
        for(auto it = consumerMap[c].begin(); it != consumerMap[c].end(); ++it){
            cout << " " << *it << " ";
            if(tmpmap.count(*it)){
                tmpmap[*it] += 1;
            }else{
                tmpmap[*it] = 1;
            }
        }
        cout << endl;
    }
    vpii res;
    for(auto it = tmpmap.begin(); it != tmpmap.end(); ++it){
        res.push_back(make_pair(it->second,it->first));
    }
    sort(res.begin(),res.end());
    reverse(res.begin(),res.end());
    vector<int> resv;
    for(auto it = res.begin(); it != res.end(); ++it){
        resv.push_back(it->second);
    }

//    todo 这里可以做文章
    vector< int >finalresult;
    int manzu = total_lost_flow;
    int tmpflow = 0 ;
    for(int i = 0 ; i < resv.size(); ++i){
        if(tmpflow < total_lost_flow){
            finalresult.push_back(resv[i]);
            tmpflow += node_actual_flow[resv[i]];
        }
    }
    vpii finalPair;
    for(int i = 0; i < finalresult.size(); ++i){
        finalPair.push_back(make_pair(finalresult[i],getLevel(node_actual_flow[finalresult[i]])));
    }
    return finalPair;
}


vpii budian(vpii a2,vector<int>tmp_consumer){

    vector<int> serverVector;
    for(int i = 0; i < a2.size(); ++i){
        serverVector.push_back(a2[i].first);
    }
    vector<int> allVector;
    for(int i = 0; i < MCF.nodeNumber; ++i){
        allVector.push_back(i);
    }

    vector<int> notServer = A_minus_B(allVector,serverVector);
    notServer = A_minus_B(notServer,tmp_consumer);
    vpii bfsV = bfs(notServer,tmp_consumer);
    a2 = combinePairAnswer(a2,bfsV);
    return a2;
}

bool cmp_flow_down(int p, int q){
    return node_actual_flow[p] > node_actual_flow[q];
}

bool cmp_level_up(pair<int,int> p, pair<int,int> q){
    return p.second < q.second;
}
bool cmp_level_up1(pair<int,int> p, pair<int,int> q){
    if (p.second == q.second)
        return node_actual_flow[p.first] < node_actual_flow[q.first];
    else
        return p.second < q.second;
}


//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num, char * filename)
{
    //    vector<pair<int,int>>answer_0{{0,4},{45,5},{55,5},{56,4},{60,4},{78,5},{105,4},{107,5},{133,4},{134,5},{142,4},{152,0},{161,1},{177,4},{236,3},{242,3},{245,4},{274,4},{278,5},{290,2},{291,0},{296,4},{314,3},{333,4},{343,2},{359,4},{373,4},{389,4},{390,2},{394,3},{409,4},{411,4},{416,4},{445,5},{458,2},{460,3},{467,4},{470,4},{495,5},{497,5},{515,4},{518,1},{526,4},{527,1},{538,4},{556,4},{557,5},{570,5},{577,4},{582,4},{586,3},{597,4},{615,1},{617,3},{625,5},{640,4},{641,4},{650,4},{656,3},{657,3},{666,4},{669,4},{683,2},{688,4},{697,4},{700,3},{714,4},{724,4},{751,4},{767,4},{804,4},{835,5},{847,3},{872,4},{883,4},{894,4},{920,1},{934,4},{940,4},{952,5},{970,0},{984,5},{991,2},{993,4},{1002,4},{1017,4},{1019,1},{1029,1},{1031,4},{1032,4},{1034,5},{1053,1},{1056,4},{1070,2},{1076,0},{1080,5},{1090,4},{1103,3},{1109,5},{1110,3},{1118,4},{1119,5},{1184,3},{1187,4}};
    return_time();
	MCF.clear();
	MCF.buildGraph(topo);


// 构造迭代
    vpii dp_answer = MCF.DPForAnswer();

    MCF.getTotalCost(dp_answer);
    int minimumLost = INT32_MAX;
    int minimumResult = INT32_MAX;
    int cnt = 10;
    vpii result_1;
    while (--cnt) {
        dp_answer = deleteLowEffiecientServers(dp_answer);//去除低效的点
        MCF.getTotalCost(dp_answer);
        dp_answer = getAnswerFromConsumerLost(dp_answer);
        MCF.getTotalCost(dp_answer);
        if(total_lost_flow < minimumLost){
            minimumLost = total_lost_flow;
            result_1 = dp_answer;
            vector<int> tmp_consumer = notEnoughConsumers();
            result_1 = addServersFromConsumer(result_1,tmp_consumer);
        }
    }




    vector<pair<int,int>> a2;
    int last_cost;

    MCF.getTotalCost(result_1);
    result_1 = adjustNodes(result_1);
    MCF.getTotalCost(result_1);


//    result_1.clear();
//    for (int zz = 0; zz < MCF.nodeNumber; ++zz){
//        if (consumer_needed_flow.find(zz) == consumer_needed_flow.end())
//            result_1.push_back(pair<int,int>(zz,num_level-1));
//    }
//
//    MCF.getTotalCost(result_1);
////    result_1 = adjustNodes(result_1);
////    MCF.getTotalCost(result_1);
//    vector<int> tmp_consumer = notEnoughConsumers();
//    result_1 = addServersFromConsumer(result_1,tmp_consumer);
//    MCF.getTotalCost(result_1);
//    result_1 = adjustNodes(result_1);
//    MCF.getTotalCost(result_1);
//    cout << MCF.isFeasibleFlow() << endl;


//    for (auto zz = consumer_needed_flow.begin(); zz != consumer_needed_flow.end(); ++zz){
//        result_1.push_back(pair<int,int>(zz->first,getLevel(zz->second)));
//    }

//    result_1 = shengjidayu(result_1);
//    MCF.getTotalCost(result_1);

    result_1 = shengjidayu(result_1);
    sort(result_1.begin(),result_1.end(),cmp_level_up1);
    int iter_num = result_1.size();
    vpii::iterator it = result_1.begin();
    while (iter_num--) {
        if (return_time() > 87)
            break;
        it = result_1.begin();
        pair<int, int> now = *it;
        result_1.erase(it);
        int last_cost = all_cost;
        MCF.getTotalCost(result_1);
        sort(result_1.begin(),result_1.begin()+iter_num,cmp_level_up1);
        if (!MCF.isFeasibleFlow()){
            vector<int> tmp_consumer = notEnoughConsumers();
            a2 = result_1;
            a2 = addServersFromConsumer(a2,tmp_consumer);
//            a2 = budian(a2,tmp_consumer);
            MCF.getTotalCost(a2);
            result_1.push_back(now);
        }
        else if (all_cost == last_cost) {

            result_1.push_back(now);
        }
    }

    unordered_set<int> nownow;
    vector<int> notin;
    if (return_time() < 87){
        for (auto it3 = best_answer.begin(); it3 != best_answer.end(); ++it3){
            nownow.insert(it3->first);
        }
        for (int z = 0; z < MCF.nodeNumber; ++z){
            if (nownow.find(z) == nownow.end())
                notin.push_back(z);
        }
        sort(notin.begin(),notin.end(),cmp_flow_down);
//        for (auto it4 = notin.begin(); it4 != notin.end(); it4+=2){
//            if (return_time() > 82)
//                break;
//            if (node_actual_flow[*it4] == 0)
//                break;
//            result_1 = best_answer;
//            result_1.push_back(pair<int,int>(*it4,getLevel(node_actual_flow[*it4])));
//            if (getLevel(node_actual_flow[*(it4+1)]) != -1)
//                result_1.push_back(pair<int,int>(*(it4+1),getLevel(node_actual_flow[*(it4+1)])));
        for (auto it4 = notin.begin(); it4 != notin.end(); ++it4){
            if (return_time() > 87)
                break;
            if (node_actual_flow[*it4] == 0)
                break;
            result_1 = best_answer;
            result_1.push_back(pair<int,int>(*it4,getLevel(node_actual_flow[*it4])));
            sort(result_1.begin(),result_1.end(),cmp_level_up1);
            iter_num = result_1.size();
            it = result_1.begin();
            while (iter_num--) {
                if (return_time() > 87)
                    break;
                it = result_1.begin();
                pair<int, int> now = *it;
                result_1.erase(it);
                int last_cost = all_cost;
                MCF.getTotalCost(result_1);
                sort(result_1.begin(),result_1.begin()+iter_num,cmp_level_up1);
                if (!MCF.isFeasibleFlow()){
                    vector<int> tmp_consumer = notEnoughConsumers();
                    a2 = result_1;
                    a2 = addServersFromConsumer(a2,tmp_consumer);
//                    a2 = budian(a2,tmp_consumer);
                    MCF.getTotalCost(a2);
                    result_1.push_back(now);
                }
                else if (all_cost == last_cost) {

                    result_1.push_back(now);
                }
            }
        }
    }

//    result_1 = shengjidayu(result_1);
//    MCF.getTotalCost(result_1);
////    result_1 = adjustNodes(result_1);
////    MCF.getTotalCost(result_1);
//    //sort(result_1.begin(),result_1.end(),cmp_flow_down);
//    //result_1.pop_back();
//    for (auto it2 = result_1.begin(); it2 != result_1.end();){
////        if (return_time() > 86)
////            break;
//        a2 = result_1;
//        a2.erase(a2.begin() + (it2 - result_1.begin()));
//        last_cost = all_cost;
//        MCF.getTotalCost(a2);
//        a2 = adjustNodes(a2);
//        MCF.getTotalCost(a2);
//        if(all_cost < last_cost){
//            result_1.erase(it2);
//            continue;
//        }
//        ++it2;
//    }

//    result_1 = adjustNodes(result_1);
//    MCF.getTotalCost(result_1);

//    best_answer = adjustNodes(best_answer);
    MCF.getTotalCost(best_answer);
    cout << "minimumLost  " << minimumLost << " current "<< current_cost << endl;
    if(MCF.isFeasibleFlow()){
        cout << "yes" << current_cost << endl;
    }


	MCF.printAllPath();
	sprintf(temp, "%d\n\n", MCF.flowCnt);
	strcat(temp, out);

// 需要输出的内容
	char * topo_file = (char *) temp;

	write_result(topo_file, filename);
#ifdef PRINT_TIME
	printf("now time is %d\n",return_time());
    cout << "net flow run " << mcf_times << " times" << endl;
#endif
}

