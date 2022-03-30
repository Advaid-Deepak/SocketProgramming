#include <bits/stdc++.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>



#include <sys/types.h>
#include <netdb.h>
using namespace std;
#define MAXDATASIZE 1000

class Neighbour{
private:
    int clientid;
    int uniqueid;
    int listening_port;
    int sock_fd;
    bool connected;
    bool hassocket;
    bool depth1 ;
    char rcvmsg[MAXDATASIZE];
    vector<string> recieved_messages ;
public:
    Neighbour(int _clientid,int _listening_port);
    bool makeConnection();
    bool makeSocket();
    bool sendMessage(string S);
    bool isConnected();
    bool hasSocket();
    bool compareNeighbours(Neighbour n1,Neighbour n2) ;
    bool checkDepth1() ;
    void setDepth1() ;
    string rcvMessage();
    string getMessage(int i) ;
    void setUniqueID(int id) ;
    int GetUniqueID() ;
    ~Neighbour();
};

Neighbour::Neighbour(int _clientid,int _listening_port)
{
    sock_fd = -1;
    clientid = _clientid;
    listening_port = _listening_port;
    connected = false;
    hassocket = false;
    depth1 = false ;
}

bool Neighbour::isConnected()
{
    return connected;
}

bool Neighbour::hasSocket()
{
    return hassocket;
}

bool Neighbour::makeSocket()
{
   sock_fd = socket(AF_INET,SOCK_STREAM,0);
   if (sock_fd < 0)
   {
        sock_fd = -1;
        hassocket = false;
        return false;
   }
   else
   {
        hassocket = true;
        return true;
   }
}
bool Neighbour::makeConnection()
{
    if (hassocket==false)
    {
        makeSocket();
        if (hassocket==false)
            return false;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(listening_port);
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        connected = false;
        return false;
    }
    else
    {
        connected = true;
        return true;
    }

}

bool Neighbour::sendMessage(string s)
{
    if (connected)
    {
        send(sock_fd,s.c_str(),sizeof(s.c_str()),0);
        return true;
    }
    else
    {
        return false;
    }
}

string Neighbour::rcvMessage()
{
    int numbytes;
    numbytes = recv(sock_fd, rcvmsg, MAXDATASIZE-1, 0);
    if(numbytes == -1)
    {
        perror("Could not recieve");
        return "";
    }
    else
    {
        string res = "";
        for (int i=0;i<numbytes;i++)
        {
            res += rcvmsg[i];
        }
        recieved_messages.push_back(res) ;
        return res;
    }
}

string Neighbour::getMessage(int i){
    return recieved_messages[i] ;
}

void  Neighbour::setUniqueID(int id){
    uniqueid = id ;
}

bool Neighbour::checkDepth1(){
    return depth1 ;
}

void Neighbour::setDepth1(){
    depth1 = true ;
}

Neighbour::~Neighbour()
{
    close(sock_fd);
}

bool Neighbour::compareNeighbours(Neighbour n1, Neighbour n2)
{
    return (n1.uniqueid < n2.uniqueid);
}

int Neighbour::GetUniqueID(){
    return uniqueid ;
}

bool compareFunction(Neighbour n1, Neighbour n2)
{
    return n1.compareNeighbours(n1,n2) ;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char *argv[])
{
    int client_id,port,unique_id,num_neighbor,num_files_needed ;
    vector<Neighbour> neighbors ;
    vector<pair<string,vector<Neighbour>>> files_needed ;
    vector<pair<string,int>> files_found ;
    string line;
    ifstream config_file;
    config_file.open(argv[1]);

   if(!config_file.is_open()) {
      perror("Error open");
      exit(EXIT_FAILURE);
   }
   int line_num = 1;
    while(getline(config_file, line)) {
       if(line_num == 1){
           istringstream line1(line);
           line1 >> client_id ;
           line1 >> port ;
           line1 >> unique_id ;
           line_num++ ;
           continue ;
       }
       if(line_num == 2){
           num_neighbor = stoi(line) ;
           line_num++ ;
           continue ;
       }
       if(line_num == 3){
            istringstream line3(line) ;
            for(int i = 0 ; i < num_neighbor ; i++){
                int a , b ;
                line3 >> a ; line3 >> b ;
                //cout << a << " " << b << endl ;
                neighbors.push_back(Neighbour(a,b)) ;
            }
            line_num++ ;
            continue ;
       }
       if(line_num == 4) {
           num_files_needed = stoi(line) ;
           line_num++ ;
           continue ;
       }
       if(line_num >= 5 && line_num < 5 + num_files_needed){
           if(line_num != 4 + num_files_needed) line = line.substr(0,line.length()-1);
           vector<Neighbour> n ;
           files_needed.push_back(make_pair(line,n));
           line_num++ ;
       }
    }
    config_file.close();

    DIR *dir;
    struct dirent *diread;
    vector<string> files_owned;

    if ((dir = opendir(argv[2])) != nullptr) {
        while ((diread = readdir(dir)) != nullptr) {
            if(diread->d_name[0] != '.'){
            files_owned.push_back(diread->d_name);
            }
        }
        closedir (dir);
    } else {
        perror ("opendir");
        return EXIT_FAILURE;
    }
    for (auto file : files_owned) cout << file << "\n";

    //for (auto file : files_needed) cout << file.first << "\n";

   fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints ;
    struct addrinfo *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, port+"", &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
                    }
                } else {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        for(j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } 

    
    return 0;
}