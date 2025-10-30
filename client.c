#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include<time.h>

struct transaction
{
        int acc_no;
        char type[10];//deposit or withdraw
        double amt;
        time_t timestamp;

};

struct account
{
        int acc_no; //account number
        char username[6]; // same as login user name
        int active; /* 0 ->not active, 1-> active*/
        double balance;
};

struct cust_cred {
    char username[6];
    char password[6];
};

struct emp_cred{
	char username[6];
	char password[6];
};
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

struct loan
{
        int acc_no;
        double amt;
        char type[20];
        char status;
	int id;
	char emp[6];
};

struct feedback{
        char username[6];
        char message[200];
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

void display_empmenu()
{
	printf("Welcome\n");
	printf("1.Add new customer\n");
	printf("2.Approve/Reject Loans\n");
	printf("3.View Assigned Loan applications\n");
	printf("4.View Customer Transactions\n");
	printf("5. Change Password\n");
	printf("6. Logout\n");
	printf("7. Exit\n");
}
void display_manmenu()
{
	printf("Welcome\n");
	printf("1. Activate/Deactivate Customer Account\n");
	printf("2.Assign Loan Application process to employee\n");
	printf("3.Review Customer Feedback\n");
	printf("4.Change password\n");
	printf("5.Logout\n");
	printf("6.Exit\n");
}

void display_adminmenu()
{
	printf("1.Add new Bank Employee\n");
//	printf("2.Modify Employee details\n");
//	printf("3.Manage user roles\n");
	printf("2.Change Password\n");
	printf("3.Logout\n");
	printf("4.Exit\n");

}
int main() {
    int sd;
    struct sockaddr_in serv;
    char buf[50] = {0};
    char buf_c[50]={0};
    char buf_e[50]={0};
    int flag_login=0;
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
    while(flag_login==0)
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

   if(ch==3)
   {
     struct man_cred acc = {0};
    printf("Enter manager  username: ");
    scanf("%5s", acc.username);
    printf("Enter manager  password: ");
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

   if(ch==4)
   {
     struct admin_cred acc = {0};
    printf("Enter administrator  username: ");
    scanf("%5s", acc.username);
    printf("Enter administrator  password: ");
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
    flag_login=1;
    }
    //customer logged in. Display Customer menu
   if(ch==1)
   { 
     int a=0;
     int flag_cust=0;
     while(flag_cust==0)
     {
     display_custmenu();
     
     printf("Enter your choice \n");
     fflush(stdout);
     scanf(" %d",&a);
     write(sd,&a, sizeof(a)); // send choice to server
     
//view account balance	 
       if(a==1)
      {
	      int account_no;
	      printf("Enter your account_no\n");
	      scanf("%d",&account_no);
	      write(sd,&account_no,sizeof(account_no)); // send account number
	      // read server response
	      double balance;
	      read(sd,&balance,sizeof(balance));
	      if(balance<0.0)
		      printf("Error reading account file\n");
	      else if(balance>0.000000)
		      printf("Your Account Balance is %f \n",balance);
	      else
		      printf("No Account found. Contact Bank \n");
      }

       //deposit money
       if(a==2)
       {
	       double amt;
	       int acc_no;
	       char buf[50];
	       printf("Enter amount to be deposited\n");
	       scanf("%lf",&amt);
	       printf("Enter your account number\n");
	       scanf("%d",&acc_no);
	       //sending in a structure otherwise sending individually was giving error
	       struct temp
	       {
		       int acc_no;
		       double amt;
	       }acc;
               acc.acc_no=acc_no;
	       acc.amt=amt;
	       //send amount to server
	       write(sd,&acc,sizeof(acc));
	       //Cwrite(sd,&amt,sizeof(acc_no));
	       int n = read(sd, buf, sizeof(buf)-1);
             if(n > 0) {
             buf[n] = '\0';
             printf("%s\n", buf);
	     }

       }

       //withdraw money
       if(a==3)
       {
	       double amt;
               int acc_no;
               char buf[50];
               printf("Enter amount to be withdrawn\n");
               scanf("%lf",&amt);
               printf("Enter your account number\n");
               scanf("%d",&acc_no);
               //sending in a structure otherwise sending individually was giving error
               struct temp
               {
                       int acc_no;
                       double amt;
               }acc;
               acc.acc_no=acc_no;
               acc.amt=amt;
               //send amount to server
               write(sd,&acc,sizeof(acc));
               //Cwrite(sd,&amt,sizeof(acc_no));
               int n = read(sd, buf, sizeof(buf)-1);
             if(n > 0) {
             buf[n] = '\0';
             printf("%s\n", buf);
             }

       }
// Transfer fund
       if(a==4)
       {
	       char buf[50];
	       struct temp
	       {
		       int source;
		       int des;
		       double amt;
	       }temp;
	       printf("Enter your account number \n");
	       scanf("%d",&temp.source);
	       printf("Enter the account number where you want to transfer fund\n");
	       scanf("%d",&temp.des);
	       printf("Enter amount\n");
	       scanf("%lf",&temp.amt);
	       write(sd,&temp,sizeof(temp));
	       int n = read(sd, buf, sizeof(buf)-1);
             if(n > 0) {
             buf[n] = '\0';
             printf("%s\n", buf);
             }
       }
//Apply for loan
  if(a==5)
  {
     struct loan ln;
     printf("Enter account number\n");
     scanf("%d",&ln.acc_no);

     printf("Enter Loan amount\n");
     scanf("%lf",&ln.amt);


     printf("Enter loan type(hom/personal/education)\n");
     scanf("%s",ln.type);

     write(sd,&ln,sizeof(ln));
     char msg[100];
     int bytes=read(sd,msg,sizeof(msg)-1);
     msg[bytes]='\0';
     printf("%s\n",msg);
  }

  //Change password
  
  if(a==6)
  {
    char msg[100];
    struct cust_cred c;
    printf("Enter username\n");
    scanf("%s",c.username);
    c.username[sizeof(c.username)-1]='\0';
    printf("Enter new password\n");
    scanf("%s",c.password);
    c.password[sizeof(c.password)-1]='\0';
    write(sd,&c,sizeof(c));
    int bytes = read(sd, msg, sizeof(msg) - 1);
    msg[bytes] = '\0';
    printf("%s\n", msg);
  }
// add feedback
  if(a==7)
  { 
    struct feedback fb;
    char msg[100];

    printf("Enter your username: ");
    scanf("%s", fb.username);
    getchar();  // consume leftover newline

    printf("Enter your feedback (max 200 chars): ");
    fgets(fb.message, sizeof(fb.message), stdin);

    fb.message[strcspn(fb.message, "\n")] = '\0';  // remove newline if present

    // Send to server
    write(sd, &fb, sizeof(fb));

    // Read response
    int bytes = read(sd, msg, sizeof(msg) - 1);
    msg[bytes] = '\0';
    printf("%s\n", msg);
  }

// View Transaction History
       
if (a == 8) {
    int acc_no;
    printf("Enter account number: ");
    scanf("%d", &acc_no);

    
    write(sd, &acc_no, sizeof(acc_no));

    
    struct transaction arr[50];
    ssize_t bytes_read = read(sd, arr, sizeof(arr));

   

    int num_records = bytes_read / sizeof(struct transaction);
    printf("\n--- Transaction History for Account %d ---\n", acc_no);

    for (int i = 0; i < num_records; i++) {
        if(acc_no==arr[i].acc_no)
	{
        char *time_str = ctime(&arr[i].timestamp);
        if (time_str) {
            time_str[strlen(time_str) - 1] = '\0'; 
        }

        printf("Account: %d | Amount: %.2lf | Type: %s | Time: %s\n",
               arr[i].acc_no, arr[i].amt, arr[i].type,
               time_str ? time_str : "Unknown");
    }

    if (num_records == 0) {
        printf("No transactions found or file empty.\n");
    }
    }

    printf("-------------------------------------------\n");
}

       
       //logout
	  if(a==9)
	  {
             flag_cust=1;
	     //Read server Response 
	     int n = read(sd, buf_c, sizeof(buf_c)-1);
             if(n > 0) {
             buf_c[n] = '\0';
             printf("%s\n", buf_c);
	  }
 	  }
//Exit
    if(a==10)
{
close(sd);
exit(0);
}
	   
     }
   }
   // employee logged in. Display Employee menu
   if(ch==2)
   {
     int flag_emp=0;
     while(flag_emp==0)
     {
     display_empmenu();
     int a ;
     printf("Enter your choice \n");
     fflush(stdout);
     scanf("%d",&a);
     write(sd,&a,sizeof(a)); // send choice to server
     //Add new Customer
     if(a==1)
     {
        struct account acc;
	 printf("Enter account number\n");
        scanf("%d",&acc.acc_no);
        printf("Enter balance to deposit(Minimum balance=1000)\n");
        scanf("%lf",&acc.balance);
        printf("Enter username(same as login)\n");
        scanf("%s",acc.username);
        acc.username[5]='\0';
        acc.active=1;
        write(sd,&acc,sizeof(acc));
     
     //read server response
     int n = read(sd, buf_e, sizeof(buf)-1);
             if(n > 0) {
             buf_e[n] = '\0';
             printf("%s\n", buf_e);
         }
     }
     //Approve/Reject Loans
     if(a==2)
     {
        int loan_id;
       char status;

    printf("Enter Loan ID to assign : ");
    scanf("%d", &loan_id);
    while (getchar() != '\n');
    printf("A.Accept R.Reject ");
    scanf("%c", &status);

    struct assign_req {
        int loan_id;
        char status;
    } req;

    req.loan_id = loan_id;
    req.status=status;
    //req.emp[sizeof(req.emp) - 1] = '\0';

    write(sd, &req, sizeof(req));

    int success;
    read(sd, &success, sizeof(success));
    if (success == 1)
        printf(" Loan status changed to %c\n",status);
    else
        printf(" Failed \n");

     }
//View Assigned Loans
     if(a==3)
     {
         struct loan loans[100];
	 char emp[6];
	 printf("Enter Employee id\n");
         scanf("%s",emp);
	 emp[6]='\0';
          ssize_t byte=read(sd,loans,sizeof(loans));
          int count=byte/sizeof(struct loan);
         // if (count == 0)
       // printf("No pending or unassigned loans.\n");

         printf("\n--- Assigned Loans ---\n");
         printf("ID\tAccount\tAmount\tType\t\tStatus\tEmployee\n");
         printf("----------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
	    if(strcmp(emp,loans[i].emp)==0)
        printf("%d\t%d\t%.2lf\t%-10s\t%c\t%s\n",
               loans[i].id, loans[i].acc_no, loans[i].amt,
               loans[i].type, loans[i].status,
               loans[i].emp);
    }

    printf("----------------------------------------------------\n");

     }
//View Transaction history
     if(a==4)
     {
          int acc_no;
    printf("Enter account number: ");
    scanf("%d", &acc_no);


    write(sd, &acc_no, sizeof(acc_no));


    struct transaction arr[50];
    ssize_t bytes_read = read(sd, arr, sizeof(arr));



    int num_records = bytes_read / sizeof(struct transaction);
    printf("\n--- Transaction History for Account %d ---\n", acc_no);

    for (int i = 0; i < num_records; i++) {
        if(acc_no==arr[i].acc_no)
        {
        char *time_str = ctime(&arr[i].timestamp);
        if (time_str) {
            time_str[strlen(time_str) - 1] = '\0';
        }

        printf("Account: %d | Amount: %.2lf | Type: %s | Time: %s\n",
               arr[i].acc_no, arr[i].amt, arr[i].type,
               time_str ? time_str : "Unknown");
    }

    if (num_records == 0) {
        printf("No transactions found or file empty.\n");
    }
    }

    printf("-------------------------------------------\n");
     }
//change password
     if(a==5)
     {
        char msg[100];
    struct cust_cred c;
    printf("Enter username\n");
    scanf("%s",c.username);
    c.username[sizeof(c.username)-1]='\0';
    printf("Enter new password\n");
    scanf("%s",c.password);
    c.password[sizeof(c.password)-1]='\0';
    write(sd,&c,sizeof(c));
    int bytes = read(sd, msg, sizeof(msg) - 1);
    msg[bytes] = '\0';
    printf("%s\n", msg);

     }
//Logout
     if(a==6)
     {
       flag_emp=1;
       int n = read(sd, buf_e, sizeof(buf)-1);
             if(n > 0) {
             buf_e[n] = '\0';
             printf("%s\n", buf_e);
         }
	  
     }
     //exit
     if(a==7)
     {
	     close(sd);
	     exit(0);
     }
     }
   }
if(ch==3)
{
	int flag_man=0;
	while(flag_man==0)
	{
          display_manmenu();
	   int a ;
           printf("Enter your choice \n");
           fflush(stdout);
           scanf("%d",&a);
           write(sd,&a,sizeof(a)); // send choice to server
	//assign loan to employees
	if(a==2)
	{
	  struct loan loans[100];
          ssize_t byte=read(sd,loans,sizeof(loans));
	  int count=byte/sizeof(struct loan);
	  if (count == 0) 
        printf("No pending or unassigned loans.\n");
        
	 printf("\n--- Pending / Unassigned Loans ---\n");
         printf("ID\tAccount\tAmount\tType\t\tStatus\tEmployee\n");
         printf("----------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        printf("%d\t%d\t%.2lf\t%-10s\t%c\t%s\n",
               loans[i].id, loans[i].acc_no, loans[i].amt,
               loans[i].type, loans[i].status,
               loans[i].emp[0] ? loans[i].emp : "NONE");
    }

    printf("----------------------------------------------------\n");
    // Now manager assigns a loan
    int loan_id;
    char emp[6];

    printf("Enter Loan ID to assign : ");
    scanf("%d", &loan_id);
    printf("Enter Employee username (max 5 chars): ");
    scanf("%5s", emp);

    struct assign_req {
        int loan_id;
        char emp[6];
    } req;

    req.loan_id = loan_id;
    strncpy(req.emp, emp, sizeof(req.emp) - 1);
    req.emp[sizeof(req.emp) - 1] = '\0';

    write(sd, &req, sizeof(req));

    int success;
    read(sd, &success, sizeof(success));
    if (success == 1)
        printf(" Loan %d assigned to %s successfully.\n", loan_id, emp);
    else
        printf(" Failed to assign loan %d (already assigned or not found).\n", loan_id);
	}

	if(a==3)
	{
		struct feedback fb[100];
    ssize_t bytes_read = read(sd, fb, sizeof(fb));

    if (bytes_read <= 0) {
        printf("No feedbacks received.\n");
        return;
    }

    int count = bytes_read / sizeof(struct feedback);
    if (count == 0) {
        printf("No feedback messages available.\n");
        return;
    }

    printf("\n--- Customer Feedback ---\n");
    printf("-----------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("From: %-6s | Message: %s\n", fb[i].username, fb[i].message);
    }
    printf("-----------------------------------------------\n");
	}

	if(a==4)
	{
              char msg[100];
    struct man_cred c;
    printf("Enter username\n");
    scanf("%s",c.username);
    c.username[sizeof(c.username)-1]='\0';
    printf("Enter new password\n");
    scanf("%s",c.password);
    c.password[sizeof(c.password)-1]='\0';
    write(sd,&c,sizeof(c));
    int bytes = read(sd, msg, sizeof(msg) - 1);
    msg[bytes] = '\0';
    printf("%s\n", msg);

	}
           //Logout manager 
           if(a==5)
	   {
              flag_man=1;
	      int n = read(sd, buf_e, sizeof(buf)-1);
             if(n > 0) {
             buf_e[n] = '\0';
             printf("%s\n", buf_e);
         }

	   }
	   if(a==6)
	   {
		   close(sd);
		   exit(0);
	   }
     //Activate/ deactivate customer account
	  if(a==1)
	  {
           struct account_status_update {
        int acc_no;
        int new_status;   // 1 -> Activate, 0 -> Deactivate
        } req;

       // Fill the structure
       printf("Enter Account Number: ");
       scanf("%d", &req.acc_no);
       printf("Enter 1 to Activate, 0 to Deactivate: ");
       scanf("%d", &req.new_status);

       // Send entire structure in one write
       write(sd, &req, sizeof(req));
       char msg[100];
       int bytes = read(sd, msg, sizeof(msg) - 1);
       msg[bytes] = '\0';
       printf("%s\n", msg);
	  } 

	}

}
if(ch==4)
{
  int flag_admin=0;
  while(flag_admin==0)
  {
	    display_adminmenu();
            int a ;
           printf("Enter your choice \n");
           fflush(stdout);
           scanf("%d",&a);
           write(sd,&a,sizeof(a));// send choice to server
	//add new employee
	if(a==1)
	{
		int c;
		printf("1.add employee\n");
		printf("2.add manager\n");
                printf("Enter your choice\n");
		fflush(stdout);
		scanf("%d",&c);
		write(sd,&c,sizeof(c));
		if(c==1)
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
		else
	        {
   struct man_cred acc = {0};
    printf("Enter manager  username: ");
    scanf("%5s", acc.username);
    printf("Enter manager  password: ");
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

	}
	//change password
	if(a==2)
	{
		char msg[100];
    struct cust_cred c;
    printf("Enter username\n");
    scanf("%s",c.username);
    c.username[sizeof(c.username)-1]='\0';
    printf("Enter new password\n");
    scanf("%s",c.password);
    c.password[sizeof(c.password)-1]='\0';
    write(sd,&c,sizeof(c));
    int bytes = read(sd, msg, sizeof(msg) - 1);
    msg[bytes] = '\0';
    printf("%s\n", msg);


	}
	//logout
	   if(a==3)
	   {
              flag_admin=1;
              int n = read(sd, buf_e, sizeof(buf)-1);
             if(n > 0) {
             buf_e[n] = '\0';
             printf("%s\n", buf_e);

      	   }
	   }
	   //exit
	   if(a==4)
	   {
		   close(sd);
		   exit(0);
	   }
  }
}
   // exit(5th option ) was chosen so close connection
   if(e==1)
    {
    close(sd);
    return 0;
    }
}


