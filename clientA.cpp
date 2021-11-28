#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <stack>

// Constant declearance
#define SRVC_PORT 25557
#define LOCALHOST "127.0.0.1"
#define BUFF_SIZE 2000

// Function declearance
void showBootUpMsg();
void showResult(char *result, char *name);

int main(int argc, char *argv[])
{
    showBootUpMsg();

    int sockfd;

    // Assign Central server information to remote_addr
    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    remote_addr.sin_port = htons(SRVC_PORT); // Remote TCP Port Number of Central Server

    // Create client TCP socket, dynamically assign port number by OS
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("socket create: ");
        return 1;
    }

    // Connect to remote Central server
    if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
    {
        perror("connect to remote server: ");
        return 1;
    }

    // Send user name to Central server
    if (send(sockfd, argv[1], strlen(argv[1]), 0) < 0)
    {
        perror("send message: ");
        return 1;
    }

    printf("The client sent %s to the Central server.\n", argv[1]);

    int byteCount;
    char buffer[BUFF_SIZE]; // Receiver buffer

    // Receive result from Central server
    if ((byteCount = recv(sockfd, buffer, BUFF_SIZE, 0)) < 0)
    {
        perror("receive result: ");
        return 1;
    }
    buffer[byteCount] = '\0';

    // Display result on screen
    showResult(buffer, argv[1]);

    // Close all sockets
    close(sockfd);

    return 0;
}

/**
 * @brief how the bootup message when clientA bootup
 * 
 */
void showBootUpMsg()
{
    printf("The Client is up and running.\n");
}

/**
 * @brief Display the result received from Central server
 * 
 * @param result Result received from Central server
 * @param name User name. For no compatibility situation.
 */
void showResult(char *result, char *name)
{
    /**
     * @brief Result pattern: xxx xxx xxx|score
     *                            path    weight
     *                    or      xxx    |null    for no compatibility situation
     *        First split path and weight into subResult[2]
     */
    char *temp = strtok(result, "|");
    char *subResult[2];
    int i = 0;
    while (temp != NULL)
    {
        subResult[i] = temp;
        i++;
        temp = strtok(NULL, "|");
    }

    // If no compatibility found
    if (std::string(subResult[1]) == "null")
    {
        printf("Found no compatibility for %s and %s.\n", name, subResult[0]);
    }

    // If compatibility found
    else
    {
        std::stack<std::string> s;
        std::string bottom;
        bool btmFlag = true;
        temp = strtok(subResult[0], " ");
        while (temp != NULL)
        {
            s.push(std::string(temp));
            if (btmFlag)
            {
                bottom = std::string(temp);
                btmFlag = false;
            }
            temp = strtok(NULL, " ");
        }

        printf("Found compatibility for %s and %s: \n", (s.top()).c_str(), bottom.c_str());
        while (s.size() != 1)
        {
            printf("%s", (s.top()).c_str());
            printf(" --- ");
            s.pop();
        }
        printf("%s \nMatching Gap: %s\n", (s.top()).c_str(), subResult[1]);
    }
}