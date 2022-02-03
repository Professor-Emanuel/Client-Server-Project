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
#include<sstream>
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
        std::cerr<<"WSAStartup failed! Socket was not initialized!";
        return -1;
    }
    //Create a socket
    //IPv4 ->AF_INET
    //TCP socket -> SOCK_STREAM
    //flag -> 0
    SOCKET listenFD = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFD == INVALID_SOCKET) {
       std::cerr<<"Failed in creating a socket!!! Try again!!!"<<std::endl;
        return -1;
    }

    //Bind the IP address and PORT to a SOCKET
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.S_un.S_addr = INADDR_ANY; //this can also be done with inet_pton
    serverAddress.sin_port = htons(SERVER_PORT_NUMBER); //htons -> host to network short
    bind(listenFD, (sockaddr*)&serverAddress, sizeof(serverAddress));

    //Place socket in passive mode ready to accept requests
    listen(listenFD, SOMAXCONN);

    // create a list, make it empty and add the listening file descriptors to it
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(listenFD, &readSet);

    // create a running server that handles multiple connections
    while (true) {
        //we 1st need to make copies of the data and then use select() -> the select call actually distroys the fd set
        //select() ->determines the status of one or more sockets, waiting if necessary to perform synchronous I/O
        //if any socket will successfully be created they will end up in "theCopyOfReadSet"
        fd_set theCopyOfReadSet = readSet;
        int socketCount = select(0, &theCopyOfReadSet, nullptr, nullptr, nullptr);

        for (int i = 0; i < socketCount; i++) {
            SOCKET someSocket = theCopyOfReadSet.fd_array[i];
            if (someSocket == listenFD) {
                // if new connection is accepted -> add it to the list of clients
                // and maybe send a message to the client, to see if it worked 

                sockaddr_in theNewClient;
                int theNewClientSize = sizeof(theNewClient);
                SOCKET theNewClientFD = accept(listenFD, (sockaddr*)&theNewClient, &theNewClientSize);
                FD_SET(theNewClientFD, &readSet);

                std::string greetings = "WELCOME!";
                send(theNewClientFD, greetings.c_str(), greetings.size() + 1 , 0);
            }
            else
            {   //if new message is accepted -> send it to clients, and not the listening socket
                char buffer[4096];
                ZeroMemory(buffer, 4096);
                int bytesInputMessage = recv(someSocket, buffer, 4096, 0);

     

                if (bytesInputMessage <= 0) {
                    closesocket(someSocket);
                    FD_CLR(someSocket, &readSet);
                }
                else
                {
                    std::cout << buffer << std::endl;
                    for (int i = 0; i < readSet.fd_count; i++) {
                        SOCKET theOutSocket = readSet.fd_array[i];
            
                        if (theOutSocket != listenFD && theOutSocket != someSocket) {

                            std::ostringstream ss;
                            ss << "The Socket number " << someSocket << ":" << buffer << "\r\n";

                            std::string strOut = ss.str();
                            send(theOutSocket, strOut.c_str(), strOut.size() + 1 , 0);
                        }

                    }
                }
            }
        }

    }
    // clean winsock
    WSACleanup();
    system("pause");

    return 0;
}

// select() -> is used to create multiple clients on windows
// FD_CLR -> removes from set
// FD_SET -> adds to set
// FD_ZERO -> clears the set
// fd_set -> is an array of integers, which has a number associated with it
// | 40 | 200 | 120 | 10 | - | - | -> number of socket we are interested in ("-" means invalid)
//  the 1st slot can be used for LISTENING, and the 3 others for CLIENTS
//so fd_set is a struct which has variables:  fd_count , fd_array[FD_SETSIZE]