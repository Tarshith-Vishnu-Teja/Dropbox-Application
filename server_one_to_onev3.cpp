#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<iostream>
#include<string>
#include<stdio.h>
#include<cstring>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
//#include<string.h>
#include<vector>
#include<signal.h>
#include<fstream>
#include<dirent.h>

using namespace std;

//--------------------------------------GLOBAL VARIABLES-------------------------------------
struct fileWriteArgs{
    int childNumber;
    int sockfd;
};

pthread_mutex_t cerr_lock;
pthread_mutex_t data_lock;
pthread_mutex_t thread_remove_lock;
pthread_mutex_t finished_thread_lock;

vector<int> finished_thread;
vector<pthread_t*> shiftregister;
vector<pthread_t*>::iterator it;

const int parent_pid = getpid();
static int check = -1; // to check if the process is freed it doesn get freed again wen we do ctrl^c

//-----------------------------------------------------------------------------------------------

void thread(int signum);
void* fileWrite(void *fd);

void uppercase(int,int);
void save(int,int);
void get(int,int);
long int get_file_size(char* name);
void list(int,int);



int main(int argc, char *argv[])
{
signal(SIGINT,thread);
int sockfd,newsockfd,portno,n;
socklen_t clilen;
char buffer[255];
struct sockaddr_in serv_addr,cli_addr;


	if(argc < 2)
	{
		cerr << "No port provided" << endl;
		exit(1);
	}

sockfd = socket(AF_INET,SOCK_STREAM,0);

	if(sockfd < 0)
	{
		cerr << "Error opening socket"<<endl;
		exit(1);
	}

bzero((char*)&serv_addr,sizeof(serv_addr));
portno = atoi(argv[1]);

serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(portno);
serv_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		cerr << "Error on Binding" << endl;
		exit(1);
	}

pthread_t *newThread;
pthread_attr_t attr;
pthread_attr_init(&attr);
int mem = sizeof(pthread_t);

listen(sockfd,100);
clilen= sizeof(cli_addr);


int noOfChilds = 0;


 while(1){

	newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);


	pthread_mutex_lock(&data_lock); //data lock
        noOfChilds++; //childNumber

       	struct fileWriteArgs args;
        args.sockfd = newsockfd;  // socket-id of the client stored in the global stuct variable..
        args.childNumber = noOfChilds;
	
		if(newsockfd < 0)
		{
			cerr << "Error on Accept" << endl;
			exit(1);
		}

	newThread = (pthread_t *)malloc(mem); //allocated memory for the new thread....
	cout << "Value of memory assigned " << newThread <<" to Thread " << noOfChilds << endl;
	shiftregister.push_back(newThread);   //saved the memory assigned in shiftregister VECTOR
	int x = pthread_create(&newThread[0], &attr, &fileWrite, &args);

    }

   // pthread_attr_destroy(&attr);

return 0;
}


void* fileWrite(void *fd)
{
int n,size;
char buffer[10];
char *data;
struct fileWriteArgs *p = (struct fileWriteArgs*)fd;
int sockfd = p->sockfd;
int childNumber = p->childNumber;
pthread_mutex_unlock(&data_lock); // data lock released....

bzero(buffer,10);
n = read(sockfd,buffer,10);//read function parameter................
	if(n<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on reading function parameter in Thread : "<< childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);
		close(sockfd);
		exit(1);
	}

if(buffer[0] == 'u'){
cout<<"\n\n\t*************File Content Capitalized*********\n\n";
uppercase(sockfd,childNumber);
}


if(buffer[0] == 's')
save(sockfd,childNumber);

if(buffer[0] == 'g')
get(sockfd,childNumber);

if(buffer[0] == 'l')
   	list(sockfd, childNumber);

pthread_mutex_lock(&finished_thread_lock); // lock to put childNumber in finished_thread vector...
finished_thread.push_back(childNumber); // putting the childNumber in the finished_thread vector so that parent can free the memory...
pthread_mutex_unlock(&finished_thread_lock);
close(sockfd);
kill(parent_pid,SIGINT); // signalling parent that it has completed..(On receiving this interrupt parent only knows that some thread is terminated in handler we free the memory location in shiftregister vector based on the first entry in finished_process vector..)
}


void thread(int signum)
{
signal(SIGINT,SIG_IGN);
pthread_mutex_lock(&finished_thread_lock);
pthread_mutex_lock(&thread_remove_lock);
unsigned int i = finished_thread[0];
	if(check != i)
	{
		free(shiftregister[i-1]);
		cout << "Value of memory released " << shiftregister[i - 1] <<" for thread " << i << endl;
		finished_thread.erase(finished_thread.begin());
		check = i;
	}
pthread_mutex_unlock(&finished_thread_lock);
pthread_mutex_unlock(&thread_remove_lock);
signal(SIGINT,thread);
}

void list(int sockfd, int childNumber){


	 DIR *dir;
	 int i = 1;
     char buffer[20];
	  struct dirent *	ent;
	  if ((dir = opendir("/home/tarshith/CMPE207")) != NULL) {
		 while ((ent = readdir(dir)) != NULL) {
				 //printf ("%d. %s\n",i, ent->d_name);
				 strcpy(buffer, ent->d_name);
				 int n = write(sockfd,buffer,sizeof(buffer));	 
				 if(n<0)	{
		
						pthread_mutex_lock(&cerr_lock);
						cerr << "Error on writing DATA in Thread : "<< childNumber << endl;
						pthread_mutex_unlock(&cerr_lock);
	  					bzero(buffer, 20);
						return;
					}
		 }
	  }
	  bzero(buffer, 20);
	  strcpy(buffer, "ENDOFFILE");
	  write (sockfd,buffer,sizeof(buffer));

}


//-----------------------------------------------------uppercase function--------------------------------------------------//
void uppercase(int sockfd,int childNumber)
{

char buffer[10];
int size;
char *data;
////////////////////////read file size////////////////////////////////
bzero(buffer,10);
int n = read(sockfd,buffer,10);//read file size................
	if(n<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on reading in Thread : "<< childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);
		return;
	}

size = atoi(buffer);
cout << "Size for Thread " << childNumber << " is " << size << endl; 
bzero(buffer,10);
buffer[0] = 'o';
buffer[1] = 'K';
buffer[2] = '\0';
n = write(sockfd,buffer,5);//write status........................

//////////////////////////////////////////////////////////////////////////

/////////////////////////read the data////////////////////////////////////
data = new char[size];
n = read(sockfd,data,size);//read the data...........................
if(n<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on reading DATA in Thread : " << childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);
		delete []data;
		/*close(sockfd);
		exit(1);*/
		return;
	}


for (unsigned int i=0; i < size; i++)
        data[i] = toupper(data[i]);

///////////////////////////////////////////////////////////////////


//////////////////////////write the data//////////////////////////////////////	
n = write(sockfd,data,size);//write the uppercase data.......................
	if(n<0)
	{
		
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on writing DATA in Thread : "<< childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);
		delete []data;
		return;
	}
////////////////////////////////////////////////////////////////////////////
delete []data;
}
//-----------------------------------------------------uppercase function ends---------------------------------------------//

//-------------------------------------------------------save function------------------------------------------------------------//

void save(int sockfd, int childNumber)
{
char buffer[10];
char filename[15];
int size;
char *data;

///////////////////////////reading filename////////////////////////

bzero(filename,15);
int n = read(sockfd,filename,sizeof(filename));
write(sockfd,"\n\theya\n\n",10);
bzero(filename,15);

n = read(sockfd,filename,sizeof(filename));
if(n<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on reading file name in Thread : "<< childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);
		return;
	}



cout << "\n\t**************Filename to save for Thread " << childNumber << " is '" << filename <<"' ************\n\n"<< endl; 
bzero(buffer,10);
buffer[0] = 'o';
buffer[1] = 'K';
buffer[2] = '\0';
n = write(sockfd,buffer,5);
////////////////////////////////////////////////////////////////////////

///////////////////////reading file size/////////////////////////////////
bzero(buffer,10);
n = read(sockfd,buffer,10);
if(n<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on reading file size in Thread : "<< childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);
		return;
	}


size = atoi(buffer);
cout << "Size for Thread " << childNumber << " is " << size << endl; 
bzero(buffer,10);
buffer[0] = 'o';
buffer[1] = 'K';
buffer[2] = '\0';
n = write(sockfd,buffer,5);

////////////////////////////////////////////////////////////////////////


///////////////////reading file data////////////////////////////////
data = new char[size];
n = read(sockfd,data,size);
if(n<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on reading DATA in Thread : " << childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);
		delete []data;
		return;
	}

ofstream myfile (filename);
myfile.write (data,size);
printf("\n\n\t*********File %s Successfully Saved*********\n\n", filename);

delete []data;
myfile.close();

////////////////////////////////////////////////////////////////////////
}
//----------------------------------------------------------------save function ends-------------------------------------------------------------------//







//-----------------------------------------------------get function--------------------------------------------------//
void get(int sockfd,int childNumber)
{

char buffer[10];
char *data;
char filename[15];
char len[8];

///////////////////////////reading filename////////////////////////

bzero(filename,15);
int n = read(sockfd,filename,15);
write(sockfd, "hii",5);
bzero(filename,15);
n = read(sockfd,filename,15);
//cout<<"\t1. ";puts(filename);
if(n<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on reading file name in Thread : "<< childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);
		return;
	}



cout << "\n\t********Filename to get from server " << childNumber << " is '" << filename <<"' ***************" << endl; 
/*bzero(buffer,10);
buffer[0] = 'o';
buffer[1] = 'K';
buffer[2] = '\0';
n = write(sockfd,buffer,5);*/
////////////////////////////////////////////////////////////////////////


//////////////////////does file exist///////////////////////////////
string file;
int flag = 0;
file = filename;
DIR *dir;
struct dirent *ent;
if ((dir = opendir ("/home/tarshith/CMPE207")) != NULL) {
  /* print all the files and directories within directory */
  while ((ent = readdir (dir)) != NULL) {
    //printf ("%s\n", ent->d_name);
	if(file.compare(ent->d_name) == 0)
	{
		bzero(buffer,10);
		buffer[0] = 'o';
		buffer[1] = 'K';
		buffer[2] = '\0';
		flag = 1;
		n = write(sockfd,buffer,10);
		break;
	}
  }
	
  	closedir (dir);
	if(flag == 0)
	{
	bzero(buffer,10);
	buffer[0] = 'F';
	buffer[1] = '\0';
	n = write(sockfd,buffer,10);
	cout << "File " << filename << " does not exist on server." << endl;
	fflush(stdout);
	return;
	}
} else {
  	bzero(buffer,10);
	buffer[0] = 'F';
	buffer[1] = '\0';
	n = write(sockfd,buffer,10);
	cout << "File " << filename << " does not exist on server." << endl;
	fflush(stdout);
	return;
}

////////////////////////////////////////////////////////////////////


////////////////////////get file size and write it////////////////////////////////

bzero(len,8);
long int size = get_file_size(filename);
sprintf(len,"%ld",size);
cout << "\nFile size is : " << size <<"\n" <<endl;
n= write(sockfd,len,8);//send size of the file.....................
//check_write(i+1,n,socketDescriptor,"Error writing SIZE to SOCKET of client : ","null");
	if(n<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on writing file size to get in Thread : "<< childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);
		return;
	}


//////////////////////////////////////////////////////////////////////////


//////////////////////////write the data//////////////////////////////////////	

data = new char[size];
ifstream infile (filename);
infile.read(data,size);
n= write(sockfd,data,size);//send the data........................
//check_write(i+1,n,socketDescriptor,"Error writing DATA to SOCKET of client : ",buffer);
	if(n<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr << "Error on writing file data to get in Thread : "<< childNumber << endl;
		pthread_mutex_unlock(&cerr_lock);
		return;
	}
	
	cout<<"\n\t***********File Sent to Client*************\n"<<endl;
infile.close();

////////////////////////////////////////////////////////////////////////////
delete []data;
}
//-----------------------------------------------------get function ends---------------------------------------------//


long int get_file_size(char* name)
{
ifstream infile (name);
infile.seekg (0,infile.end);
long int size = infile.tellg();
infile.seekg (0);
infile.close();
return size;
}

