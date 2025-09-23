#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

struct cust_cred {
    char username[6];
    char password[6];
};

struct emp_cred{
        char username[6];
        char password[6];
};

//Verify Employee Credentials

int verify_emp(struct emp_cred recv_acc,int nsd)
{
        int fd = open("e.txt", O_CREAT | O_RDWR, 0744);
            if(fd < 0) { perror("file open"); close(nsd); exit(1); }

            lseek(fd, 0, SEEK_SET);
            struct emp_cred temp;

            int found = 0;

            while(read(fd, &temp, sizeof(temp)) == sizeof(temp)) {
                temp.username[5] = '\0';
                temp.password[5] = '\0';
                temp.username[strcspn(temp.username, "\0")] = '\0';
                temp.password[strcspn(temp.password, "\0")] = '\0';

                if(strcmp(temp.username, recv_acc.username) == 0)
                {
                 if( strcmp(temp.password, recv_acc.password) == 0) {
                    found = 1;
                    break;
                }
                 else
                 found=-1;
            }
            }
            return found;
            close(fd);


}
//Verify Customer Credentials
int verify_cust(struct cust_cred recv_acc,int nsd)
{
	int fd = open("a.txt", O_CREAT | O_RDWR, 0744);
            if(fd < 0) { perror("file open"); close(nsd); exit(1); }

            lseek(fd, 0, SEEK_SET);
            struct cust_cred temp;
            
            int found = 0;

            while(read(fd, &temp, sizeof(temp)) == sizeof(temp)) {
                temp.username[5] = '\0';
                temp.password[5] = '\0';
                temp.username[strcspn(temp.username, "\0")] = '\0';
                temp.password[strcspn(temp.password, "\0")] = '\0';

                if(strcmp(temp.username, recv_acc.username) == 0)
		{
                 if( strcmp(temp.password, recv_acc.password) == 0) {
                    found = 1;
                    break;
                }
		 else 
	         found=-1;
            }
	    }
	    return found;
            close(fd);


}


//Create new employee account
void signup_emp(struct emp_cred recv_acc, int nsd)
{
	        int fd= open("e.txt", O_CREAT | O_RDWR, 0744);
                lseek(fd, 0, SEEK_END);
                write(fd, &recv_acc, sizeof(recv_acc));
                write(nsd, "Sign Up successful\n", 19);
		close(fd);
}

//create new customer account
void signup_cust(struct cust_cred recv_acc, int nsd)
{
                int fd= open("a.txt", O_CREAT | O_RDWR, 0744);
                lseek(fd, 0, SEEK_END);
                write(fd, &recv_acc, sizeof(recv_acc));
                write(nsd, "Sign Up successful\n", 19);
                close(fd);
}




int main() {
    int sd, nsd, sz;
    struct sockaddr_in serv, cli;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0) { perror("socket"); exit(1); }

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(5057);

    if(bind(sd, (struct sockaddr *)&serv, sizeof(serv)) < 0) { perror("bind"); exit(1); }

    listen(sd, 5);
    printf("Server listening on port 5057...\n");


    while(1) {
        sz = sizeof(cli);
        nsd = accept(sd, (struct sockaddr *)&cli, &sz);
        if(nsd < 0) { perror("accept"); continue; }

        if(fork() == 0) { // child process
            close(sd);
	    int ch;
	    read(nsd,&ch,sizeof(ch));
	    //printf("Choice entered is %d",ch);
	    if(ch==1)
	    {
            
            struct cust_cred recv_acc = {0};
            if(read(nsd, &recv_acc, sizeof(recv_acc)) != sizeof(recv_acc)) {
                perror("read failed");
                close(nsd);
                exit(1);
            }

            // Ensure null-termination
            recv_acc.username[5] = '\0';
            recv_acc.password[5] = '\0';
            int found=verify_cust(recv_acc,nsd);

            if(found==1)
	    {
                write(nsd, "Login successful\n", 17);
	    }
	    else if(found==-1)
	    {
		    write(nsd, "Wrong password try again\n",strlen("Wrong password try again\n"));
	    }
            else {
              signup_cust(recv_acc,nsd);  
            }
	    }
              
	    if(ch==2)
            {

            struct emp_cred recv_acc = {0};
            if(read(nsd, &recv_acc, sizeof(recv_acc)) != sizeof(recv_acc)) {
                perror("read failed");
                close(nsd);
                exit(1);
            }

            // Ensure null-termination
            recv_acc.username[5] = '\0';
            recv_acc.password[5] = '\0';
            int found=verify_emp(recv_acc,nsd);

            if(found==1)
            {
                write(nsd, "Login successful\n", 17);
            }
            else if(found==-1)
            {
                    write(nsd, "Wrong password try again\n",strlen("Wrong password try again\n"));
            }
            else {
              signup_emp(recv_acc,nsd);
            }
            }

            //close(fd);
            close(nsd);
            exit(0);
        } else {
            close(nsd); // parent closes client socket
        }
    }

    close(sd);
    return 0;
}
 
