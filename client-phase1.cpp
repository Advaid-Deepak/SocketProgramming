#include <bits/stdc++.h>
#include <dirent.h>

using namespace std;


int main(int argc, char *argv[])
{
    int client_id,port,unique_id,num_neighbor,num_files_needed ;
    vector<pair<int,int>> neighbors ;
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
                neighbors.push_back(make_pair(a,b)) ;
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
    return 0;
}