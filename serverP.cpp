#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <limits.h>
#include <set>
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <iostream>

#define UDP_PORT_P 23557
#define C_UDP_PORT 24557
#define LOCALHOST "127.0.0.1"
#define BUFF_SIZE 2000

void showBootupMsg();
void split_name(char *request, char *name[]);
void split_request(char *request, char *subRequest[]);
float calculateWeight(int node1_weight, int node2_weight);
std::map<std::string, int> rebuildScoreMap(char *scores);

/**
 * @brief class Edge and class Graph are reused from https://github.com/ShengHangNB/Graph/blob/main/graph.hpp
 * with modification in dijkstra() to return a std::string with shortest path and score.
 * 
 */
class Edge
{
public:
    std::string vertex;
    float weight;

    Edge(std::string, float);
    bool operator<(const Edge &) const;
    bool operator==(const Edge &) const;
};

Edge::Edge(std::string vertex, float weight)
{
    this->vertex = vertex;
    this->weight = weight;
}

bool Edge::operator<(const Edge &obj) const
{
    return obj.vertex > vertex;
}

bool Edge::operator==(const Edge &obj) const
{
    return obj.vertex == vertex;
}

class Graph
{
public:
    std::map<std::string, std::set<Edge>> adj;
    std::map<std::pair<float, std::string>, std::pair<float, std::string>> pre;

    void show();

    Graph(char *, std::map<std::string, int>);
    bool contain(const std::string &);
    bool adjacent(const std::string &, const std::string &);
    std::vector<std::string> get_vertices();
    void addVertex(const std::string &);
    void addEdge(const std::string &, const std::string &, float weight);
    std::string dijkstra(std::string, std::string);
    std::string shortestPath(std::pair<float, std::string>, std::pair<float, std::string>);
};

Graph::Graph(char *request, std::map<std::string, int> scoreMap)
{
    std::set<std::string> name_set;

    char *temp;
    std::string backup = std::string(request);
    char *buffer = request;
    char *outer_pointer, *inner_pointer;

    while ((temp = strtok_r(buffer, "\n", &outer_pointer)) != NULL)
    {
        buffer = temp;
        while ((temp = strtok_r(buffer, " ", &inner_pointer)) != NULL)
        {
            name_set.insert(temp);
            buffer = NULL;
        }
        buffer = NULL;
    }

    for (std::set<std::string>::iterator iter = name_set.begin(); iter != name_set.end(); iter++)
    {
        this->addVertex(*iter);
    }

    std::string node1;
    std::string node2;
    buffer = (char *)backup.c_str();

    while ((temp = strtok_r(buffer, "\n", &outer_pointer)) != NULL)
    {
        buffer = temp;
        int i = 0;
        while ((temp = strtok_r(buffer, " ", &inner_pointer)) != NULL)
        {
            if (i == 0)
            {
                node1 = temp;
            }
            else
            {
                node2 = temp;
            }
            buffer = NULL;
            i++;
        }
        this->addEdge(node1, node2, calculateWeight(scoreMap[node1], scoreMap[node2]));
        buffer = NULL;
    }
}

void Graph::show()
{
    for (const auto &node1 : adj)
    {
        std::cout << "Vertex " << node1.first << ": ";
        for (const auto &node2 : adj[node1.first])
        {
            std::cout << "(Adjacent: " << node2.vertex << ", Weight: " << node2.weight << ") ";
        }
        std::cout << std::endl;
    }
}

bool Graph::contain(const std::string &node)
{
    return adj.find(node) != adj.end();
}

bool Graph::adjacent(const std::string &node1, const std::string &node2)
{
    for (auto edge : adj[node1])
        return true;

    return false;
}

std::vector<std::string> Graph::get_vertices()
{
    std::vector<std::string> vertices;
    for (auto vertex : adj)
    {
        vertices.push_back(vertex.first);
    }
    return vertices;
}

void Graph::addVertex(const std::string &node)
{
    if (!contain(node))
    {
        std::set<Edge> edge_list;
        adj[node] = edge_list;
    }
}

void Graph::addEdge(const std::string &node1, const std::string &node2, float weight)
{
    adj[node1].insert(Edge(node2, weight));
    adj[node2].insert(Edge(node1, weight));
}

std::string Graph::dijkstra(std::string source, std::string destination)
{
    std::map<std::string, float> dis;

    std::priority_queue<std::pair<float, std::string>, std::vector<std::pair<float, std::string>>, std::greater<std::pair<float, std::string>>> q;

    for (std::string vertex : get_vertices())
    {
        if (vertex == source)
            dis[source] = 0;
        else
            dis[vertex] = INT_MAX;
    }

    std::set<std::string> visited;

    q.push(std::make_pair(0, source));

    while (!q.empty())
    {
        auto front = q.top();
        q.pop();

        std::string u = front.second;

        if (visited.find(u) != visited.end())
            continue;
        else
            visited.insert(u);

        float shortest_distance_to_u = front.first;
        dis[u] = shortest_distance_to_u;

        for (auto v : adj[u])
        {
            if (visited.find(v.vertex) == visited.end())
            {
                float distance_to_v = v.weight;
                std::pair<float, std::string> pair = std::make_pair(shortest_distance_to_u + distance_to_v, v.vertex);
                q.push(pair);
                pre[pair] = front;
            }
        }
    }

    std::string path;
    path = shortestPath(std::make_pair(0, source), std::make_pair(dis[destination], destination));
    path += "|";
    path += std::to_string(dis[destination]);

    return path;
}

std::string Graph::shortestPath(std::pair<float, std::string> s, std::pair<float, std::string> v)
{
    bool flag = true;
    std::string path = "";
    while (flag)
    {
        path += v.second + " ";
        v = pre[v];
        if (v.second == s.second)
        {
            path += v.second + " ";
            flag = false;
        }
    }
    return path;
}

int main(void)
{
    showBootupMsg();

    int sockfd;

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    local_addr.sin_port = htons(UDP_PORT_P);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("serverT: socket()");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("serverT: bind()");
        return 1;
    }

    while (1)
    {
        int len;
        char buffer[BUFF_SIZE];
        struct sockaddr_in remote_addr;
        socklen_t sock_size = sizeof(remote_addr);
        if ((len = recvfrom(sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *)&remote_addr, &sock_size)) < 0)
        {
            perror("serverP: recvfrom()");
            return 1;
        }

        printf("The SerevrP received the tolopogy and score information.\n");

        char *subRequest[3];

        split_request(buffer, subRequest);

        if (std::string(subRequest[1]) == "null")
        {
            std::string fail = "null";
            if ((len = sendto(sockfd, fail.c_str(), BUFF_SIZE, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr))) < 0)
            {
                perror("serverP: sendto()");
                return 1;
            }

            printf("The ServerP finished sending the result to Central.\n");

            close(sockfd);

            return 0;
        }

        std::map<std::string, int> map;
        map = rebuildScoreMap(subRequest[2]);

        Graph topology = Graph(subRequest[1], map);

        char *name[2];

        split_name(subRequest[0], name);

        std::string path = topology.dijkstra(std::string(name[0]), std::string(name[1]));

        if ((len = sendto(sockfd, path.c_str(), BUFF_SIZE, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr))) < 0)
        {
            perror("serverP: sendto()");
            return 1;
        }

        printf("The ServerP finished sending the result to Central.\n");
    }

    close(sockfd);

    return 0;
}

void split_request(char *request, char *subRequest[])
{
    char *temp = strtok(request, "|");
    int i = 0;
    while (temp != NULL)
    {
        subRequest[i] = temp;
        i++;
        temp = strtok(NULL, "|");
    }
}

float calculateWeight(int node1, int node2)
{
    return (float)abs(node1 - node2) / (float)(node1 + node2);
}

std::map<std::string, int> rebuildScoreMap(char *scores)
{
    std::map<std::string, int> map;
    char *node;
    int score;

    char *temp;
    char *buffer = scores;
    char *outer_pointer, *inner_pointer;

    /**
     * @brief this nested strtok_r() is reused from https://stackoverflow.com/questions/4693884/nested-strtok-function-problem-in-c
     * with little modification to fit in my code
     * 
     */
    while ((temp = strtok_r(buffer, "\n", &outer_pointer)) != NULL)
    {
        buffer = temp;
        int i = 0;
        while ((temp = strtok_r(buffer, " ", &inner_pointer)) != NULL)
        {
            if (i == 0)
            {
                node = temp;
            }
            else
            {
                score = atoi(temp);
            }
            buffer = NULL;
            i++;
        }
        map[node] = score;
        buffer = NULL;
    }
    return map;
}

void split_name(char *request, char *name[])
{
    char *temp = strtok(request, " ");
    int i = 0;
    while (temp != NULL)
    {
        name[i] = temp;
        i++;
        temp = strtok(NULL, " ");
    }
}

void showBootupMsg()
{
    printf("The ServerP is up and running using UDP on port %d.\n", UDP_PORT_P);
}