//The code is for Volunteer Management system for Non Profit Organisation with using MySql database.

#include<iostream>
#include<string>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <vector>
#include <memory>
#include <cstdint>
#include <cppconn/prepared_statement.h> 

using namespace std;

class volunteer{
  protected:
  int volunteer_id;
  string name;
  string interest;
  int experience;
  int64_t contact_no;
  public:
  volunteer(int a,string b,string c,int d,int64_t e){
  volunteer_id=a;
  name=b;
  interest=c;
  experience=d;
  contact_no=e;
  }
  volunteer(const volunteer &a){
  volunteer_id=a.volunteer_id;
  name=a.name;
  interest=a.interest;
  experience=a.experience;
  contact_no=a.contact_no;
  }
    
  friend void assign_lead(sql::Connection* con,int p_id);
  friend void assign_volunteers_to_tasks(sql::Connection* con, int project_id, int task_id);
  };
   
class project{
 protected:
 int project_id;
 string type;
 string project_name;
 string description;
 string date;
 int lead_id;
 public:
 project(int a,string b,string c,string d,string e){
 project_id=a;
 type=b;
 project_name=c;
 description=d;
 date=e;
 lead_id=0;
 }
 project(const project &a){
 project_id=a.project_id;
 type=a.type;
 project_name=a.project_name;
 description=a.description;
 date=a.date;
 }
 friend void assign_lead(sql::Connection* con,int p_id);
 };
 
class task{
 protected:
 int project_id;
 int task_id;
 int priority;
 int min_people;
 string description;
 string volunteer_ids;
 public:
 task(int a,int b,int d,int e,string c){
 project_id=a;
 task_id=b;
 priority=d;
 min_people=e;
 description=c;
  }
 task(const task &a){
 project_id=a.project_id;
 task_id=a.task_id;
 priority=a.priority;
 min_people=a.min_people;
 description=a.description;
 }
 };
 
class npo:protected volunteer,protected project,protected task{
public:
  };

class spendings{
static long int fund;
string service;
long int expenditure;
public:
spendings(long int a,string b,long int c){
fund=a;
service=b;
expenditure=c;
}
};

class authenticate{
    private:
    string user;
    string password;
    public:
    authenticate(string a,string b){
    user=a;
    password=b;}
    friend int login();
    };
    
//function for login
int login(){
authenticate a("admin","npo@321");
authenticate b("volunteer","service");
string usr,pass;
cout<<"enter the username "<<endl;
cin>>usr;
cout<<"enter the password "<<endl;
cin>>pass;

if(usr==a.user){
  if(pass==a.password){
  return 1;}}
  
else if(usr==b.user){
  if(pass==b.password){
  return 2;}}
return 0;
}

//function to match interest(check substr
bool check_interest(const string& str, const string& substr) {
    return str.find(substr) != string::npos;
}
//to get the volunteers in dynamic array using vector
vector<volunteer> fetch_volunteers(sql::Connection* con) {
    vector<volunteer> volunteers;
    try {
      
        unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM volunteers"));
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            int id = res->getInt("volunteer_id");
            string name = res->getString("name");
            string interest = res->getString("interest");
            int exp = res->getInt("experience");
            int64_t phone = res->getInt64("contact_no");
            volunteers.emplace_back(id, name, interest, exp,phone);
        }
    } catch (sql::SQLException &e) {
        cout << "Error fetching volunteers: " << e.what() << endl;
    }
    return volunteers;
}
// to add the lead id to the project in database 
void update_project_lead(sql::Connection* con, int project_id, int lead_id) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("UPDATE projects SET lead_id = ? WHERE project_id = ?"));
        pstmt->setInt(1, lead_id);
        pstmt->setInt(2, project_id);
        pstmt->executeUpdate();
        cout << "Project lead updated successfully." << endl;
    } catch (sql::SQLException &e) {
        cout << "Error updating project lead: " << e.what() << endl;
    }
}
//function for assigning leader for a project.
 void assign_lead(sql::Connection* con,int p_id) {
        volunteer* best_volunteer = nullptr;
        int max_experience = 0;
         std::unique_ptr<sql::Statement> stmt(con->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT type from projects where project_id = " + to_string(p_id)));
       if (res->next()) { 
       string type = res->getString("type");
        vector<volunteer> volunteers = fetch_volunteers(con);
        // Loop through the volunteers using auto 
        for (auto& v : volunteers) {
            if (check_interest(type,v.interest)) {
                // If this volunteer has more experience, assign them as the lead
                if (v.experience > max_experience) {
                    best_volunteer = &v;
                    max_experience = v.experience;
                }
            }
        }
        if (best_volunteer) {
            int lead_id=best_volunteer->volunteer_id;
            update_project_lead(con,p_id,lead_id);
            cout << "Lead assigned: Volunteer ID " << best_volunteer->volunteer_id << " with experience " << best_volunteer->experience << endl;
        } else {
            cout << "No suitable volunteer found for this task." << endl;
        } } else {
            cout << "No project found with the provided project ID." << endl;
        }
    }
// Function to assign volunteers to tasks
void assign_volunteers_to_tasks(sql::Connection* con, int project_id, int task_id) {
    try {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        std::unique_ptr<sql::ResultSet> task_res(stmt->executeQuery("SELECT description, min_people FROM task WHERE task_id = " + to_string(task_id)));
        string task_description;
        int min_people = 0;

        if (task_res->next()) {
            task_description = task_res->getString("description");
            min_people = task_res->getInt("min_people");
        }

        // used to find project lead to exclude them from assingning to the tasks of the projrct again
        std::unique_ptr<sql::ResultSet> project_res(stmt->executeQuery("SELECT lead_id FROM projects WHERE project_id = " + to_string(project_id)));
        int lead_id = 0;
        if (project_res->next()) {
            lead_id = project_res->getInt("lead_id");
        }
        std::unique_ptr<sql::ResultSet> volunteer_res(stmt->executeQuery("SELECT * FROM volunteers WHERE volunteer_id != " + to_string(lead_id)));

        vector<volunteer> assigned_volunteers;
        int people_assigned = 0;
        string assigned_volunteer_ids = "";
        
        // Loop through volunteers and assign them based on their interest
        while (volunteer_res->next() && people_assigned < min_people) {
            int vol_id = volunteer_res->getInt("volunteer_id");
            string vol_name = volunteer_res->getString("name");
            string vol_interest = volunteer_res->getString("interest");
            int vol_experience = volunteer_res->getInt("experience");
            int64_t vol_contact = volunteer_res->getInt64("contact_no");

            volunteer v(vol_id, vol_name, vol_interest, vol_experience, vol_contact);
            // Checks interest with the task description
            if (check_interest(task_description, v.interest)) {
                assigned_volunteers.push_back(v);
                people_assigned++;
                cout << "Assigned " << vol_name << " to task. Total assigned: " << people_assigned << endl;
                if (!assigned_volunteer_ids.empty()) {
                    assigned_volunteer_ids += ",";
                }
                assigned_volunteer_ids += to_string(vol_id);
            }
        }

        // Checks for min needed people for the task
        if (people_assigned < min_people) {
            cout << "Not enough volunteers. Only " << people_assigned << " out of " << min_people << " assigned." << endl;
        } else {
            cout << "Task assignment successful. " << people_assigned << " volunteers assigned." << endl;
            try {
                // Update the task with assigned volunteer IDs
                stmt->executeUpdate("UPDATE task SET volunteer_ids = '" + assigned_volunteer_ids + "' WHERE task_id = " + to_string(task_id));
                cout << "Volunteer IDs updated in the task successfully." << endl;
            } 
            catch (sql::SQLException& e) {
                cout << "Error updating task: " << e.what() << endl;
            }
        }

    } catch (sql::SQLException &e) {
        cout << "Error: " << e.what() << endl;
    }
}
//function for updating volunteer profile
void update_volunteer_profile(sql::Connection* con, int volunteer_id) {
    cout << "Choose which detail you want to update:\n1 --> Name\n2 --> Interest\n3 -->Contact No\n4--> Exit" << endl;
    int choice;
    cin >> choice;

    switch (choice) {
    case 1: {  
        string new_name;
        cout << "Enter new name: ";
        cin >> new_name;

        try {
            std::unique_ptr<sql::Statement> stmt(con->createStatement());
            stmt->executeUpdate("UPDATE volunteers SET name = '" + new_name + "' WHERE volunteer_id = " + to_string(volunteer_id));
            cout << "Name updated successfully!" << endl;
        }
        catch (sql::SQLException& e) {
            cout << "Error: " << e.what() << endl;
        }
        break;
    }
    case 2: { 
        string new_interest;
        cout << "Enter new interest: ";
        cin >> new_interest;

        try {
            std::unique_ptr<sql::Statement> stmt(con->createStatement());
            stmt->executeUpdate("UPDATE volunteers SET interest = '" + new_interest + "' WHERE volunteer_id = " + to_string(volunteer_id));
            cout << "Interest updated successfully!" << endl;
        }
        catch (sql::SQLException& e) {
            cout << "Error: " << e.what() << endl;
        }
        break;
    }
    
    case 3: { 
        long int new_contact;
        cout << "Enter new contact number: ";
        cin >> new_contact;

        try {
            std::unique_ptr<sql::Statement> stmt(con->createStatement());
            stmt->executeUpdate("UPDATE volunteers SET contact_no = " + to_string(new_contact) + " WHERE volunteer_id = " + to_string(volunteer_id));
            cout << "Contact number updated successfully!" << endl;
        }
        catch (sql::SQLException& e) {
            cout << "Error: " << e.what() << endl;
        }
        break;
    }
    case 4: 
        break;

    default:
        cout << "Invalid choice. Try again." << endl;
        update_volunteer_profile(con, volunteer_id);
    }
}
//function to add volunteer
void add_volunteer(sql::Connection* con) {
    try {
        string name, interest;
        int experience,id;
        long int contact_no;
        cout<<"enter volunteer's ID:";
        cin>>id;

        cout << "Enter volunteer's name: ";
        cin.ignore();
        getline(cin, name);
        
        cout << "Enter volunteer's interest: ";
        getline(cin, interest);
        
        cout << "Enter volunteer's experience (in years): ";
        cin >> experience;
        
        cout << "Enter volunteer's contact number: ";
        cin >> contact_no;

        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->executeUpdate("INSERT INTO volunteers (volunteer_id,name, interest, experience, contact_no) VALUES (" + to_string(id) +",'" + name + "', '" + interest + "', " + to_string(experience) + ", " + to_string(contact_no) + ")");
        cout << "Volunteer added successfully!" << endl;

    } catch (sql::SQLException &e) {
        cout << "Error: " << e.what() << endl;
    }
}
//function to add project
void add_project(sql::Connection* con) {
    try {
        string project_name, type, description, date;
        int id;
        cout<<"enter the project id: ";
        cin>>id;
        cout << "Enter project name: ";
        cin.ignore();
        getline(cin, project_name);
        
        cout << "Enter project type: ";
        getline(cin, type);
        
        cout << "Enter project description: ";
        getline(cin, description);
        
        cout << "Enter project start date (YYYY-MM-DD): ";
        getline(cin, date);

        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->executeUpdate("INSERT INTO projects (project_id,project_name, type, description, date) VALUES (" + to_string(id) +",'" + project_name + "', '" + type + "', '" + description + "', '" + date + "')");
        cout << "Project added successfully!" << endl;

    } catch (sql::SQLException &e) {
        cout << "Error: " << e.what() << endl;
    }
}
//function to add task for a project
void add_task(sql::Connection* con) {
    try {
        int project_id, priority, min_people,id;
        string description;
        
        cout << "Enter the project ID for the task: ";
        cin >> project_id;
        
        cout<<"enter the task id which is (projectId with task id like project 1 task 1-->11)";
        cin>>id; 
         
        cout << "Enter task priority (1-10): ";
        cin >> priority;
        
        cout << "Enter minimum people needed: ";
        cin >> min_people;
        
        cout << "Enter task description: ";
        cin.ignore();
        getline(cin, description);

        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->executeUpdate("INSERT INTO task (project_id,task_id, priority, min_people, description) VALUES (" + to_string(project_id) + "," + to_string(id) +", " + to_string(priority) + ", " + to_string(min_people) + ", '" + description + "')");
        cout << "Task added successfully!" << endl;

    } catch (sql::SQLException &e) {
        cout << "Error: " << e.what() << endl;
    }
}
//function to add spendings in databases
void add_spending(sql::Connection* con) {
    try {
        long int fund,expenditure;
        string service;

        cout << "Enter fund given : ";
        cin >> fund;

        cout << "Enter service name: ";
        cin.ignore();
        getline(cin, service);
        
        cout << "Enter expenditure amount: ";
        cin >> expenditure;

        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->executeUpdate("INSERT INTO spendings (fund, service, spending) VALUES (" + to_string(fund) + ", '" + service + "', " + to_string(expenditure) + ")");
        cout << "Spending details added successfully!" << endl;

    } catch (sql::SQLException &e) {
        cout << "Error: " << e.what() << endl;
    }
}
//function to view spending from databases
void view_spendings(sql::Connection* con) {
    try {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM spendings"));

        cout << " Spending Details " << endl;
       
        while (res->next()) {
            cout << "Fund " << res->getDouble("fund")<<endl; 
             cout << "Service" << res->getString("service") <<endl;
              cout << "Expenditure" <<  res->getDouble("spending") <<endl; 
        }
        cout<<"-------------------------------"<<endl;
        std::unique_ptr<sql::ResultSet> res1(stmt->executeQuery("SELECT sum(fund),sum(spending) FROM spendings"));
        if (res1->next()) {
            cout << "Total fund " << res1->getDouble("sum(fund)")<<endl; 
            cout << "Total expenditure" <<  res1->getDouble("sum(spending)") <<endl; 
       }
    } catch (sql::SQLException &e) {
        cout << "Error: " << e.what() << endl;
    }
}

//volunteers login 
void login_volunteer(sql::Connection* con){
cout<<"WELCOME VOLUNTEER"<<endl;
cout<<"enter your volunteer ID"<<endl;
int id;
cin>>id;
int d;
std::unique_ptr<sql::Statement> stmt(con->createStatement());
std::unique_ptr<sql::ResultSet> res1(stmt->executeQuery("SELECT name FROM volunteers where volunteer_id="+ to_string(id)));
if (res1->next()){
 cout << "HELLO " << res1->getString("name") << endl;
}

 do {
cout<<"choose \n1-->To view your projects\n2-->To view the tasks\n3-->To view the spendings of the NPO\n4-->To update ur profile\n5-->exit"<<endl;
cin>>d;
 switch (d){
case 1:
 try {
 std::unique_ptr<sql::Statement> stmt(con->createStatement());
 std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM projects WHERE project_id IN (SELECT project_id FROM task WHERE volunteer_ids  LIKE '%"+ to_string(id)+"%')"));  
 cout<<"your projects"<<endl;
 while (res->next()) {
      cout << "Project ID: " << res->getInt("project_id") << endl;
      cout << "Project Name: " << res->getString("project_name") << endl;
      cout << "Type: " << res->getString("type") << endl;
      cout << "Description: " << res->getString("description") << endl;
      cout << "Date: " << res->getString("date") << endl;
      cout<<endl;
      }
  } catch (sql::SQLException &e) {
      cout << "Error: " << e.what() << endl;
    }
 
break;
case 2:
 try {
 std::unique_ptr<sql::Statement> stmt(con->createStatement());
 std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM task where volunteer_ids  LIKE  '%"+ to_string(id) +"%'"));                 
 cout << "YOUR Tasks:" << endl;
  while (res->next()) {
     cout << "Task ID: " << res->getInt("task_id") << endl;
     cout << "Priority: " << res->getInt("priority") << endl;
    cout << "Min People: " << res->getInt("min_people") << endl;
     cout << "Description: " << res->getString("description") << endl;
     cout<<"Team members :"<< res->getString("volunteer_ids") << endl;
     cout<<endl;
      }
  } catch (sql::SQLException &e) {
      cout << "Error: " << e.what() << endl;
    }
break;
case 3:
view_spendings(con);
break;
case 4:
update_volunteer_profile(con, id);
break;
case 5:
 cout << "Exiting ....\n";
break;
default:cout<<"enter right choice";
login_volunteer(con);}}
while(d!=5);
}
//admin login
void admin_login(sql::Connection* con) {
    int choice,p_id,t_id;
    
    do {
        cout << "\nADMIN MENU\n1--> Add Volunteer\n2--> Add Project\n3--> Assign lead to the project \n4--> Add Task to Project\n5--> Assign volunteers to tasks\n6--> Record Spendings\n7--> Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                add_volunteer(con);
                break;
            case 2:
                add_project(con);
                break;
            case 3:
            cout<<"enter the project_id";
            cin>>p_id;
            assign_lead(con,p_id);
            break;    
            case 4:
                add_task(con);
                break;
            case 5:
            cout<<"enter the project_id: ";
            cin>>p_id;
            cout<<"enter the task_id: ";
            cin>>t_id;
            assign_volunteers_to_tasks(con,p_id,t_id);
            break;
            
            case 6:
                add_spending(con);
                break; 
            case 7:
                cout << "Exiting ....\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
                break;
        }
    } while (choice != 7);
}

  
int main(){
cout<<"\033[1;33m" << "                 GOOD DAY (<_>)" << "\033[0m" << endl;
cout<<"\033[1;31m" << "            SERVICE AT ITS BEST --->>> ATY NGO" << "\033[0m" << endl;
sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "tinku"));
con->setSchema("NGO");
std::unique_ptr<sql::Statement> stmt(con->createStatement());
int c=login();
if(c==0){
int j=2;
while(c==0 && j!=0){
cout<<"INVALID credentials \n try login again \n--left out tries-- "<<j<<endl;
c=login();
j--;}
if(c==0){
cout<<"Sorry The portal is locked ur authentication is invalid"; 
return -1;
}
}
switch (c){
case 1:
admin_login(con.get()) ;
break;
case 2:
login_volunteer(con.get());
break;
}
}
  

  
  
