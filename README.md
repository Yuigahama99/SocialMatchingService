## Socket Programming Project: Social Matching Service
Social networks are nowadays in every moment of our lives. The information built upon interactions among people has led to different types of applications. Crowdsourced apps such as Uber, Lyft, Waze use information for navigation purposes. Other apps such as dating apps provide matching algorithms to connect users who share similar behaviours and increase their compatibility chances for future success. In this project we shall implement a simplified version of a matching app that'll help us understand how matching systems work in the real world. Specifically, you'll be given a network topology consisting of social connections between different users in the network. This will consist of nodes representing users and the edges between them. Beside social network topology, you will also be given a database consisting of compatibility test scores. This database will be in plain text and consist of multiple key (the user), value (the
corresponding score) pairs.

In this project, you will implement a model of a social matching service where two clients issue a request for finding their compatibility. This request will be sent to a Central Server which in turn interacts with three other backend servers for pulling information and data processing. The Central server will connect to the Topology server (server T) which has the user social network information. Central server has to connect as well to the Score server (server S) which stores the compatibility scores for each user. Finally, the server will use the network topology and scores to generate a graph that connects both users, and provide the smallest matching gap between them. The procedure to complete this task is provided in phase 2’s description. Both the matching gap and the graph generated will be sent back to both clients.
![image](https://user-images.githubusercontent.com/76589915/143735982-7c1c7770-1d8d-4a5c-94c5-167b27266595.png)
Server T has access to a database file named edgelist.txt, and Server S has access to a database file named scores.txt. Both clients and the Central server communicate over a TCP connection while the communication between Central and the Back-Servers T, S & P is over a UDP connection. This setup is illustrated
in Figure 1.
***
## **Files function:**
* ***clientA.cpp:*** login using command **./clientA \<username>**, receive and display compatibility result with clientB from server.
* ***clientB.cpp:*** login using command **./clientB \<username>**, receive and display compatibility result with clientA from server.
* ***central.cpp:*** run using command **./serverC**, receive request from clients, dispatch to other designated servers, return results to clients.
* ***serverT.cpp:*** run using command **./serverT**, receive request from Central server, find topology, return to Central server.
* ***serverS.cpp:*** run using command **./serverS**, receive request from Central server, find nodes' scores in topology, return to Central server.
* ***serverP.cpp:*** run using command **./serverP**, receive request from Central server, find shortest path from A to B, return to Central server.
* ***Makefile:*** compile all .cpp file using command **make all**, clean all .o file using command **make clean**.
* ***edgelist.txt:*** store edges of social network.
* ***scores.txt:*** store scores of all nodes in social network.
***
## **Message format:**
* ***clientA -> serverC:*** 
"[username]"
* ***clientB -> serverC:*** 
"[username]"
* ***serverC -> serverT:*** 
"[username] [username]"
* ***serverT -> serverC:*** 
If reachable: "[nodename] [nodename]\n[nodename] [nodename]\n..."
Other situation: "null" 
* ***serverC -> serverS:*** 
If reachable: "[nodename] [nodename]\n[nodename] [nodename]\n..." 
Other situation: "null" 
* ***serverS -> serverC:***
If reachable: "[nodename] [score]\n[nodename] [score]\n..." 
Other situation: "null 0"
* ***serverC -> serverP:*** 
If reachable: "[username] [username]|[nodename] [nodename]\n[nodename] [score]\n...|[nodename] [score]\n[nodename] [score]\n..." 
Other situation: "[username] [username]|null|null 0" 
* ***serverP -> serverC:*** 
If reachable: "[nodename] [nodename] [nodename]...|[weight]"
Other situation: "null"
* ***serverC -> clientA:***
If reachable: "[nodename] [nodename] [nodename]...|[weight]"
Other situation: "[username]|null"
* ***serverC -> clientB:***
If reachable: "[nodename] [nodename] [nodename]...|[weight]"
Other situation: "[username]|null"
***
## **Reused Code:**
* Part of the Graph class from https://github.com/ShengHangNB/Graph/blob/main/graph.hpp with modification in dijkstra() to return the message metioned in **Message format** section **serverP -> serverC**.
* Code using strtok_r() to seperate message with multiple tokens in server from https://stackoverflow.com/questions/4693884/nested-strtok-function-problem-in-c. Also with little modification to fit in my code.
