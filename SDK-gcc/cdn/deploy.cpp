#include "deploy.h"


char out[550000] = "";
char temp[550000] = "";
int count_flow = 0;
int all_cost = 0;
int all_cost_min;
char last_out[550000] = "";

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
    //printf("%s date/time is: %s \tused time is %lu s %d ms.\n", head, asctime(timeinfo1), out_s1, out_ms1);
    return out_s1;
}

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    print_time1("start");
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
    unordered_set<int> server_del2;
    unordered_set<int>::iterator itu,itu2;
    cost_flow cf;
    int unordered_servers[customer_num];
    int distance[network_num+1][2];
//    vector<int> answer;
    map<int,int> network_customer;
    stack<int> stk;
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
        network_customer[b] = a;
    }
    //printf("server = %d\n",servers.size());
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
    //printf("delete");
    for (itu = server_del.begin(); itu != server_del.end(); ++itu) {
        servers.erase(*itu);
        //printf("%d ", *itu);
    }
    //printf("\nserver = %d\n",servers.size());

//    for (its = servers.begin(); its != servers.end(); ++its){
//        printf("%d,",its->first);
//        answer.push_back(its->first);
//    }
    //return;
    //断开服务器与超级汇点
    for (its = servers.begin(); its != servers.end(); ++its){
        edges[network_num].erase(its->first);
        edges[its->first].erase(network_num);
//        distance[its->first][0] = 0;
//        distance[its->first][1] = -2;
//        extend.push(its->first);
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
    for (its = servers.begin(); its != servers.end(); ++its){
        if (network_customer.find(its->first) != network_customer.end()){
            sprintf(temp,"%d %d %d\n",its->first,network_customer[its->first],customers[network_customer[its->first]].demand_usage_t);
            strcat(out,temp);
            edges[its->first].erase(network_num);
            count_flow += 1;
        }
    }
    while (true) {
        //每次寻找一条最小费用流代替部分服务器负载
        memset(distance, -1, sizeof(distance));
        for (its = servers.begin(); its != servers.end(); ++its) {
            extend.push(its->first);
            distance[its->first][0] = 0;
            distance[its->first][1] = -2;
        }
        max_price = server_cost;
        //cf = mini_cost_flow(servers, edges, extend, b/a, a);
        while (!extend.empty()) {

            current = extend.front();
            extend.pop();

            //printf("get a node %d\n", current);
            for (ite = edges[current].begin(); ite != edges[current].end(); ++ite) {
                if (edges[current][ite->first].flow < edges[current][ite->first].cab
                    && edges[current][ite->first].cost + distance[current][0] <= max_price)
                    if (distance[ite->first][0] == -1 ||
                        (distance[ite->first][0] > edges[current][ite->first].cost + distance[current][0])) {
                        distance[ite->first][0] = edges[current][ite->first].cost + distance[current][0];
                        distance[ite->first][1] = current;
                        if (ite->first != network_num)
                            extend.push(ite->first);
                    }
            }
        }
        //printf("%d\n",distance[network_num][0]);
        if (distance[network_num][0] == -1)
            break;
        else {
            current = distance[network_num][1];
            min_flow = edges[current][network_num].cab - edges[current][network_num].flow;

            while (distance[current][1] != -2) {
                min_flow = min(min_flow, edges[distance[current][1]][current].cab -
                                         edges[distance[current][1]][current].flow);
                current = distance[current][1];
            }
//            cf.flow = min_flow;
//            cf.cost = min_flow * distance[network_num][0];
            all_cost += min_flow * distance[network_num][0];

            current = distance[network_num][1];
            stk.push(network_customer[current]);
            edges[current][network_num].flow += min_flow;
            while (distance[current][1] != -2) {
                stk.push(current);
                edges[distance[current][1]][current].flow += min_flow;
                current = distance[current][1];
            }
            stk.push(current);
            while (!stk.empty()){
                sprintf(temp,"%d ", stk.top());
                stk.pop();
                strcat(out,temp);
            }
            sprintf(temp,"%d\n", min_flow);
            strcat(out,temp);
            count_flow += 1;
        }

    }
//    //检查每个消费节点的需求与实际流量
//    for (i = 0; i < customer_num; ++i){
//        if (edges[customers[i].no_network_usage].find(network_num) != edges[customers[i].no_network_usage].end())
//            printf("%d %d\n",edges[customers[i].no_network_usage][network_num].cab,edges[customers[i].no_network_usage][network_num].flow);
//    }
    sprintf(temp,"%d\n\n",count_flow);
    strcat(temp,out);
    all_cost += server_cost * servers.size();
    all_cost_min = all_cost;
    //printf("cost = %d\n", all_cost);
    strcpy(last_out,temp);
    all_cost = 0;
    count_flow = 0;
    //printf("del %d\n",server_del2.size());
    //printf("time %d\n",print_time1("stop"));
    for (itu2 = server_del2.begin(); itu2 != server_del2.end(); ++itu2){
        if (print_time1("1") > 85)
            break;
        sprintf(out,"");
        sprintf(temp,"");
        //printf("qindian %d\n",*itu2);
        for (i = 0; i < customer_num; ++i){
            edges[customers[i].no_network_usage].erase(network_num);
            servers[customers[i].no_network_usage] = {customers[i].demand_usage_t,0};
            edges[customers[i].no_network_usage][network_num] = {INT_MAX,0,0,0};
            edges[network_num][customers[i].no_network_usage] = {INT_MAX,0,0,0};
        }
        //printf("server = %d %d\n",servers.size(),a);
        for (i = 0; i < network_num; ++i)
            for (ite = edges[i].begin(); ite != edges[i].end(); ++ite) {
                ite->second.flow = 0;
                ite->second.t_flow = 0;
            }
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
        //printf("delete %d\n",server_del.size());
        //断开服务器与超级汇点
        for (its = servers.begin(); its != servers.end(); ++its){
            edges[network_num].erase(its->first);
            edges[its->first].erase(network_num);
//        distance[its->first][0] = 0;
//        distance[its->first][1] = -2;
//        extend.push(its->first);
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
        for (its = servers.begin(); its != servers.end(); ++its){
            if (network_customer.find(its->first) != network_customer.end()){
                sprintf(temp,"%d %d %d\n",its->first,network_customer[its->first],customers[network_customer[its->first]].demand_usage_t);
                strcat(out,temp);
                edges[its->first].erase(network_num);
                count_flow += 1;
            }
        }
        while (true) {
            //每次寻找一条最小费用流代替部分服务器负载
            memset(distance, -1, sizeof(distance));
            for (its = servers.begin(); its != servers.end(); ++its) {
                extend.push(its->first);
                distance[its->first][0] = 0;
                distance[its->first][1] = -2;
            }
            max_price = server_cost;
            //cf = mini_cost_flow(servers, edges, extend, b/a, a);
            while (!extend.empty()) {

                current = extend.front();
                extend.pop();

                //printf("get a node %d\n", current);
                for (ite = edges[current].begin(); ite != edges[current].end(); ++ite) {
                    if (edges[current][ite->first].flow < edges[current][ite->first].cab
                        && edges[current][ite->first].cost + distance[current][0] <= max_price)
                        if (distance[ite->first][0] == -1 ||
                            (distance[ite->first][0] > edges[current][ite->first].cost + distance[current][0])) {
                            distance[ite->first][0] = edges[current][ite->first].cost + distance[current][0];
                            distance[ite->first][1] = current;
                            if (ite->first != network_num)
                                extend.push(ite->first);
                        }
                }
            }
            //printf("%d\n",distance[network_num][0]);
            if (distance[network_num][0] == -1)
                break;
            else {
                current = distance[network_num][1];
                min_flow = edges[current][network_num].cab - edges[current][network_num].flow;

                while (distance[current][1] != -2) {
                    min_flow = min(min_flow, edges[distance[current][1]][current].cab -
                                             edges[distance[current][1]][current].flow);
                    current = distance[current][1];
                }
//            cf.flow = min_flow;
//            cf.cost = min_flow * distance[network_num][0];
                all_cost += min_flow * distance[network_num][0];

                current = distance[network_num][1];
                stk.push(network_customer[current]);
                edges[current][network_num].flow += min_flow;
                while (distance[current][1] != -2) {
                    stk.push(current);
                    edges[distance[current][1]][current].flow += min_flow;
                    current = distance[current][1];
                }
                stk.push(current);
                while (!stk.empty()){
                    sprintf(temp,"%d ", stk.top());
                    stk.pop();
                    strcat(out,temp);
                }
                sprintf(temp,"%d\n", min_flow);
                strcat(out,temp);
                count_flow += 1;
            }

        }
//    //检查每个消费节点的需求与实际流量
//    for (i = 0; i < customer_num; ++i){
//        if (edges[customers[i].no_network_usage].find(network_num) != edges[customers[i].no_network_usage].end())
//            printf("%d %d\n",edges[customers[i].no_network_usage][network_num].cab,edges[customers[i].no_network_usage][network_num].flow);
//    }
        sprintf(temp,"%d\n\n",count_flow);
        strcat(temp,out);
        all_cost += server_cost * servers.size();
        //printf("cost = %d\n", all_cost);
        if (all_cost < all_cost_min){
            all_cost_min = all_cost;
            strcpy(last_out,temp);
        }
        all_cost = 0;
        count_flow = 0;
    }
//    //输出所有边（测试）
//    for (i = 0; i < network_num; ++i) {
//        for (it = edges[i].begin(); it != edges[i].end(); ++it) {
//            printf("%d %d %d %d %d %d\n", i, it->first, it->second.cab, it->second.cost, it->second.flow, it->second.t_flow);
//        }
//    }

    //printf("mini cost = %d\n",all_cost_min);
    // 需要输出的内容
    char * topo_file = (char *)last_out;
    //print_time1("end");


    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
    write_result(topo_file, filename);
    //print_time1("write");
}
