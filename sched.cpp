#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <queue>
#include <algorithm>
#include "sched.h"

using namespace std;

vector<string> outputTable;
ifstream randomNumbers;
vector<int> randvals;
int randomRange, randomOffset = 0;
int maxPriority = 4;
/*global accounting counters*/
int totalCpuBusyTime = 0;
int totalIoBusyTime = 0;
int simulationFinishTime = 0; // Updated as the simulation ends
int ioActiveCount = 0;        // Number of processes currently doing I/O
int ioBusyStart = 0;          // Timestamp when I/O activity started
/*global accounting counters end*/


class Scheduler{
    protected:
        queue<Process*> runQueue, expiredQueue;
    public:
        int quantum;
        Scheduler(int qtm) : quantum(qtm) {
            this->quantum = qtm;
        }
        virtual ~Scheduler() {}    // virtual destructor
        virtual void add_process(Process* process) = 0;
        virtual void add_expired_process(Process* process) = 0;
        virtual Process* get_next_process() = 0;
        virtual bool unblock_preempt(Process* running, Process* unblocked){ return false; }
        virtual int sizeOfRunQ() { return runQueue.size(); }
        virtual int sizeOfExpQ() { return expiredQueue.size(); }
};

class FCFS_Scheduler : public Scheduler{
    private:
        queue<Process*> runQueue, expiredQueue;
    public:
        string name = "FCFS";
        int quantum;
        FCFS_Scheduler(int qtm) : Scheduler(qtm) {
            this->quantum = qtm;
        }
        ~FCFS_Scheduler() {}
        void add_process(Process* process) override {
            runQueue.push(process);
        }
        void add_expired_process(Process* process) override {
            expiredQueue.push(process);
        }
        Process* get_next_process() override {
            if(runQueue.empty()){
                return nullptr;
            }
            Process* nextProcess = runQueue.front();
            runQueue.pop();
            return nextProcess;
        }
        int sizeOfRunQ() override {
            return runQueue.size();
        }
        int sizeOfExpQ() override {
            return expiredQueue.size();
        }
};

void get_randomNumber();
int mydrndom(int burst);
void simulationLoop();
void printOutcome();
void readInputFile(EventQueue* evenQ, string inputFile);


vector<Process*> outcomeProcesses;

int main(int argc, char *argv[]) {
    string inputFile = argv[1];
    string rfile = argv[2];
    string lineOfProcess;
    EventQueue* eventQueue = new EventQueue();
    FCFS_Scheduler scheduler(10000);
    randomNumbers.open(rfile);
    get_randomNumber();
    // while(getline(readFile, lineOfProcess)){
    //     Process* currentProcess = get_processObj(lineOfProcess);
    //     outcomeProcesses.push_back(currentProcess);
    //     Event* newEvent = new Event(currentProcess->arrivalTime, currentProcess, Transition::TRANS_TO_READY);
    //     eventQueue->insertEvent(newEvent);
    //     // cout << currentProcess.arrivalTime << currentProcess.totalCpuTime << currentProcess.cpuBurst << currentProcess.ioBurst << endl; //for test purposes
    // }
    readInputFile(eventQueue, inputFile);
    //simulationLoop();
    printOutcome();
    // cout << mydrndom(10) << endl; //for test purposes
    // cout << mydrndom(10) << endl; //for test purposes
    // cout << mydrndom(10) << endl; //for test purposes
    // cout << mydrndom(10) << endl; //for test purposes
    // cout << mydrndom(10) << endl; //for test purposes
    // cout << mydrndom(20) << endl; //for test purposes
    // cout << mydrndom(21) << endl; //for test purposes
    // cout << mydrndom(11) << endl; //for test purposes
    // cout << mydrndom(32) << endl; //for test purposes
    // cout << mydrndom(12) << endl; //for test purposes
    // cout << "Processing events: " << endl;
    EventQueue temQ = *eventQueue;
    while(Event* currentEvent = temQ.getEvent()){
        cout << "arrival time: " << currentEvent->get_timestamp()    //for test purposes
            <<" PID: " << currentEvent->get_process()->processId
            << " transition: " << (int)currentEvent->get_transition() 
            << endl;
        temQ.removeEvent();
    }
    return 0;
}

// Process* get_processObj(int processId, string lineOfProcess) {
//     int arrivalTime, totalCpuTime, cpuBurst, ioBurst;
//     istringstream iss(lineOfProcess);
//     iss >> arrivalTime >> totalCpuTime >> cpuBurst >> ioBurst;

//     Process* process = new Process();
//     process->processId = processId;
//     process->arrivalTime = arrivalTime;
//     process->totalCpuTime = totalCpuTime;
//     process->copy_totalCpuTime = totalCpuTime;
//     process->cpuBurst = cpuBurst;
//     process->copy_cpuBurst = cpuBurst;
//     process->ioBurst = ioBurst;
//     process->state_ts = arrivalTime;
//     process->cpuWaitingTime = 0;
//     process->ioTime = 0;
//     process->newState = ProcessStates::CREATED;

//     return process;
// }

void get_randomNumber(){
    string line;
    int randomNumber;
    getline(randomNumbers, line);
    istringstream randRange(line);
    randRange >> randomRange;
    while(getline(randomNumbers, line)){
        istringstream iss(line);
        iss >> randomNumber;
        randvals.push_back(randomNumber);
    }
}

int mydrndom(int burst){
    int number = 1+(randvals[randomOffset]%burst);
    randomOffset = (randomOffset+1)%randomRange;
    return number;
}

// void simulationLoop(){
//     Event* currentEvent;
//     Process* CURRENT_RUNNING_PROCESS = nullptr;
//     int CURRENT_TIME = 0;
//     while(currentEvent = eventQueue->getEvent()){
//         Process *proc = currentEvent->process; //this is the process the event works on
//         CURRENT_TIME = currentEvent->timeStamp;
//         Transition transaction = currentEvent->transition;
//         int timeInPrevState = CURRENT_TIME - proc->state_ts;
//         delete currentEvent;
//         switch(transaction){
//             case Transition::TRANS_TO_READY:{
//                 scheduler.add_process(proc);
//                 break;
//             }
//         }
//     }
   
// }

void printOutcome(){
    double sumTurnaroundTime = 0;
    double sumCpuWaitingTime = 0;
    int processCount = outcomeProcesses.size();
    //cout << scheduler.name << endl;
    for(int i = 0; i < processCount; i++){
        Process* process = outcomeProcesses[i];
        int turnaroundTime = process->finishTime - process->arrivalTime;
        sumTurnaroundTime += turnaroundTime;
        sumCpuWaitingTime += process->cpuWaitingTime;
        printf("%04d: %4d %4d %4d %4d %4d | %5d %5d %5d %5d\n",
            process->processId, process->arrivalTime, process->totalCpuTime, process->cpuBurst, process->ioBurst, process->priority,
            process->finishTime, turnaroundTime, process->ioTime, process->cpuWaitingTime);
    }
    double avgTurnaroundTime = sumTurnaroundTime / processCount;
    double avgCpuWaitingTime = sumCpuWaitingTime / processCount;
    double cpuUtilization = 100.0 * (double) totalCpuBusyTime / simulationFinishTime;
    double ioUtilization = 100.0 * (double) totalIoBusyTime / simulationFinishTime;
    double throughput = 100.0 * (double) processCount / simulationFinishTime;
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
        outcomeProcesses[processCount-1]->finishTime, cpuUtilization, ioUtilization, avgTurnaroundTime, avgCpuWaitingTime, throughput);
}

void readInputFile(EventQueue* evenQ, string inputFile){
    fstream file;
    file.open(inputFile);
    string lineOfProcess;
    int processId = 0;
    int arrivalTime, totalCpuTime, cpuBurst, ioBurst;
    while(file >> arrivalTime >> totalCpuTime >> cpuBurst >> ioBurst){
        int prioiry = mydrndom(maxPriority) - 2;
        Process* currentProcess = new Process(processId, arrivalTime, totalCpuTime, cpuBurst, ioBurst, prioiry);
        outcomeProcesses.push_back(currentProcess);
        Event* newEvent = new Event(currentProcess->arrivalTime, currentProcess, Transition::TRANS_TO_READY);
        evenQ->insertEvent(newEvent);
        processId++;
    }
}
