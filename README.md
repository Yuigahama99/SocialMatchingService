## **Full name:** Bin Fang
## **Student ID:** 4398708557
***
## **What I have done:**
* ***Phase 1A:*** establish the connections between the Clients and Server C.
* ***Phase 1B:*** establish the connections between Server C and all other backend servers.
* ***Phase 2:*** dispatch and process in designated backend servers (serverT: topology; serverS: scores; serverP: process).
* ***Phase 3:*** return the shortest path from serverP to server C. Then send to the clients. Clients will display the result on screen.
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