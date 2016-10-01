#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include<fstream>
#include<sys/wait.h>
#include<pthread.h>
#include <time.h>

pthread_mutex_t connection_lock;
pthread_mutex_t cerr_lock;
timespec diff(timespec start, timespec end);

void uppercase(int,char*);
void save(int,char*);
void get(int,char*);
void list(int,char*);

void check_write(int client,int n,int socketDescriptor,char* msg,char* buffer);
void check_read(int client,int n,int socketDescriptor,char* msg,char* buffer);
long int get_file_size(char* name);
void is_connection(int client,int connection);
void is_socket(int client,int connection);

using namespace std;

struct sockaddr_in serv_addr;

int main(int argc, char * argv[])
{
int status = 0;
int wpid;
/////////////////////parameter value////////////////////////////////////
char operation;
if(argv[3][0] != '-')
{
cout << "function  parameter format incorrect" <<endl;
exit(1);
}
else{
	switch(argv[3][1])
	{
		case 's':
			operation = argv[3][1];
			break;
		case 'u':
			operation = argv[3][1];
			break;
		case 'g':
			operation = argv[3][1];
			break;
		case 'l':
			operation = argv[3][1];
			break;
		default:
			cout << "No such option available" << endl;
			exit(1);
	}	
    }
//////////////////////////////////////////////////////////////////////
/*for(unsigned j=0;j<argc-4;j++)
	{
	cout << argv[j+4] << endl;
	}*/
	int portno,n,i,pid;
	
	struct hostent *server;
	char buffer[256];
	if(argc<3)
	{
		cerr<<"Error";
		exit(0);
	}
	pid = getpid();
	portno = atoi(argv[2]);

	server = gethostbyname(argv[1]);
		if(server== NULL)
		{
			cerr<<"Hostdoes not exist";
		}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = PF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port=htons(portno);	
	

	
for(i=0;i<argc-4;i++)
{
	int pid_temp = getpid();
	if(pid_temp == pid)
	{
		int pidch = fork();
		if(pidch == 0)
		{	
			switch(operation)
			{
				case 'u': uppercase(i,argv[i+4]);
						break;

				case 's': save(i,argv[i+4]);
						break;

				case 'g': get(i,argv[i+4]);
						break;
					
				case 'l': list(i,argv[i+4]);
						break;
			}
		}
	}	
}

for(i=0;i<argc-4;i++)
wpid = wait(&status);

	return 0;
}


timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec - start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} 
	else {
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec =  (end.tv_nsec - start.tv_nsec);
	}
	return temp;
}

/////////////////////////////////functions implemented/////////////////////////////////////////////////////////


void list(int i,char *name){

	timespec time1, time2;
	clock_gettime(CLOCK_REALTIME, &time1);
	char file_list[20];
	int serialNo = 1;
	
	pthread_mutex_lock(&connection_lock);
	int socketDescriptor=socket(PF_INET,SOCK_STREAM,0);
	is_socket(i+1,socketDescriptor);
	pthread_mutex_unlock(&connection_lock);
	

	int connection= connect(socketDescriptor,(struct sockaddr *) &serv_addr, sizeof(serv_addr));
	is_connection(i+1,connection);
	
	int w = write(socketDescriptor,"l",2);
	check_write(i+1,w,socketDescriptor,"Error writing Function parameter to SOCKET of client : ","null");
	
	bzero(file_list,20);
	
	cout<<"\nRequesting File List...";
	
	cout<<"\n\n The List of Files On Server Database are Listed Below: \n\n";
		
	while(read(socketDescriptor,file_list,sizeof(file_list))) {

			if(strcmp(file_list,"ENDOFFILE")==0) {
				break;
			}
			printf("\t%d. ",serialNo);
				puts(file_list);
				serialNo++;
				printf("\n");
	}
	bzero(file_list,20);
	
	clock_gettime(CLOCK_REALTIME, &time2);

	///pthread_mutex_lock(&cerr_lock);
	double t1 = (double)diff(time1,time2).tv_sec;
	double t2 = diff(time1,time2).tv_nsec*0.0000000001;
	//cout<<"Time taken for client "<< i + 1<< " is " << diff(time1,time2).tv_sec<<":"<<diff(time1,time2).tv_nsec<<endl;
	cout<<"Time taken for client "<< i + 1<< " is " << t1 + t2<<endl;
	//pthread_mutex_unlock(&cerr_lock);*0.0000000001
	
	close(socketDescriptor);
}

void uppercase(int i,char *name)
{
	timespec time1, time2;
	clock_gettime(CLOCK_REALTIME, &time1);
	char *buffer;
	char len[8];

	pthread_mutex_lock(&connection_lock);
	int socketDescriptor=socket(PF_INET,SOCK_STREAM,0);
	is_socket(i+1,socketDescriptor);
	pthread_mutex_unlock(&connection_lock);

	int connection= connect(socketDescriptor,(struct sockaddr *) &serv_addr, sizeof(serv_addr));
	is_connection(i+1,connection);

	cout<<"\nRequesting Capitalization...\n\n";

	int w = write(socketDescriptor,"u",2);
	check_write(i+1,w,socketDescriptor,"Error writing Function parameter to SOCKET of client : ","null");
			

	bzero(len,8);
	long int size = get_file_size(name);
	sprintf(len,"%ld",size);
	int n= write(socketDescriptor,len,8);
	check_write(i+1,n,socketDescriptor,"Error writing SIZE to SOCKET of client : ","null");


	bzero(len,8);
	n = read(socketDescriptor,len,8);
	check_read(i+1,n,socketDescriptor,"Error on reading the SIZE_WRITE status of client : ","null");

									
	buffer = new char[size];
	ifstream infile (name);
	infile.read(buffer,size);
	n= write(socketDescriptor,buffer,size);
	check_write(i+1,n,socketDescriptor,"Error writing DATA to SOCKET of client : ",buffer);

			
	bzero(buffer,size);
	n= read(socketDescriptor,buffer,size);
	check_read(i+1,n,socketDescriptor,"Error reading DATA from  SOCKET of client : ",buffer);

					
	infile.close();

	ofstream myfile (name);
	myfile.write (buffer,size);

	cout<<"\t****************File Content Capitalized************\n\n";
	clock_gettime(CLOCK_REALTIME, &time2);

	///pthread_mutex_lock(&cerr_lock);
	double t1 = (double)diff(time1,time2).tv_sec;
	double t2 = diff(time1,time2).tv_nsec*0.0000000001;
	//cout<<"Time taken for client "<< i + 1<< " is " << diff(time1,time2).tv_sec<<":"<<diff(time1,time2).tv_nsec<<endl;
	cout<<"Time taken for client "<< i + 1<< " is " << t1 + t2<<endl;
	//pthread_mutex_unlock(&cerr_lock);*0.0000000001

	myfile.close();
	delete []buffer;
	close(socketDescriptor);
}


void save(int i,char* name)
{
	timespec time1, time2;
	clock_gettime(CLOCK_REALTIME, &time1);
	char *buffer;
	char len[8];
	char filename[15];	
	//sprintf(filename,"%s",name);
	strcpy(filename, name);
	cout<<"\nSaving file: "<<filename<<"...\n";
	pthread_mutex_lock(&connection_lock);
	int socketDescriptor=socket(PF_INET,SOCK_STREAM,0);
	is_socket(i+1,socketDescriptor);
	pthread_mutex_unlock(&connection_lock);

	int connection= connect(socketDescriptor,(struct sockaddr *) &serv_addr, sizeof(serv_addr));
	is_connection(i+1,connection);

	int w = write(socketDescriptor,"s",2); //when connection is established every client sends the function parameter first......
	check_write(i+1,w,socketDescriptor,"Error writing Function parameter to SOCKET of client : ","null");

	int n= write(socketDescriptor,filename,15);//send the name of the file...............
	check_write(i+1,n,socketDescriptor,"Error writing filename to SOCKET of client : ","null");
	
	bzero(filename,15);
	read(socketDescriptor, filename, sizeof(filename));
	bzero(filename,15);
	
	strcpy(filename, name);		
	n= write(socketDescriptor,filename,15);//send the name of the file...............
	check_write(i+1,n,socketDescriptor,"Error writing filename to SOCKET of client : ","null");
	

	bzero(len,8);
	n = read(socketDescriptor,len,8);
	cout<<"Status from Server: ";
	puts(len);
	check_read(i+1,n,socketDescriptor,"Error on reading the filename status of client : ","null");
			
	bzero(len,8);
	long int size = get_file_size(name);
	sprintf(len,"%ld",size);
	n= write(socketDescriptor,len,8);//send size of the file.....................
	check_write(i+1,n,socketDescriptor,"Error writing SIZE to SOCKET of client : ","null");

	bzero(len,8);
	n = read(socketDescriptor,len,8);
	check_read(i+1,n,socketDescriptor,"Error on reading the SIZE_WRITE status of client : ","null");
									
	buffer = new char[size];
	ifstream infile (name);
	infile.read(buffer,size);
	n= write(socketDescriptor,buffer,size);//send the data........................
	check_write(i+1,n,socketDescriptor,"Error writing DATA to SOCKET of client : ",buffer);
	infile.close();

	cout<<"\n\t************File Successfully Saved on the Server***********\n"<<endl;
	
	clock_gettime(CLOCK_REALTIME, &time2);

	///pthread_mutex_lock(&cerr_lock);
	double t1 = (double)diff(time1,time2).tv_sec;
	double t2 = diff(time1,time2).tv_nsec*0.0000000001;
	//cout<<"Time taken for client "<< i + 1<< " is " << diff(time1,time2).tv_sec<<":"<<diff(time1,time2).tv_nsec<<endl;
	cout<<"Time taken for client "<< i + 1<< " is " << t1 + t2<<endl;
	//pthread_mutex_unlock(&cerr_lock);*0.0000000001

	delete []buffer;
	close(socketDescriptor);
}

void get(int i,char* name)
{
	timespec time1, time2;
	clock_gettime(CLOCK_REALTIME, &time1);
	char *buffer;
	char status[10];
	char len[8];
	char filename[15];	
	sprintf(filename,"%s",name);
	
		cout<<"\nRetreiving file: "<<filename<<"...\n";

	pthread_mutex_lock(&connection_lock);
	int socketDescriptor=socket(PF_INET,SOCK_STREAM,0);
	is_socket(i+1,socketDescriptor);
	pthread_mutex_unlock(&connection_lock);

	int connection= connect(socketDescriptor,(struct sockaddr *) &serv_addr, sizeof(serv_addr));
	is_connection(i+1,connection);

	int w = write(socketDescriptor,"g",2);
	check_write(i+1,w,socketDescriptor,"Error writing Function parameter to SOCKET of client : ","null");
	


	int n= write(socketDescriptor,filename,15);
	check_write(i+1,n,socketDescriptor,"Error writing File name to fetch to SOCKET of client : ","null");
	bzero(filename,15);
	read(socketDescriptor, filename, sizeof(filename));
	
		bzero(filename,15);
		strcpy(filename, name);
		n= write(socketDescriptor,filename,15);
	
	
	bzero(status,10);
	n = read(socketDescriptor,status,10);
	check_read(i+1,n,socketDescriptor,"Error on reading the availabilty of file status of client : ","null");
	
	if(status[0] == 'F')
	{
		check_write(i+1,-1,socketDescriptor,"File does not exist : ","null");
	}
	cout << "\nStatus from server : " << status << "\n"<<endl;

	bzero(len,8);
	n = read(socketDescriptor,len,8);
	check_read(i+1,n,socketDescriptor,"Error on reading the SIZE of file from server of client : ","null");

	long int size = atoi(len);
	buffer = new char[size];
	cout << "\t********File size from server : " << size <<" has been Downloadedok********\n"<<endl;
	bzero(buffer,size);
	n= read(socketDescriptor,buffer,size);
	check_read(i+1,n,socketDescriptor,"Error reading DATA from  SOCKET of client : ",buffer);

	ofstream myfile (name);
	myfile.write (buffer,size);

	clock_gettime(CLOCK_REALTIME, &time2);

	///pthread_mutex_lock(&cerr_lock);
	double t1 = (double)diff(time1,time2).tv_sec;
	double t2 = diff(time1,time2).tv_nsec*0.0000000001;
	//cout<<"Time taken for client "<< i + 1<< " is " << diff(time1,time2).tv_sec<<":"<<diff(time1,time2).tv_nsec<<endl;
	cout<<"Time taken for client "<< i + 1<< " is " << t1 + t2<<endl;
	//pthread_mutex_unlock(&cerr_lock);*0.0000000001

	myfile.close();
	delete []buffer;
	close(socketDescriptor);
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long int get_file_size(char* name)
{
ifstream infile (name);
infile.seekg (0,infile.end);
long int size = infile.tellg();
infile.seekg (0);
infile.close();
return size;
}


void check_write(int client,int n,int socketDescriptor,char* msg,char* buffer)
{
	if(n<0)
	{
	pthread_mutex_lock(&cerr_lock);
	cerr << msg << client << endl;
	pthread_mutex_unlock(&cerr_lock);
	close(socketDescriptor);
	pthread_mutex_unlock(&cerr_lock);
	if(buffer != "null")
		delete[] buffer;
	exit(1);
	}
}


void check_read(int client,int n,int socketDescriptor,char* msg,char* buffer)
{
	if(n<0)
	{
	pthread_mutex_lock(&cerr_lock);
	cerr << msg << client << endl;
	pthread_mutex_unlock(&cerr_lock);
	close(socketDescriptor);
	pthread_mutex_unlock(&cerr_lock);
	if(buffer != "null")
		delete[] buffer;
	exit(1);
	}
}



void is_connection(int client,int connection)
{
	if(connection<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr<<"Error Connecting : " << client << endl;
		pthread_mutex_unlock(&cerr_lock);
		exit(1);
	}
}



void is_socket(int client,int socketDescriptor)
{
	if(socketDescriptor<0)
	{
		pthread_mutex_lock(&cerr_lock);
		cerr<<"Error Opening the socket for client : " << client << endl;
		pthread_mutex_unlock(&cerr_lock);
		exit(1);
	}
}
