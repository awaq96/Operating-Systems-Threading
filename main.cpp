#include <iostream>
#include<queue>
#include <fstream>
#include <string>
using namespace std;
/*

    This program simulates the process scheduling of a tablet accouting for Core Requests, SSD requests and Requests
*/

// This structure creates the node that will store all the information for the program. Most of the data
// it stores will come from parsing the input
struct node{
    string operation;
    int param;
    int index;
    int PID;
    int Start;
    int First_Line;
    int Last_Line;
    int Current_Line;
    string state;
    string device;
    string status;
    int totalBusyTime;
    node*next,*prev;
};

// These global variables will simulate the process movements,
queue<node*> Iqueue,NIqueue,SSD;
int NCORES;
int sysClock =0; // Variable will hold and simulate time
bool SSD_free=true;

// List class creates the linked list the program will use to order the processes from the input
class List {
public:
    node *head, *tail;
public:
    List() {
        head = NULL;
        tail = NULL;
    }

    //this function will take all the commands from the input and order them into a linked list.
    void add(string in,int param,int index) {
        node *tmp;
        tmp = new node();

        tmp->operation = in;
        tmp->param=param;
        tmp->index=index;

        tmp->next = NULL;
        tmp->prev = NULL;

        if (head == NULL) {
            head = tmp;
            tail = tmp;
        } else {
            tail->next = tmp;
            tail->prev = tmp;
            tail = tail->next;
        }
    }

    // This function will form the process table. The table will show Process ID, Process Start Time, First line # of
    // process, Line # from input where process ends, current Line process is on and the state of the process
    void addPTable(int PID,int Start, int First, int Last, int Curr, string state) {
        node *tmp;
        tmp = new node();

        tmp->PID = PID;
        tmp->Start=Start;
        tmp->First_Line=First;
        tmp->Last_Line=Last;
        tmp->Current_Line=Curr;
        tmp->state="    ";

        tmp->next = NULL;
        tmp->prev = NULL;

        if (head == NULL) {
            head = tmp;
            tail = tmp;
        } else {
            tail->next = tmp;
            tail->prev = tmp;
            tail = tail->next;
        }
    }

    // This function will create the device table
    void addDevice(string device, string status, int totBusyTime) {
        node *tmp;
        tmp = new node();
        tmp->device=device;
        tmp->status=status;
        tmp->totalBusyTime=totBusyTime;

        tmp->next = NULL;
        tmp->prev = NULL;

        if (head == NULL) {
            head = tmp;
            tail = tmp;
        } else {
            tail->next = tmp;
            tail->prev = tmp;
            tail = tail->next;
        }
    }

};
// Forward Declaration
int parse(List *input_table,string *arr);
void Process_Table(List *input_table, List *process_table);
void Device_Table(List *Dev_table);
void core_release(node*cu,List *pass2,string *arr,bool isinteractive);
void core_request(node*cu,string *arr,List *pass2,bool isinteractive);
void arrival(node*cu,string *arr,List *pass2);
void TTY_release(bool isinteractive,node*cu,string *arr,List*pass2);
void TTY_request(int tasktime,node*cu,string *arr,List*pass2);
void SSD_release(string arr,node*pass2,node*cu);
void SSD_request(int time,node *cu,string arr,List*pass2);
void printArrival(node *cu,node*traverse);
void printTerminated(node *cu,node*traverse);




/* This function will parse the input file. First it will take the # of cores from the first line of input.
 * For the following lines, it will put the line in a string array, and also add it to a linked list.
 * in the linked list the input lines will be seperated into, operation and parameter value, the list will also index the
 * inut line #
*/
int parse(List *input_table,string *arr){

    // Open and Parse Input.txt
    string input;
    ifstream infile;
    infile.open ("input11.txt");
    int index=0;
    int instruction_count=0;
    while(!infile.eof())
    {
        getline(infile,input);
        if (input == "") continue; //ignore blank lines
        if(input !="END"){

            string operation = input.substr(0, input.find(' ')); // get the operation string input
            string param_in = input.substr(input.find(' ') + 1); // get the numerical value of input
            if(input !=" "){
                int param = stoi(param_in);
                if(operation =="NCORES"){
                    NCORES=param;//Stores # of cores
                }else{
                    arr[index]=input;
                    input_table->add(operation, param,index);// Otherwise, put input into a linked list
                    index++;
                    instruction_count++;
                } }}}

    infile.close();
    return instruction_count;//This will let main know how long to run the processes
}

/*
 * The function will take information from the parsing and create a process table
 */
void Process_Table(List *input_table, List *process_table){

    node *cu= input_table->head;
    int PID,Start,First,Last,Current;
    string State;
    while(cu!=NULL){
        if(cu->operation=="START"){
            Start=cu->param;
            First=cu->index;
            PID=cu->next->param;
        }
        if(cu->next!=NULL){
        if(cu->next->operation=="START"){
            Last=cu->index;
            //cout<<PID<<" "<<Start<<" "<<First<<" " <<Last<<endl;

            process_table->addPTable(PID,Start,First,Last,First,"Ready");
        }}
        cu=cu->next;
    }
}

// Creates a device table
void Device_Table(List *Dev_table){
    Dev_table->addDevice("CPU"," ",0);
    Dev_table->addDevice("SSD"," ",0);
    Dev_table->addDevice("TTY"," ",0);

}


//Once a task's time in the core is done, it is released. After the release it checks if there are any tasks in queue
// the interactive queue gets priority, both return back to core request function so their time can be calculated
void core_release(node*cu,List *pass2,string *arr,bool isinteractive){
    int PID;
    node*pd = pass2->head;
    node* next_task;
    if(!Iqueue.empty()){
        next_task = Iqueue.front();
        Iqueue.pop();
        core_request(next_task,arr,pass2,isinteractive);


    } else if(!NIqueue.empty()){
        next_task = NIqueue.front();
        NIqueue.pop();
        core_request(next_task,arr,pass2,isinteractive);
    }


}

//This function calculates core usage time, if cores are all in use, the function queues the task.
void core_request(node*cu,string *arr,List *pass2,bool isinteractive){
    string command;
    if(NCORES>0){
        NCORES--;
        cu->state="Running";
       string timetakentext=arr[cu->Current_Line].substr(arr[cu->Current_Line].find(' ') + 1);
       int timetaken=stoi(timetakentext);
       cu->Current_Line+=1;
       if(cu->Current_Line>cu->Last_Line){
           cu->state="Terminated";
       }

        sysClock+=timetaken;
        core_release(cu,pass2,arr,isinteractive);
    }
    else{
        if(isinteractive){
            Iqueue.push(cu);
            cu->state="Ready";
        }
        else{
            NIqueue.push(cu);
            cu->state="Ready";
        }
    }

}


// This function runs when a new process enters the processor, it assesses if the process is interactive and then sends it to the core request
void arrival(node*cu,string *arr,List *pass2){
    cu->Current_Line=cu->Current_Line+1;
    string task=arr[cu->index].substr(0, arr[cu->index].find(' '));
    bool isinteractive=false;
    if(task=="TTY")
        isinteractive=true;

    core_request(cu,arr,pass2,isinteractive);
}


// Once the TTY delay is calculated, the process is released and sent to a core
void TTY_release(bool isinteractive,node*cu,string *arr,List*pass2){
    core_request(cu,arr,pass2,isinteractive);
}



// TTY request occurs outside of the CPU, This function calculates the time delay caused by TTY
// It then sends the next core task to the core request as interactive. While in TTY, the process is in Blocked state
void TTY_request(int tasktime,node*cu,string *arr,List*pass2){
    sysClock+=tasktime;
    bool isinteractive = true;
    cu->status="Blocked";
    TTY_release(isinteractive,cu,arr,pass2);

}

//Once one SSD command is complete and released the process goes to the core. SSD has a queue, so it will check for
// any SSD requests that are waiting and complete them.
void SSD_release(string *arr,List*pass2,node*cu){

    if(!SSD.empty()){
        node* ssdTask=SSD.front();
        SSD.pop();
        core_request(cu,arr,pass2,false);
    } else
    {
        SSD_free=true;
    }


}
//SSD Commands are processed through this function. While in SSD the process is Blocked
void SSD_request(int time,node *cu,string *arr,List*pass2){
    if(SSD_free){
        SSD_free=false;
        sysClock+=time;
        cu->status="Blocked";
    }
    else
    {
        SSD.push(cu);
    }
    SSD_release(arr,pass2,cu);

}


//Prints a status at every process' arrival
void printArrival(node *cu,node*traverse){
    cout<<"Process "<<cu->PID<<"arrives and starts at time: "<<cu->Start<<" ms"<<endl;
    cout<<"Process Table:"<<endl;
    while(traverse!=NULL){
        cout<<"Process "<<traverse->PID<<" is "<<traverse->status<<endl;
        traverse=traverse->next;
    }


}

//Prints a status at everytime a process' is terminated
void printTerminated(node *cu,node*traverse){

    cout<<"Process "<<cu->PID<<"terminates at time: "<<cu->Start<<" ms"<<endl;
    cout<<"Process Table:"<<endl;
    while(traverse!=NULL){
        cout<<"Process "<<traverse->PID<<" is "<<traverse->status<<endl;
        traverse=traverse->next;

    }

}

int main() {


    // Parse Input File and Create Input Table
    string *array;
    array=new string[500];
    List input_table;
    List *pass = &input_table;

    int instruction_count=parse(pass,array); // Fun returns # of cores

    // Create Process Table
    List process_table;
    List *pass2 = &process_table;
    Process_Table(pass,pass2);

    //Create Device total
    List device_table;
    List *pass3=&device_table;
    Device_Table(pass3);

    sysClock=pass2->head->Start; //initial start time;

    queue<int>  Process;

    node*cu=pass2->head;
    // Queue up the processes

    int i=0;
    int total_num_process=0;
    //This first while loop will setup the arrival of each process into the Processor
    while(cu!=NULL){
        arrival(cu,array,pass2);
        printArrival(pass2->head,cu); // Print status at each arrival
        cu=cu->next;
        i++;
    }

    total_num_process=i;
    int num_of_SSD_access=0;

    node*tasktraverse;
    string taskcommand;
    string tasktimetxt;
    bool isinteractive;
    int tasktime;

    // This while loop will run for the remaining tasks
    while(i<instruction_count){
        tasktraverse=pass2->head; // Will start at the top of the process table and work its way through iterating
        // one task per process at a time
        while(tasktraverse!=NULL){
            // If current line is greater than last line, the process is done and will be labeled as terminated
            if(tasktraverse->Current_Line<=tasktraverse->Last_Line){
                //Parse command from string array so we can use the index and access it directly and quickly
                taskcommand=array[tasktraverse->Current_Line].substr(0, array[tasktraverse->Current_Line].find(' '));
                tasktimetxt =array[tasktraverse->Current_Line].substr(array[tasktraverse->Current_Line].find(' ') + 1);
                tasktime=stoi(tasktimetxt);
                if(taskcommand=="TTY"){ // This is for TTY commands
                    TTY_request(tasktime,tasktraverse,array,pass2);
                    isinteractive=true;
                    core_request(tasktraverse->next,array,pass2,isinteractive);
                }
                else if(taskcommand=="SSD"){ // This is for SSD commands
                    SSD_request(tasktime,tasktraverse,array,pass2);
                    isinteractive=false;
                    core_request(tasktraverse->next,array,pass2,isinteractive);
                    num_of_SSD_access++;


                }
                else{ // This is for Core commands
                    isinteractive=false;
                    core_request(tasktraverse->next,array,pass2,isinteractive);

                }

            }

            tasktraverse=tasktraverse->next;
        }
        printTerminated(pass2->head,cu); // At end of inner while loop, a process will have terminated print status
        i++;
    }


    // Print final report
    cout<<"Summary: "<<endl;
    cout<<"Total elapsed time: "<<sysClock<<" ms"<<endl;
    cout<<"Number of processes that completed: "<<total_num_process<<endl;
    cout<<"Total number of SSD accesses: "<< num_of_SSD_access<<endl;


    return 0;
}