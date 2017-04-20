#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "lib_io.h"
#include <string.h>
#include <map>
#include <limits.h>
#include <stdio.h>
#include <queue>
#include <memory.h>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <stdint.h>



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


extern vector <pair<int,int>> best_answer;
extern vector <int> DirectNode;

const int NODEMAX = 10005, EDGEMAX = 50000, INF = 0x3f3f3f3f ;

extern unordered_map<int,pair<int,int>> Level;
extern int positionPrice[NODEMAX + 10];

extern bool push_better;
extern bool pushable;

extern int nodesOutFlow[NODEMAX + 10];
extern vector <int> ReplaceNodes;

extern int out_degree[1500];


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
//  todo 这里分几个等级
    int deployCost;
//  todo 每个节点也需要部署费用
    ;



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
        G[from] = edgeorder;
        edge[++edgeorder] = Edge(from,0,-cost,G[to]);
        G[to] = edgeorder;
        ++out_degree[from];
    }



/*+++++++++++++++建图only once++++++++++++++++++*/

    void buildGraph(char *topo[EDGEMAX]){
        int pos = 0;
        memset(G, 0, sizeof(G));

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


/*+++++++++++++++建超级源点++++++++++++++++++*/
    void buildSource(vector<int> servers){
        for(auto server:servers){
            // todo 这里流量不是无限大注意更改，cost也注意修改
            if(server2edge.count(server)){
                int tmpEdgeNum = server2edge[server];
                //todo 实际中肯定不是无穷大
                edge[tmpEdgeNum].flow = maxBandWidth;
                edge[tmpEdgeNum ^ 1].flow = 0;
            }else{
                //todo 实际中肯定不是无穷大
                addEdge(vSource,server,maxBandWidth,0);
                server2edge[server] = edgeorder - 1;
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
            }else{
                //todo 实际中肯定不是无穷大
                addEdge(vSource,server.first,Level[server.second].first,0);
                server2edge[server.first] = edgeorder - 1;
            }
        }
    }



/*+++++++++++++++clear上次的计算内容，图置为初始情况(源点)++++++++++++++++++*/
    void clear(){

        currentFlows = 0;
        flowCnt = 0;
        memset(dist,63,sizeof(dist));
        memset(inQueue,0,sizeof(inQueue));
        memset(preV,0,sizeof(preV));
        memset(preE,0,sizeof(preE));

        //memset(edge,0,sizeof(edge));
        //todo 所有边置为读进来的情况，注意有没有需要特殊处理的情况，比如汇点和源点
        for(int i = 2; i <= edgeorder; i += 2){
            edge[i].flow += edge[i^1].flow;
            edge[i^1].flow = 0;
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
        for(int i = 2; i <= edgeorder; i += 2){
            edge[i].flow += edge[i^1].flow;
            edge[i^1].flow = 0;
        }

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
            sprintf(temp,"%d %d %d\n",Net2Consumer[last],minimum,server2level[server]);
            strcat(out,temp);
        }
        return haspath;
    }

/*+++++++++++++++打印所有路径++++++++++++++++++*/
    void printAllPath(){
        for(auto server:best_answer){
            server2level[server.first] = server.second;
        }
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
                cout << v << " ";
                last = v;
            }
            cout << Net2Consumer[last];
            cout << endl;
        }
        return haspath;
    }

/*+++++++++++++++获取整个cost信息++++++++++++++++++*/
    //首先默认为最高级，然后根据求得的结果以及百分比，来优化等级信息
    bool getTotalCost(vector<int> servers,double percent){
        bool better = false;
        push_better = false;
        pushable = false;
        clear();
        // 默认为用的最高级的case
        buildSource(servers);
        auto beforeResult = solve();
        if(!isFeasibleFlow()){
            current_cost = -1;
//            cout << "fail" << endl;
            return false;
        }
        int beforeTotalCost = 0;
        int afterTotalCost = 0;

        int server_position_cost = 0;
        int before_server_level_cost = 0;
        int after_server_level_cost = 0;
        // solve之后 每个服务器的出度应该求出来了
        // 再对所有的服务器看看能降级的是否应该降个级
        int beforeDegradationCost = 0;
        // 求出所有节点对应的服务器等级
        // 同时更改服务器等级 得到newServers

        int decreasePostionCost = 0;

        vector<pair<int,int>> origiServers;
        vector<pair<int,int>> newServers;
        for(auto server : servers){
            int edgeNumber = server2edge[server];
            int outflows = edge[edgeNumber ^ 1].flow;

            server_position_cost += positionPrice[server];
            int levelOder = 0;

            for(int i = 0; i < Level.size(); ++i){
                // 找出等级 并适当的降级
                if (outflows <= Level[i].first){
                    origiServers.push_back(make_pair(server,i));
                    before_server_level_cost += Level[i].second;

                    if(i == 0){
                        if(outflows > Level[0].first * percent){
                            newServers.push_back(make_pair(server, i));
                            after_server_level_cost += Level[i].second;
                            break;
                        }
                        // 因为当前点取消，所以当前的位置价格也会取消
                        decreasePostionCost += positionPrice[server];
                        break;
                    }
                    if((outflows - Level[i - 1].first) < (Level[i].first - Level[i-1].first) * percent){
                        newServers.push_back(make_pair(server, i - 1));
                        after_server_level_cost += Level[i-1].second;
                    }else{
                        newServers.push_back(make_pair(server, i));
                        after_server_level_cost += Level[i].second;
                    }
                    break;
                }
            }
        }
        beforeTotalCost = server_position_cost + before_server_level_cost + beforeResult;

        clear();
        buildSource(newServers);
        int afterResult = solve();
        afterTotalCost = server_position_cost - decreasePostionCost + after_server_level_cost + afterResult;

        if (isFeasibleFlow()) {
            pushable = true;
            if (afterTotalCost < beforeTotalCost){
                push_better = true;
                if(afterTotalCost < all_cost){
                    all_cost = afterTotalCost;
                    best_answer = newServers;
                    better = true;
                }else{
                    //cout << "expensive" <<endl;
                }
            }else{
                if(beforeTotalCost < all_cost){
                    all_cost = beforeTotalCost;
                    best_answer = origiServers;
                    better = true;
                }else{
                    //cout << "expensive" <<endl;
                }
            }
        } else{
            current_cost = -1;
            if(beforeTotalCost < all_cost){
                all_cost = beforeTotalCost;
                best_answer = origiServers;
                better = true;
            }else{
                //cout << "expensive" <<endl;
            }
            clear();
            buildSource(best_answer);
            solve();
        }
        return better;
    }

    //输入服务器等级，计算总cost
    bool getTotalCost(vector<pair<int,int>> servers){
        clear();
        // 默认为用的最高级的case
        buildSource(servers);
        auto flowTotalCost = solve();
        if(!isFeasibleFlow()){
            return false;
        }
        int positionPriceTotal = 0;
        int serverLevelPrice = 0;
        for(auto server: servers){
            positionPriceTotal += positionPrice[server.first];
            serverLevelPrice += Level[server.second].second;
        }
        int totalPrice = positionPriceTotal + serverLevelPrice + flowTotalCost;
        if(totalPrice < all_cost) {
            best_answer = servers;
            all_cost = totalPrice;
            return true;
        }else{
            return false;
        }
    }
};



void deploy_server(char * graph[MAX_EDGE_NUM], int edge_num, char * filename);

	

#endif
