#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

//session to store customer username and password
struct cust_cred {
    char username[6];
    char password[6];
};
// structure to store employee username and password
struct emp_cred{
        char username[6];
        char password[6];
};
//structure to store details of an active session
struct session
{
	//int nsd;
	int fd;
	off_t offset;
	char username[6];
	int active;
};
//Store Account details of customer
struct account
{
	int acc_no; //account number
	char username[6]; // same as login user name
	int active; /* 0 ->not active, 1-> active*/
        double balance;
};

struct session sessions[10];  // for storing details about the logged in user for releasing the lock on the file when the user logs out

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
	    off_t offset=0;

            while(read(fd, &temp, sizeof(temp)) == sizeof(temp)) {
                temp.username[5] = '\0';
                temp.password[5] = '\0';
                temp.username[strcspn(temp.username, "\0")] = '\0';
                temp.password[strcspn(temp.password, "\0")] = '\0';

                if(strcmp(temp.username, recv_acc.username) == 0)
		{
                 if( strcmp(temp.password, recv_acc.password) == 0) {
                    
		    struct flock lock;
		    lock.l_type=F_WRLCK;
		    lock.l_whence=SEEK_SET;
		    lock.l_start=offset;
		    lock.l_len=sizeof(temp);
		    lock.l_pid=getpid();

		    if(fcntl(fd,F_SETLK,&lock)==-1)
			    found=-2;  //already locked (user logged in)
		    else
		    {
			    found=1;  // login success +session locked
		           for(int i=0;i<10;i++)
			   {
				   if(!sessions[i].active)
				   {
				   strcpy(sessions[i].username,recv_acc.username);                                  
				   sessions[i].username[5]='\0';
				  // sessions[i].nsd=nsd;
				   sessions[i].offset=offset;
				   sessions[i].active=1;
				   sessions[i].fd=fd;
				   break;
				   }
			   }
		            
                 
                    return found;
		    }
                }
		 else 
	         found=-1; // wrong password
	         break;
            }
		offset=offset+sizeof(temp);
	    }
	    close(fd);
	    return found;
            


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

//Logging out
void logout_cust(int fd,off_t offset)
{
	struct flock lock;
	lock.l_type=F_UNLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=offset;
	lock.l_len=sizeof(struct cust_cred);
	fcntl(fd,F_SETLK,&lock);
}

//Add Customer
void add_customer(struct account acc,int nsd)
{
	//struct account acc1;
/*	printf("Enter account number\n");
	scanf("%d",&acc.acc_no);
	printf("Enter balance to deposit(Minimum balance=1000)\n");
	scanf("%lf",&acc.balance);
	printf("Enter username(same as login)\n");
	scanf("%s",acc.username);
	acc.username[5]='\0';
	acc.active=1;
	*/
	read(nsd,&acc,sizeof(acc));
	int fd=open("account.txt",O_CREAT|O_RDWR,0744);
	lseek(fd,0,SEEK_END);
	write(fd,&acc,sizeof(acc));
	close(fd);

}

//View Balance
double view_balance(int account_no)
{  
  struct account acc;
  int fd=open("account.txt",O_RDWR,0744);
  lseek(fd,0,SEEK_SET);
  while(read(fd,&acc,sizeof(acc))==sizeof(acc))
  {
	  if(account_no==acc.acc_no)
		  return acc.balance;
	  else
		  return 0.0;
  }

}

// 
int main() {
    int sd, nsd, sz;
    struct sockaddr_in serv, cli;
    int found;
    int ch;
    struct cust_cred recv_acc = {0};

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0) { perror("socket"); exit(1); }

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(5057);

    if(bind(sd, (struct sockaddr *)&serv, sizeof(serv)) < 0) { perror("bind"); exit(1); }

    listen(sd, 5);
    printf("Server listening on port 5057...\n");


    while(1) {

	for(int i=0;i<10;i++)
	{
		sessions[i].active=0;
	}
        sz = sizeof(cli);
        nsd = accept(sd, (struct sockaddr *)&cli, &sz);
        if(nsd < 0) { perror("accept"); continue; }

        if(fork() == 0) { // child process
            close(sd);
	    
	   // int ch;
	    read(nsd,&ch,sizeof(ch));
	    //printf("Choice entered is %d",ch);
	    if(ch==1)
	    {
            
           // struct cust_cred recv_acc = {0};
            if(read(nsd, &recv_acc, sizeof(recv_acc)) != sizeof(recv_acc)) {
                perror("read failed");
                close(nsd);
                exit(1);
            }

            // Ensure null-termination
            recv_acc.username[5] = '\0';
            recv_acc.password[5] = '\0';
            found=verify_cust(recv_acc,nsd);

            if(found==1)
	    {
                write(nsd, "Login successful\n", 17);
	    }
	    else if(found==-1)
	    {
		    write(nsd, "Wrong password try again\n",strlen("Wrong password try again\n"));
	    }

	    else if(found==-2)
	    {          
		    write(nsd,"User already Logged in\n",strlen("User already Logged in\n"));                                                                               
	    }
            else {
              signup_cust(recv_acc,nsd);  
            }
	    }
	    

            if(found==1)
	    {
	    int a;
	    
         	    
	    read(nsd,&a,sizeof(a));
	    if(a==1)
	    {
		    //View Account Balance
		    double balance;
		    int account_no;
	            read(nsd,&account_no,sizeof(account_no));
		    balance=view_balance(account_no);
		    write(nsd,&balance,sizeof(balance));

	    }
	    if(a==9)
	    {
		    //Logout Customer
		    int fd;
		    off_t offset;
	        for(int i=0;i<10;i++)
		{
			if(strcmp(recv_acc.username,sessions[i].username)==0)
			{
				fd=sessions[i].fd;
				fd=sessions[i].offset;
				sessions[i].active=0;
				break;
			}
		}
		 logout_cust(fd,offset);
		 write(nsd,"Logged out successfully\n",strlen("Logged out successfully\n"));
		 
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
	    
	    if(found==1)
	    {
              
		    int a ;
		    
                 
		    read(nsd,&a,sizeof(a));
		    if(a==1)
		    {   //Add new Customer 
			 struct account acc;
			 add_customer(acc,nsd);
			 write(nsd,"New Customer Account Added\n",strlen("New Customer Account Added\n"));
		    }
		 
	    }
            }
         
            //close(fd);
            // close(nsd);
            exit(0);
        } else {
            close(nsd); // parent closes client socket
        }
	
    }

    close(sd);
    return 0;
}
 
