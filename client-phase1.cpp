#include <bits/stdc++.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
using namespace std;


class Neighbour{
private:
    int clientid;
    int uniqueid;
    int listening_port;
    int sock_fd;
    bool connected;
    bool hassocket;
public:
    Neighbour(int _clientid,int _listening_port);
    bool makeConnection();
    bool makeSocket();
    bool sendMessage(string S);
    bool isConnected();
    bool hasSocket();
    ~Neighbour();
};

Neighbour::Neighbour(int _clientid,int _listening_port)
{
    sock_fd = -1;
    clientid = _clientid;
    listening_port = _listening_port;
    connected = false;
    hassocket = false;
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

Neighbour::~Neighbour()
{
    close(sock_fd);
}


void incoming_threadfn(int sock_fd)
{
    while(true)
    {
       //need to fill in action once we have recieved connections 
    }
}

int main(int argc, char *argv[])
{
    int client_id,port,unique_id,num_neighbor,num_files_needed ;
    vector<Neighbour> neighbors ;
    vector<string> files_needed ;
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
           files_needed.push_back(line);
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



    //to handle incoming connections
    int sock_fd = -1;
    struct sockaddr_in addr;
    sock_fd = socket(AF_INET,SOCK_STREAM,0);
    if (sock_fd == -1)
    {
        perror("Couldnt create socket to listen");
        exit(EXIT_FAILURE);
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Could not bind");
        exit(EXIT_FAILURE);
    }
    else if(listen(sock_fd,15) < 0)
    {
        perror("Could not listen");
        exit(EXIT_FAILURE);
    }
    
    thread recving(incoming_threadfn,sock_fd);



    //to handle outgoing connections




    close(sock_fd);
    return 0;
}
