/*
TCP/IP client
*/



#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#define SERVER_PORT 5060
#define SERVER_IP_ADDRESS "127.0.0.1"

int main(int argc, char const *argv[])
{

    //open the file
    char authentication_buffer[20]="4596";
    FILE *fp1,*coppy;
    fp1 = fopen("2mb.txt", "r");
    //Calculates the size of the file
    coppy=fp1;
    long long int size_file;
    for(size_file = 0; getc(coppy) != EOF; ++size_file);

    // Open the socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock == -1)
    {
        printf("Could not create socket : %d\n",errno);
    }

    // "sockaddr_in" is the "derived" from sockaddr structure used for IPv4 communication.
    struct sockaddr_in serverAddress;
    //Put the character 0 to sizeof(serverAddress) the first characters of serverAddress
    //means resets the serverAddress
    memset(&serverAddress, 0, sizeof(serverAddress));

    //inserts values into serverAddress
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    //Converts a web address in its standard text format to its numeric binary form.
    int rval = inet_pton(AF_INET, (const char*)SERVER_IP_ADDRESS, &serverAddress.sin_addr);
    if (rval <= 0)
    {
        printf("inet_pton() failed\n");
        return -1;
    }

    // Make a connection to the server with socket SendingSocket.
    if (connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1)
    {
        printf("connect() failed with error code : %d\n",errno);
    }

    printf("connected to server\n");

    //Asks the user to enter the number of times he wants to send the file and puts it in the x variable.
    // In addition, he sends it to the receiver so that he knows how many times he is going to receive the file.
    char loop[50]={'\0'};
    printf("Enter the number of times you want to send the file,\nnote that you cannot send more than 50 times\n");
    gets(loop);
    send(sock, loop, sizeof(loop), 0);
    int x = atoi(loop);
    //Checks if x>50, if so Stops the software (detailed in pdf).
    if(x>50){
        printf("Error The number you entered is greater than 50\n");
        fclose(fp1);
        close(sock);
        return 1;
    }

    while(x>0){
        //change CC algorithm to cubic
        char buffer[65536] = {0}; //buffer[1024*1000*2] = {0}
        strcpy(buffer, "cubic");
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buffer, strlen(buffer)) != 0) {
            perror("setsockopt\n");
            return -1;
        }
        //get current CC
        socklen_t len = sizeof(buffer);
        if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buffer, &(len)) != 0) {
            perror("getsockopt\n");
            return -1;
        }

        int b=0;//length of bites that install to the buffer
        int count =0;//total bites that send for sure
        printf("Sends the first part of the file.....\n");
        //Sending the first part
        while(count<=size_file/2){
            fgets(buffer, 65536, fp1);
            b=send(sock, buffer, sizeof(buffer), 0);
            //b=fread(buffer,1,sizeof(buffer),fp1);
            if(-1==b){
                printf("error in sending file: %d\n",errno);
            }
            else if(0==b){
                printf("peer has closed the tcp connection prior to send \n");
            }

            else{

                count +=b;
            }
            bzero(buffer, 65536);
        }
        printf("Finished sending the first part of the file\n");




        //getting from server the authentication and checking it out
        char received_authentication[20] = {'\0'};
        int return_status = recv(sock, &received_authentication, sizeof(received_authentication),0);
        if (return_status > 0) {
            if(strcmp(received_authentication,authentication_buffer) !=0){
                printf("Error in authentication\n");
                return -1;
            }else{
                printf("authentication test passed successfully\n");
            }

        }
        else {
            printf("recvfrom() failed with error code  : %d\n", errno);
            return -1;
        }




        //change CC algorithm to reno
        strcpy(buffer, "reno");
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buffer, strlen(buffer)) != 0) {
            perror("setsockopt");
            return -1;
        }
        printf("After change TCP CONGESTION CONTROL: %s\n", buffer);

        printf("Sends the second part of the file.....\n");
        b=0;//length of bites that install to the buffer
        //Sending the second part
        while(count<size_file){
            fgets(buffer, 65536, fp1);
            b=send(sock, buffer, sizeof(buffer), 0);
            if(-1==b){
                printf("error in sending file: %d",errno);
            }
            else if(0==b){
                printf("peer has closed the tcp connection prior to send \n");
            }else{
                //printf("sent %d bytes\n",b);
                count +=b;
            }
            bzero(buffer, 65536);
        }
        printf("Finished sending the secod part of the file\n");
        //change CC algorithm to cubic
        strcpy(buffer, "cubic");
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buffer, strlen(buffer)) != 0) {
            perror("setsockopt");
            return -1;
        }
        printf("After change TCP CONGESTION CONTROL: %s\n", buffer);

        x--;
    }
    printf("END ! \n");
    sleep(3);
    fclose(fp1);
    close(sock);

    return 0;
}

