#include "deploy.h"


//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    char out[55000] = "";
    char temp[55000] = "";
    int a,b,c,d,i,j,count;
    int network_num, edge_num, customer_num, server_cost, last_flow, min_flow, max_cost, max_price, current;
    sscanf(topo[0],"%d%d%d",&network_num,&edge_num,&customer_num);
    sscanf(topo[2], "%d", &server_cost);
    map<int,edge> edges[network_num];
    map<int,edge>::iterator ite;
    customer_server customers[customer_num];
    map<int,customer_server> servers;
    map<int,customer_server>::iterator its,its2;
    queue<int> extend;
    unordered_set<int> server_del;
    unordered_set<int>::iterator itu;
    cost_flow cf;
    int distance[network_num+1][2];
    for (i = 4; i < edge_num + 4; ++i){
        sscanf(topo[i],"%d%d%d%d",&a,&b,&c,&d);
        edges[a][b] = {c,d,0,0};
        edges[b][a] = {c,d,0,0};
    }

    sprintf(out,"%d\n\n", customer_num);
    for (i = line_num - customer_num; i < line_num; ++i) {
        sscanf(topo[i],"%d%d%d",&a,&b,&c);
        customers[a] = {b,c};
        servers[b] = {c, 0};
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

    //遍历服务器寻找替代者
    server_del.clear();
    for (its = servers.begin(); its != servers.end(); ++its){
        //暂时断开当前节点与超级汇点
        edges[its->first].erase(network_num);
        last_flow = its->second.no_network_usage;
        max_cost = server_cost;
        while (last_flow > 0){
            //每次寻找一条最小费用流代替部分服务器负载
            extend.push(its->first);
            max_price = max_cost / last_flow;
            //cf = mini_cost_flow(servers, edges, extend, b/a, a);
            memset(distance,-1, sizeof(distance));
            distance[its->first][0] = 0;
            while (!extend.empty()){
                current = extend.front();
                extend.pop();
                for (ite = edges[current].begin(); ite != edges[current].end(); ++ite){
                    if (edges[ite->first][current].flow + edges[ite->first][current].t_flow < edges[ite->first][current].cab
                        && edges[ite->first][current].cost + distance[current][0] <= max_price)
                        if (distance[ite->first][0] == -1 || (distance[ite->first][0] > edges[ite->first][current].cost + distance[current][0])){
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
                while (current != its->first) {
                    min_flow = min(min_flow, edges[current][distance[current][1]].cab -
                                             edges[current][distance[current][1]].flow -
                                             edges[current][distance[current][1]].t_flow);
                    current = distance[current][1];
                }
                cf.flow = min_flow;
                cf.cost = min_flow * distance[network_num][0];

                current = distance[network_num][1];
                servers[current].demand_usage_t += min_flow;
                while (current != its->first) {
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
            //服务器替代成功 将临时流量与负载加入正式
            printf("%d\n",its->first);
            for (i = 0; i < network_num; ++i)
                for (ite = edges[i].begin(); ite != edges[i].end(); ++ite) {
                    ite->second.flow += ite->second.t_flow;
                    ite->second.t_flow = 0;
                }
            for (its2 = servers.begin(); its2 != servers.end(); ++its2){
                its2->second.no_network_usage += its2->second.demand_usage_t;
                its2->second.demand_usage_t = 0;
            }
            server_del.insert(its->first);
        }
        else {
            //服务器替代失败 清空临时流量与负载 恢复服务器与超级汇点
            printf("!%d\n",its->first);
            edges[its->first][network_num] = {INT_MAX, 0, 0, 0};
            for (i = 0; i < network_num; ++i)
                for (ite = edges[i].begin(); ite != edges[i].end(); ++ite)
                    ite->second.t_flow = 0;
            for (its2 = servers.begin(); its2 != servers.end(); ++its2)
                its2->second.demand_usage_t = 0;
        }
    }
    for (itu = server_del.begin(); itu != server_del.end(); ++itu)
        servers.erase(*itu);
    memset(distance, -1, sizeof(distance));
    //断开服务器与超级汇点 将服务器节点的距离设为0 将服务器节点推入队列
    for (its = servers.begin(); its != servers.end(); ++its){
        edges[its->first].erase(network_num);
        distance[its->first][0] = 0;
        distance[its->first][1] = -2;
        extend.push(its->first);
    }
    //将与消费节点相连的网络节点连上超级汇点 容量为消费节点的需求
    for (i = 0; i < customer_num; ++i){
        edges[customers[i].no_network_usage][network_num] = {customers[i].demand_usage_t,0,0,0};
    }
    for (its = servers.begin(); its != servers.end(); ++its){
        printf("%d ",its->first);
    }


//    //输出所有边（测试）
//    for (i = 0; i < network_num; ++i) {
//        for (it = edges[i].begin(); it != edges[i].end(); ++it) {
//            printf("%d %d %d %d %d %d\n", i, it->first, it->second.cab, it->second.cost, it->second.flow, it->second.t_flow);
//        }
//    }



    // 需要输出的内容
    char * topo_file = (char *)out;

    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
    write_result(topo_file, filename);

}
