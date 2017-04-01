#include "deploy.h"

char out[550000] = "";
char temp[550000] = "";
int count_flow = 0;


template<class Flow = int, class Cost = int>
struct MinCostFlow {
    int DeployCost;
    int NetNodeNums;
    int EdgeNums;
    int ConsumerNums;
    int vs;
    int vt;
    int NeedSum = 0;
    map<int,int> nc;
    struct Edge {
        int t;
        Flow f;
        Cost c;
        Edge*next, *rev;
        Edge(int _t, Flow _f, Cost _c, Edge*_next) :
                t(_t), f(_f), c(_c), next(_next) {
        }
    };

    vector<Edge*> E;

    void addV() {
        E.push_back((Edge*) 0);
    }

    Edge* makeEdge(int s, int t, Flow f, Cost c) {
        return E[s] = new Edge(t, f, c, E[s]);
    }

    void addEdge(int s, int t, Flow f, Cost c) {
        Edge*e1 = makeEdge(s, t, f, c), *e2 = makeEdge(t, s, 0, -c);
        e1->rev = e2, e2->rev = e1;
    }

    pair<Flow, Cost> minCostFlow(int vs, int vt) { //flow,cost
        auto n = E.size();
        Flow flow = 0;
        Cost cost = 0;
//		const Cost MAX_COST = numeric_limits<Cost>::max();
//		const Flow MAX_FLOW = numeric_limits<Flow>::max();
        const Cost MAX_COST = ~0U >> 1;
        const Flow MAX_FLOW = ~0U >> 1;
        for (;;) {
            vector<Cost> dist(n, MAX_COST);
            vector<Flow> am(n, 0);
            vector<Edge*> prev(n);
            vector<bool> inQ(n, false);
            queue<int> que;

            dist[vs] = 0;
            am[vs] = MAX_FLOW;
            que.push(vs);
            inQ[vs] = true;

            while (!que.empty()) {
                int u = que.front();
                Cost c = dist[u];
                que.pop();
                inQ[u] = false;
                for (Edge*e = E[u]; e; e = e->next)
                    if (e->f > 0) {
                        Cost nc = c + e->c;
                        if (nc < dist[e->t]) {
                            dist[e->t] = nc;
                            prev[e->t] = e;
                            am[e->t] = min(am[u], e->f);
                            if (!inQ[e->t]) {
                                que.push(e->t);
                                inQ[e->t] = true;
                            }
                        }
                    }
            }

            if (dist[vt] == MAX_COST)
                break;
            Flow by = am[vt];
            int u = vt;
            flow += by;
            cost += by * dist[vt];
            vector<int> prt;
//            cout << "flow  "<<by << "   :";
            while (u != vs) {
                Edge*e = prev[u];
                if(u != vs) prt.push_back(u);
                e->f -= by;
                e->rev->f += by;
                u = e->rev->t;
            }
            //cout<< vs - 1;
            int ccc = nc[prt[1]-1];
            while(prt.size()>1){
                sprintf(temp,"%d ",prt.back() - 1);
                strcat(out,temp);
                //cout << " " << prt.back() - 1;
                prt.pop_back();
            }
            prt.pop_back();

            //cout << "  "<<by;
            sprintf(temp,"%d %d\n",ccc,by);
            strcat(out,temp);
            count_flow += 1;
            //cout << endl;
        }
        return make_pair(flow, cost);
    }
    // 选取节点作为DeployNodes
    // vector<int> DeployNodes
    void BuildVS(vector<int> deployNodes){
        for (auto node:deployNodes) {
            addEdge(vs,node+1,INT32_MAX,0);
        }
    }

    //读取文件建NetWork
    void BuildNetWork(char *topo[2000]){
//        freopen(file,"r",stdin);
//        cin>>NetNodeNums;
//        cin>>EdgeNums;
//        cin>>ConsumerNums;
        sscanf(topo[0],"%d %d %d",&NetNodeNums,&EdgeNums,&ConsumerNums);
        //cin >> DeployCost;
        sscanf(topo[2],"%d",&DeployCost);

        for (int i = 0; i <= NetNodeNums+1; ++i) {
            addV();
        }
        vs = 0;
        string line;
        vt = NetNodeNums+1;
        int from,to,capacity,cost;
        for (int i = 0; i < EdgeNums; ++i) {
            sscanf(topo[4+i],"%d %d %d %d",&from,&to,&capacity,&cost);
//            cin >> from >> to >> capacity >> cost;
            addEdge(from+1,to+1,capacity,cost);
            addEdge(to+1,from+1,capacity,cost);
        }
        //getline(cin,line);
        // 添加消费节点到虚拟汇点得边
        int consumer,netnode,need;
        for (int i = 0; i < ConsumerNums; ++i) {
//            cin >> consumer >> netnode >> need;
            sscanf(topo[5 + i + EdgeNums],"%d %d %d",&consumer,&netnode,&need);
            NeedSum += need;
            nc[netnode] = consumer;
//            sscanf(line.c_str(),"%d %d %d",&consumer, &netnode ,&need);
            addEdge(netnode+1,vt,need,0);
        }
    }
};
//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{

    int a,b,c,d,i,j,count;
    int network_num, edge_num, customer_num, server_cost, last_flow, min_flow, max_cost, max_price, current;
    sscanf(topo[0],"%d%d%d",&network_num,&edge_num,&customer_num);
    sscanf(topo[2], "%d", &server_cost);
    map<int,edge> edges[network_num+1];
    map<int,edge>::iterator ite;
    customer_server customers[customer_num];
    map<int,customer_server> servers;
    map<int,customer_server>::iterator its,its2;
    queue<int> extend;
    unordered_set<int> server_del;
    unordered_set<int>::iterator itu;
    cost_flow cf;
    int unordered_servers[customer_num];
    int distance[network_num+1][2];
    vector<int> answer;
    for (i = 4; i < edge_num + 4; ++i){
        sscanf(topo[i],"%d%d%d%d",&a,&b,&c,&d);
        edges[a][b] = {c,d,0,0};
        edges[b][a] = {c,d,0,0};
    }

    //sprintf(out,"%d\n\n", customer_num);
    for (i = line_num - customer_num; i < line_num; ++i) {
        sscanf(topo[i],"%d%d%d",&a,&b,&c);
        customers[a] = {b,c};
        servers[b] = {c, 0};
        edges[network_num][b] = {INT_MAX,0,0,0};
        edges[b][network_num] = {INT_MAX,0,0,0};
    }
    //删除无用边
    do {
        count = 0;
        for (i = 0; i < network_num; ++i) {
            if (edges[i].size() == 1) {
                for (j = 0; j < customer_num; ++j) {
                    if (customers[j].no_network_usage == i)
                        break;
                }
                if (j == customer_num) {
                    edges[edges[i].begin()->first].erase(i);
                    edges[i].erase(edges[i].begin());
                    ++count;
                }
            }
        }
    } while (count > 0);

    i = 0;
    a = servers.size();
    for (its = servers.begin(); its != servers.end(); ++its)
        unordered_servers[i++] = its->first;
    for (i = 0; i < a; ++i)
        for (j = 0; j < a - i - 1; ++j)
            if (servers[unordered_servers[j]].no_network_usage > servers[unordered_servers[j+1]].no_network_usage) {
                b = unordered_servers[j];
                unordered_servers[j] = unordered_servers[j + 1];
                unordered_servers[j + 1] = b;
            }
    //遍历服务器寻找替代者
    server_del.clear();
    //for (its = servers.begin(); its != servers.end(); ++its) {
    for (j = 0; j < a; ++j){
        //printf("now %d\n", unordered_servers[j]);
        //暂时断开当前节点与超级汇点
        edges[network_num].erase(unordered_servers[j]);
        edges[unordered_servers[j]].erase(network_num);
        last_flow = servers[unordered_servers[j]].no_network_usage;
        max_cost = server_cost;
        while (last_flow > 0) {
            //printf("start to find\n");
            //每次寻找一条最小费用流代替部分服务器负载
            extend.push(unordered_servers[j]);
            max_price = max_cost / last_flow;
            //cf = mini_cost_flow(servers, edges, extend, b/a, a);
            memset(distance, -1, sizeof(distance));
            distance[unordered_servers[j]][0] = 0;
            while (!extend.empty()) {

                current = extend.front();
                extend.pop();
                //printf("get a node %d\n", current);
                for (ite = edges[current].begin(); ite != edges[current].end(); ++ite) {
                    if (edges[ite->first][current].flow + edges[ite->first][current].t_flow <
                        edges[ite->first][current].cab
                        && edges[ite->first][current].cost + distance[current][0] <= max_price)
                        if (distance[ite->first][0] == -1 ||
                            (distance[ite->first][0] > edges[ite->first][current].cost + distance[current][0])) {
                            distance[ite->first][0] = edges[ite->first][current].cost + distance[current][0];
                            distance[ite->first][1] = current;
                            if (ite->first != network_num)
                                extend.push(ite->first);
                        }
                }
            }
            if (distance[network_num][0] == -1)
                cf.flow = -1;
            else {
                min_flow = last_flow;
                current = distance[network_num][1];
                while (current != unordered_servers[j]) {
                    min_flow = min(min_flow, edges[current][distance[current][1]].cab -
                                             edges[current][distance[current][1]].flow -
                                             edges[current][distance[current][1]].t_flow);
                    current = distance[current][1];
                }
                cf.flow = min_flow;
                cf.cost = min_flow * distance[network_num][0];

                current = distance[network_num][1];
                servers[current].demand_usage_t += min_flow;
                while (current != unordered_servers[j]) {
                    edges[current][distance[current][1]].t_flow += min_flow;
                    current = distance[current][1];
                }
            }
            if (cf.flow == -1)
                break;
            last_flow -= cf.flow;
            max_cost -= cf.cost;
        }
        if (cf.flow != -1) {
            count += 1;
            //服务器替代成功 将临时流量与负载加入正式
            //printf("%d\n", unordered_servers[j]);
            for (i = 0; i < network_num; ++i)
                for (ite = edges[i].begin(); ite != edges[i].end(); ++ite) {
                    ite->second.flow += ite->second.t_flow;
                    ite->second.t_flow = 0;
                }
            for (its2 = servers.begin(); its2 != servers.end(); ++its2) {
                its2->second.no_network_usage += its2->second.demand_usage_t;
                its2->second.demand_usage_t = 0;
            }
            server_del.insert(unordered_servers[j]);
        } else {
            //服务器替代失败 清空临时流量与负载 恢复服务器与超级汇点
            //printf("!%d\n", unordered_servers[j]);
            edges[network_num][unordered_servers[j]] = {INT_MAX, 0, 0, 0};
            edges[unordered_servers[j]][network_num] = {INT_MAX, 0, 0, 0};
            for (i = 0; i < network_num; ++i)
                for (ite = edges[i].begin(); ite != edges[i].end(); ++ite)
                    ite->second.t_flow = 0;
            for (its2 = servers.begin(); its2 != servers.end(); ++its2)
                its2->second.demand_usage_t = 0;
        }
    }
    for (itu = server_del.begin(); itu != server_del.end(); ++itu)
        servers.erase(*itu);

    for (its = servers.begin(); its != servers.end(); ++its){
        //printf("%d,",its->first);
        answer.push_back(its->first);
    }

    memset(distance, -1, sizeof(distance));
    //断开服务器与超级汇点 将服务器节点的距离设为0 将服务器节点推入队列
    for (its = servers.begin(); its != servers.end(); ++its){
        edges[network_num].erase(its->first);
        edges[its->first].erase(network_num);
        distance[its->first][0] = 0;
        distance[its->first][1] = -2;
        extend.push(its->first);
    }
    //将与消费节点相连的网络节点连上超级汇点 容量为消费节点的需求
    for (i = 0; i < customer_num; ++i){
        edges[customers[i].no_network_usage][network_num] = {customers[i].demand_usage_t,0,0,0};
    }

    for (i = 0; i < network_num; ++i)
        for (ite = edges[i].begin(); ite != edges[i].end(); ++ite) {
            ite->second.flow = 0;
            ite->second.t_flow = 0;
        }


//    //输出所有边（测试）
//    for (i = 0; i < network_num; ++i) {
//        for (it = edges[i].begin(); it != edges[i].end(); ++it) {
//            printf("%d %d %d %d %d %d\n", i, it->first, it->second.cab, it->second.cost, it->second.flow, it->second.t_flow);
//        }
//    }

    MinCostFlow<int,int> MCF;
    MCF.BuildNetWork(topo);
    //vector<int> deployNodes{7,14,16,17,19,25,29,32,35,43,44,59,70,73,74,82,84,88,89,93,96,102,103,111,112,120,124,126,129,133,137,138,147,161,164,166,167,170,175,178,184,186,187,188,194,195,198,200,203,205,218,223,227,234,238,242,243,252,254,259,263,267,268,270,271,275,278,281,286,288,297,301,308,320,321,326,328,331,333,334,335,336,338,346,349,363,370,375,381,383,385,387,396,397,400,402,409,413,415,416,423,424,426,438,459,463,464,466,474,475,481,482,488,491,496,497,499,500,503,505,506,507,520,525,529,537,538,541,542,548,552,554,556,557,570,575,576,584,592,594,599,625,632,634,638,640,641,643,646,651,653,660,663,668,670,671,675,677,679,685,687,711,718,719,724,731,739,741,742,745,751,753,755,756,757,760,765,767,770,774,780,784,791,795,797};
    MCF.BuildVS(answer);
    auto result = MCF.minCostFlow(MCF.vs,MCF.vt);
    cout << "Flow   " << result.first <<"   Cost  " <<result.second  + answer.size() * MCF.DeployCost<< endl;
    cout << MCF.NeedSum<< endl;
    sprintf(temp,"%d\n\n",count_flow);
    strcat(temp,out);
    // 需要输出的内容
    char * topo_file = (char *)temp;

    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
    write_result(topo_file, filename);

}
