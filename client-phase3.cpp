#include <bits/stdc++.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
using namespace std;
#define MAXDATASIZE 1000
pthread_mutex_t print_mutex;

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
    vector<string> files_to_ask;
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
    int getUniqueID() ;
    int getPort();
    void addFileToAsk(string file);
    string getFileToAsk();
    ~Neighbour();
};

void Neighbour::addFileToAsk(string file){
    files_to_ask.push_back(file);
}

string Neighbour::getFileToAsk(){
    string r = "";
    for (auto &u:files_to_ask){
        r+=u;
        r+=" ";
    }
    return r;
}

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

int Neighbour::getPort(){
    return listening_port;
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
        send(sock_fd,s.c_str(),s.length(),0);
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
        //recieved_messages.push_back(res) ;
        //Commented this out, as it will cause memory issues for large files
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

int Neighbour::getUniqueID(){
    return uniqueid ;
}

bool compareFunction(Neighbour n1, Neighbour n2)
{
    return n1.compareNeighbours(n1,n2) ;
}



bool send_file(int new_fd, string path, string filename)
{
    //Protocol
    //p1: filename + nofile     filename + yesfile + length_of_file + numpackets
    //filepackets: filename + serialno + numbytes + payload
    //ack: yes + serialno + filename
    fstream fin(path);
    string packet;
    if(!fin.is_open())
    {
        packet = filename + " nofile";
        send(new_fd,packet.c_str(),packet.length(),0);
        return false;
    }
    int length;
    fin.seekg(0,ios::end);
    length = fin.tellg();
    fin.seekg(0,ios::beg);

    packet = filename + " yesfile";
    packet = packet + " " + to_string(length);
    int maxpacketpayload = MAXDATASIZE - 100;
    int numpackets = ceil(double(length)/double(maxpacketpayload));
    packet = packet + " " + to_string(numpackets);
    send(new_fd,packet.c_str(),packet.length(),0);
    int bytes_sent = 0;
    char rcvmsg[MAXDATASIZE];
    int serialno = 0;
    
    while(fin.good())
    {
        packet = "";
        packet += filename + " " + to_string(serialno);
        
        fin.read(rcvmsg,maxpacketpayload);
        int numbytes = fin.gcount();
        packet += " " + to_string(numbytes) + " ";
        for (int i=0;i<numbytes;i++)
        {
            packet += rcvmsg[i];
        }
        bytes_sent += numbytes;
        bool acked = false;
        int maxtries = 16;
        int tries = 0;
        while(!acked)
        {
            tries++;
            if (tries > maxtries) return false;
            send(new_fd,packet.c_str(),packet.length(),0);
            int numbytes = recv(new_fd,rcvmsg,MAXDATASIZE-1,0);
            int timeout = 0;
            if(numbytes == -1)
            {
                perror("Could not recieve");
                acked = false;
                sleep(timeout);
                timeout++;
            }
            else
            {
                string res = "";
                for (int i=0;i<numbytes;i++)
                {
                    res += rcvmsg[i];
                }
                if (res == "yes " + to_string(serialno) + " " + filename)
                {
                    acked = true;
                    serialno++;
                }
                else
                {
                    sleep(timeout);
                    timeout++;
                }
                
            }
        }

    }


    return true;

}


void incoming_threadfn(int sock_fd, int clientid, int uniqueid,vector<string> files_owned, string path)
{
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int new_fd;
    stringstream ss;
    ss << clientid <<" " << uniqueid;
    string s = ss.str() ;
    if (path[path.length()-1]!='/')
        path += "/" ;
    while(true)
    {
        
        sin_size = sizeof their_addr;
        new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size);
        if (!fork())
        {
            //wait for reply with list of files to search for
            //send file I found
            close(sock_fd); // child doesn't need the listener
            if (send(new_fd, s.c_str(), s.length(), 0) == -1)
                perror("send");
            int numbytes;
            char rcvmsg[MAXDATASIZE] ;
            numbytes = recv(new_fd, rcvmsg, MAXDATASIZE-1, 0);
            string res = "";
            if(numbytes == -1)
            {
               perror("Could not recieve");
            }
            else
            {
              for (int i=0;i<numbytes;i++)
               {
                   res += rcvmsg[i];
               }
            }
            // pthread_mutex_lock(&print_mutex);
            // cout<<"===Recieved search request for\n"<<res<<"==="<<endl;
            // pthread_mutex_unlock(&print_mutex);
            istringstream fil_search_stream(res);
            stringstream file_to_be_sent_stream;
            vector<pair<string,string>> file_list;
            while(fil_search_stream){
               string  file ;
               fil_search_stream >> file ;
               
               for(int i = 0 ; i < files_owned.size() ; i++){
                    if(files_owned[i] == file) {
                        file_to_be_sent_stream << file << " " ;
                        file_list.push_back({path + file,file});
                    }
               }
            }
            string files_to_send = file_to_be_sent_stream.str() ;
            if (send(new_fd, files_to_send.c_str(), files_to_send.length(), 0) == -1)
                    perror("send");

            
            numbytes =  recv(new_fd, rcvmsg, MAXDATASIZE-1, 0);
            files_to_send = "" ;
            if(numbytes == -1)
            {
               perror("Could not recieve");
            }
            else
            {
              for (int i=0;i<numbytes;i++)
               {
                   files_to_send += rcvmsg[i];
               }
               istringstream file_share_stream(files_to_send);
                while(file_share_stream){
                     string  file ;
                     file_share_stream >> file ;
                     for(int i = 0 ; i < files_owned.size() ; i++){
                            if(files_owned[i] == file) {
                                send_file(new_fd,path + file,file);
                            }
                     }
                }

            }
            
            //



            close(new_fd);
            exit(0);
        }
        close(new_fd);

    }
}



int main(int argc, char *argv[])
{
    int client_id,port,unique_id,num_neighbor,num_files_needed ;
    vector<Neighbour> neighbors ;
    vector<pair<string,vector<Neighbour>>> files_needed ;
    vector<pair<string,int>> files_found ;
    string line;
    ifstream fin;
    fin.open(argv[1]);

   if(!fin.is_open()) {
      perror("Error open");
      exit(EXIT_FAILURE);
   }

    fin >> client_id >> port >> unique_id;
    fin >> num_neighbor ;
    for(int i = 0 ; i < num_neighbor ; i++){
        int a , b ;
        fin >> a >> b ;
        neighbors.push_back(Neighbour(a,b)) ;
    }
    fin >> num_files_needed ;
    for (int i=0;i<num_files_needed;i++)
    {
        string file_name ;
        fin >> file_name ;
        vector<Neighbour> n ;
        files_needed.push_back(make_pair(file_name,n));
    }

    fin.close();

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
    
    thread recving(incoming_threadfn,sock_fd,client_id,unique_id,files_owned,argv[2]);



    //to handle outgoing connections
    // send what files needed
    //wait for reply with list of files found
    //Print
    while (true)
    {
        bool allconnected = true;
        int count = 0 ;
        for (auto &u:neighbors)
        {   
            if (!u.isConnected())
            {   
                if(u.makeConnection()){
                    string s = u.rcvMessage();
                    istringstream details(s);
                    int cl_id,un_id;
                    details >> cl_id >> un_id;
                    pthread_mutex_lock(&print_mutex);
                    cout<<"Connected to " <<cl_id<< " with unique-ID " << un_id <<" on port "<<u.getPort()<<endl;
                    pthread_mutex_unlock(&print_mutex);
                    u.setUniqueID(un_id);
                    stringstream f ;
                    for(int i = 0 ; i < files_needed.size(); i++){
                        f << files_needed[i].first << " " ;
                    }
                    string f_send = f.str() ;
                    // pthread_mutex_lock(&print_mutex);
                    // cout<<"Sending a search request to "<<u.GetUniqueID()<<"\n"<<f_send<<endl; 
                    // pthread_mutex_unlock(&print_mutex);
                    u.sendMessage(f_send);
                            //u.setDepth1() ;   
                    s = u.rcvMessage(); //reply from files it asked for.
                    // pthread_mutex_lock(&print_mutex);
                    // cout<<"Received reply from "<<u.GetUniqueID()<<"\n"<<s<<endl;
                    // pthread_mutex_unlock(&print_mutex);
                    istringstream reply_stream(s);
                    string file_name ;
                    while(reply_stream){
                        reply_stream >> file_name ;
                        for(int i = 0 ; i < files_needed.size() ; i++){
                            if(files_needed[i].first == file_name){
                                files_needed[i].second.push_back(u) ;
                                break;
                            }
                        }
                    }
                    // pthread_mutex_lock(&print_mutex);
                    // cout<<s<<endl;
                    // pthread_mutex_unlock(&print_mutex);
                
                }
                allconnected = false;
                
                
            }

            if (allconnected)
            break;
        }
    }


    for(int i = 0 ; i < files_needed.size() ; i++){
            sort(files_needed[i].second.begin(),files_needed[i].second.end(),compareFunction);
            if(!files_needed[i].second.empty()) {
                //files_needed[i].second contains a copy of the neighbour. For all networking reasons, use the one in neighbors list
                for (auto &u:neighbors)
                {
                    if (u.getUniqueID() == files_needed[i].second[0].getUniqueID())
                    {
                        u.addFileToAsk(files_needed[i].first);
                        break;
                    }
                }
                cout << "Found " <<  files_needed[i].first <<" at "<<files_needed[i].second[0].getUniqueID() <<" with MD5 0 at depth 1\n" ;

            }
            else {
                cout << "Found " <<  files_needed[i].first <<" at 0 with MD5 0 at depth 0\n" ;
            }
            //cout<<files_needed[i].first<<" "<<files_needed[i].second.size()<<"\n";
        }

    //p1: filename + nofile     filename + yesfile + length_of_file + numpackets
    //filepackets: filename + serialno + numbytes + payload
    //ack: yes + serialno + filename
    for (auto &u:neighbors)
    {
        string req = u.getFileToAsk();
        u.sendMessage(req);
        for (int i=0;i<u.files_to_ask.size();i++)
        {
            string s = u.rcvMessage();
            istringstream details(s);
            string file_name;
            details >> file_name;
            string foundfile;
            details >> foundfile;
            if (foundfile=="nofile") continue;
            int length,numpackets;
            details >> length >> numpackets;
            string packet;
            ofstream fil;
            string filepath(argv[2]);
            filepath.append("/downloads/");
            filepath.append(file_name);
            fil.open(filepath.c_str(),ios::binary);
            for (int j=0;j<numpackets;j++)
            {
                packet = u.rcvMessage();
                istringstream packet_stream(packet);
                string filename_rc;
                int serialno,numbytes;
                packet_stream >> filename_rc >> serialno >> numbytes;
                char c;
                packet_stream.get(c);
                string payload="";
                for (int i=0;i<numbytes;i++){
                    packet_stream.get(c);
                    payload += c;
                }
                
                fil.write(payload.c_str(),numbytes);
                u.sendMessage("ack "+to_string(serialno)+" "+filename_rc);
            }
            fil.close();
        }

    }
    

    recving.join();

    

    close(sock_fd);
    return 0;
}
