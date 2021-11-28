#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <string>

// Constant declearance
#define T_UDP_PORT 21557
#define S_UDP_PORT 22557
#define P_UDP_PORT 23557
#define UDP_PORT_C 24557
#define TCP_PORT_A 25557
#define TCP_PORT_B 26557
#define LOCALHOST "127.0.0.1"
#define BUFF_SIZE 2000

// Founction declearance
void showBootupMsg();
int setupTCPSocket(int portNumber);
int setupUDPSocket(int portNumber);
int getPortNumber(int socket);

int main()
{
    showBootupMsg();

    // Create, bind and listen for TCP sockets A and B
    int listen_sock_A = setupTCPSocket(TCP_PORT_A);
    int listen_sock_B = setupTCPSocket(TCP_PORT_B);

    // Create and bind UDP socket
    int UDP_sockfd = setupUDPSocket(UDP_PORT_C);

    while (1)
    {
        // Define a socket poll
        int sock_count = 2;
        struct pollfd poll_fd[sock_count];
        poll_fd[0].fd = listen_sock_A;
        poll_fd[0].events = POLLIN | POLLOUT;
        poll_fd[1].fd = listen_sock_B;
        poll_fd[1].events = POLLIN | POLLOUT;

        struct sockaddr_in remote_addr;
        socklen_t addr_size = sizeof(remote_addr);
        int child_sock[2];

        std::string name[2] = {"", ""}; // Array to store user name from clientA(0) and client B(1)

        int flag = 1; // Flag indicates that whether received all clients' user name
        while (flag)
        {
            poll(poll_fd, sock_count, 2000);

            for (int i = 0; i < sock_count; i++)
            {
                if (poll_fd[i].revents & POLLIN)
                {
                    // Accept a TCP connect request, create a child socket for data transmission
                    if ((child_sock[i] = accept(poll_fd[i].fd, (struct sockaddr *)&remote_addr, &addr_size)) < 0)
                    {
                        perror("central: accept()");
                        return 1;
                    }

                    int byteCount;
                    char buffer_TCP[BUFF_SIZE]; // Receive buffer

                    // Receive user name from remote client
                    if ((byteCount = recv(child_sock[i], buffer_TCP, BUFF_SIZE, 0)) < 0)
                    {
                        perror("central: recv()");
                        return 1;
                    }
                    buffer_TCP[byteCount] = '\0';

                    printf("The Central server received input=\"%s\" from the client using TCP over port %d\n", buffer_TCP, getPortNumber(poll_fd[i].fd));

                    // Store into array for later request
                    name[i] = buffer_TCP;
                }
            }

            // If received both clients' user name, quit loop
            if (name[0] != "" && name[1] != "")
            {
                flag = 0;
            }
        }

        // Assign serverT information to remote_addr
        memset(&remote_addr, 0, sizeof(remote_addr));
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
        remote_addr.sin_port = htons(T_UDP_PORT);

        int len;
        std::string request = name[0] + " " + name[1];

        // Send user names to serverT to get topology
        if ((len = sendto(UDP_sockfd, request.c_str(), BUFF_SIZE, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr))) < 0)
        {
            perror("central: sendto()");
            return 1;
        }

        printf("The central server sent a request to Backend-Server T.\n");

        char buffer[BUFF_SIZE]; // Receive buffer

        // Receive topology from serverT
        if ((len = recvfrom(UDP_sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *)&remote_addr, &addr_size)) < 0)
        {
            perror("central: recvfrom()");
            return 1;
        }

        // Store topology for serverP request
        std::string serverTMsg = std::string(buffer);

        printf("The Central server received information from Backend-Server T using UDP over port %d.\n", getPortNumber(UDP_sockfd));

        // Assign serverS information to remote_addr
        memset(&remote_addr, 0, sizeof(remote_addr));
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
        remote_addr.sin_port = htons(S_UDP_PORT);

        // Send topology to serverS to get all nodes' scores
        if ((len = sendto(UDP_sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr))) < 0)
        {
            perror("central: sendto()");
            return 1;
        }

        printf("The central server sent a request to Backend-Server S.\n");

        // Receive all nodes' scores from serverS
        if ((len = recvfrom(UDP_sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *)&remote_addr, &addr_size)) < 0)
        {
            perror("central: recvfrom()");
            return 1;
        }

        printf("The Central server received information from Backend-Server S using UDP over port %d.\n", getPortNumber(UDP_sockfd));

        // Prepare request to serverP containing user names, topology and scores, seperate by "|"
        request = std::string(name[0]) + " " + std::string(name[1]) + "|" + serverTMsg + "|" + std::string(buffer);

        // Assign serverP information to remote_addr
        memset(&remote_addr, 0, sizeof(remote_addr));
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
        remote_addr.sin_port = htons(P_UDP_PORT);

        // Send request to serverP for processing
        if ((len = sendto(UDP_sockfd, request.c_str(), BUFF_SIZE, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr))) < 0)
        {
            perror("central: sendto()");
            return 1;
        }

        printf("The central server sent a processing request to Backend-Server P.\n");

        // Receive processing result from serverP
        if ((len = recvfrom(UDP_sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *)&remote_addr, &addr_size)) < 0)
        {
            perror("central: recvfrom()");
            return 1;
        }

        printf("The Central server received the results from backend server P.\n");

        // If no compatibility found
        if (std::string(buffer) == "null")
        {
            // Prepare result for clientA containing clientB's name
            std::string fail_A = std::string(name[1]) + "|" + std::string(buffer);
            // Prepare result for clientB containing clientA's name
            std::string fail_B = std::string(name[0]) + "|" + std::string(buffer);

            // Send result to clientA
            if (send(child_sock[0], fail_A.c_str(), strlen(fail_A.c_str()), 0) < 0)
            {
                perror("clientA: send()");
                return 1;
            }

            printf("The Central server sent the results to client A.\n");

            // Send result to clientB
            if (send(child_sock[1], fail_B.c_str(), strlen(fail_A.c_str()), 0) < 0)
            {
                perror("clientA: send()");
                return 1;
            }

            printf("The Central server sent the result to client B.\n");

            // Close all child sockets
            close(child_sock[0]);
            close(child_sock[1]);
        }

        // If compatibility found
        else
        {
            // Send result to clientA
            if (send(child_sock[0], buffer, strlen(buffer), 0) < 0)
            {
                perror("clientA: send()");
                return 1;
            }

            printf("The Central server sent the results to client A.\n");

            // Send result to clientB
            if (send(child_sock[1], buffer, strlen(buffer), 0) < 0)
            {
                perror("clientA: send()");
                return 1;
            }

            printf("The Central server sent the result to client B.\n");

            // Close all child sockets
            close(child_sock[0]);
            close(child_sock[1]);
        }
    }

    // Close all sockets
    close(UDP_sockfd);
    close(listen_sock_A);
    close(listen_sock_B);

    return 0;
}

/**
 * @brief Show the bootup message when Central sever bootup.
 * 
 */
void showBootupMsg()
{
    printf("The Central server is up and running.\n");
}

/**
 * @brief Server side TCP listen socket setup. Including create, bind and listen.
 * 
 * @param portNumber Port number assigned to this socket
 * @return int Socket descriptor of the created listen socket 
 */
int setupTCPSocket(int portNumber)
{
    int listen_sockfd;

    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));          // Clear all fields
    srv_addr.sin_family = AF_INET;                   // Use IPv4
    srv_addr.sin_addr.s_addr = inet_addr(LOCALHOST); // Bind with IP address 127.0.0.1
    srv_addr.sin_port = htons(portNumber);           // Bind with assigned port number

    if ((listen_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("central: TCP setup socket()");
        exit(1);
    }

    if (bind(listen_sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
    {
        perror("central: TCP setup bind()");
        exit(1);
    }

    listen(listen_sockfd, 20);

    return listen_sockfd;
}

/**
 * @brief Central Server side UDP socket setup. Including create and bind.
 * 
 * @param portNumber Port number assigned to this socket
 * @return int Socket descriptor of the created UDP socket
 */
int setupUDPSocket(int portNumber)
{
    int sockfd;

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));        // Clear all fields
    local_addr.sin_family = AF_INET;                   // Use IPv4
    local_addr.sin_addr.s_addr = inet_addr(LOCALHOST); // Bind with IP address 127.0.0.1
    local_addr.sin_port = htons(portNumber);           // Bind with assigned port number

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Central: UDP socket create");
        exit(1);
    }

    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("Central: UDP socket bind");
        exit(1);
    }
    return sockfd;
}

/**
 * @brief Get the port number of a specific socket
 * 
 * @param socket socket descriptor
 * @return int Port number assigned to the given socket
 */
int getPortNumber(int socket)
{
    struct sockaddr_in sock_addr;
    socklen_t sock_len = sizeof(sock_addr);
    getsockname(socket, (struct sockaddr *)&sock_addr, &sock_len);
    int portNumber = ntohs(sock_addr.sin_port);
    return portNumber;
}