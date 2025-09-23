#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

void display_custmenu()
{
	printf("Welcome\n");
	printf("1.View Account Balance\n");
	printf("2.Deposit Money\n");
	printf("3.Withdraw Money\n");
        printf("4. Transfer funds\n");
	printf("5. Apply for loan\n");
	printf("6. Change Password\n");
	printf("7. Add Feedback\n");
	printf("8. View Transaction History\n");
	printf("9. Logout\n");
	printf("10. Exit\n");
}
int main() {
    int sd;
    struct sockaddr_in serv;
    char buf[50] = {0};
    char buf_c[50]={0};
    int flag=0;
    int e=0;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0) { perror("socket"); exit(1); }

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv.sin_port = htons(5057);

    if(connect(sd, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        perror("connect"); exit(1);
    }
    int ch;
    // if Wrong password or a new user signup happened display the login menu again
    while(flag==0)
    {
    printf("Welcome to banking management system\n");
    printf("____________________________________\n");
    printf("1.Login as Customer\n");
    printf("2.Login as Employee\n");
    printf("3.Login as Manager\n");
    printf("4. Login as Administrator\n");
    printf("5. Exit\n");
    printf("Enter choice\n");
    scanf("%d",&ch);
    write(sd,&ch,sizeof(ch));

    //User Wants to Exit
    if(ch==5)
    {
	e=1;
	break;
    }

   if(ch==1)
   {
    struct cust_cred acc = {0};
    printf("Enter username: ");
    scanf("%5s", acc.username);
    printf("Enter password: ");
    scanf("%5s", acc.password);

    acc.username[5] = '\0';
    acc.password[5] = '\0';

    // send struct
    if(write(sd, &acc, sizeof(acc)) != sizeof(acc)) {
        perror("write failed");
        close(sd);
        exit(1);
    }

    // read server response
    int n = read(sd, buf, sizeof(buf)-1);
    if(n > 0) {
        buf[n] = '\0';
        printf("%s\n", buf);
    }
   }

   if(ch==2)
   {
    struct emp_cred acc = {0};
    printf("Enter employee  username: ");
    scanf("%5s", acc.username);
    printf("Enter employee  password: ");
    scanf("%5s", acc.password);

    acc.username[5] = '\0';
    acc.password[5] = '\0';

    // send struct
    if(write(sd, &acc, sizeof(acc)) != sizeof(acc)) {
        perror("write failed");
        close(sd);
        exit(1);
    }

    // read server response
    int n = read(sd, buf, sizeof(buf)-1);
    if(n > 0) {
        buf[n] = '\0';
        printf("%s\n", buf);
    }
   }

    if(strcmp(buf,"Login successful\n")==0) // login successful so dont need to show the login menu again 
    flag=1;
    }
    //customer logged in. Display Customer menu
   if(ch==1)
   { 
     display_custmenu();
     int a;
     printf("Enter your choice \n");
     fflush(stdout);
     scanf("%d",&a);
     write(sd,&a, sizeof(a)); // send choice to server

     if(a==9)
     {
	    
	    
	   // read server response
          
             int n = read(sd, buf_c, sizeof(buf)-1);
             if(n > 0) {
             buf_c[n] = '\0';
             printf("%s\n", buf_c);
         }
     }
   }
   // employee logged in. Display Employee menu
   if(ch==2)
	   printf("Employee menu coming soon..\n");
   // exit(5th option ) was chosen so close connection
   if(e==1)
    {
    close(sd);
    return 0;
    }
}

