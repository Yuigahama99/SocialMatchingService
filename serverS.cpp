#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <set>
#include <string>
#include <iostream>
#include <map>
#include <fstream>

#define UDP_PORT_S 22557
#define C_UDP_PORT 24557
#define LOCALHOST "127.0.0.1"
#define BUFF_SIZE 2000
#define FILE_PATH "scores.txt"

void showBootupMsg();
std::set<std::string> split_request(char *request);

class Score
{
public:
    std::map<std::string, int> map;
    const char *file_path;

    Score(const char *);
};

Score::Score(const char *path = FILE_PATH) : file_path(path)
{
    std::ifstream file(file_path);
    std::string name;
    int score;
    if (!file)
    {
        perror("serverS: file()");
        exit(1);
    }

    if (file.is_open())
    {
        while (file >> name >> score)
        {
            map[name] = score;
        }
    }

    file.close();
}

int main(void)
{
    int flag = 1;
    while (flag)
    {
        showBootupMsg();
        flag = 0;
    }

    Score scores;

    int sockfd;

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    local_addr.sin_port = htons(UDP_PORT_S);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("serverS: socket()");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("serverS: bind()");
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
            perror("serverS: recvfrom()");
            return 1;
        }

        printf("The ServerS received a request from Central to get the scores.\n");

        std::set<std::string> name_set;
        name_set = split_request(buffer);
        std::string nodeScore = "";
        std::string space = " ";
        std::string lineFeed = "\n";
        for (std::set<std::string>::iterator iter = name_set.begin(); iter != name_set.end(); iter++)
        {
            nodeScore += (*iter).c_str() + space + std::to_string(scores.map[*iter]) + lineFeed;
        }

        if ((len = sendto(sockfd, nodeScore.c_str(), BUFF_SIZE, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr))) < 0)
        {
            perror("serverS: sendto()");
            return 1;
        }

        printf("The ServerS finished sending the scores to Central.\n");
    }

    close(sockfd);

    return 0;
}

std::set<std::string> split_request(char *request)
{
    std::set<std::string> name_set;

    char *temp;
    char *buffer = request;
    char *outer_pointer, *inner_pointer;

    /**
     * @brief this nested strtok_r() is reused from https://stackoverflow.com/questions/4693884/nested-strtok-function-problem-in-c
     * with little modification to fit in my code
     * 
     */
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

    return name_set;
}

void showBootupMsg()
{
    printf("The ServerS is up and running using UDP on port %d.\n", UDP_PORT_S);
}