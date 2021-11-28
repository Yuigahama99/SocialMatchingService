#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <list>
#include <vector>
#include <queue>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <algorithm>

// Constant declearance
#define UDP_PORT_T 21557
#define C_UDP_PORT 24557
#define LOCALHOST "127.0.0.1"
#define FILE_PATH "edgelist.txt"
#define BUFF_SIZE 2000

// Founction declearance
void showBootupMsg();
void split_request(char *name[], char *request);

class Graph
{
public:
    int vertex_num;           // Number of vertices in graph
    std::list<int> *vertices; // Adjacent list for each vertex

    Graph(int);                       // Constructor
    void insertEdge(int, int);        // Insert edge into graph
    std::vector<int> BFS(int, int[]); // BFS algorithem
    int adjacent(int, int);           // Judge whether two nodes are adjacent
};

Graph::Graph(int vertex_num)
{
    this->vertex_num = vertex_num;
    vertices = new std::list<int>[vertex_num + 1];
}

void Graph::insertEdge(int vertex1, int vertex2)
{
    vertices[vertex1].push_back(vertex2);
    vertices[vertex2].push_back(vertex1);
}

std::vector<int> Graph::BFS(int root, int visited[])
{
    std::queue<int> queue;
    queue.push(root);
    visited[root] = true;

    std::vector<int> reachable;

    while (!queue.empty())
    {
        int vertex = queue.front();
        queue.pop();
        reachable.push_back(vertex);

        for (std::list<int>::iterator iter = vertices[vertex].begin(); iter != vertices[vertex].end(); iter++)
        {
            if (!visited[*iter])
            {
                visited[*iter] = true;
                queue.push(*iter);
            }
        }
    }

    return reachable;
}

int Graph::adjacent(int node1, int node2)
{
    for (auto node : vertices[node1])
    {
        if (node == node2)
        {
            return 1;
        }
    }
    return 0;
}

class EdgeList
{
public:
    std::map<std::string, int> map; // Mapping node name and integer descriptor
    Graph *socialNetwork = NULL;    // Graph created by edgelist.txt
    const char *file_path;          // edgelist.txt

    EdgeList(const char *);                                   // Constructor
    std::string findNodeName(int);                            // Find node name by interger descriptor
    void createSocialNetwork();                               // Build social network from edgelist.txt
    std::string findReachableUsers(std::string, std::string); // Find subgraph containing both clients
};

EdgeList::EdgeList(const char *path = FILE_PATH) : file_path(path)
{
    std::ifstream file(file_path);
    std::string name1, name2;
    if (!file)
    {
        perror("serverT: file()");
        exit(1);
    }

    std::set<std::string> name_set;

    if (file.is_open())
    {
        while (file >> name1 >> name2)
        {
            name_set.insert(name1);
            name_set.insert(name2);
        }
    }

    int index = 0;
    for (std::set<std::string>::iterator iter = name_set.begin(); iter != name_set.end(); iter++)
    {
        map[*iter] = index++;
    }

    file.close();
}

std::string EdgeList::findNodeName(int node)
{
    for (auto &item : map)
    {
        if (item.second == node)
        {
            return item.first;
        }
    }
    perror("ServerT: findNodeName()");
    exit(1);
}

void EdgeList::createSocialNetwork()
{
    if (map.size() == 0)
    {
        perror("serverT: createSocialNetwork()");
        exit(1);
    }

    socialNetwork = new Graph(map.size());
    std::ifstream file(file_path);
    std::string name1, name2;

    if (file.is_open())
    {
        while (file >> name1 >> name2)
        {
            socialNetwork->insertEdge(map[name1], map[name2]);
        }
    }

    file.close();
}

std::string EdgeList::findReachableUsers(std::string name1, std::string name2)
{
    int vertex_num = socialNetwork->vertex_num;
    int visited[vertex_num + 1];
    memset(visited, 0, sizeof(visited));
    std::string subGraph = "";

    std::vector<int> reachable;

    if (map.find(name1) == map.end())
    {
        return subGraph;
    }

    int index = map[name1];
    if (!visited[index])
    {
        reachable = socialNetwork->BFS(index, visited);
    }

    int inOneGraph = 1;
    if (!std::count(reachable.begin(), reachable.end(), map[name2]))
    {
        inOneGraph = 0;
    }
    if (!inOneGraph)
    {
        return subGraph;
    }

    std::string node1;
    std::string node2;
    for (int i = 0; i < reachable.size() - 1; i++)
    {
        for (int j = i + 1; j < reachable.size(); j++)
        {
            if (socialNetwork->adjacent(reachable[i], reachable[j]))
            {
                node1 = findNodeName(reachable[i]);
                node2 = findNodeName(reachable[j]);
                subGraph += node1 + ' ' + node2 + '\n';
            }
        }
    }

    return subGraph;
}

int main(void)
{
    showBootupMsg();

    // Build social network from edgelist.txt
    EdgeList topology;
    topology.createSocialNetwork();

    int sockfd;

    // Assign serverT information to local_addr
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    local_addr.sin_port = htons(UDP_PORT_T);

    // Create serverT UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("serverT: socket()");
        return 1;
    }

    // Bind serverT UDP socket with local_addr
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

        // Receive request from Central server
        if ((len = recvfrom(sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *)&remote_addr, &sock_size)) < 0)
        {
            perror("serverT: recvfrom()");
            return 1;
        }

        printf("The ServerT received a request from Central to get the Topology.\n");

        char *name[2];
        split_request(name, buffer);
        std::string subGraph = topology.findReachableUsers(std::string(name[0]), std::string(name[1]));

        if (subGraph == "")
        {
            subGraph = "null";
        }

        if ((len = sendto(sockfd, subGraph.c_str(), BUFF_SIZE, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr))) < 0)
        {
            perror("serverT: sendto()");
            return 1;
        }

        printf("The ServerT finished sending the topology to Central.\n");
    }

    close(sockfd);
    return 0;
}

void showBootupMsg()
{
    printf("The ServerT is up and running using UDP on port %d.\n", UDP_PORT_T);
}

void split_request(char *name[], char *request)
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