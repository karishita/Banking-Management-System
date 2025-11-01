# Banking Management System (C Project)

## Overview
A menu-driven client–server application in C that simulates core banking operations: accounts, transactions, loans, feedback, and role-based access for Admin, Manager, Employee, and Customer. Data is persisted using file-based storage with POSIX system calls.
It supports multiple user roles — Admin, Manager, Employee, and Customer — each with role-specific permissions and functionalities.

## Features

### Customer
- View Account Balance
- Deposit Money
- Withdraw Money
- Transfer Funds
- Apply for Loan
- View Transaction History
- Submit Feedback
- Change Password

### Employee
- Add New Customers
- Approve/Reject Loans
- View Assigned Loan Applications
- View Customer Transactions
- Change Password

### Manager
- Activate/Deactivate Customer Accounts
- Assign Loan Applications to Employees
- Review Customer Feedback
- Change Password

### Admin
- Add New Bank Employees
- Change Password

## System Architecture

Persistent data files used:

| File            | Purpose                                 |
|-----------------|-----------------------------------------|
| `account.txt`   | Account details and balances            |
| `transactions.txt` | Deposit/withdraw/transfer records   |
| `loan.txt`      | Loan applications and assignment status |
| `feedback.txt`  | Customer feedback messages              |
| `cust_cred.txt` | Customer login credentials              |
| `emp_cred.txt`  | Employee login credentials              |
| `man_cred.txt`  | Manager login credentials               |
| `admin_cred.txt`| Admin login credentials                 |

## Concepts Used

- Socket Programming
- System Calls: open, read, write, lseek
- File based persistent storage
- Structure based data management
- Role based access with one login session per user
- File Locking to prevent race condition and inconsistent updates

  ## Class Diagram
  <img width="1438" height="898" alt="image" src="https://github.com/user-attachments/assets/41e0282f-327b-47d8-8dac-c057dd053212" />

 
## How to Run

gcc client.c -o client
gcc server.c -o server

# In one terminal
./server
# In a separate terminal
./client
