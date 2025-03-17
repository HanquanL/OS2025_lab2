#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <queue>
#include <algorithm>

using namespace std;

vector<string> outputTable;
ifstream randomNumbers;
vector<int> randvals;
int randomRange, randomOffset = 0;
int globalProcessId = 0;
/*global accounting counters*/
int totalCpuBusyTime = 0;
int totalIoBusyTime = 0;
int simulationFinishTime = 0; // Updated as the simulation ends
int ioActiveCount = 0;        // Number of processes currently doing I/O
int ioBusyStart = 0;          // Timestamp when I/O activity started
/*global accounting counters end*/

enum class ProcessStates{
    CREATED,
    READY,
    RUNNING,
    BLOCK,
    TERMINATED  
};

class Process {
    public:
        int processId;
        int arrivalTime;
        int totalCpuTime;
        int cpuBurst;
        int ioBurst;
        int copy_arrivalTime;
        int copy_totalCpuTime;
        int copy_cpuBurst;
        int finishTime;
        int state_ts;    // the time when the process changed its state
        int cpuWaitingTime;    // the time the process has been waiting in the run queue
        int ioTime;    // the time spent in I/O blocked
        ProcessStates State;
        int priority;
        int cuurentPriority;
};

enum class Transition{
    TRANS_TO_READY,
    TRANS_TO_RUN,
    TRANS_TO_BLOCK,
    TRANS_TO_PREEMPT,
    TRANS_TO_TERMINATE
};

class Event {
    public:
        int timeStamp;
        Process* process;
        Transition transition;
        Event() {}
        Event(int ts, Process* p, Transition trans) : timeStamp(ts), process(p), transition(trans) {}
};

class EventQueue{
    private:
        list<Event*> events;
    
    public:
        void insertEvent(Event* event){
            auto it = events.begin();
            while(it != events.end() && (*it)->timeStamp < event->timeStamp){
                it++;
            }
            events.insert(it, event);
        }

        Event* getEvent(){
            if(events.empty()){
                return nullptr;
            }
            Event* event = events.front();
            events.pop_front();
            return event;
        }

        bool removeEvent(Event* event){
            for(auto it = events.begin(); it != events.end(); it++){
                if(*it == event){
                    events.erase(it);
                    delete *it;
                    return true;
                }
            }
            return false;
        }

        bool isEmpty(){
            return events.empty();
        }
};

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
};

Process* get_processObj(string lineOfProcess);
void get_randomNumber();
int mydrndom(int burst);
void simulationLoop();
void printOutcome();
EventQueue eventQueue;
FCFS_Scheduler scheduler(10000);
vector<Process*> outcomeProcesses;

int main(int argc, char *argv[]) {
    
    string inputFile = argv[1];
    string rfile = argv[2];
    ifstream readFile (inputFile);
    string lineOfProcess;
    randomNumbers.open(rfile);
    get_randomNumber();
    while(getline(readFile, lineOfProcess)){
        Process* currentProcess = get_processObj(lineOfProcess);
        outcomeProcesses.push_back(currentProcess);
        Event* newEvent = new Event(currentProcess->arrivalTime, currentProcess, Transition::TRANS_TO_READY);
        eventQueue.insertEvent(newEvent);
        // cout << currentProcess.arrivalTime << currentProcess.totalCpuTime << currentProcess.cpuBurst << currentProcess.ioBurst << endl; //for test purposes
    }
    //simulationLoop();
    //printOutcome();
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
    while(!eventQueue.isEmpty()){
        Event* event = eventQueue.getEvent();
        cout << "Time: " << event -> timeStamp
            << " Process ID: " << event -> process -> processId
            << " State: " << (int)event -> transition << endl;
        delete event;
    }
    
    return 0;
}

Process* get_processObj(string lineOfProcess) {
    int arrivalTime, totalCpuTime, cpuBurst, ioBurst;
    istringstream iss(lineOfProcess);
    iss >> arrivalTime >> totalCpuTime >> cpuBurst >> ioBurst;

    Process* process = new Process();
    process->processId = globalProcessId++;
    process->arrivalTime = arrivalTime;
    process->totalCpuTime = totalCpuTime;
    process->copy_totalCpuTime = totalCpuTime;
    process->cpuBurst = cpuBurst;
    process->copy_cpuBurst = cpuBurst;
    process->ioBurst = ioBurst;
    process->state_ts = arrivalTime;
    process->cpuWaitingTime = 0;
    process->ioTime = 0;
    process->State = ProcessStates::CREATED;

    return process;
}

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

void simulationLoop(){
    Event* currentEvent;
    Process* CURRENT_RUNNING_PROCESS = nullptr;
    int CURRENT_TIME = 0;
   
}

void printOutcome(){
    double sumTurnaroundTime = 0;
    double sumCpuWaitingTime = 0;
    int processCount = outcomeProcesses.size();
    cout << scheduler.name << endl;
    for(int i = 0; i < processCount; i++){
        Process* process = outcomeProcesses[i];
        int turnaroundTime = process->finishTime - process->arrivalTime;
        sumTurnaroundTime += turnaroundTime;
        sumCpuWaitingTime += process->cpuWaitingTime;
        printf("%04d: %4d %4d %4d %4d | %5d %5d %5d %5d\n",
            process->processId, process->arrivalTime, process->totalCpuTime, process->cpuBurst, process->ioBurst,
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
