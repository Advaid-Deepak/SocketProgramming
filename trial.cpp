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

queue<pair<int,string>> buffer_in_to_out;
pthread_mutex_t buffer_in_to_out_mutex;

map<int,vector<string>> map_out_to_in ;
pthread_mutex_t map_out_to_in_mutex;

pthread_mutex_t depth2_1_mutex;


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

void send_thread(int new_fd,string files_to_send_dep2){
     while(true){
          if (send(new_fd, files_to_send_dep2.c_str(), files_to_send_dep2.length(), 0) == -1)
                    perror("send");
     }        
}

void incoming_depth2_1_thread(int new_fd,vector<string> files_owned){
        while(true){
            int numbytes_depth2;
            char rcvmsg_depth2[MAXDATASIZE] ;
            numbytes_depth2 = recv(new_fd, rcvmsg_depth2, MAXDATASIZE-1, 0);
            cout << "Files someone asking for their neighbour " << endl ;
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
            cout << res_depth2 << " Depth 2" << endl ;
            istringstream fil_dep2_search_stream(res_depth2);
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
            string files_to_send_dep2 = file_to_be_sent_dep2_stream.str() + " ";
            cout << "I have got this " << files_to_send_dep2 << endl ;
            //pthread_mutex_lock(&depth2_1_mutex);
            if (send(new_fd, files_to_send_dep2.c_str(), files_to_send_dep2.length(), 0) == -1)
                    perror("send");
            //thread sending(send_thread,new_fd,files_to_send_dep2) ;
        }
}

void outgoing_depth2_1_thread(vector<Neighbour> neighbors ){
        while (true){
        pthread_mutex_lock(&buffer_in_to_out_mutex);
        if (buffer_in_to_out.size() == 0)
        {
            pthread_mutex_unlock(&buffer_in_to_out_mutex);
            continue;
        }
        string curr = buffer_in_to_out.front().second + " ";
        int new_fd = buffer_in_to_out.front().first;
        buffer_in_to_out.pop();
        //pthread_mutex_unlock(&buffer_in_to_out_mutex);
        
        vector<string> responses ;
        int count = 0 ; 
       
        for (auto &u:neighbors)
        {
            cout << "My neighbour need this " << curr << endl ;
            u.sendMessage(curr);
        }
        //pthread_mutex_lock(&depth2_1_mutex) ;
        for (auto &u:neighbors)
        {
            string r = u.rcvMessage() ;
            cout << r << " Depth2 part 1 over" << endl ;
            count++ ;
            if(r.find_first_not_of (' ') != r.npos ){
                  string response = to_string(u.GetUniqueID()) + "    " + r;
                  cout << "Response " << response << endl ;
                  u.setDepth2_1() ;
                  // print the response after processing
                  responses.push_back(response) ;
            }
            
        
                
            
        }
        //pthread_mutex_unlock(&depth2_1_mutex) ;
        if(!responses.empty()){
              pthread_mutex_lock(&map_out_to_in_mutex);
              map_out_to_in[new_fd] = responses ;
              pthread_mutex_unlock(&map_out_to_in_mutex);
              //break ; 
        }
    }
}

void sub_incoming_threadfn(int new_fd, int clientid, int uniqueid,vector<string> files_owned, string s)
{
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
            cout << res + "  Result" << endl ;
            // pthread_mutex_lock(&print_mutex);
            // cout<<"===Recieved search request for\n"<<res<<"==="<<endl;
            // pthread_mutex_unlock(&print_mutex);
            istringstream fil_search_stream(res);
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
            cout << " Files I don't have "<< depth2_files << endl ;
            string files_to_send = file_to_be_sent_stream.str() + "  " ;

            //cout<<"Sending "<< files_to_send<<endl;
    
            if (send(new_fd, files_to_send.c_str(), files_to_send.length(), 0) == -1)
                    perror("send");
           // cout << "Hi_inc" << endl ;
            if (counter > 0)
            {
                pthread_mutex_lock(&buffer_in_to_out_mutex);
                buffer_in_to_out.push(make_pair(new_fd,depth2_files));
                pthread_mutex_unlock(&buffer_in_to_out_mutex);
            }
      


            //Recieve here  the file to be searched for at depth 2
            //process, and reply like in phase 2
            //cout<<"Reached here\n";
            
           //thread
           thread in_dep_2_1(incoming_depth2_1_thread,new_fd,files_owned) ;

           // Depth 2. 2
            while(true){
                pthread_mutex_lock(&map_out_to_in_mutex);
                if (!map_out_to_in.count(new_fd))
                {
                   pthread_mutex_unlock(&map_out_to_in_mutex);
                   continue;
                }
                int i = map_out_to_in.count(new_fd) ;
                vector<string> responses = map_out_to_in[new_fd] ;
                //process and reply with what files we got by searching our neighbors
                pthread_mutex_unlock(&map_out_to_in_mutex);
                stringstream files_found_depth2_stream ;
                //cout << "Size of responses " << responses.size() << " " << i << endl ;
                for(int i = 0 ; i < responses.size() ; i++){
                    files_found_depth2_stream << responses[i] << " || " ;
                }
                string files_found_depth2 = files_found_depth2_stream.str() + " " ;
                //cout << files_found_depth2 << " Sending for depth 2 part 2" << endl ;
                if (send(new_fd, files_found_depth2.c_str(), files_found_depth2.length(), 0) == -1)
                    perror("send");
            }

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



int main(int argc, char *argv[])
{
    int client_id,port,unique_id,num_neighbor,num_files_needed ;
    vector<Neighbour> neighbors ;
    vector<pair<string,vector<int>>> files_needed ;
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
    
    thread recving(incoming_threadfn,sock_fd,client_id,unique_id,files_owned);



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
                    string s = u.rcvMessage(); //Recieve unique_id of neighbor
                    istringstream details(s);
                    int cl_id,un_id;
                    details >> cl_id >> un_id;
                    //pthread_mutex_lock(&print_mutex);
                    cout<<"Connected to " <<cl_id<< " with unique-ID " << un_id <<" on port "<<u.getPort()<<endl;
                    //pthread_mutex_unlock(&print_mutex);
                    u.setUniqueID(un_id);

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
            //cout<< "Made all connections" << endl ;
            break ;
        }
    }

    for (auto &u:neighbors)
    {
        stringstream f ;
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
    
    bool found_at_depth1[files_needed.size()] ;
    for(int i = 0 ; i < files_needed.size() ; i++){
            sort(files_needed[i].second.begin(),files_needed[i].second.end());
            if(!files_needed[i].second.empty()) {
                found_at_depth1[i] = true ;
                cout << "Found " <<  files_needed[i].first <<" at "<<files_needed[i].second[0] <<" with MD5 0 at depth 1" << endl ;
            }
            else {
                found_at_depth1[i] = false ; 
                cout << "Found " <<  files_needed[i].first <<" at 0 with MD5 0 at depth 0" << endl ;
            }
            //cout<<files_needed[i].first<<" "<<files_needed[i].second.size()<<"\n";
    }

    //separate thread
    
    thread out_dep2_1(outgoing_depth2_1_thread,neighbors) ;

    //sleep(5);
    while(true){
      int count = 0 ;
      for(auto &u:neighbors){
          if(u.isConnected()){
               //pthread_mutex_lock(&depth2_1_mutex) ;
               if(u.checkDepth2_2()) continue ;
               string files_from_depth2 = u.rcvMessage() ;
               //pthread_mutex_unlock(&depth2_1_mutex) ;
               cout << files_from_depth2 << " Message recieved " << endl ;
               istringstream files_from_depth2_stream(files_from_depth2) ;
               int client_depth2 ;
               string s1 ;
               files_from_depth2_stream >> s1 ;
               if(s1.find_first_not_of (' ') == s1.npos || s1 == "") continue ;
               cout << "Converting to integer "<<s1 << endl ;
               try{
                   client_depth2 =  stoi(s1) ;
               }
               catch (const std::invalid_argument & e) {
                  continue ;
                }
                u.setDepth2_2() ;
               string files_data ;
               while(files_from_depth2_stream){
                    files_from_depth2_stream >> files_data ;
                    if(files_data == "||"){
                        if(files_from_depth2_stream.str().empty()) break ;
                        files_from_depth2_stream >> files_data ;
                        if(files_data == "||") break ;
                        cout <<"Converting to integer" <<files_data << endl ;
                        client_depth2 =  stoi(files_data) ;
                        continue ;
                    }
                    for(int i = 0 ; i < files_needed.size() ; i++){
                            if(files_needed[i].first == files_data){
                                cout << "Found one" << endl ;
                                files_needed[i].second.push_back(client_depth2) ;
                                break;
                            }
                    }
               }
               count++ ;
          }
      }
      if(count == neighbors.size()){
          break ;
      }
    }

    for(int i = 0 ; i < files_needed.size() ; i++){
        if(!found_at_depth1[i]){
            sort(files_needed[i].second.begin(),files_needed[i].second.end());
            if(!files_needed[i].second.empty()) {
                cout << "Found " <<  files_needed[i].first <<" at "<<files_needed[i].second[0] <<" with MD5 0 at depth 2\n" ;
            }
            else {
                cout << "Found " <<  files_needed[i].first <<" at 0 with MD5 0 at depth 0\n" ;
            }
        }
    }




    recving.join();

    

    close(sock_fd);
    return 0;
}
