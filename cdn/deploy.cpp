#include "deploy.h"
// #define PRINTTIME
char out[550000] = "";
char temp[550000] = "";
int count_flow = 0;
int flows = 0;
int server_cost;
int all_cost = INT_MAX;
int current_cost;
int flowtime = 0;
vector <int> DirectNode;

vector <int> best_answer;


const int maxn = 20000,maxm = 50000, inf = 100000000;
int src,sink;
map <int,int> net2cons;
struct MinCostFlow {
    struct Edge {
        Edge() {};
        int v, f, w, nxt;  // v表示  这个边得入点，即u---->v  f为容量 w为费用
        Edge(int a, int b, int c, int d) {
            v = a;
            f = b;
            w = c;
            nxt = d;
        };
    };
    int g[maxn + 10];
    Edge e[maxn + 10];
    int needsum;
    int nume;
    int deploycost;
    int nnum,cnum,edgenum;
// u--->v   容量 c 费用 为w
    void addedge(int u,int v,int c,int w){
//        cout << "nume number::" << nume << endl;
        e[++nume] = Edge(v,c,w,g[u]);
        g[u] = nume;
        e[++nume] = Edge(u,0,-w,g[v]);
        g[v] = nume;
    }


    void buildflow(char *topo[MAX_EDGE_NUM]){
        needsum = 0;
        nume = 1;// critical
//        cin >> nnum >> edgenum >> cnum;
        sscanf(topo[0],"%d %d %d",&nnum,&edgenum,&cnum);
//        cout << "here" << endl;
        sink = nnum+1;
        src = nnum;
//        cin >> deploycost;
        sscanf(topo[2],"%d",&deploycost);
        for (int i = 0; i < edgenum; ++i) {
            int _u,_v,_flow,_cost;
            sscanf(topo[4+i],"%d %d %d %d",&_u,&_v,&_flow,&_cost);

//            cin >> _u >> _v >> _flow >> _cost;
            addedge(_u,_v,_flow,_cost);
            addedge(_v,_u,_flow,_cost);
        }
//        cout << "here";
        //0 8 40 // 注：消费节点0，相连网络节点ID为8，视频带宽消耗需求为40
        bool d = DirectNode.empty();
        for (int i = 0; i < cnum; ++i) {
            int _c,_n,_need;
            sscanf(topo[5 + i + edgenum],"%d %d %d",&_c,&_n,&_need);
            if(d){
                DirectNode.push_back(_n);
            }
            addedge(_n,sink,_need,0);
            needsum += _need;
            net2cons[_n] = _c;
        }
    }


    void buildSource(vector<int>server){

        for(auto s:server){
            addedge(src,s,INT32_MAX,0);
        }
    }
    queue<int> que;
    int dist[maxn + 10];
    bool inQue[maxn +10];
    int preV[maxn + 10];
    int preE[maxn + 10];



    bool findpath(){

        memset(dist,63,sizeof(dist));
        dist[src] = 0;
        while (!que.empty()) que.pop();
        que.push(src);
        inQue[src] = true;

        while(!que.empty()){
            int u = que.front();
            que.pop();
            for(int i = g[u];i; i = e[i].nxt){
                if(e[i].f > 0 && dist[u] + e[i].w < dist[e[i].v]){
                    dist[e[i].v] = dist[u] + e[i].w;
                    preV[e[i].v] = u;
                    preE[e[i].v] = i;
                    if(!inQue[e[i].v]) {
                        inQue[e[i].v] = true;
                        que.push(e[i].v);
                    }
                }
            }
            inQue[u] = false;
        }
        if(dist[sink] < inf)
            return true;
        else
            return false;
    }

    int augment(){
        int u = sink;
        int delta = inf;
        while(u != src){
            if (e[preE[u]].f < delta) delta = e[preE[u]].f;
            u = preV[u];
        }
        u = sink;
        while (u != src){
            e[preE[u]].f -= delta;
            e[preE[u] ^ 1].f += delta;
            u = preV[u];
        }
        flows += delta;
        return dist[sink] * delta;

    }

    bool can(){
        return needsum == flows;
    }

    int minconstflow(){
        int cur = 0,ans = 0;
        while (findpath()){
            cur += augment();
            if(cur > ans) ans = cur;
        }
        return ans;

    }


    bool printPath(int sink){
        int minimum = 100000000;
        bool haspath = false;
        vector<int> tmpPre;
        vector<int> tmpPree;
        int u = sink;
        while(u != src) {
            for (int i = g[u]; i; i = e[i].nxt) {
                if ((i & 1) == 0) continue;
                if (e[i].f > 0) {
                    haspath = true;
                    tmpPre.push_back(e[i].v);
                    minimum = min(minimum,e[i].f);
                    tmpPree.push_back(i);
                    u = e[i].v;
                    break;
                }
            }
            if (minimum == 100000000) break;
        }
        if (haspath) {
            count_flow += 1;
            for (auto edgen : tmpPree) {
                e[edgen].f -= minimum;
            }
//            cout << "flow "<< minimum<< "   :";
            reverse(tmpPre.begin(),tmpPre.end());
            int last;
            for (auto v:tmpPre) {
                if(v == src) continue;
                sprintf(temp,"%d ",v);
                strcat(out,temp);
//                cout << v << " ";
                last = v;
            }
            sprintf(temp,"%d %d\n",net2cons[last],minimum);
            strcat(out,temp);
//            cout << net2cons[last];
//            cout << endl;
        }
        return haspath;
    }
};

MinCostFlow MCF;

void getanswer(char * topo[MAX_EDGE_NUM], vector<int> answer){
    flowtime ++;
    flows = 0;

    memset(MCF.e,0,sizeof(MCF.e));
    memset(MCF.g,0,sizeof(MCF.g));
    memset(MCF.dist,63,sizeof(MCF.dist));
    memset(MCF.inQue,0,sizeof(MCF.inQue));
    memset(MCF.preV,0,sizeof(MCF.preV));
    memset(MCF.preE,0, sizeof(MCF.preE));
    MCF.buildflow(topo);

    MCF.buildSource(answer);

    auto result = MCF.minconstflow();
    if (MCF.can()) {
        current_cost = result + server_cost * answer.size();

        if (current_cost < all_cost) {
            all_cost = current_cost;
            //strcpy(last, temp);
            best_answer = answer;
        } else;
            //printf("expensive\n");
    } else {
        current_cost = -1;
        //printf("cannot\n");
        return;
    }
}

int print_time1(const char *head)
{
    struct timeb rawtime1;
    struct tm * timeinfo1;
    ftime(&rawtime1);
    timeinfo1 = localtime(&rawtime1.time);
    static bool first = true;
    static int ms1;
    static unsigned long s1;
    if (first) {
        ms1 = rawtime1.millitm;
        s1 = rawtime1.time;
        first = false;
    }
    int out_ms1 = rawtime1.millitm - ms1;
    unsigned long out_s1 = rawtime1.time - s1;


    if (out_ms1 < 0)
    {
        out_ms1 += 1000;
        out_s1 -= 1;
    }
//    printf("%s date/time is: %s \tused time is %lu s %d ms.\n", head, asctime(timeinfo1), out_s1, out_ms1);
    return out_s1;
}


//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    print_time1("begin");
    int a,b,c,d,i,j,count;
    int network_num, edge_num, customer_num, last_flow, min_flow, max_cost, max_price, current;
    sscanf(topo[0],"%d%d%d",&network_num,&edge_num,&customer_num);
    sscanf(topo[2], "%d", &server_cost);
    map<int,edge> edges[network_num+1];
    map<int,edge>::iterator ite;
    customer_server customers[customer_num];
    map<int,customer_server> servers;
    map<int,customer_server>::iterator its,its2;
    queue<int> extend;
    unordered_set<int> server_del,server_del2;
    unordered_set<int>::iterator itu,itu2;
    cost_flow cf;
    int unordered_servers[customer_num];
    int distance[network_num+1][2];
    vector<int> answer,answer1;
    vector<int>::iterator ita;
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
    server_del2.clear();
    //for (its = servers.begin(); its != servers.end(); ++its) {
//    for (j = a-1; j > -1; --j){
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
            server_del2.insert(unordered_servers[j]);
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

    }


    for (i = 0; i < network_num; ++i)
        for (ite = edges[i].begin(); ite != edges[i].end(); ++ite) {
            ite->second.flow = 0;
            ite->second.t_flow = 0;
        }



    //printf("answer = %d\n",answer.size());
    getanswer(topo,answer);
#ifdef PRINTTIME
    printf("original consumer node cost %d\n",all_cost);
#endif

//    printf("cost_ori = %d, time = %d\n",all_cost,print_time1("ori"));


    for (itu2 = server_del2.begin(); itu2 != server_del2.end(); ++itu2){
        if (print_time1("time") > 9)
            break;
        //sprintf(out,"");
        //sprintf(temp,"");
        count_flow = 0;
        answer.clear();
        for (i = 0; i < customer_num; ++i){
            edges[customers[i].no_network_usage][network_num] = {INT_MAX,0,0,0};
            edges[network_num][customers[i].no_network_usage] = {INT_MAX,0,0,0};
            servers[customers[i].no_network_usage] = {customers[i].demand_usage_t,0};
        }
        //遍历服务器寻找替代者
        server_del.clear();
        //for (its = servers.begin(); its != servers.end(); ++its) {
        for (j = 0; j < a; ++j){
            if (unordered_servers[j] == *itu2)
                continue;
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

        }


        for (i = 0; i < network_num; ++i)
            for (ite = edges[i].begin(); ite != edges[i].end(); ++ite) {
                ite->second.flow = 0;
                ite->second.t_flow = 0;
            }

        getanswer(topo,answer);

    }
#ifdef PRINTTIME
    printf("delete add node opt cost_ori %d %d\n", all_cost, print_time1("mid"));
#endif
    while (print_time1("1") < 89) {

        if (best_answer == answer)
            break;
        answer = best_answer;
        for (ita = answer.begin(); ita != answer.end(); ++ita) {
            if (print_time1("1") < 89) {
                //printf("123\n");
                answer1 = answer;
                answer1.erase(answer1.begin() + (ita - answer.begin()));
                //printf("%d\n", answer1.size());
                getanswer(topo, answer1);
            } else
                break;
        }
    }
#ifdef PRINTTIME
    printf("odered delete node cost = %d, time = %d\n",all_cost,print_time1("end"));
#endif


    // 把直连的点不在best_answer中的加上看看能不能有优化：
    vector<int> tmpbest,tmpbest2;

    for (auto Dite = DirectNode.begin(); Dite != DirectNode.end(); ++Dite) {
        if (print_time1("time") > 88)
            break;
//        cout <<*Dite<<endl;
        tmpbest = best_answer;
        auto _b = tmpbest.begin();
        auto _e = tmpbest.end();
        auto it = find(_b,_e,*Dite);
        if(it == _e){
            for(int _i = 0; _i < tmpbest.size();++_i){
            	if (print_time1("time") > 88)
            		break;
//                cout << "here" << endl;
                tmpbest2 = tmpbest;
                tmpbest2.erase(tmpbest2.begin() + _i);
//                cout << "here" << endl;
                tmpbest2.push_back(*Dite);
//                cout << "here" << endl;
                getanswer(topo,tmpbest2);
//                cout << "here" << endl;
            }
        }
    }

#ifdef PRINTTIME
    printf("add directnode opt = %d, time = %d\n",all_cost,print_time1("end"));
#endif
    // 需要输出的内容


    flows = 0;
//    MinCostFlow MCF;
    memset(MCF.e,0,sizeof(MCF.e));
    memset(MCF.g,0,sizeof(MCF.g));
    memset(MCF.dist,63,sizeof(MCF.dist));
    memset(MCF.inQue,0,sizeof(MCF.inQue));
    memset(MCF.preV,0,sizeof(MCF.preV));
    memset(MCF.preE,0, sizeof(MCF.preE));
    MCF.buildflow(topo);

    MCF.buildSource(best_answer);

    auto result = MCF.minconstflow();

    current_cost = result + server_cost * answer.size();
        //printf("cost = %d\n",result+server_cost*answer.size());
//    cout << "here" ;
        //cout << "Flow   " << result.first <<"   Cost  " <<result.second  + answer.size() * MCF.DeployCost<< endl;
        //cout << MCF.NeedSum<< endl;
        while (MCF.printPath(sink));
        sprintf(temp, "%d\n\n", count_flow);
        strcat(temp, out);

    //printf("cost = %d\n",all_cost);
    char * topo_file = (char *)temp;

    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
    write_result(topo_file, filename);
#ifdef PRINTTIME
    printf("time = %d\n",print_time1("end"));
    cout << flowtime;
#endif
}
