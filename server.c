#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>

#define MAXCLIENTS 1000//maximum number of clients that could be handled at once
#define MAXLINE  1024//buffer size used
#define TOTAL_LINES 15 //file lines intilisation

char * calculate_size(char *filename); //function to calculate the size of the file
char * file_type_predict(char *filename);//function which tells the type of the file
char *  header_generation(int flag,char *file_len,char *file_type,char *version_http,char connection[100]);//Generate header to send to client
char * predict_file_path(char *filename);//function to resolve file path
int client_handle(int cfd);//process the client request
void signal_handler(int);//handle kill signal
void parse(char buf_parser[1024]);//parse the user input
void parser();//parse the uset input


//structure to store the commands and its attributes
struct node{ 
        char comm[10];//command name
        char filename[100];//filename
        char post_data[1024];//data to be posted
	char version[100];//HTTP version
	char connection[100];//Connection type
};
struct node *com_fname[TOTAL_LINES];


int count=0,comm_count=0,sfd,port,timer_value;

char buf[MAXLINE],buf1[MAXLINE],file_type_class[100],temp[MAXLINE],header[MAXLINE],ROOT[100],html_type[100],htm_type[100],txt_type[100],png_type[100],gif_type[100],jpg_type[100],css_type[100],js_type[100],ico_type[100],root_path[1000],*timeout_value;



//Function which gets the file and give it to client
void get_process(char version[100],int cfd,char file_name[100],char connection[100]){
	char header_send[MAXLINE],*file_type,path[10000],*file_size,*header_temp,mesg[100];
	int bytes_read,fd;
	printf("\n\nBotained file path is %s in get process\n\n and version is %s\n",file_name,version);
	if ( strncmp( version, "HTTP/1.0", 8)!=0 && strncmp( version, "HTTP/1.1", 8)!=0 && strncmp( version, "HTTP/2.0", 8)!=0  &&  strncmp( version, "HTTP/0.9", 8)!=0){
		header_temp=header_generation(2,"55",html_type,version,connection);
	        strcpy(header_send,header_temp);
	        write (cfd,header_send,strlen(header_send));
		strcpy(mesg,"400 Bad Request Reason: Invalid HTTP-Version:");
		strncat(mesg,version,strlen(version));
		printf("message for ivalid http version:%s",mesg);
	        write(cfd,mesg,strlen(mesg));
		write(cfd,"\n\n",strlen("\n\n"));
		return ;
	}
	else{
		if ( strncmp(file_name, "/\0", 2)==0 )
			file_name = "/index.html";        
		file_type=file_type_predict(file_name);
		if(strcmp(file_type,"unknown")==0){
			header_temp=header_generation(5,"50",html_type,version,connection);
			strcpy(header_send,header_temp);
		       	write (cfd,header_send,strlen(header_send));
			strcpy(mesg,"501 Not Implemented\n\n");
			write(cfd,mesg,strlen(mesg));
			write(cfd,"\n\n",strlen("\n\n"));
			return ;
		}
		else{
			strcpy(path,predict_file_path(file_name));
			printf("\nPredicted file path is %s\n",path);
			printf("file: %s\n", path);
			if(strcmp(path,"notpresent")!=0){
				file_size=calculate_size(path);
				printf("\n\nObtained file size is %s\n\n",file_size);
				file_type=file_type_predict(path);
				printf("\n\nObtained file type is %s\n\n",file_type);
				if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
				{
					printf("\nversion_http is %s\n",version);
					header_temp=header_generation(1,file_size,file_type,version,connection);
					strcpy(header_send,header_temp);
					printf("\nHeder is \n\n%s\n",header_send);
		    			write (cfd,header_send,strlen(header_send));
					while ( (bytes_read=read(fd, buf1,MAXLINE))>0 ){
						write (cfd,buf1,bytes_read);
					}
					write(cfd,"\n\n",strlen("\n\n"));
					if(bytes_read==-1){
						perror("read API failed");
						header_temp=header_generation(6,"80",html_type,version,connection);
						strcpy(header_send,header_temp);
						write (cfd,header_send,strlen(header_send));
						strcpy(mesg,"500 Internal Server Error: cannot allocate memory");
						write(cfd,mesg,strlen(mesg));
						write(cfd,"\n\n",strlen("\n\n"));
						exit(0);
					}
				}
			}
			else{
				header_temp=header_generation(0,"50",html_type,version,connection);
				strcpy(header_send,header_temp);
				write (cfd,header_send,strlen(header_send));
				strcpy(mesg," 404 Not Found Reason URL does not exist\n\n");
				write(cfd,mesg,strlen(mesg));
				write(cfd,"\n\n",strlen("\n\n"));
				return ;
			}
		}
	}

}

//function which will write the given data to file and give it back to client
void post_process(char post_data[100],char path[1024],char version[100],int cfd,char connection[100]){
	char *file_type,ch,*new_path,header_send[MAXLINE],*header_temp,mesg[100],cwd[1024];
	FILE *fp1,*fp2;
	printf("\nIts post command\n");
	if (getcwd(cwd, sizeof(cwd)) != NULL)
       		fprintf(stdout, "Current working dir: %s\n", cwd);
   	else
       		perror("getcwd() error");
	strncat(cwd,"/www/temp1.html",strlen("/www/temp1.html"));
	printf("\n\nPath for the file is %s\n\n",cwd);
	printf("\n\nData to be posted is %s\n\n",post_data);
	if ( strncmp(path, "/\0", 2)==0 ){
        	strcpy(path,"/index.html");
		printf("\nsince no file name is specified we take index.html as default\n");
	}
	printf("\n\nReceived file path is %s\n\n",path);
	strcpy(path,predict_file_path(path));
        printf("\nPredicted file path is %s\n",path);
        if(strcmp(path,"notpresent")==0){
		printf("\nyou cannot ask to post in file that doesn't exists!!!\n");
		header_temp=header_generation(0,"50",html_type,version,connection);
		strcpy(header_send,header_temp);
		write (cfd,header_send,strlen(header_send));
		strcpy(mesg," 404 Not Found Reason URL does not exist\n\n");
		write(cfd,mesg,strlen(mesg));
		write(cfd,"\n\n",strlen("\n\n"));
		return;
        }
        else{
		file_type=file_type_predict(path);
		if((fp1=fopen(path,"r"))==NULL)     perror("fopen");
		if((fp2=fopen(cwd,"a"))==NULL)      perror("fopen");
		fprintf(fp2,"%s",post_data);    
	       	while (1) {
			ch = fgetc(fp1);
		        if (ch == EOF) 
		               	break;
		        else
		               	putc(ch, fp2);
		}
		fclose(fp1);
		fclose(fp2);    
		/*if((fp1=fopen(path,"w"))==NULL)     perror("fopen");
		if((fp2=fopen("temp1","r"))==NULL)      perror("fopen");
		while (1) {
			ch = fgetc(fp2);
		        if (ch == EOF) 
		        	break;
		        else
		              	putc(ch, fp1);
		}
		fclose(fp1);
		fclose(fp2);*/
		strcpy(path,cwd);
		new_path=strrchr(path,'/');
		printf("\n\nNew path to be sent is %s\n\nand version to be sent %s\n\n",new_path,version);
		printf("\n\nThe file modified is %s\n\n",path);
		get_process(version,cfd,new_path,connection);
		remove(cwd);
		
	}
}



int main(int argc,char **argv){
	int cfd,childpid,client_len;
	struct sockaddr_in cliaddr, servaddr;
	for(int i=0;i<TOTAL_LINES;i++){
                com_fname[i]=(struct node *)malloc(sizeof(struct node));
        }	
	parser();
	if((sfd=socket(AF_INET,SOCK_STREAM, 0))==-1){
		perror("Scoket creation failed");
		exit(1);
	}
	//server address binding
	servaddr.sin_family = AF_INET;
 	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 	servaddr.sin_port = htons(port);	
	client_len=sizeof(cliaddr);
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (struct sockaddr *) &servaddr, sizeof(servaddr));
	if(bind (sfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0){
		perror("could not bind\n");
		exit(1);
	}

	if((listen(sfd,MAXCLIENTS))==-1){
		perror("Listening of server failed");
		exit(1);
	}	
	printf("\nListen is done and waiting for clients to accept\n");
	while(1){
  		cfd = accept(sfd, (struct sockaddr *) &cliaddr, &client_len);
		if(cfd==-1){
			perror("accept of client is failed");
		}
		if ( (childpid = fork ()) == 0 ) 
		{
			close(sfd);
			printf("\n\n am child and my sfd is %d\n\n",sfd);
			client_handle(cfd);
			while(timer_value){
				timer_value--;
			}
			close(cfd);
			printf("\nDone handling the client\n");
			exit(0);
 		}
		close(cfd);
	}
	signal(SIGINT,signal_handler);
	return 0;
}

//function which takes request from client and runs it
int client_handle(int cfd){
	char *command[3],file_type_rece[100],size[100],*line[TOTAL_LINES],line_cp[1024],*operation,*op_filename,version_http[100],*header_temp,header_send[MAXLINE],post_data[1024],path[10000],*com_version,mesg[100],connection[100],version_send[100];
	FILE *fd1;
	int n,comm_fd,new_file_size,i,conn_len=0;

	n = recv(cfd, buf,MAXLINE,0);
	printf("Received from client is %s\n",buf);
	if (n < 0) {
		perror("Read error");
                header_temp=header_generation(6,"80",html_type,"HTTP/1.1","\r\nConnection: keep-alive\r\n\r\n");
                strcpy(header_send,header_temp);
                write (cfd,header_send,strlen(header_send));
                strcpy(mesg,"500 Internal Server Error: cannot allocate memory");
                write(cfd,mesg,strlen(mesg));
		write(cfd,"\n\n",strlen("\n\n"));
	}
	if(access("temp",F_OK)==0){
		if(remove("temp")==0)	printf("\nFile deleted successful\n");
	}
	comm_fd=open("temp",O_RDWR | O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
	if(comm_fd==-1)	perror("open");
	write(comm_fd,buf,n);
        close(comm_fd);
	for(int i=0;i<TOTAL_LINES;i++){
                line[i]=(char *)malloc(1024*sizeof(char));
        }
        if((fd1=fopen("temp","r"))==0)        perror("fopen");
        for(int i=0;i<TOTAL_LINES;i++){
                fgets(line[i],1024,fd1);
        }
        fclose(fd1);
        for(i=0;i<TOTAL_LINES;i++){
		if((strstr(line[i],"GET"))!=NULL){
			printf("\n\ncount %d\n\n ",comm_count);
			printf("\nProcessing the line %s and length %d\n",line[i],strlen(line[i]));
			if((strlen(line[i])>=14)){
				strcpy(line_cp,line[i]);
				operation=strtok(line_cp, " ");
				strcpy(com_fname[comm_count]->comm,operation);
				op_filename=strtok (NULL, " \t");
				strcpy(com_fname[comm_count]->filename,op_filename);
				com_version=strtok (NULL, " \t\n");
				strcpy(com_fname[comm_count]->version,com_version);
			}
			printf("\n%s---->%s\n",com_fname[comm_count]->comm,com_fname[comm_count]->filename);
			for(i=i+1;i<TOTAL_LINES;i++){
				if((strstr(line[i],"Connection: keep-alive"))!=NULL){
					printf("\n\nConnection type is %s\n\n",line[i]);
					strcpy(com_fname[comm_count]->connection,"\r\nConnection: keep-alive\r\n\r\n");
					break;
				}
				else if(((strstr(line[i],"POST"))!=NULL)|((strstr(line[i],"GET"))!=NULL)|(strlen(line[i])==1)){
					i=i-1;
					strcpy(com_fname[comm_count]->connection,"\r\nConnection: Close\r\n\r\n");
					printf("\n\nSorry connection type not obtained\n\n");
					break;
				}
			}
			printf("\n%s---->%s\n",com_fname[comm_count]->comm,com_fname[comm_count]->filename);
			comm_count++;
			printf("\n\ncount %d\n\n ",comm_count);
		}
		else if((strstr(line[i],"POST"))!=NULL){
			if((strlen(line[i])>=14)){
				printf("\n\ncount %d\n\n ",comm_count);
				printf("\nProcessing the line %s\n",line[i]);
				strcpy(line_cp,line[i]);
				operation=strtok(line_cp, " ");
				strcpy(com_fname[comm_count]->comm,operation);
				op_filename=strtok (NULL, " \t");
		                strcpy(com_fname[comm_count]->filename,op_filename);
				com_version=strtok (NULL, " \t\n");
		                strcpy(com_fname[comm_count]->version,com_version);
			}
			printf("\n%s---->%s\n",com_fname[comm_count]->comm,com_fname[comm_count]->filename);
			for(i=i+1;i<TOTAL_LINES;i++){
                                printf("\n\nCommand is POST and lets get data to be posted and @line %s\n\n",line[i]);
                                int len=strlen(line[i]);
                                if(len==1){
                                        printf("\nGot empty line and also no connection type\n");
					strcpy(com_fname[comm_count]->post_data,"<html><body><pre><h1>");
                        		strcat(com_fname[comm_count]->post_data,line[i+1]);
					for(i=i+2;i<TOTAL_LINES;i++){
						if(strlen(line[i])!=1)	strcat(com_fname[comm_count]->post_data,line[i]);
						else break;
					}
                        		strcat(com_fname[comm_count]->post_data,"</h1></pre>");
                                        printf("\n\nData to be posted is %s\n\n",com_fname[comm_count]->post_data);
					if(conn_len==0){
						printf("\n\nSorry connection type not got yet as length is %d\n\n",strlen(com_fname[comm_count]->connection));
						strcpy(com_fname[comm_count]->connection,"\r\nConnection: Close\r\n\r\n");	
					}
                                 	break;
                                }
				else{
					if((strstr(line[i],"Connection: keep-alive"))!=NULL){
						strcpy(com_fname[comm_count]->connection,"\r\nConnection: keep-alive\r\n\r\n");
                                        	printf("\n\nConnection type is %s\n\n",com_fname[comm_count]->connection);
						conn_len=strlen(com_fname[comm_count]->connection);
                                	}
                                	else if(((strstr(line[i],"POST"))!=NULL)|((strstr(line[i],"GET"))!=NULL)){
						printf("\n\nSorry connection type not obtained\n\n");
						if(strlen(com_fname[comm_count]->connection)<1)
							strcpy(com_fname[comm_count]->connection,"\r\nConnection: Close\r\n\r\n");
                                        	i=i-1;
                                        	break;
                                	}
				}	
                        } 
			comm_count++;
			printf("\n\ncount %d\n\n ",comm_count);
		}
        }
	printf("\nTotal number of commands to be completed is %d\n",comm_count);
	for(int k=0;k<comm_count;k++){
		printf("\n%s->%s->%s->%s\n",com_fname[k]->comm,com_fname[k]->filename,com_fname[k]->post_data,com_fname[k]->connection);
	}
	if(comm_count>1){
		printf("\nNeed to process %d pipelined commands\n",comm_count);
		int temp=comm_count;
		comm_count=0;
		while(temp>0){
			if(strcmp(com_fname[comm_count]->comm,"GET")==0){
				printf("\nprocessing the get command in pipeline and is %d in buffer\n",temp);
				strncpy(version_send,com_fname[comm_count]->version,strlen(com_fname[comm_count]->version));
				get_process(version_send,cfd,com_fname[comm_count]->filename,com_fname[comm_count]->connection);
			}
			else{
				printf("\nProcessing the post command in pipeline\n");
				strncpy(version_send,com_fname[comm_count]->version,strlen(com_fname[comm_count]->version));
				post_process(com_fname[comm_count]->post_data,com_fname[comm_count]->filename,version_send,cfd,com_fname[comm_count]->connection);
			}
			comm_count++;
			temp--;
		}
	}
	else{
		command[0] = com_fname[0]->comm;
		command[1] = com_fname[0]->filename;
		command[2] = com_fname[0]->version;
		strcpy(post_data,com_fname[0]->post_data);
		strcpy(connection,com_fname[0]->connection);
		strncpy(version_http,com_fname[0]->version,strlen(com_fname[0]->version));
		printf("\nReceived version is %s\n",version_http);
		if(((strcmp(command[0],"GET")==0)|(strcmp(command[0],"POST")==0))&&(strlen(command[1])>=1)){
			if ( strcmp(command[0], "GET")==0 )
			{
				get_process(version_http,cfd,command[1],connection);
			}
			else if(strcmp(command[0], "POST")==0){
				post_process(post_data,command[1],version_http,cfd,connection);
			}
			else{
				header_temp=header_generation(3,"60",html_type,version_http,connection);
				strcpy(header_send,header_temp);
				write (cfd,header_send,strlen(header_send));
				strcpy(mesg," 400 Bad Request Reason: Invalid Method :\n\n");
				write(cfd,mesg,strlen(mesg));
				write(cfd,"\n\n",strlen("\n\n"));
			}
		}
		else{
			printf("Invalid URL generation");
		        header_temp=header_generation(4,"38",html_type,version_http,connection);
		        strcpy(header_send,header_temp);
		        write (cfd,header_send,strlen(header_send));
		        strcpy(mesg,"400 Bad Request Reason: Invalid URL:\n\n");
		        write(cfd,mesg,strlen(mesg));
			write(cfd,"\n\n",strlen("\n\n"));
			return 0;
		}
	}
}
//function to calculate the file size
char * calculate_size(char filename[100]){
	size_t file_size;
	FILE *fp;
	if((fp=fopen(filename,"r"))==NULL)
		perror("Opening the file");
	fseek(fp,0,SEEK_END);
	file_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	fclose(fp);
	sprintf(temp,"%d",file_size);
	return temp;
}
//funciton which predicts the filename type
char * file_type_predict(char * filename){
	char filename_copy[100],*file_type;
	strcpy(filename_copy,filename);
	file_type = strrchr(filename, '.');
	printf("\nFile type is %s\n",file_type);
	if(!(file_type))
		strcpy(file_type_class,"unknown");
	else if(strcmp(file_type,".html")==0){
		strcpy(file_type_class,html_type);
	}
	else if(strcmp(file_type,".htm")==0){
		strcpy(file_type_class,htm_type);
	}
	else if(strcmp(file_type,".txt")==0)
		strcpy(file_type_class,txt_type);
	else if(strcmp(file_type,".gif")==0)
		strcpy(file_type_class,gif_type);
	else if(strcmp(file_type,".jpg")==0)
		strcpy(file_type_class,jpg_type);
	else if(strcmp(file_type,".png")==0)
		strcpy(file_type_class,png_type);
	else if(strcmp(file_type,".css")==0)
		strcpy(file_type_class,css_type);
	else if(strcmp(file_type,".js")==0)
		strcpy(file_type_class,js_type);
	else if(strcmp(file_type,".ico")==0)
		strcpy(file_type_class,ico_type);
	else
		strcpy(file_type_class,"unknown");
	return file_type_class;
}
//generate the required header to post data into clinet browser
char * header_generation(int flag,char *file_len,char *file_type,char version_http[100],char connection[100]){
	printf("version received for header generation is %s",version_http);
	char version[100];
	if(strncmp(version_http,"HTTP/1.0",8)==0)	strcpy(version,"HTTP/1.0");
	else if(strncmp(version_http,"HTTP/1.1",8)==0)	strcpy(version,"HTTP/1.1");
	else if(strncmp(version_http,"HTTP/2.0",8)==0)	{printf("\n\n\nMatch found for version\n\n\n\n");strcpy(version,"HTTP/2.0");}
	else if(strncmp(version_http,"HTTP/0.9",8)==0)	strcpy(version,"HTTP/0.9");
	else strcpy(version,"HTTP/1.0");
	if(flag==1){	
		strcpy(header,version);
		printf("\nHeader so far\n%s\n",header);
		strcat(header, " 200 OK\r\nContent-Length:");
		printf("\nHeader so far\n%s\n",header);
        	strcat(header,file_len);
		printf("\nHeader so far\n%s\n",header);
        	strcat(header,"\r\nContent-Type:");
		printf("\nHeader so far\n%s\n",header);
        	strcat(header,file_type);
		printf("\nHeader so far\n%s\n",header);
	}
	else if(flag==0){
		strcpy(header,version);
		printf("\nFile not present header\n");
		strcat(header, " 404 Not Found Reason URL does not exist\r\nContent-Length:");
                strcat(header,file_len);
                strcat(header,"\r\nContent-Type:");
                strcat(header,file_type);
	}
	else if(flag==2){
		printf("\nversion not supported\n");
                strcat(header, "HTTP/1.1 400 Bad Request  Reason: Invalid HTTP-Version:\r\nContent-Length:");
                strcat(header,file_len);
                strcat(header,"\r\nContent-Type:");
                strcat(header,file_type);
	}
	else if(flag==3){
		strcpy(header,version);
                strcat(header, " 400 Bad Request Reason: Invalid Method :\r\nContent-Length:");
                strcat(header,file_len);
                strcat(header,"\r\nContent-Type:");
                strcat(header,file_type);
	}
	else if(flag==4){
		strcpy(header,version);
                strcat(header, " 400 Bad Request Reason: Invalid URL:\r\nContent-Length:");
                strcat(header,file_len);
                strcat(header,"\r\nContent-Type:");
                strcat(header,file_type);
	}
	else if(flag==5){
		strcpy(header,version);
                strcat(header, " 501 Not Implemented\r\nContent-Length:");
                strcat(header,file_len);
                strcat(header,"\r\nContent-Type:");
                strcat(header,file_type);

	}
	else if(flag==6){
		strcpy(header,version);
                printf("\nInternel server error\n");
                strcat(header, " 500 Internal Server Error: cannot allocate memory\r\nContent-Length:");
                strcat(header,file_len);
                strcat(header,"\r\nContent-Type:");
                strcat(header,file_type);

	}
	printf("\nIn header generation and connection type is \n%s\n",connection);
        strcat (header,connection);
	return header;
}


void signal_handler(int sig){

	printf("\nyou are exiting from program\n");
	signal(sig, SIG_IGN);
	close(sfd);	
	exit(0);
}

void parse(char buf_parser[1024]){
        char *str1,*str2,*str3,buf[MAXLINE];
	strcpy(buf,buf_parser);
        str1=strtok(buf_parser," ");
        str2=strtok(NULL,"NULL");
        if(strcmp(str1,"Listen")==0){
                port=atoi(str2);
                printf("\nport is %d\n",port);
        }
        else if(strcmp(str1,"DocumentRoot")==0){
                strcpy(ROOT,str2);
		str3=strtok(ROOT,"\"");
                strcpy(ROOT,str3);
        }
        else if(strcmp(str1,".html")==0){
                strcpy(html_type,"text/html");
        }
        else if(strcmp(str1,".htm")==0){
                strcpy(htm_type,"text/html");
        }
        else if(strcmp(str1,".txt")==0){
                strcpy(txt_type,"text/plain");
        }
        else if(strcmp(str1,".png")==0){
                strcpy(png_type,"image/png");
        }
	else if(strcmp(str1,".gif")==0){
                strcpy(gif_type,"image/gif");
        }
        else if(strcmp(str1,".jpg")==0){
                strcpy(jpg_type,"image/jpg");
        }
        else if(strcmp(str1,".css")==0){
                strcpy(css_type,"text/css");
        }
        else if(strcmp(str1,".js")==0){
                strcpy(js_type,"text/javascript");
        }
        else if(strcmp(str1,".ico")==0){
                strcpy(ico_type,"image/x-icon");
        }
	else if(strcmp(str1,"Keep-Alive")==0){
		strtok(buf," ");
		strtok(NULL," ");
		timeout_value=strtok(NULL,"NULL");
		timer_value=atoi(timeout_value);
	}

}

void parser(){
	FILE *fp;
        char buf_parser[1024];
        fp=fopen("ws.conf","r");
        while(fgets(buf_parser,1024,fp)){
                parse(buf_parser);
        }
	fclose(fp);
}

char* predict_file_path(char filename[100]){
	printf("\n\nSerching the file %s  in %s path\n\n",filename,ROOT);
	strcpy(root_path,ROOT);
	strcat(root_path,"/");
	strcat(root_path,filename);
	printf("\n\nsearching for %s file\n\n",root_path);
	if(access(root_path,R_OK)!=0){
		strcpy(root_path,ROOT);
		strcat(root_path,"/fancybox/");
		strcat(root_path,filename);
		printf("\n\nsearching for %s file\n\n",root_path);
	}
	if(access(root_path,R_OK)!=0){
		strcpy(root_path,ROOT);
                strcat(root_path,"/images/");
                strcat(root_path,filename);
	printf("\n\nsearching for %s file\n\n",root_path);
	}
	if(access(root_path,R_OK)!=0){
		strcpy(root_path,ROOT);
                strcat(root_path,"/css/");
                strcat(root_path,filename);		
	printf("\n\nsearching for %s file\n\n",root_path);
	}
	
	if(access(root_path,R_OK)!=0){
                strcpy(root_path,ROOT);
                strcat(root_path,"/files/");
		strcat(root_path,filename);
	printf("\n\nsearching for %s file\n\n",root_path);
        }
        if(access(root_path,R_OK)!=0){
                strcpy(root_path,ROOT);
                strcat(root_path,"/graphics/");
                strcat(root_path,filename);
	printf("\n\nsearching for %s file\n\n",root_path);
	}       
	if (access(root_path,R_OK)!=0){
		return "notpresent";
	}
	return root_path;
}	
