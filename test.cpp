#include "RtMidi.h"
#include <iostream>
#include <cstdlib>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/types.h>
#include "RtMidi.h"
#include "Button.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define MY_PORT "3490"
#define BACKLOG 10
#define MAX_SIZE 128
#pragma comment(lib, "ws2_32.lib")

struct addrinfo settings;
struct addrinfo* res, * p; // will point to the result
struct sockaddr_storage client_addr;
int sockfd, new_fd, status;
char yes = '1';
socklen_t addr_size;
char hostname[128];
size_t size;
char str[MAX_SIZE];
char msg[MAX_SIZE];
int len;
size_t bytes_rec;

void setup(struct addrinfo* settings)
{

    memset(settings, 0, sizeof settings); // make sure the struct is empty
    settings->ai_family = AF_INET;        // don't care if IPv4 or IPv6
    settings->ai_flags = AI_PASSIVE;      // use my IP address
    settings->ai_socktype = SOCK_STREAM;  // use TCP stream sockets
}

//IPv4 or IPv6
void* get_in_addr(struct sockaddr* sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


namespace buttons {
    enum abletonButtons { PLAY, RECORD, STOP, CUE };
}

int portNbr = 3; /* The port I'm interested in*/
std::vector<unsigned char> message;

Button PLAY, RECORD, STOP, CUE;
void set_buttons(buttons::abletonButtons button, RtMidiOut* midiout) {

    switch (button)
    {
    case buttons::PLAY:
    {
        std::cout << " PLAY \n";
        message = PLAY.getMessage();
        midiout->sendMessage(&message);
        break;
    }
    case buttons::RECORD:
    {
        std::cout << " RECORDING \n";
        message = RECORD.getMessage();
        midiout->sendMessage(&message);
        break;
    }
    case buttons::STOP:
    {       
        std::cout << " STOP \n";
        message = STOP.getMessage();
        midiout->sendMessage(&message);
        break;
    }
    case buttons::CUE:
    {
        std::cout << " CUE \n";
        message = CUE.getMessage();
        midiout->sendMessage(&message);
        break;
    }

    }
}


int main()
{
    RtMidiIn* midiin = 0;
    RtMidiOut* midiout = 0;
    // RtMidiIn constructor
    try {
        midiin = new RtMidiIn();
    }
    catch (RtMidiError & error) {
        error.printMessage();
        exit(EXIT_FAILURE);
    }
    // Check inputs.
    unsigned int nPorts = midiin->getPortCount();
    std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
    std::string portName;
    for (unsigned int i = 0; i < nPorts; i++) {
        try {
            portName = midiin->getPortName(i);
        }
        catch (RtMidiError & error) {
            error.printMessage();
            goto cleanup;
        }
        std::cout << "  Input Port #" << i + 1 << ": " << portName << '\n';
    }
    // RtMidiOut constructor

    try {
        midiout = new RtMidiOut();
    }
    catch (RtMidiError & error) {
        error.printMessage();
        exit(EXIT_FAILURE);
    }
    // Check outputs.
    nPorts = midiout->getPortCount();
    std::cout << "\nThere are " << nPorts << " MIDI output ports available.\n";
    for (unsigned int i = 0; i < nPorts; i++) {
        try {
            portName = midiout->getPortName(i);
        }
        catch (RtMidiError & error) {
            error.printMessage();
            goto cleanup;
        }
        if (portName.find("loopMIDI") == 0) {
            std::cout << "Found your port Peter!" << " : " << portName << '\n';
            portNbr = i;
        }
        std::cout << "  Output Port #" << i + 1 << ": " << portName << '\n';
    }
    std::cout << '\n';
    midiout->openPort(portNbr); /* Open the port of interest*/

    // Send out a series of MIDI messages

  
    /******************************************SOCKETS***************************************************************/

    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);

    setup(&settings);
    if ((status = getaddrinfo(NULL, "3490", &settings, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 1;
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server:socket");
            continue;
        }
        /* Set this for re-use of port*/
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            closesocket(sockfd);
            fprintf(stderr, "Could not bind to %s", MY_PORT);
            continue;
        }
        break;
    }
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections... \n");
    freeaddrinfo(res);

    while (1)
    {
        addr_size = sizeof client_addr;
        //one client at a time
        new_fd = accept(sockfd, (struct sockaddr*) & client_addr, &addr_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }
        else
        {
            struct sockaddr_in* sa = (struct sockaddr_in*) & client_addr;
            inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr*) & client_addr), str, res->ai_addrlen);
            printf("server: got connection from %s \n", str);

            while ((bytes_rec = recv(new_fd, msg, 128, 0)) != 0)
            {   
                std::cout << " Map keys... \n";
                //printf("Message received: %s, bytes received: %lu \n", msg, bytes_rec);
                switch (msg[0]) {
                case 'H':
                    set_buttons(buttons::PLAY, midiout);
                    break;
                case 'R':
                    set_buttons(buttons::RECORD, midiout);
                    break;
                case 'S':
                    set_buttons(buttons::STOP, midiout);
                    break;
                case 'O':
                    set_buttons(buttons::CUE, midiout);
                    break;
                };
            }
            printf("Connection closed: %d\n", new_fd);
        }
    }
    // Clean up
cleanup:
    delete midiin;
    delete midiout;
    return 0;
}