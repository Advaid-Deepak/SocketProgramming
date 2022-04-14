#include <bits/stdc++.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;
#define MAXDATASIZE 1000
pthread_mutex_t print_mutex;
vector<pair<string,vector<int>>> files_needed ;

queue<tuple<string,string,string,string>> buffer_in_to_out;
pthread_mutex_t buffer_in_to_out_mutex;
pthread_mutex_t files_needed_mutex ;


map<int,int> uniqueid_portmap ;
pthread_mutex_t portmap_mutex;

pthread_mutex_t depth2_1_mutex;

pthread_mutex_t glob_neigh_d1_mutex;
vector<pair<int,int>> d1_neigh_data;      //uniqid , port

pthread_mutex_t glob_neigh_d2_mutex;
set<pair<int,int>> d2_neigh_data;




string file_path_search;

//vector<thread> threads_global ;

class Neighbour{
private:
    int clientid;
    int uniqueid;
    int listening_port;
    int sock_fd;
    bool connected;
    bool hassocket;
    bool depth1 ;
    bool depth2_1 ;
    bool depth2_2 ;
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
    void setDepth2_1() ;
    void setDepth2_2() ;
    bool checkDepth2_1() ;
    bool checkDepth2_2() ;
    string rcvMessage();
    string getMessage(int i) ;
    void setUniqueID(int id) ;
    int GetPort() ;
    int GetClientId();
    int GetUniqueID() ;
    int getPort();
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
    depth2_1 = false ;
    depth2_2 = false ;
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

bool Neighbour::checkDepth2_1(){
    return depth2_1 ;
}

void Neighbour::setDepth2_1(){
    depth2_1 = true ;
}

bool Neighbour::checkDepth2_2(){
    return depth2_2 ;
}

void Neighbour::setDepth2_2(){
    depth2_2 = true ;
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

int Neighbour::GetPort(){
    return listening_port ;
}

int Neighbour::GetClientId(){
    return clientid ;
}

bool check_message(string& message, string depth, string type){
     istringstream mssg(message) ;
     int i = 0 ;
     bool check = true ;
     string m = "" ;
     string part ;
     while(mssg){
         mssg >>  part ;
         if(i == 0){
             if (part != depth) return false ;
             else continue ;
         }
         if(i == 1){
             if(part != type) return  false ;
             else continue ;
         }
         m = m + " " + part ;
     }
     message = m ;
     return check ;
}



void outgoing_depth2_1_thread(vector<Neighbour> neighbors ){
        while (true){
        pthread_mutex_lock(&buffer_in_to_out_mutex);
        if (buffer_in_to_out.size() == 0)
        {
            pthread_mutex_unlock(&buffer_in_to_out_mutex);
            //sleep(1) ;
            continue;
        }
        //cout << "Entered here" << endl;
        string curr = get<3>(buffer_in_to_out.front()) + " ";
        string  port_send = get<2>(buffer_in_to_out.front());
        string  uniqueid_send = get<1>(buffer_in_to_out.front());
        string clientid_send = get<0>(buffer_in_to_out.front());
        buffer_in_to_out.pop();
        pthread_mutex_unlock(&buffer_in_to_out_mutex);
        
        vector<string> responses ;
        int count = 0 ; 
       
        for (auto &u:neighbors)
        {
            //cout << "My neighbour need this " << curr << endl ;
            u.sendMessage(clientid_send +" "+ uniqueid_send +" "+port_send + " " + curr);
        }
        //pthread_mutex_lock(&depth2_1_mutex) ;
    }
}

string conv_str(char arr[], int numbytes){
    string res = "";
    for (int i=0;i<numbytes;i++){
        res += arr[i];
    }
    return res;
}

void sub_incoming_threadfn(int new_fd, int clientid, int uniqueid,vector<string> files_owned, string s)
{
    
           if (send(new_fd, s.c_str(), s.length(), 0) == -1)
                perror("send");
            string port_incoming ;
            string clientid_incoming ;
            string uniqueid_incoming ;
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
            //cout << res + "  Result" << endl ;
            // pthread_mutex_lock(&print_mutex);
            // cout<<"===Recieved search request for\n"<<res<<"==="<<endl;
            // pthread_mutex_unlock(&print_mutex);
            istringstream fil_search_stream(res);
            fil_search_stream >> clientid_incoming  ;
            if(clientid_incoming == "depth2"){
                   string client_id_replied ;
                   string unique_id_replied ;
                   fil_search_stream >> client_id_replied ;
                   fil_search_stream >> unique_id_replied ;
                   string file_reply ;
                   while(fil_search_stream){
                       fil_search_stream >> file_reply ;
                       pthread_mutex_lock(&files_needed_mutex) ;
                       for(int i = 0 ; i < files_needed.size() ; i++){
                            if(files_needed[i].first == file_reply){
                                files_needed[i].second.push_back(stoi(unique_id_replied)) ;
                                //cout << "Found one" << endl ;
                                break;
                            }
                    }
                      pthread_mutex_unlock(&files_needed_mutex) ;
                   }
                   close(new_fd) ;
                   return ;
            }
            else if (clientid_incoming == "filereq"){
                string reqfilname;
                fil_search_stream >> reqfilname;
                cout<<"Recieved a request for "<<reqfilname<<endl;
                
                //check if reqfilename exists
                bool found = false ;
                for(int i = 0 ; i < files_owned.size() ; i++){
                    if(files_owned[i] == reqfilname){
                        found = true ;
                        break ;
                    }
                }
                if (!found){
                    string p = "No";
                    send(new_fd,p.c_str(),p.length(),0);
                    close(new_fd);
                    return;
                }
                string msg = "Yes";
                cout<<"Sending "<<msg<<endl;
                send(new_fd,msg.c_str(),msg.length(),0);
                int num = recv(new_fd,rcvmsg,MAXDATASIZE-1,0);
                string r = conv_str(rcvmsg,num);
                cout<<"Recieved "<<r<<endl;
                //Send filename with its length
                string fil = file_path_search + "/" + reqfilname;
                fstream fin(fil, ios::in | ios::binary);
                int length;
                fin.seekg(0, ios::end);
                length = fin.tellg();
                fin.seekg(0, ios::beg);
                char *buffer = new char[length];
                fin.read(buffer, length);
                fin.close();
                
                int maxpacketsize = 4*MAXDATASIZE/5;
                int numpackets = ceil(1.0*length/(1.0*maxpacketsize));
                msg = to_string(length) + " " + to_string(numpackets);
                send(new_fd,msg.c_str(),msg.length(),0);
                cout<<"Sent: "<<msg<<endl;
                //cout<<"Sent: "<<msg<<endl;
                recv(new_fd,rcvmsg,MAXDATASIZE-1,0);
                int index = 0;
                for (int i=0;i<numpackets;i++)
                {
                    //cout<<"Send Packet number " <<i<<endl;
                    string pack = "";
                    for (int j=0;j<maxpacketsize && index < length;j++,index++){
                        pack += buffer[index];
                    }
                    send(new_fd,pack.c_str(),pack.length(),0);
                    recv(new_fd,rcvmsg,MAXDATASIZE-1,0);
                }
                close(new_fd);
                cout<<"Sent "<<reqfilname<<endl;
                delete buffer;

                return;


            }else if (clientid_incoming == "port_d2"){

                string res = "";
                pthread_mutex_lock(&glob_neigh_d1_mutex);
                for (auto &u:d1_neigh_data){
                    res += to_string(u.first) + " " + to_string(u.second) + " ";
                }
                pthread_mutex_unlock(&glob_neigh_d1_mutex);
                //cout << res << " D2-info " << endl ;
                send(new_fd,res.c_str(),res.length(),0);
                close(new_fd);
                return;

            }
            else{
               fil_search_stream >> uniqueid_incoming ;
               fil_search_stream >> port_incoming  ;
            //    string dep2_neigh_uid , dep2_neigh_port  ;
            // while(true){
            //      fil_search_stream >> dep2_neigh_uid ;
            //      if(dep2_neigh_uid == "||")  break ;
            //      fil_search_stream >> dep2_neigh_port ;
            //      int uid_int ,prt_int ;
            //      uid_int = stoi(dep2_neigh_uid) ;
            //      prt_int = stoi(dep2_neigh_port) ;
            //      if (uid_int != uniqueid){
            //        pthread_mutex_lock(&glob_neigh_d2_mutex) ;
            //        d2_neigh_data.insert({uid_int,prt_int});
            //        pthread_mutex_unlock(&glob_neigh_d2_mutex) ;
            //        pthread_mutex_lock(&portmap_mutex);
            //        uniqueid_portmap[uid_int] =  prt_int;
            //        pthread_mutex_unlock(&portmap_mutex);
            //     }  
            // }
            stringstream file_to_be_sent_stream;
            stringstream file_for_depth2_stream;
            int counter = 0;
            while(fil_search_stream){
               string  file ;
               fil_search_stream >> file ;
               bool found = false;
               for(int i = 0 ; i < files_owned.size() ; i++){
                    if(files_owned[i] == file) {
                        file_to_be_sent_stream << file << " " ;
                        found = true;
                    }
               }
               if (!found){
                    counter++;
                    file_for_depth2_stream << file << " " ;
               }
            }
            string depth2_files = file_for_depth2_stream.str() ;
            //cout << " Files I don't have "<< depth2_files << endl ;
            string files_to_send = file_to_be_sent_stream.str() + "  " ;

            //cout<<"Sending "<< files_to_send<<endl;
    
            if (send(new_fd, files_to_send.c_str(), files_to_send.length(), 0) == -1)
                    perror("send");
           // cout << "Hi_inc" << endl ;
            if (counter > 0)
            {
                //cout << "Trying to lock" << endl ;
                pthread_mutex_lock(&buffer_in_to_out_mutex);
                //cout << "Added to buffer" << endl ;
                buffer_in_to_out.push(make_tuple(clientid_incoming,uniqueid_incoming,port_incoming ,depth2_files));
                pthread_mutex_unlock(&buffer_in_to_out_mutex);
            }
      


              //Recieve here  the file to be searched for at depth 2
              //process, and reply like in phase 2
              //cout<<"Reached here\n";
            
              //thread
              //cout << "Thread" << endl ;
            while(false){
            int numbytes_depth2;
            char rcvmsg_depth2[MAXDATASIZE] ;
            numbytes_depth2 = recv(new_fd, rcvmsg_depth2, MAXDATASIZE-1, 0);
            
            string res_depth2 = "";
            if(numbytes_depth2 == -1)
            {
               perror("Could not recieve");
            }
            else
            {
              for (int i=0;i<numbytes_depth2;i++)
               {
                   res_depth2 += rcvmsg_depth2[i];
               }
            }
            if(res_depth2.find_first_not_of (' ') == res_depth2.npos || res_depth2 == ""){
               continue ;
            }
            //cout << res_depth2 << " Depth 2" << endl ;
            //cout << "Files someone asking for their neighbour " << endl ;
            istringstream fil_dep2_search_stream(res_depth2);
            string new_client ;
            fil_dep2_search_stream >> new_client ;
            string new_uniqueid ;
            fil_dep2_search_stream >> new_uniqueid  ;
            string new_port ;
            fil_dep2_search_stream >> new_port ;
            pthread_mutex_lock(&portmap_mutex);
            uniqueid_portmap[stoi(new_uniqueid)] =  stoi(new_port);
            pthread_mutex_unlock(&portmap_mutex);
            stringstream file_to_be_sent_dep2_stream;
            while(fil_dep2_search_stream){
               string  file ;
               fil_dep2_search_stream >> file ;
               for(int i = 0 ; i < files_owned.size() ; i++){
                    if(files_owned[i] == file) {
                        file_to_be_sent_dep2_stream << file << " " ;
                        
                    }
               }
            }
            if(!file_to_be_sent_dep2_stream.str().empty()){
                string files_to_send_dep2 ="depth2 " + to_string(clientid) + " " + to_string(uniqueid) + " " + file_to_be_sent_dep2_stream.str() + " ";
                //cout << "I have got this " << files_to_send_dep2 << endl ;
                //pthread_mutex_lock(&depth2_1_mutex);
                Neighbour new_neighbour(stoi(new_client),stoi(new_port)) ;
                new_neighbour.setUniqueID(stoi(new_uniqueid)) ;
                new_neighbour.makeConnection();
                new_neighbour.sendMessage(files_to_send_dep2);
            }
            
            //thread sending(send_thread,new_fd,files_to_send_dep2) ;
        }
            }
            

           // Depth 2. 2

            close(new_fd);

}

void incoming_threadfn(int sock_fd, int clientid, int uniqueid,vector<string> files_owned)
{
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int new_fd;
    stringstream ss;
    ss << clientid <<" " << uniqueid;
    string s = ss.str() ;
    vector<thread> threads ;
    while(true)
    {
        
        sin_size = sizeof their_addr;
        new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size);
/*        if (!fork())
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
            while(fil_search_stream){
               string  file ;
               fil_search_stream >> file ;
               for(int i = 0 ; i < files_owned.size() ; i++){
                    if(files_owned[i] == file) {
                        file_to_be_sent_stream << file << " " ;
                        
                    }
               }
            }
            string files_to_send = file_to_be_sent_stream.str() ;
            if (send(new_fd, files_to_send.c_str(), files_to_send.length(), 0) == -1)
                    perror("send");
                                        
            close(new_fd);
            exit(0);
        }*/
        threads.emplace_back(thread(sub_incoming_threadfn,new_fd,clientid,uniqueid,files_owned,s));
        

    }
}

bool downloadFile(int uniqueid, string filname){
    Neighbour n(-1,uniqueid_portmap[uniqueid]);
    n.setUniqueID(uniqueid);
    n.makeConnection();
    string req = "filereq " + filname;
    string p = n.rcvMessage();
    n.sendMessage(req);
    p = n.rcvMessage();
    if (p.substr(0,2)=="No")
    {
        //cout<<"File not found"<<endl;
        return false;
    }
    //cout<<"File found!"<<endl;
    string dirname = file_path_search + '/'+"Downloaded";
    mkdir(dirname.c_str(),0777);
    fstream fout(dirname+'/'+filname,ios::out|ios::binary);
    n.sendMessage("OK");
    p = n.rcvMessage();
    //cout<<"Recieved "<<p<<endl;
    stringstream ss(p);
    int length, num_chunks;
    ss >> length >> num_chunks;
    n.sendMessage("OK");
    for (int i=0;i<num_chunks;i++)
    {
        //cout<<"RCV Packet num "<<i<<endl;
        string s = n.rcvMessage();
        for (int j=0;j<s.length();j++){
            fout.put(s[j]);
        }
        n.sendMessage("OK");
    }

    fout.close();
    return true;
}



int main(int argc, char *argv[])
{
    int client_id,port,unique_id,num_neighbor,num_files_needed ;
    vector<Neighbour> neighbors ;
    
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
        vector<int> n ;
        files_needed.push_back(make_pair(file_name,n));
    }

    fin.close();

    file_path_search = argv[2];

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
    sort(files_owned.begin(),files_owned.end());
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
    int iSetOption = 1;


    //=======
    //added this to stop that bind error on rerun. Unsure of keeping it
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR,(char*)&iSetOption,
        sizeof(iSetOption));
    //=======


    thread recving(incoming_threadfn,sock_fd,client_id,unique_id,files_owned);



    //to handle outgoing connections
    // send what files needed
    //wait for reply with list of files found
    //Print
    pthread_mutex_lock(&glob_neigh_d1_mutex);
    while (true)
    {
        bool allconnected = true;
        int count = 0 ;
        for (auto &u:neighbors)
        {   
            if (!u.isConnected())
            {   
                if(u.makeConnection()){
                    string s = u.rcvMessage(); //Recieve unique_id of neighbor
                    istringstream details(s);
                    int cl_id,un_id;
                    details >> cl_id >> un_id;
                    //pthread_mutex_lock(&print_mutex);
                    cout<<"Connected to " <<cl_id<< " with unique-ID " << un_id <<" on port "<<u.getPort()<<endl;
                    //pthread_mutex_unlock(&print_mutex);
                    u.setUniqueID(un_id);
                    d1_neigh_data.push_back({un_id,u.getPort()});


                    pthread_mutex_lock(&portmap_mutex);
                    uniqueid_portmap[un_id] = u.GetPort() ;
                    pthread_mutex_unlock(&portmap_mutex);

                    // pthread_mutex_lock(&print_mutex);
                    // cout<<s<<endl;
                    // pthread_mutex_unlock(&print_mutex);
                
                }
                allconnected = false;
                
                
            }
            /*
            else if(u.checkDepth1()){
                cout<<u.rcvMessage()<<endl;
                string file_given ;
                istringstream files_rec(u.getMessage(1)) ;
                int j = 0 ;
                while(files_rec){
                    string data ;
                    files_rec >> data ;
                    for(int i = 0 ; i < files_needed.size() ; i++){
                             if(data == files_needed[i].first){
                                 files_needed[i].second.push_back(u);
                             }
                    }
                    j++ ;
                }
                count++ ;while (true){
        pthread_mutex_lock(&buffer_in_to_out_mutex);
        if (buffer_in_to_out.size() == 0)
        {
            pthread_mutex_unlock(&buffer_in_to_out_mutex);
            continue;
        }
        string curr = buffer_in_to_out.front().second + " ";
        int new_fd = buffer_in_to_out.front().first;
        buffer_in_to_out.pop();
        pthread_mutex_unlock(&buffer_in_to_out_mutex);
        vector<string> responses ;
        int count = 0 ; 
        for (auto &u:neighbors)
        {
            
            u.sendMessage(curr);
            string r = u.rcvMessage() ;
            cout << r << " Depth2 part 1 over" << endl ;
            count++ ;
            if(r != " "){
                  string response = to_string(u.GetUniqueID()) + "    " + r;
                  cout << "Response " << response << endl ;
                  u.setDepth2_1() ;
                  // print the response after processing
                  responses.push_back(response) ;
            }
            
        
                
            
        }
        if(!responses.empty()){
              pthread_mutex_lock(&map_out_to_in_mutex);
              map_out_to_in[new_fd] = responses ;
              pthread_mutex_unlock(&map_out_to_in_mutex);
              break ; 
        }
    }
            }
            */
        }
        /*
        if(count == neighbors.size()){
            for(int i = 0 ; i < files_needed.size() ; i++){
                    sort(files_needed[i].second.begin(),files_needed[i].second.end(),compareFunction);
                    if(!files_needed[i].second.empty()) {
                       cout << "Found " <<  files_needed[i].first <<" at "<<files_needed[i].second[0].GetUniqueID() <<" with MD5 0 at depth 1\n" ;
                    }
                    else {
                       cout << "Found " <<  files_needed[i].first <<" at 0 with MD5 0 at depth 0\n" ;
                    }
               }
        }*/
        if (allconnected)
        {
            //cout<< "Made all connections " << d1_neigh_data.size() <<endl ;
            pthread_mutex_unlock(&glob_neigh_d1_mutex);
            break ;
        }
    }

    for (auto &u:d1_neigh_data){
        Neighbour p(-1,u.second);
        p.setUniqueID(u.first);
        p.makeConnection();
        p.rcvMessage();
        p.sendMessage("port_d2");
        string s = p.rcvMessage();
        //cout << s << "Message" << endl ;
        stringstream ss(s);
        while (ss){
            string uid,prt;
            ss >> uid >> prt;
            //cout << uid << " " << prt << endl ;
            int uid_int , prt_int ;
            try{
               uid_int = stoi(uid) ;
               prt_int = stoi(prt) ;
            }
            catch(exception &err){
                //cout << "Problem here ?" << endl ;
            }
            
            //cout << "Problem here ?" << endl ;
            if (uid_int != unique_id){
                d2_neigh_data.insert({uid_int,prt_int});
                pthread_mutex_lock(&portmap_mutex);
                uniqueid_portmap[uid_int] =  prt_int;
                pthread_mutex_unlock(&portmap_mutex);
            }
        }


    }


    for (auto &u:neighbors)
    {
        stringstream f ;
        f << to_string(client_id) << " " ;
        f << to_string(unique_id) << " " ;
        f << to_string(port) << " " ;
        // for (auto &u:d1_neigh_data){
        //     f << u.first << " " << u.second << " " ;
        // }
        // f << "|| " ;
        for(int i = 0 ; i < files_needed.size(); i++){
            f << files_needed[i].first << " " ;
        }
        string f_send = f.str() + " ";
        // pthread_mutex_lock(&print_mutex);
        // cout<<"Sending a search request to "<<u.GetUniqueID()<<"\n"<<f_send<<endl; 
        // pthread_mutex_unlock(&print_mutex);
        //cout<<"Sending "<<f_send<<" to "<<u.GetUniqueID()<<endl;
        u.sendMessage(f_send);
                //u.setDepth1() ; 
        //cout << "Blocked" << endl ; 
        string s = u.rcvMessage(); //reply from files it asked for.
        //u.sendMessage("ACK") ;
        //cout << "Hi_out" << endl ; 
        // pthread_mutex_lock(&print_mutex);
        // cout<<"Received reply from "<<u.GetUniqueID()<<"\n"<<s<<endl;
        // pthread_mutex_unlock(&print_mutex);
        istringstream reply_stream(s);
        string file_name ;
        while(reply_stream){
            reply_stream >> file_name ;
            for(int i = 0 ; i < files_needed.size() ; i++){
                if(files_needed[i].first == file_name){
                    files_needed[i].second.push_back(u.GetUniqueID()) ;
                    break;
                }
            }
        }
        //cout<<"Done with neighbour "<<u.GetUniqueID()<<endl;
    }

    
    
    vector<pair<int,string>> to_download_d1,to_download_d2;
    bool found_at_depth1[files_needed.size()] ;
    for(int i = 0 ; i < files_needed.size() ; i++){
            sort(files_needed[i].second.begin(),files_needed[i].second.end());
            if(!files_needed[i].second.empty()) {
                found_at_depth1[i] = true ;
                int uid = files_needed[i].second[0];
                string filname = files_needed[i].first;
                //cout << "Found " <<  files_needed[i].first <<" at "<<files_needed[i].second[0] <<" with MD5 0 at depth 1" << endl ;
                //cout<<"Going to request for file now!"<<endl;
                //downloadFile(uid,filname);
                to_download_d1.push_back({uid,filname});
                
            }
            else {
                found_at_depth1[i] = false ; 
                //cout << "Found " <<  files_needed[i].first <<" at 0 with MD5 0 at depth 0" << endl ;
            }
            //cout<<files_needed[i].first<<" "<<files_needed[i].second.size()<<"\n";
    }

    //separate thread

    //thread out_dep2_1(outgoing_depth2_1_thread,neighbors) ;

    
    //    sleep(4) ;

    // for(int i = 0 ; i < files_needed.size() ; i++){
    //     if(!found_at_depth1[i]){
    //         sort(files_needed[i].second.begin(),files_needed[i].second.end());
    //         if(!files_needed[i].second.empty()) {
    //             //cout << "Found " <<  files_needed[i].first <<" at "<<files_needed[i].second[0] <<" with MD5 0 at depth 2\n" ;
    //             int uid = files_needed[i].second[0];
    //             string filname = files_needed[i].first;
    //             to_download_d2.push_back({uid,filname});


    //         }
    //         else {
    //             cout << "Found " <<  files_needed[i].first <<" at 0 with MD5 0 at depth 0\n" ;
    //         }
    //     }
    // }
    /*
    For each file not found in depth 1:
    GO over the set of depth 2 neighbours from the end
    call downloadFile with the values of the current element from set
    */
   


    for (auto u:to_download_d1){
        downloadFile(u.first,u.second);
        cout << "Found " <<  u.second <<" at "<<u.first <<" with MD5 0 at depth 1" << endl ;
    }

    // for (auto u:to_download_d2){
    //     downloadFile(u.first,u.second);
    //     cout << "Found " <<  u.second <<" at "<<u.first <<" with MD5 0 at depth 2" << endl ;
    // }
    for(int i = 0 ; i < files_needed.size() ; i++){
        if(!found_at_depth1[i]){
            bool check = false ;
            set<pair<int,int>>::reverse_iterator rit;
            for (rit = d2_neigh_data.rbegin(); rit != d2_neigh_data.rend(); rit++){
                if(downloadFile(rit->first,files_needed[i].first)){
                    check = true ;
                    cout << "Found " << files_needed[i].first <<" at "<< rit->first <<" with MD5 0 at depth 2" << endl ;
                }
            }
            if(!check){
                cout << "Found " << files_needed[i].first <<" at "<< rit->first <<" with MD5 0 at depth 0" << endl ;
            } 
    
        }
    }



    recving.join();

    

    close(sock_fd);
    return 0;
}
