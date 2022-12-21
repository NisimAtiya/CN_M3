/*
    TCP/IP-server
*/

#include <netinet/in.h>
#include <netinet/tcp.h>
#define BILLION  1000000000.0
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#define SERVER_PORT 5060  //The port that the server listens

int main()
{
    //open the file
    FILE *fp1,*coppy;
    fp1 = fopen("2mb.txt", "r");
    coppy=fp1;
    //Calculates the size of the file
    long long int size_file;
    for(size_file = 0; getc(coppy) != EOF; ++size_file);
    sleep(3);
    fclose(fp1);

    //Arrays for storing the running times of the algorithms
    float end_first_arr[50],end_second_arr[50];
    int cnt_first=0,cnt_second=0;
    char authentication_buffer[20]="4596";

    // Open the listening (server) socket
    int listeningSocket = -1;

    if((listeningSocket = socket(AF_INET , SOCK_STREAM , 0 )) == -1)
    {
        printf("Could not create listening socket : %d\n",errno);
    }

    // Reuse the address if the server socket on was closed
    // and remains for 45 seconds in TIME-WAIT state till the final removal.
    //
    int enableReuse = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0)
    {
        printf("setsockopt() failed with error code : %d\n" , errno);
    }

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    struct sockaddr_in serverAddress;
    //Put the character 0 to sizeof(serverAddress) the first characters of serverAddress
    //means resets the serverAddress
    memset(&serverAddress, 0, sizeof(serverAddress));
    //inserts values into serverAddress
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);  //network order

    // Bind the socket to the port with any IP at this port
    if (bind(listeningSocket, (struct sockaddr *)&serverAddress , sizeof(serverAddress)) == -1)
    {
        printf("Bind failed with error code : %d\n" ,	errno);
        // TODO: close the socket
        return -1;
    }

    printf("Bind() success\n");

    // Make the socket listening; actually mother of all client sockets.
    if (listen(listeningSocket, 500) == -1) //500 is a Maximum size of queue connection requests
        //number of concurrent connections
    {
        printf("listen() failed with error code : %d\n",errno);
        // TODO: close the socket
        return -1;
    }

    //Accept and incoming connection
    printf("Waiting for incoming TCP-connections...\n");

    struct sockaddr_in clientAddress;  //
    socklen_t clientAddressLen = sizeof(clientAddress);

    //Put the character 0 to sizeof(clientAddress) the first characters of clientAddress
    //means resets the clientAddress
    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddressLen = sizeof(clientAddress);
    int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
    if (clientSocket == -1)
    {
        printf("listen failed with error code : %d\n",errno);
        // TODO: close the sockets
        return -1;
    }

    printf("A new client connection accepted\n");
    //Receives from the sender how many times he is going to send the file and stores in x
    char loop[20];
    recv(clientSocket, &loop, sizeof(loop),0);
    int x=atoi(loop);
    //Checks if x>50, if so Stops the software (detailed in pdf).
    if(x>50){
        printf("Error\n");
        close(listeningSocket);
        exit(1);
    }
    while (x>0)
    {
        //opens a clock
        struct timespec start, end;
        char bufferReply[65536] ={'\0'};
        clock_gettime(CLOCK_REALTIME, &start);
        int b=0;
        int count=0;
        printf("Gets the first part of the file.....\n");
        //receiver the first part
        while(count<size_file/2){
            // try to receive some data, this is a blocking call
            if ((b=recv(clientSocket, bufferReply, sizeof(bufferReply) -1, 0)) == -1)
            {
                printf("recvfrom() failed with error code  : %d\n", errno	);
                return -1;
            }
            count+=b;
        }
        //Stops the clock
        clock_gettime(CLOCK_REALTIME, &end);
        //Calculates the time it took to send the first part and stores it in the time array
        float time_taken_first = (float)(end.tv_sec - start.tv_sec) +
                                 (end.tv_nsec - start.tv_nsec) / BILLION;
        if(cnt_first==50){
            printf("Error ! memory not allocated.\n");
            return -1;
        }
        end_first_arr[cnt_first]=time_taken_first;
        cnt_first++;

        printf("Finished getting the first part of the file\n");


        //Reply to sender the authentication
        int bytesSent = send(clientSocket, authentication_buffer, sizeof(authentication_buffer), 0);
        if (bytesSent == -1)
        {
            printf("send() failed with error code : %d\n" ,errno);
        }
        else if (bytesSent==0)
        {
            printf("peer has closed the TCP connection prior to send().\n");
        }else if(bytesSent<sizeof(int)){
            printf("only %d bytes sent of %ld bytes\n",bytesSent,sizeof(int));
        }
        else
        {
            printf("authentication was successfully sent.\n");
        }

        char bufer[65536] = { '\0' };
        //change CC algorithm to reno
        printf("Changing CC algorithm\n");
        strcpy(bufer, "reno");
        int len = sizeof(bufer);
        if (setsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, bufer, len) != 0)
        {
            perror("setsockopt\n");
            return -1;
        }
        printf("New CC: %s\n", bufer);

        //opens a clock
        struct timespec start_second, end_second;
        clock_gettime(CLOCK_REALTIME, &start_second);
        b=0;
        printf("Gets the second part of the file.....\n");
        //reciver the second part
        while(count<size_file){
            // try to receive some data, this is a blocking call
            if ((b=recv(clientSocket, bufferReply, sizeof(bufferReply) -1, 0)) == -1)
            {
                printf("recvfrom() failed with error code  : %d\n", errno	);
                return -1;
            }
            count+=b;
        }
        //Stops the clock
        clock_gettime(CLOCK_REALTIME, &end_second);
        //Calculates the time it took to send the first part and stores it in the time array
        float time_taken_second = (float)(end_second.tv_sec - start_second.tv_sec) +
                                  (end_second.tv_nsec - start_second.tv_nsec) / BILLION;
        if(cnt_second==50){
            printf("Error ! memory not allocated.\n");
            exit(1);
        }
        end_second_arr[cnt_second]=time_taken_second;
        cnt_second++;
        printf("Finished getting the first part of the file\n");

        //change CC algorithm to cubic
        strcpy(bufer, "cubic");
        if (setsockopt(clientSocket, IPPROTO_TCP, TCP_CONGESTION, bufer, strlen(bufer)) != 0) {
            perror("setsockopt\n");
            return -1;
        }
        printf("After change TCP CONGESTION CONTROL: %s\n", bufer);
        x--;
    }
    //Calculates the average times of the two parts, prints all the times in the time arrays, and prints the average times
    float avarge_first,avarge_second;
    for(int i=0; i<cnt_first;i++){
        printf("time that take in %d iteration is %f seconds\n",i+1,end_first_arr[i]);
        printf("time that take in %d iteration is %f seconds\n",i+1,end_second_arr[i]);
        avarge_first+=end_first_arr[i];
        avarge_second+=end_second_arr[i];
    }
    printf("the avarge first part time is: %f seconds\n",avarge_first);
    printf("the avarge second part time is: %f seconds\n",avarge_second);

    sleep(3);
    close(listeningSocket);
    exit(1);
    return 0;
}