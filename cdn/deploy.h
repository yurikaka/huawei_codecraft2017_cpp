#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "lib_io.h"
#include <string.h>
#include <map>
#include <limits.h>
#include <stdio.h>
#include <queue>
#include <set>
#include <memory.h>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <stdint.h>
#include <random>
#include <chrono>



//for return time
#include <assert.h>
#include <time.h>
#include <sys/timeb.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

using namespace std;


/* sj

/*
输出文件得限制（并不是输入文件得限制）
1. 网络路径数量不得超过300000条。
2. 单条路径的节点数量不得超过10000个。
3. 不同网络路径可按任意先后顺序输出。
4. 网络节点ID与消费节点ID的数值必须与输入文件相符合，如果ID数值不存在于输入文件中，则将被视为无效结果。
5. 文本文件中出现的所有数值必须为大于等于0的整数，数值大小不得超过1000000。
*/

//todo 需要注意EDGEMAX 得值，最好在读取文件得过程中重新分配一次 INF的设置还需要稍微琢磨一下(比如出题人的数据量)
extern char out[550000];
extern char temp[550000];
extern int all_cost;
extern int current_cost;

extern int num_level;

extern int mcf_times;
extern vector <pair<int,int>> best_answer;
extern vector <int> DirectNode;

const int NODEMAX = 1500, EDGEMAX = 50000, INF = 0x3f3f3f3f ;

extern unordered_map<int,pair<int,int>> Level;
extern int positionPrice[NODEMAX + 10];

extern bool push_better;
extern bool pushable;
extern bool better;

extern int nodesOutFlow[NODEMAX + 10];
extern vector <int> ReplaceNodes;


//新版本逻辑
/**************************新加变量****************************/
extern int out_degree[NODEMAX];//出度
extern int out_flow[NODEMAX];//出边流量
extern double out_percent[NODEMAX];


extern int node_actual_flow[NODEMAX];//todo 每次跑完后统计策略后需要清除
extern int total_lost_flow;//总的缺失费用
extern unordered_map<int,int>consumer_lost_flow;//每个消费节点的缺失费用
extern unordered_map<int,int>consumer_needed_flow;//每个消费节点的需求

extern int node_selected_cnt[NODEMAX];//todo 每个点被选择的次数
extern int node_level[NODEMAX];//每个消费节点的level
extern int source_server_flow[NODEMAX];



extern unordered_set<int>consumerNodes;
extern unordered_set<int> deletedNodes;//删除过的点不会在加进来

/**************************动态规划分组背包****************************/
//todo 动态规划分组背包  很关键的近似解
//有多少个点就有多少个组，bandwidth是容量(即总需求)，cost是价格，每组等级个点
//先统计  每个点的价值 buildGraph 时统计 价值包括链路成本(链路成本用加权平均)
extern int node_average_chain_cost[NODEMAX];//加权平均 建图的时候求一次 为价值做辅助
extern int node_cost[NODEMAX];//包含地价以及链路成本
extern pair<int,int> group[NODEMAX][12];


//edge 的定义
struct Edge{
    Edge(){};
//  from -- > to   flow为当前流量， cost为单位费用， next为from开始得下一条边
    int to, flow, cost, next;
    Edge(int _to, int _flow, int _cost,int _next){
        to = _to;
        flow = _flow;
        cost = _cost;
        next = _next;
    };
};

//最小费用最大流
struct MCMF{

/*+++++++++++++++节点数量信息++++++++++++++++++*/
    int nodeNumber,edgeNumber,consumerNumber;
    int vSource,vSink; //虚拟源和虚拟汇
    int needSum; // 官方需要的总流量
    int currentFlows; // 当前部署下的总流量
    int edgeorder; //记录当前加到第几条边了
    int flowCnt = 0; // 打印流时一打印了多少条路径
    int solveTimes = 0; // 记录一共调用了求解多少次
    int maxBandWidth;
    int levelNum = 0;
    int mincost = 0;
    int cost = 0;

    /*+++++++++++++++zkw  变量++++++++++++++++++*/
    bool vis[NODEMAX];
    int oricost[EDGEMAX];

    int G[NODEMAX];
    Edge edge[EDGEMAX];
    int Net2Consumer[NODEMAX];
    unordered_map <int,int> server2level;

/*+++++++++++++++求解最小流全局变量以及各种辅助变量++++++++++++++++++*/
    queue<int> Q;
    int dist[NODEMAX + 10];
    bool inQueue[NODEMAX +10];
    int preV[NODEMAX + 10]; //回溯打印节点需要
    int preE[NODEMAX + 10]; //回溯打印节点需要
    unordered_map<int,int>server2edge;


/*+++++++++++++++添加边前向星星++++++++++++++++++*/
    void addEdge(int from,int to,int capacity,int cost){

        edge[++edgeorder] = Edge(to,capacity,cost,G[from]);
        oricost[edgeorder] = cost;
        G[from] = edgeorder;
        edge[++edgeorder] = Edge(from,0,-cost,G[to]);
        oricost[edgeorder] = -cost;
        G[to] = edgeorder;
        ++out_degree[from];
        out_flow[from] += capacity;
    }



    /*+++++++++++++++zkw aug++++++++++++++++++*/
    int zkwAug(int node, int flow){
        if( node == vSink){
            mincost += cost * flow;
            currentFlows += flow;
            return flow;
        }
        vis[node] = true;
        int tmp = flow;
        for(int i = G[node]; i ; i = edge[i].next){

            if(edge[i].flow && !edge[i].cost && !vis[edge[i].to]){
                int delta = zkwAug(edge[i].to,tmp < edge[i].flow ? tmp : edge[i].flow );
                edge[i].flow -= delta;
                edge[i^1].flow += delta;
                tmp -= delta;
                if(!tmp) return flow;
            }
        }
        return flow - tmp;
    }
/*+++++++++++++++zkw modlabel()++++++++++++++++++*/
    bool zkwModlabel()
    {
        int delta = INF;
        for(int u = 0; u < nodeNumber+2; ++u)
            if(vis[u])
                for(int i = G[u]; i ; i = edge[i].next) {
                    if (edge[i].flow && !vis[edge[i].to] && edge[i].cost < delta) delta = edge[i].cost;
                }
        if(delta == INF) return false;
        for(int u = 0; u < nodeNumber+2; u++)
            if(vis[u])
                for(int i = G[u]; i; i = edge[i].next) {
                    edge[i].cost -= delta, edge[i ^ 1].cost += delta;
                }
        cost += delta;
        return true;
    }

    /*+++++++++++++++建图only once++++++++++++++++++*/

    void buildGraph(char *topo[EDGEMAX]){

        int pos = 0;
        mincost = 0;

        memset(G, 0, sizeof(G));
        memset(node_level,-1,sizeof(node_level));

        needSum = 0;
        edgeorder = 1;
        sscanf(topo[pos],"%d %d %d",&nodeNumber,&edgeNumber,&consumerNumber);


        vSink = nodeNumber + 1;
        vSource = nodeNumber;


        int level = 0,bandwidth = 0,price = 0;
        for(pos = 2; pos < 13; ++pos){
            if(topo[pos][0] == '\n' || topo[pos][0] == '\r' || topo[pos][0] == '\0') {
                maxBandWidth = bandwidth;
                break;
            }
            sscanf(topo[pos],"%d%d%d",&level,&bandwidth,&price);
            Level[pos-2].first = bandwidth;
            Level[pos-2].second = price;
        }
        num_level = pos - 2;
        pos++;

        int postion;
        for (int i = 0; i < nodeNumber; ++i) {
            sscanf(topo[i + pos], "%d%d", &postion, &price);
            positionPrice[postion] = price;

        }
        pos += nodeNumber +1;

        for (int i = 0; i < edgeNumber; ++i) {
            int _from,_to,_flow,_cost;
            sscanf(topo[i + pos],"%d %d %d %d",&_from,&_to,&_flow,&_cost);

            nodesOutFlow[_from] += _flow;
            nodesOutFlow[_to] += _flow;
            addEdge(_from,_to,_flow,_cost);
            addEdge(_to,_from,_flow,_cost);

        }
        pos += edgeNumber + 1;

        bool d = DirectNode.empty();
        for (int i = 0; i < consumerNumber; ++i) {
            int consumer,netnode,need;
            sscanf(topo[i + pos],"%d %d %d",&consumer,&netnode,&need);
            if(d){
                DirectNode.push_back(netnode);
            }
            nodesOutFlow[netnode] += need;
            addEdge(netnode,vSink,need,0);
            // 每个消费节点的需求
            consumer_needed_flow[netnode] += need;
            consumer_lost_flow[netnode] = 0;
            consumerNodes.insert(netnode);
            needSum += need;
            Net2Consumer[netnode] = consumer;
        }

        vector<pair <int,int>> rankNodes;
        for(int i = 0; i < nodeNumber; ++i){
            rankNodes.push_back(make_pair(nodesOutFlow[i],i));
        }
        sort(rankNodes.begin(),rankNodes.end());
        for(int i = 0; i < nodeNumber; ++i){
            ReplaceNodes.push_back(rankNodes[i].second);
        }
        reverse(ReplaceNodes.begin(),ReplaceNodes.end());

    }

    vector<pair<int,int>> DPForAnswer(){
//    求node_average_chain_cost 每个点的链路平均成本
        for (int node = 0; node < nodeNumber; ++node) {
            for(int i = G[node]; i ;i = edge[i].next){
                if(i&1) continue;
                int flow = edge[i].flow;
                int cost = edge[i].cost;
                node_average_chain_cost[node] +=(flow/out_flow[node])*cost;
            }
        }
//        初始化group
        for(int node = 0; node < nodeNumber; ++node){
            for(int level = 0; level < num_level; ++level){
                int bandwidth = min(Level[level].first,out_flow[node]);
//                模拟实际情况求得cost
                int cost = Level[level].second + positionPrice[node] + bandwidth * node_average_chain_cost[node];
                group[node][level] = make_pair(bandwidth,cost);
            }
        }

        int **dp = new int*[nodeNumber + 1];
        for (int i = 0; i < nodeNumber + 1; ++i)
            dp[i] = new int[needSum + 1];

        pair<int,int> **zhuangtai = new pair<int,int>*[nodeNumber + 1];
        for (int i = 0; i < nodeNumber + 1; ++i)
            zhuangtai[i] = new pair<int,int>[needSum + 1];


        memset(dp[0],0x3f, sizeof(int)*(needSum + 1));

        dp[0][0] = 0;
        int t = clock();
        for(int i = 0; i < nodeNumber; ++i){
            memcpy(dp[i+1], dp[i], (1 + needSum) * sizeof(int));
            memcpy(zhuangtai[i+1], zhuangtai[i], (1 + needSum) * sizeof(pair<int,int>));


            for(int need = 0; need <= needSum; ++need){
                for(int k = 0; k < num_level; ++k){
                    int tmpNeed = min(need+group[i][k].first,needSum);
                    if(dp[i+1][tmpNeed] > dp[i][need] + group[i][k].second){

                        dp[i+1][tmpNeed] = dp[i][need] + group[i][k].second;
                        zhuangtai[i+1][tmpNeed] = make_pair(i<<16|need,k);
                    }
                }
            }
        }

        int first,second;
        first = nodeNumber;
        second = needSum;
        int cnt = 0;
        vector<pair<int,int>>answer;
        while (first){
            pair<int,int>tmp;
            tmp = zhuangtai[first][second];
            first = tmp.first >> 16;
            second = tmp.first & 65535;
            answer.push_back(make_pair(first,tmp.second));
            ++cnt;
        }

        int result = 0;
        for(int i = 0; i < answer.size(); ++i){
            auto e = answer[i];
            result += group[e.first][e.second].second;
        }
        return answer;
    }



/*+++++++++++++++建超级源点++++++++++++++++++*/
    void buildSource(vector<int> servers){
        for(auto server:servers){
            // todo 这里流量不是无限大注意更改，cost也注意修改
            if(server2edge.count(server)){
                int tmpEdgeNum = server2edge[server];
                //todo 实际中肯定不是无穷大
                edge[tmpEdgeNum].flow = maxBandWidth;
                edge[tmpEdgeNum ^ 1].flow = 0;
                node_level[server] = num_level - 1;
            }else{
                //todo 实际中肯定不是无穷大
                addEdge(vSource,server,maxBandWidth,0);
                server2edge[server] = edgeorder - 1;
                node_level[server] = num_level - 1;
            }
        }
    }


    void buildSource(vector<pair<int,int>> servers){
        for(auto server:servers){
            // todo 这里流量不是无限大注意更改，cost也注意修改
            if(server2edge.count(server.first)){
                int tmpEdgeNum = server2edge[server.first];
                //todo 实际中肯定不是无穷大
                edge[tmpEdgeNum].flow = Level[server.second].first;
                edge[tmpEdgeNum ^ 1].flow = 0;
                node_level[server.first] = server.second;
            }else{
                //todo 实际中肯定不是无穷大
                addEdge(vSource,server.first,Level[server.second].first,0);
                server2edge[server.first] = edgeorder - 1;
                node_level[server.first] = server.second;
            }
        }
    }



/*+++++++++++++++clear上次的计算内容，图置为初始情况(源点)++++++++++++++++++*/
    void clear(){
        cost = 0;
        currentFlows = 0;
        flowCnt = 0;
        mincost = 0;

        memset(dist,63,sizeof(dist));
        memset(inQueue,0,sizeof(inQueue));
        memset(preV,0,sizeof(preV));
        memset(preE,0,sizeof(preE));
        memset(node_actual_flow,0,sizeof(node_actual_flow));
        memset(node_level,0,sizeof(node_level));
        memset(vis, 0 ,sizeof(vis));
        //memset(edge,0,sizeof(edge));
        //todo 所有边置为读进来的情况，注意有没有需要特殊处理的情况，比如汇点和源点
        for(int i = 2; i <= edgeorder; i += 2){
            edge[i].flow += edge[i^1].flow;
            edge[i^1].flow = 0;
        }
        //恢复cost
        for(int i = 2; i <= edgeorder; ++i){
            edge[i].cost = oricost[i];
        }

        for(auto it = server2edge.begin(); it != server2edge.end(); ++it){
            int tmpEdgeNum = (*it).second;
            edge[tmpEdgeNum].flow = 0;
            edge[tmpEdgeNum ^ 1].flow = 0;
        }
    }

/*+++++++++++++++clear上次的计算内容，顺便根据要删除的和不删除的点建好新的边++++++++++++++++++*/

    void deleteServers(vector<int> toDeletServers){
        for(auto server: toDeletServers){
            if(server2edge.count(server)){
                int tmpEdgeNum = server2edge[server];
                edge[tmpEdgeNum].flow = 0;
                edge[tmpEdgeNum ^ 1].flow = 0;
            }else{
                continue;
            }
        }
    }

    void addServers(vector<int> toAddServers){
        for(auto server: toAddServers){
            if(server2edge.count(server)){
                int tmpEdgeNum = server2edge[server];
                //todo  maxBandWidth
                edge[tmpEdgeNum].flow = maxBandWidth;
                edge[tmpEdgeNum ^ 1].flow = 0;
            }else{
                //todo maxBandWidth
                addEdge(vSource,server,maxBandWidth,0);
                server2edge[server] = edgeorder - 1;
            }
        }
    }

    void clear(vector<int> toDeletServers,vector<int> toAddServers){

        currentFlows = 0;
        flowCnt = 0;
        memset(dist,63,sizeof(dist));
        memset(inQueue,0,sizeof(inQueue));
        memset(preV,0,sizeof(preV));
        memset(preE,0,sizeof(preE));

        //todo 所有边置为读进来的情况，注意有没有需要特殊处理的情况，比如汇点和源点
//        for(int i = 2; i <= edgeorder; i += 2){
//            edge[i].flow += edge[i^1].flow;
//            edge[i^1].flow = 0;
//        }

        deleteServers(toDeletServers);
        addServers(toAddServers);
    }





/*+++++++++++++++是否有一条路径++++++++++++++++++*/
    bool hasPath(){
        memset(dist,63,sizeof(dist));

        dist[vSource] = 0;
        while (!Q.empty()) Q.pop();
        Q.push(vSource);
        inQueue[vSource] = true;

        while(!Q.empty()){
            int from = Q.front();
            Q.pop();
            for(int i = G[from];i; i = edge[i].next){
                if(edge[i].flow > 0 && dist[from] + edge[i].cost < dist[edge[i].to]){
                    dist[edge[i].to] = dist[from] + edge[i].cost;
                    preV[edge[i].to] = from;
                    preE[edge[i].to] = i;
                    if(!inQueue[edge[i].to]) {
                        inQueue[edge[i].to] = true;
                        Q.push(edge[i].to);
                    }
                }
            }
            inQueue[from] = false;
        }
        return dist[vSink] < INF;
    }


/*+++++++++++++++返回当前路径增广的成本++++++++++++++++++*/

    int augment(){
        int from = vSink;
        int delta = INF;
        while(from != vSource){
            if (edge[preE[from]].flow < delta) delta = edge[preE[from]].flow;
            from = preV[from];
        }
        from = vSink;
        while (from != vSource){

            edge[preE[from]].flow -= delta;
            edge[preE[from] ^ 1].flow += delta;
            from = preV[from];
        }
        currentFlows += delta;
        return dist[vSink] * delta;
    }

/*+++++++++++++++判断是否是可行流++++++++++++++++++*/
    bool isFeasibleFlow(){

        return needSum == currentFlows;
    }




/*+++++++++++++++求解可行流++++++++++++++++++*/
    int solve(){
        solveTimes ++;
        int cur = 0,ans = 0;
        while (hasPath()){
            cur += augment();
            if(cur > ans) ans = cur;
        }
        return ans;
    }


    int zkwSolve(){
        solveTimes ++;
        do{
            do{
                memset(vis,0,sizeof(vis));
            }while (zkwAug(vSource,INF));
        }while (zkwModlabel());
        return mincost;
    }
/*+++++++++++++++统计所有信息++++++++++++++++++*/

    void statistic(){
        //总的缺失信息
        total_lost_flow = needSum - currentFlows;
        for(int n = 0; n<nodeNumber; ++n){
            for(int i = G[n]; i ; i = edge[i].next){
                if(i & 1)continue;
                node_actual_flow[n] += edge[i^1].flow;
            }
        }
        for(int i = G[vSink]; i; i = edge[i].next){
            if(i&1){
                consumer_lost_flow[edge[i].to] = edge[i^1].flow;
            }
        }

        for(int i = G[vSource]; i ; i = edge[i].next){
            if (i&1) continue;
            if (edge[i^1].flow  == 0)continue;
            int v = edge[i].to;
            source_server_flow[v] = edge[i^1].flow;
//            cout << "node " << v << "    actual  "<<node_actual_flow[v] << " ability "<< min(out_flow[v],Level[node_level[v]].first) << endl;
            out_percent[v] = float(node_actual_flow[v]) / float(out_flow[v]) * 100;
        }
    }

/*+++++++++++++++打印一条路径++++++++++++++++++*/
    //一般最后一步才会去调用 调用方式是while(printOnePath);
    bool printOnePath(int sink){
        int minimum = INF;
        bool haspath = false;
        vector<int> tmpPre;
        vector<int> tmpPree;
        int from = vSink;
        while(from != vSource) {
            for (int i = G[from]; i; i = edge[i].next) {
                if ((i & 1) == 0) continue;
                if (edge[i].flow > 0) {
                    haspath = true;
                    tmpPre.push_back(edge[i].to);
                    minimum = min(minimum,edge[i].flow);
                    tmpPree.push_back(i);
                    from = edge[i].to;
                    break;
                }
            }
            if (minimum == INF) break;
        }
        if (haspath) {
            for (auto edgen : tmpPree) {
                edge[edgen].flow -= minimum;
            }
            reverse(tmpPre.begin(),tmpPre.end());
            int last;
            int server = tmpPre[1];
            for (auto v:tmpPre) {
                // todo 需要处理边的情况
                if(v == vSource) continue;
                sprintf(temp,"%d ",v);
                strcat(out,temp);
                last = v;
            }
            // todo 需要处理输出
            sprintf(temp,"%d %d %d\n",Net2Consumer[last],minimum,node_level[server]);
            strcat(out,temp);
        }
        return haspath;
    }

/*+++++++++++++++打印所有路径++++++++++++++++++*/
    void printAllPath(){
        while(printOnePath(vSink))
            ++flowCnt;
    }

/*+++++++++++++++打印指定起点的路径++++++++++++++++++*/
//  while(printSpecificPathFromServer(serverNode));
    bool printSpecificPathFromServer(int serverNode){
        int minimum = INF;
        bool haspath = false;
        vector<int> tmpPre;
        vector<int> tmpPree;
        int from = serverNode;
        tmpPre.push_back(from);
        while(from != vSink) {
            for (int i = G[from]; i; i = edge[i].next) {
                if ((i & 1) == 1) continue;
                if (edge[i^1].flow > 0) {
                    haspath = true;
                    tmpPre.push_back(edge[i].to);
                    minimum = min(minimum,edge[i^1].flow);
                    tmpPree.push_back(i^1);
                    from = edge[i].to;
                    break;
                }
            }
            if (minimum == INF) break;
        }
        if (haspath) {
            for (auto edgen : tmpPree) {
                edge[edgen].flow -= minimum;
                edge[edgen ^ 1].flow += minimum;
            }

            int last;
            for (auto v:tmpPre) {
                if(v == vSink) continue;
//                cout << v << " ";
                last = v;
            }
//            cout << Net2Consumer[last];
//            cout << endl;
        }
        return haspath;
    }

    //输入服务器等级，计算总cost
    bool getTotalCost(vector<pair<int,int>> servers){
        mcf_times += 1;
        better = false;
        clear();
        // 默认为用的最高级的case
        buildSource(servers);
        auto flowTotalCost = zkwSolve();
//        cout << flowTotalCost << endl;
//        if(!isFeasibleFlow()){
//            return false;
//        }

        int positionPriceTotal = 0;
        int serverLevelPrice = 0;
        for(auto server: servers){
            positionPriceTotal += positionPrice[server.first];
            serverLevelPrice += Level[server.second].second;
        }
        int totalPrice = positionPriceTotal + serverLevelPrice + flowTotalCost;
        current_cost = totalPrice;
        if(isFeasibleFlow()){
            if(totalPrice < all_cost) {
                best_answer = servers;
                all_cost = totalPrice;
                better = true;
            }
        }
        statistic();
//        cout <<"totalPrice : " <<totalPrice << "   total_needed_lost "<< total_lost_flow << " answer.size  "<< servers.size()<< endl;
    }
};



void deploy_server(char * graph[MAX_EDGE_NUM], int edge_num, char * filename);



#endif
