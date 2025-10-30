#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include<time.h>
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
//structure to store details of manager username and password
struct man_cred
{
	char username[6];
	char password[6];
};

struct admin_cred
{
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
// store transaction details
struct transaction
{
	int acc_no;
	char type[10];//deposit or withdraw
	double amt;
	time_t timestamp;

};
// structure to store loan details of a customer
struct loan
{
	int acc_no;
	double amt;
	char type[20];
	char status;
	int id;
	char emp[6];
};
//store feedback of customer
struct feedback{
	char username[6];
	char message[200];
};
//lock file
void lock_file(int fd, int lock_type) {
    struct flock lock;
    lock.l_type = lock_type;   // F_RDLCK (read) or F_WRLCK (write)
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;            // entire file
    lock.l_pid = getpid();

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl lock");
    }
}
// add feedback
int add_feedback(const char* username, const char *msg)
{
	struct feedback fb;
	int fd=open("feedback.txt",O_CREAT|O_APPEND|O_WRONLY,0744);
	if (fd < 0) {
        perror("open feedback.txt");
         return;
    }
    lock_file(fd,F_WRLCK);
    strncpy(fb.username, username, sizeof(fb.username));
    fb.username[sizeof(fb.username) - 1] = '\0';
    strncpy(fb.message, msg, sizeof(fb.message));
    fb.message[sizeof(fb.message) - 1] = '\0';
    if (write(fd, &fb, sizeof(fb)) != sizeof(fb)) {
        perror("write feedback");
        close(fd);
        return -1;
    }
   lock_file(fd,F_UNLCK);
    close(fd);
    return 1;

}
//Change password for customer
int change_password_cust(const char *username, const char *new)
{
	struct cust_cred c;
        int found=0;
	int fd=open("a.txt",O_RDWR);
	if(fd<0)
	{
		perror("Open a.txt");
		return -1;
	}
        lock_file(fd,F_WRLCK);
	//search for username
	while(read(fd,&c,sizeof(c))==sizeof(c))
	{
		if (strcmp(c.username, username) == 0)
		{
			found=1;
			strncpy(c.password,new,sizeof(c.password));
			c.password[sizeof(c.password)-1]='\0';
			lseek(fd,-sizeof(c),SEEK_CUR);
			write(fd,&c,sizeof(c));
			lock_file(fd,F_UNLCK);
			close(fd);
			return 1; // password changed successfully
		 
		}
	}
	
		       lock_file(fd,F_UNLCK);
			close(fd);
			return 0; // username incorrect
}

//change password for employee
int change_password_emp(const char *username, const char *new)
{
        struct emp_cred c;
        int found=0;
        int fd=open("e.txt",O_RDWR);
        if(fd<0)
        {
                perror("Open e.txt");
                return -1;
        }
        lock_file(fd,F_WRLCK);
        //search for username
        while(read(fd,&c,sizeof(c))==sizeof(c))
        {
                if (strcmp(c.username, username) == 0)
                {
                        found=1;
                        strncpy(c.password,new,sizeof(c.password));
                        c.password[sizeof(c.password)-1]='\0';
                        lseek(fd,-sizeof(c),SEEK_CUR);
                        write(fd,&c,sizeof(c));
                        lock_file(fd,F_UNLCK);
                        close(fd);
                        return 1; // password changed successfully

                }
        }

                       lock_file(fd,F_UNLCK);
                        close(fd);
                        return 0; // username incorrect
}


//apply for loan
int apply_loan(int acc_no,double amt,const char *type)
{
	struct account acc;
	struct loan ln={0};
	int found=0;
	int fd=open("account.txt",O_RDONLY);
	if(fd<0)
	{
		perror("open account");
		return -1;
	}
	while(read(fd,&acc,sizeof(acc))==sizeof(acc))
	{
		if(acc.acc_no=acc_no)
		{
			found=1;
			break;
		}
	}
	close(fd);
	if(!found)
		return 0; // account not found
	//prepare loan record
	int fd_l=open("loan.txt",O_CREAT|O_APPEND|O_RDWR,0744);
	ln.acc_no=acc_no;
	ln.amt=amt;
	strncpy(ln.type,type,sizeof(ln.type));
	ln.type[sizeof(ln.type)-1]='\0';
	ln.status='P';
        ln.emp[0]='\0';
	ln.id=rand()%900+100;
	write(fd_l,&ln,sizeof(ln));
	close(fd_l);
	return 1; // success
}

struct session sessions[10];  // for storing details about the logged in user for releasing the lock on the file when the user logs out
struct session sessions_emp[10];
struct session sessions_man[10];
struct session sessions_admin[10];
//Verify Employee Credentials

int verify_emp(struct emp_cred recv_acc,int nsd)
{
        int fd = open("e.txt", O_CREAT | O_RDWR, 0744);
            if(fd < 0) { perror("file open"); close(nsd); exit(1); }

            lseek(fd, 0, SEEK_SET);
            struct emp_cred temp;

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
                                   if(!sessions_emp[i].active)
                                   {
                                   strcpy(sessions_emp[i].username,recv_acc.username);
                                   sessions_emp[i].username[5]='\0';
                                  // sessions[i].nsd=nsd;
                                   sessions_emp[i].offset=offset;
                                   sessions_emp[i].active=1;
                                   sessions_emp[i].fd=fd;
                                   break;
                                   }
                           }
                       return found;

                   
		    }
                }
                 else
                 found=-1;//wrong password
	         break;
            }
             offset=offset+sizeof(temp);
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

//Logging out customer
void logout_cust(int fd,off_t offset)
{
	struct flock lock;
	lock.l_type=F_UNLCK;
	lock.l_whence=SEEK_SET;
	lock.l_start=offset;
	lock.l_len=sizeof(struct cust_cred);
	fcntl(fd,F_SETLK,&lock);
}

//logging out employee
void logout_emp(int fd, off_t offset)
{
	struct flock lock;
        lock.l_type=F_UNLCK;
        lock.l_whence=SEEK_SET;
        lock.l_start=offset;
        lock.l_len=sizeof(struct emp_cred);
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
  if(fd<0)
  {
	  perror("open");
	  return -1.0;
  }
  lock_file(fd,F_RDLCK);
  lseek(fd,0,SEEK_SET);
  while(read(fd,&acc,sizeof(acc))==sizeof(acc))
  {
	  if(account_no==acc.acc_no)
	  {  
		  //found
		  close(fd); 
		  return acc.balance;
	  }
  }
      lock_file(fd,F_UNLCK);
	  close(fd);
	  return 0.0;//not found
  
}

//deposit money
int deposit_money(double amt,int account_no)
{
printf("amount to be deposited is %lf\n",amt);
struct account acc;
struct transaction tx={0};
int fd=open("account.txt",O_RDWR,0744);
int fd_t = open("transactions.txt", O_CREAT | O_APPEND | O_WRONLY, 0744);
if(fd<0)
{
	perror("open");
	return -1;
}
lseek(fd,0,SEEK_SET);

lock_file(fd,F_WRLCK);
while(read(fd,&acc,sizeof(acc))==sizeof(acc))
{
	printf("Looking for acc_no=%d, got acc_no=%d from file\n", account_no, acc.acc_no);

	if(account_no==acc.acc_no)
	{

	    if(acc.active==0)
	    {
		   lock_file(fd,F_UNLCK);
		   close(fd);
		   close(fd_t);
		   return -3;
	    }
            acc.balance=acc.balance+amt;
	    lseek(fd,-sizeof(acc),SEEK_CUR);
	    write(fd,&acc,sizeof(acc));
            tx.acc_no=account_no;
	    strcpy(tx.type,"deposit");
	    //tx.type[10]='\0';
	    tx.amt=amt;
	    tx.timestamp=time(NULL);
	    write(fd_t,&tx,sizeof(tx));
	    lock_file(fd,F_UNLCK);
	    close(fd);
	    close(fd_t);
	    return 1;
	}
}
lock_file(fd,F_UNLCK);
close(fd);
close(fd_t);
return 0;

}
//withdraw money
int withdraw_money(double amt,int account_no)
{
struct transaction tx={0};
printf("amount to be deposited is %lf\n",amt);
struct account acc;
int fd=open("account.txt",O_RDWR,0744);
int fd_t = open("transactions.txt", O_CREAT | O_APPEND | O_WRONLY, 0744);
if(fd<0)
{
        perror("open");
        return -1;
}
lseek(fd,0,SEEK_SET);
lock_file(fd,F_WRLCK);
while(read(fd,&acc,sizeof(acc))==sizeof(acc))
{
        printf("Looking for acc_no=%d, got acc_no=%d from file\n", account_no, acc.acc_no);

        if(account_no==acc.acc_no)
        {
		 if(acc.active==0)
            {
                   lock_file(fd,F_UNLCK);
                   close(fd);
                   close(fd_t);
                   return -3;
            }

		if(acc.balance-amt<1000)
		{
                lock_file(fd,F_UNLCK);
		return -2;
		}
	
            else
	    {
            acc.balance=acc.balance-amt;
            lseek(fd,-sizeof(acc),SEEK_CUR);
            write(fd,&acc,sizeof(acc));
	    tx.acc_no=account_no;
            strcpy(tx.type,"withdraw");
            tx.type[10]='\0';
            tx.amt=amt;
            tx.timestamp=time(NULL);
            write(fd_t,&tx,sizeof(tx));
            lock_file(fd,F_UNLCK);
            close(fd);

            return 1;
	    }
        }
}
lock_file(fd,F_UNLCK);
close(fd);
return 0;

}
 
//Transfer Funds

int transfer(int source,int dest,double amt)
{

	struct account acc;
	struct transaction tx={0};
	struct transaction tx1={0};
	int d=0;
	int fd=open("account.txt",O_RDWR,0744);
	int fd_t = open("transactions.txt", O_CREAT | O_APPEND | O_WRONLY, 0744);
        if(fd<0)
{
        perror("open");
        return -1;
}
lseek(fd,0,SEEK_SET);
lock_file(fd,F_WRLCK);
//looking for source account
	while(read(fd,&acc,sizeof(acc))==sizeof(acc))
{
        printf("Looking for acc_no=%d, got acc_no=%d from file\n", source, acc.acc_no);

        if(source==acc.acc_no)
        {
		 if(acc.active==0)
            {
                   lock_file(fd,F_UNLCK);
                   close(fd);
                   close(fd_t);
                   return -3;
            }

                if(acc.balance-amt<1000)
		{ 
			lock_file(fd,F_UNLCK);
                        return -2;
		}
            else
            {
	    //deducting amt
            acc.balance=acc.balance-amt;
            lseek(fd,-sizeof(acc),SEEK_CUR);
            write(fd,&acc,sizeof(acc));
	    tx.acc_no=source;
            strcpy(tx.type,"withdraw");
            tx.type[10]='\0';
            tx.amt=amt;
            tx.timestamp=time(NULL);
            write(fd_t,&tx,sizeof(tx));

            //close(fd);
            //return 1;
	    d=1;
            }
        
}

}
if(d==0)
{
	lock_file(fd,F_UNLCK);
	return 0;
}
// looking for destination account

lseek(fd,0,SEEK_SET);
while(read(fd,&acc,sizeof(acc))==sizeof(acc))
{
        printf("Looking for acc_no=%d, got acc_no=%d from file\n", dest, acc.acc_no);

        if(dest==acc.acc_no)
        {
		 if(acc.active==0)
            {
                   lock_file(fd,F_UNLCK);
                   close(fd);
                   close(fd_t);
                   return -3;
            }


            acc.balance=acc.balance+amt;
            lseek(fd,-sizeof(acc),SEEK_CUR);
            write(fd,&acc,sizeof(acc));
	    tx1.acc_no=dest;
            strcpy(tx1.type,"deposit");
            tx1.type[10]='\0';
            tx1.amt=amt;
            tx1.timestamp=time(NULL);
            write(fd_t,&tx1,sizeof(tx));
            lock_file(fd,F_UNLCK);
            close(fd);
	    close(fd_t);
            return 1;
        }
}
lock_file(fd,F_UNLCK);
close(fd);
return 0;

}


// view transaction history

int view_transaction(int acc_no, struct transaction* arr, int max) {
    struct transaction tx;
    int fd = open("transactions.txt", O_RDONLY);
    if (fd < 0) {
        perror("open transactions");
        return -1;  // error
    }

    int count = 0;
    while (read(fd, &tx, sizeof(tx)) == sizeof(tx)) {
        if (tx.acc_no == acc_no) {
            if (count < max) {   
                arr[count++] = tx;
            }
        }
    }

    close(fd);
    return count;   
}

//verify manager credential
int verify_man(struct man_cred recv_acc,int nsd)
{
        int fd = open("m.txt", O_CREAT | O_RDWR, 0744);
            if(fd < 0) { perror("file open"); close(nsd); exit(1); }

            lseek(fd, 0, SEEK_SET);
            struct man_cred temp;

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
                                   if(!sessions_man[i].active)
                                   {
                                   strcpy(sessions_man[i].username,recv_acc.username);
                                   sessions_man[i].username[5]='\0';
                                  // sessions[i].nsd=nsd;
                                   sessions_man[i].offset=offset;
                                   sessions_man[i].active=1;
                                   sessions_man[i].fd=fd;
                                   break;
                                   }
                           }
                       return found;


                    }
                }
                 else
                 found=-1;//wrong password
                 break;
            }
             offset=offset+sizeof(temp);
            }
            return found;
            close(fd);
}
//Signup for manager 
void signup_man(struct man_cred recv_acc, int nsd)
{
                int fd= open("m.txt", O_CREAT | O_RDWR, 0744);
                lseek(fd, 0, SEEK_END);
                write(fd, &recv_acc, sizeof(recv_acc));
                write(nsd, "Sign Up successful\n", 19);
                close(fd);
}

//Activate/ deactivate customer account
int change_account_status(int acc_no,int new_status)
{
	struct account acc;
	int fd = open("account.txt", O_RDWR, 0744);
        if (fd < 0) {
        perror("open account file");
        return -1;
    }
	lock_file(fd,F_WRLCK);
	while (read(fd, &acc, sizeof(acc)) == sizeof(acc)) {
        if (acc.acc_no == acc_no) {
            acc.active = new_status;
            lseek(fd, -sizeof(acc), SEEK_CUR);
            write(fd, &acc, sizeof(acc));
	    close(fd);
	    return 1;// successfully changed
	}
	lock_file(fd,F_UNLCK);
	close(fd);
	return 0;// account not found
}
}
//Review Feedback
void view_feedback(int nsd)
{
  struct feedback fb;
  struct feedback all[100];
  int count=0;
  int fd=open("feedback.txt",O_RDONLY);
   if (fd < 0) {
        perror("open feedback.txt");
        return;
    }
while (read(fd, &fb, sizeof(fb)) == sizeof(fb)) {
        if (count < 100)
            all[count++] = fb;
    }
    close(fd);

    if (count > 0)
        write(nsd, all, count * sizeof(struct feedback));
    else {
        // send nothing if no feedback
        write(nsd, all, 0);
    }
}

//Change password for manager 
int change_password_man(const char *username, const char *new)
{
        struct man_cred c;
        int found=0;
        int fd=open("m.txt",O_RDWR);
        if(fd<0)
        {
                perror("Open m.txt");
                return -1;
        }
        lock_file(fd,F_WRLCK);
        //search for username
        while(read(fd,&c,sizeof(c))==sizeof(c))
        {
                if (strcmp(c.username, username) == 0)
                {
                        found=1;
                        strncpy(c.password,new,sizeof(c.password));
                        c.password[sizeof(c.password)-1]='\0';
                        lseek(fd,-sizeof(c),SEEK_CUR);
                        write(fd,&c,sizeof(c));
                        lock_file(fd,F_UNLCK);
                        close(fd);
                        return 1; // password changed successfully

                }
        }

                       lock_file(fd,F_UNLCK);
                        close(fd);
                        return 0; // username incorrect
}

//verify admin credential
int verify_admin(struct admin_cred recv_acc,int nsd)
{
        int fd = open("ad.txt", O_CREAT | O_RDWR, 0744);
            if(fd < 0) { perror("file open"); close(nsd); exit(1); }

            lseek(fd, 0, SEEK_SET);
            struct emp_cred temp;

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
                                   if(!sessions_admin[i].active)
                                   {
                                   strcpy(sessions_admin[i].username,recv_acc.username);
                                   sessions_admin[i].username[5]='\0';
                                  // sessions[i].nsd=nsd;
                                   sessions_admin[i].offset=offset;
                                   sessions_admin[i].active=1;
                                   sessions_admin[i].fd=fd;
                                   break;
                                   }
                           }
                       return found;

                   
		    }
                }
                 else
                 found=-1;//wrong password
	         break;
            }
             offset=offset+sizeof(temp);
            }
            return found;
            close(fd);


}

//signup for admin
void signup_admin(struct admin_cred recv_acc, int nsd)
{
                int fd= open("ad.txt", O_CREAT | O_RDWR, 0744);
                lseek(fd, 0, SEEK_END);
                write(fd, &recv_acc, sizeof(recv_acc));
                write(nsd, "Sign Up successful\n", 19);
                close(fd);
}
//logout admin
void logout_admin(int fd, off_t offset)
{
        struct flock lock;
        lock.l_type=F_UNLCK;
        lock.l_whence=SEEK_SET;
        lock.l_start=offset;
        lock.l_len=sizeof(struct admin_cred);
        fcntl(fd,F_SETLK,&lock);

}

int change_password_admin(const char *username, const char *new)
{
        struct admin_cred c;
        int found=0;
        int fd=open("ad.txt",O_RDWR);
        if(fd<0)
        {
                perror("Open ad.txt");
                return -1;
        }
        lock_file(fd,F_WRLCK);
        //search for username
        while(read(fd,&c,sizeof(c))==sizeof(c))
        {
                if (strcmp(c.username, username) == 0)
                {
                        found=1;
                        strncpy(c.password,new,sizeof(c.password));
                        c.password[sizeof(c.password)-1]='\0';
                        lseek(fd,-sizeof(c),SEEK_CUR);
                        write(fd,&c,sizeof(c));
                        lock_file(fd,F_UNLCK);
                        close(fd);
                        return 1; // password changed successfully

                }
        }

                       lock_file(fd,F_UNLCK);
                        close(fd);
                        return 0; // username incorrect
}

int main() {

     srand(time(NULL));
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
          // deposit money
	    if(a==2)
	    {
		    //double amt;
		    //int acc_no;
		     struct temp
               {
                       int acc_no;
                       double amt;
               }acc;

		    read(nsd,&acc,sizeof(acc));
		    //read(nsd,&acc_no,sizeof(acc_no));
		    int status;
		    
		    status=deposit_money(acc.amt,acc.acc_no);
		   

		    if(status==1)
			    write(nsd,"Amount deposited\n",strlen("Amount deposited\n"));
		    else if(status==0)
			    write(nsd,"No account found\n",strlen("No account found\n"));
		    else if(status==-3)
                          write(nsd,"Your account is deactivated\n",strlen("Your account is deactivated\n"));
		    else
			    write(nsd,"Error!",strlen("Error!"));

	    }
	    //Withdraw money
	    if(a==3)
	    {
	    struct temp
               {
                       int acc_no;
                       double amt;
               }acc;

                    read(nsd,&acc,sizeof(acc));
                    //read(nsd,&acc_no,sizeof(acc_no));
                    int status;

                    status=withdraw_money(acc.amt,acc.acc_no);


                    if(status==1)
                            write(nsd,"Amount withdrawn\n",strlen("Amount deposited\n"));
                    else if(status==0)
                            write(nsd,"No account found\n",strlen("No account found\n"));
		    else if(status==-2)
			    write(nsd,"Insufficient fund!cant withdraw\n",strlen("Insufficient fund!cant withdraw\n"));
		    else if(status==-3)
                          write(nsd,"Your account is deactivated\n",strlen("Your account is deactivated\n"));

                    else
                            write(nsd,"Error!",strlen("Error!"));

 
	    }
       
	    //Transfer funds
	    if(a==4)
	    {
		     struct temp
               {
                       int source;
                       int des;
                       double amt;
               }acc;

                  read(nsd,&acc,sizeof(acc));
		  int status=transfer(acc.source,acc.des,acc.amt);
		  if(status==1)
			  write(nsd,"Transfer successful\n",strlen("Transfer successful\n"));
		  else if(status==0)
			  write(nsd,"No account found\n",strlen("No account found\n"));
		  else if(status==-2)
			  write(nsd,"Insufficient fund\n",strlen("Insufficient fund\n"));
		  else if(status==-3)
                          write(nsd,"Your account is deactivated\n",strlen("Your account is deactivated\n"));

		  else
			  write(nsd,"Error\n",strlen("Error\n"));

	    }
//Apply for Loan
  if(a==5)
  {
     struct loan ln;
     read(nsd,&ln,sizeof(ln));
     int result=apply_loan(ln.acc_no,ln.amt,ln.type);
     if(result==1)
	     write(nsd,"Loan Application Submitted Successfully\n",sizeof("Loan Application Submitted Successfully\n"));
     else if(result==0)
	     write(nsd,"Account not found\n",sizeof("Account not found\n"));
     else
	     write(nsd,"Could not open file\n",sizeof("Could not open file\n"));
  }

//change password
  if(a==6)
  {
    struct cust_cred c;
    read(nsd,&c,sizeof(c));
    int result=change_password_cust(c.username, c.password);
    if(result==1)
	    write(nsd,"Password changed successfully\n",sizeof("Password changed successfully\n"));
    else if(result==0)
	    write(nsd,"Incorrect username\n",sizeof("Incorrect username\n"));
 else
	 write(nsd,"Error opening file\n",sizeof("Error opening file\n"
));
  }


// add feedback
  if(a==7)
  {
	  struct feedback fb;
    read(nsd, &fb, sizeof(fb));

    int result = add_feedback(fb.username, fb.message);

    if (result == 1)
        write(nsd, "Feedback submitted successfully\n", sizeof("Feedback submitted successfully\n"));
    else
        write(nsd, "Error submitting feedback\n", sizeof("Error submitting feedback\n"));

  }
// View Transaction History
	    if(a==8)
	    {
		    if (a == 8) {
    int acc_no;
    if (read(nsd, &acc_no, sizeof(acc_no)) <= 0) {
        perror("read acc_no");
        return;
    }

    struct transaction tx[100];
    int n = view_transaction(acc_no, tx, 100);

    if (n == -1) {
        write(nsd, "Error opening file\n", sizeof("Error opening file\n"));
    } else if (n == 0) {
        write(nsd, "No transaction found\n", sizeof("No transaction found\n"));
    } else {
        
        write(nsd, tx, n * sizeof(struct transaction));
    }
}
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
				offset=sessions[i].offset;
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
	    else if(found==-2)
            {
                    write(nsd,"Employee already Logged in\n",strlen("Employee already Logged in\n"));
            }

            else {
              write(nsd,"No such Employee exists\n",strlen("No such Employee exists\n"));
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
		    //Approve/Reject Loans
		    if(a==2)
		    {
	            struct assign_req {
                    int loan_id;
                     char status;
                     } req;

    if (read(nsd, &req, sizeof(req)) <= 0) {
        perror("read assign request");
        return;
    }

    int success = 0;
    int fd = open("loan.txt", O_RDWR);
    if (fd < 0) {
        perror("open for update");
        write(nsd, &success, sizeof(success));
        return -1;
    }
    struct loan ln;
    while (read(fd, &ln, sizeof(ln)) == sizeof(ln)) {
        if (ln.id == req.loan_id) {
                //strncpy(ln.emp, req.emp, sizeof(ln.emp) - 1);
               // ln.emp[sizeof(ln.emp) - 1] = '\0';
	        ln.status=req.status;
                lseek(fd, -sizeof(ln), SEEK_CUR);
                write(fd, &ln, sizeof(ln));
                success = 1;

            break;
        }
    }

    close(fd);

    // Send result back to client
    write(nsd, &success, sizeof(success));
     }
		    //View Assigned Loan Applications
		    if(a==3)
		    {
                           struct loan ln;
                      struct loan pending[100];  // buffer
                       int count = 0;
                       int fd = open("loan.txt", O_RDONLY);
                       if (fd < 0) {
                       perror("open loan.txt");
                       return -1;

                     }
                       while (read(fd, &ln, sizeof(ln)) == sizeof(ln)) {
                      if (ln.emp[0] !='\0') {
                      pending[count++] = ln;
        }
    }
    close(fd);
                  //sending pending loan to client
                    write(nsd, pending, count * sizeof(struct loan));

		    }
//View Customer Transaction
 if(a==4)
  {

                           
    int acc_no;
    if (read(nsd, &acc_no, sizeof(acc_no)) <= 0) {
        perror("read acc_no");
        return;
    }

    struct transaction tx[100];
    int n = view_transaction(acc_no, tx, 100);

    if (n == -1) {
        write(nsd, "Error opening file\n", sizeof("Error opening file\n"));
    } else if (n == 0) {
        write(nsd, "No transaction found\n", sizeof("No transaction found\n"));
    } else {

        write(nsd, tx, n * sizeof(struct transaction));
    }
   

  }
//Change Password
   if(a==5)
   {
	    struct emp_cred c;
    read(nsd,&c,sizeof(c));
    int result=change_password_emp(c.username, c.password);
    if(result==1)
            write(nsd,"Password changed successfully\n",sizeof("Password changed successfully\n"));
    else if(result==0)
            write(nsd,"Incorrect username\n",sizeof("Incorrect username\n"));
 else
         write(nsd,"Error opening file\n",sizeof("Error opening file\n"
));

   }
          //Logout Employee
		    if(a==6)
		    {
                     int fd;
                    off_t offset;
                for(int i=0;i<10;i++)
                {
                        if(strcmp(recv_acc.username,sessions_emp[i].username)==0)
                        {
                                fd=sessions_emp[i].fd;
                                offset=sessions_emp[i].offset;
                                sessions_emp[i].active=0;
                                break;
                        }
                }
                 logout_emp(fd,offset);
                 write(nsd,"Logged out successfully\n",strlen("Logged out successfully\n"));

		    }            

    
	    }		 
	    
    }

   if(ch==3)
{
  
	struct man_cred recv_acc = {0};


            if(read(nsd, &recv_acc, sizeof(recv_acc)) != sizeof(recv_acc)) {
                perror("read failed");
                close(nsd);
                exit(1);
            }

            // Ensure null-termination
            recv_acc.username[5] = '\0';
            recv_acc.password[5] = '\0';
            int found=verify_man(recv_acc,nsd);

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
                    write(nsd,"Employee already Logged in\n",strlen("Employee already Logged in\n"));
            }
                       else {
              write(nsd,"Manager not found\n",strlen("Manager not found\n"));
            }
      if(found==1)
            {

                    int a ;
                    read(nsd,&a,sizeof(a));
		    //Activate/ Deactivate customer account
		    if(a==1)
		    {
                         struct account_status_update {
                          int acc_no;
                          int new_status;
                        } req;

                  // Read the structure
                if (read(nsd, &req, sizeof(req)) <= 0) {
                 perror("read account_status_update");
                 return;
               }

              
             int s= change_account_status(req.acc_no, req.new_status);
	     if(s==0)
		     write(nsd,"Account not found\n",strlen("Account not found\n"));
	     else if(s==1)
		     write(nsd,"Account status changed \n",strlen("Account status changed \n"));
		    }
                    //assign loan
		    if(a==2)
		    {
                       struct loan ln;
                      struct loan pending[100];  // buffer
                       int count = 0;
		       int fd = open("loan.txt", O_RDONLY);
                       if (fd < 0) {
                       perror("open loan.txt");
                       return -1;

                     }
		       while (read(fd, &ln, sizeof(ln)) == sizeof(ln)) {
                      if (ln.emp[0] == '\0') {
                      pending[count++] = ln;
        }
    }
    close(fd);
                  //sending pending loan to client
                    write(nsd, pending, count * sizeof(struct loan));
                  // Receive manager’s selection (loan_id + emp)
                   struct assign_req {
                    int loan_id;
                     char emp[6];
                     } req;

    if (read(nsd, &req, sizeof(req)) <= 0) {
        perror("read assign request");
        return;
    }
    //  Update the selected loan
    int success = 0;
    fd = open("loan.txt", O_RDWR);
    if (fd < 0) {
        perror("open for update");
        write(nsd, &success, sizeof(success));
        return -1;
    }

    while (read(fd, &ln, sizeof(ln)) == sizeof(ln)) {
        if (ln.id == req.loan_id) {
                strncpy(ln.emp, req.emp, sizeof(ln.emp) - 1);
                ln.emp[sizeof(ln.emp) - 1] = '\0';
                lseek(fd, -sizeof(ln), SEEK_CUR);
                write(fd, &ln, sizeof(ln));
                success = 1;
           
            break;
        }
    }

    close(fd);

    // Send result back to client
    write(nsd, &success, sizeof(success));
}
      if(a==3)
{
	view_feedback(nsd);
}
 //change password
if(a==4)
{
	struct man_cred c;
    read(nsd,&c,sizeof(c));
    int result=change_password_man(c.username, c.password);
    if(result==1)
            write(nsd,"Password changed successfully\n",sizeof("Password changed successfully\n"));
    else if(result==0)
            write(nsd,"Incorrect username\n",sizeof("Incorrect username\n"));
 else
         write(nsd,"Error opening file\n",sizeof("Error opening file\n"
));

}
		    
		    //Logout Manager
		    if(a==5)
		    {
                          int fd;
                    off_t offset;
                for(int i=0;i<10;i++)
                {
                        if(strcmp(recv_acc.username,sessions_man[i].username)==0)
                        {
                                fd=sessions_man[i].fd;
                                offset=sessions_man[i].offset;
                                sessions_man[i].active=0;
                                break;
                        }
                }
                 logout_emp(fd,offset);
                 write(nsd,"Logged out successfully\n",strlen("Logged out successfully\n"));

  
		    }
	    }
}

if(ch==4)
{

	struct admin_cred recv_acc = {0};


            if(read(nsd, &recv_acc, sizeof(recv_acc)) != sizeof(recv_acc)) {
                perror("read failed");
                close(nsd);
                exit(1);
            }

            // Ensure null-termination
            recv_acc.username[5] = '\0';
            recv_acc.password[5] = '\0';
            int found=verify_admin(recv_acc,nsd);

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
                    write(nsd,"Employee already Logged in\n",strlen("Employee already Logged in\n"));
            }
                       else {
              signup_admin(recv_acc,nsd);
		       }
             if(found==1)
            {

                    int a ;
                    read(nsd,&a,sizeof(a));
		    //Add new Employee
		    if(a==1)
		    {
			    int c;
			    read(nsd,&c,sizeof(c));
			    //Add new Employee
			    if(c==1)
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
		        signup_emp(recv_acc,nsd);
			    }
                          //Add new manager 
			    else
			    {
			    struct man_cred recv_acc = {0};
                            if(read(nsd, &recv_acc, sizeof(recv_acc)) != sizeof(recv_acc)) {
                            perror("read failed");
                            close(nsd);
                             exit(1);
                            }

            // Ensure null-termination
                       recv_acc.username[5] = '\0';
                       recv_acc.password[5] = '\0'; 
		        signup_man(recv_acc,nsd);
			    }
		    }
		    //Change password
		    if(a==2)
		    {
                           struct admin_cred c;
    read(nsd,&c,sizeof(c));
    int result=change_password_admin(c.username, c.password);
    if(result==1)
            write(nsd,"Password changed successfully\n",sizeof("Password changed successfully\n"));
    else if(result==0)
            write(nsd,"Incorrect username\n",sizeof("Incorrect username\n"));
 else
         write(nsd,"Error opening file\n",sizeof("Error opening file\n"
));

		    }
		    //Logout Administrator
		    if(a==3)
		    {
                           int fd;
                    off_t offset;
                for(int i=0;i<10;i++)
                {
                        if(strcmp(recv_acc.username,sessions_admin[i].username)==0)
                        {
                                fd=sessions_admin[i].fd;
                                offset=sessions_admin[i].offset;
                                sessions_admin[i].active=0;
                                break;
                        }
                }
                 logout_admin(fd,offset);
                 write(nsd,"Logged out successfully\n",strlen("Logged out successfully\n"));

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
 
