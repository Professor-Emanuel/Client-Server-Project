/*********************************************************************************************************
                                                The Server
    1. Initialize the WINSOCK
    2. Create a socket -> make a ne communication
    3. Bind the local IP address of socket to a port
    4. Place socket in passive mode ready to accept requests
    5. Take next request from queue (or wait) and create new socket for the respective client connection
    6. Block until connection from client
    7. Process request
    8. Close the socket
*********************************************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define SERVER_PORT_NUMBER 8181
#include <windows.h>
//#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include<iostream>
#include<io.h>
#include<string>
#pragma comment(lib, "ws2_32.lib") //Need to link with Ws2_32.lib

int main() {

    int error;

    //Initialize the winsock
    //on Windows platforms, the function WSAStartup() has to be called before calling the socket() function.
    WORD version;
    WSADATA wsaData;

    version = MAKEWORD(2, 2);// use the MAKEWORD(lbyte,hbyte) macro declared in Windef.h
    error = WSAStartup(version, &wsaData); //If successful, the WSAStartup function returns zero, otherwise, it returns an error code
    if (error != 0) {
        // Tell the user that we could not find a usable Winsock DLL.
        printf("WSAStartup failed with error: %d\n", error);
        return 1;
    }
    //Create a socket
    //IPv4 ->AF_INET
    //TCP socket -> SOCK_STREAM
    //flag -> 0
    SOCKET listenFD = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFD == INVALID_SOCKET) {
        printf("Failed in creating a socket!\n");
        return 1;
    }

    //Bind the IP address and PORT to a SOCKET
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.S_un.S_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT_NUMBER); //htons -> host to network short
    bind(listenFD, (sockaddr*)&serverAddress, sizeof(serverAddress));

    //Place socket in passive mode ready to accept requests
    listen(listenFD, SOMAXCONN);

    //Wait for a connection
    sockaddr_in theClient;
    int theClientSize = sizeof(theClient);

    // at this point the server is started
    // the server wait for a client request done by calling accept()
    // accept() runs an infinite loop, to keep the server always running
    SOCKET theClientFD = accept(listenFD, (sockaddr*)&theClient, &theClientSize);
    if (theClientFD == INVALID_SOCKET) {
        printf("Failed in creating a client socket!\n");
        return 1;
    }

    char hostIP[NI_MAXHOST]; //Client's IP address
    char portNR[NI_MAXSERV]; //Client's PORT number connection

    ZeroMemory(hostIP, NI_MAXHOST); //similar to memset(hostIP, 0, NI_MAXHOST);
    ZeroMemory(portNR, NI_MAXSERV);

    inet_ntop(AF_INET, &theClient.sin_addr, hostIP, NI_MAXHOST);
    std::cout << hostIP << " connected to port " << ntohs(theClient.sin_port) << std::endl;
    //network to host short -> ntohs

    //Close the listening socket
    closesocket(listenFD);

    char buffer[4096];
    while (true) {
        ZeroMemory(buffer, 4096);
        //ZeroMemory clears a block of memory in any programming language
        // Wait for the client to send data
        int bytesReceived = recv(theClientFD, buffer, 4096, 0);

        if (strncmp(buffer, "exit", 4) == 0) {
            std::cout << "Client left" << std::endl;
            return 1;
        }
        
        if (bytesReceived == SOCKET_ERROR) {
            printf("Error in recv().\n");
            break;
        }

        if (bytesReceived == 0) {
            printf("The client disconnected!\n");
            break;
        }
        std::cout << buffer << std::endl;
        send(theClientFD, buffer, bytesReceived + 1, 0);
    }

    //Close the socket
    closesocket(theClientFD);
    WSACleanup();
    //Terminate winsock
    return 0;
}

// select() -> is used to create multiple clients on windows
// FD_CLR -> removes from set
// FD_SET -> adds to set
// FD_ZERO -> clears the set
// fd_set -> is an array of integers, which has a number associated with it
// | 40 | 200 | 120 | 10 | - | - | -> number of socket we are interested in ("-" means invalid)
//  the 1st slot can be used for LISTENING, and the 3 others for CLIENTS