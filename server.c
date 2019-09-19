#include "common.h"

char* homepath = "/home/jason/Desktop/jikeshijian_network";
static void sig_int(int signo) {
    printf("\nreceived signal SIGINT\n");
    exit(0);
}

int main(int argc, char **argv) {
    chdir(homepath);//change home diretory
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (argc != 2) {
        error(1, 0, "usage: server <port>");
    }
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    int rt1 = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (rt1 < 0) {
        error(1, errno, "bind failed ");
    }

    int rt2 = listen(listenfd, LISTENQ);
    if (rt2 < 0) {
        error(1, errno, "listen failed ");
    }

    signal(SIGINT, sig_int);

    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while(1)
    {
    	if ((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len)) < 0) 
	{
        	error(1, errno, "bind failed ");
    	}
	char str[20];
	inet_ntop(AF_INET,&client_addr.sin_addr, str, sizeof(str));
	printf("client connect: %s\n", str);

    	
	char message[MAXLINE];

    	for (;;) 
	{
        	int n = read(connfd, message, MAXLINE);
        	if (n < 0) {
            		error(1, errno, "error read");
        	} 
		else if (n == 0) //client quit, server continue waiting client connecting
		{  
            		printf( "client closed \n");
			close(connfd);
			break;
        	}
        	message[n] = 0;
        	printf("received %d bytes: %s\n", n, message);
		
		
		if (strncmp(message, "pwd", 3) == 0) 
		{
			char* send_line;
			send_line = getcwd(NULL, 0);
			int write_nc = send(connfd, send_line, strlen(send_line), 0);
        		printf("send %d bytes: %s\n", write_nc, send_line);
        		if (write_nc < 0) 
			{
            			error(1, errno, "error write");
        		}
		}
		else if(strncmp(message, "cd", 2) == 0)
		{
			printf("path is: %s\n", message+3);
			if(strncmp(message+3, "~", 1) == 0)
			{
				int ret = chdir(homepath);
				if(ret < 0)
				{
					char* s = "please input right path\n";
					send(connfd, s, strlen(s), 0);
				}
			}
			else
			{
				char* path = message+3;
				printf("server change path:%s\n", path);
				int ret = chdir(path);
				if(ret < 0)
				{
					char* s = "please input right path\n";
					send(connfd, s, strlen(s), 0);
				}
			}
		}
		else if(strncmp(message, "ls", 2) == 0)
		{
			char buf[MAXLINE] = {0};
			char path[MAXLINE] = {0};
			struct dirent** ptr;
			char *dirname = getcwd(NULL, 0);
    			int num = scandir(dirname, &ptr, NULL, alphasort);
   
    			int i;
    			for(i=0; i<num; ++i)
		    	{
				char* name = ptr[i]->d_name;

				// 拼接文件的完整路径
				sprintf(path, "%s/%s", dirname, name);
				printf("path = %s ===================\n", path);
				struct stat st;
				stat(path, &st);

				// 如果是文件
				if(S_ISREG(st.st_mode))
				{
				    sprintf(buf+strlen(buf), "%s", name);
				}
				// 如果是目录
				else if(S_ISDIR(st.st_mode))
				{
				    sprintf(buf+strlen(buf), "%s/", name);
				}
				printf("send %d bytes: %s\n", strlen(buf), buf);
				send(connfd, buf, strlen(buf), 0);
				memset(buf, 0, sizeof(buf));
		    	}
		}
		else
		{
			int write_nc = send(connfd, message, strlen(message), 0);
        		printf("send %d bytes: %s\n", write_nc, message);
        		if (write_nc < 0) 
			{
            			error(1, errno, "error write");
        		}
		}
    	}
    }

}


