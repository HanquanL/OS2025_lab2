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
int globalProcessId = 0;
bool CALL_SCHEDULER = false;
int CURRENT_TIME = 0;
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
        int remainingCpuTime;
        int cpuBurst;
        int ioBurst;
        int finishTime;
        int state_ts;    // the time when the process changed its state
        int cpuWaitingTime;    // the time the process has been waiting in the run queue
        int ioTime;    // the time spent in I/O (blocked)
        ProcessStates newState;
        ProcessStates oldState;

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

        int get_next_event_time(){
            if(events.empty()){
                return -1;
            }
            return events.front()->timeStamp;
        }

        bool isEmpty(){
            return events.empty();
        }
};

class Scheduler{
    public:
        virtual ~Scheduler() {}    // virtual destructor
        virtual void add_process(Process* process) = 0;
        virtual Process* get_next_process() = 0;
        virtual bool unblock_preempt(Process* running, Process* unblocked){ return false; }
};

class FCFS_Scheduler : public Scheduler{
    private:
        queue<Process*> runQueue;
    public:
        void add_process(Process* process) override {
            runQueue.push(process);
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
int get_randomNumber();
int mydrndom(int burst);
void simulationLoop();
void printOutcome();
Process* CURRENT_RUNNING_PROCESS = nullptr;
EventQueue eventQueue;
FCFS_Scheduler scheduler;
vector<Process*> outcomeProcesses;

int main(int argc, char *argv[]) {
    
    string inputFile = argv[1];
    string rfile = argv[2];
    ifstream readFile (inputFile);
    string lineOfProcess;
    string stringTotalRandomNumbers;
    randomNumbers.open(rfile);
    getline(randomNumbers, stringTotalRandomNumbers);
    while(getline(readFile, lineOfProcess)){
        Process* currentProcess = get_processObj(lineOfProcess);
        outcomeProcesses.push_back(currentProcess);
        Event* newEvent = new Event();
        newEvent->timeStamp = currentProcess->arrivalTime;
        newEvent->process = currentProcess;
        newEvent->transition = Transition::TRANS_TO_READY;
        eventQueue.insertEvent(newEvent);
        // cout << currentProcess.arrivalTime << currentProcess.totalCpuTime << currentProcess.cpuBurst << currentProcess.ioBurst << endl; //for test purposes
    }
    simulationLoop();
    printOutcome();
    // cout << get_randomNumber() << endl; //for test purposes
    // cout << get_randomNumber() << endl; //for test purposes
    // cout << get_randomNumber() << endl; //for test purposes
    // cout << get_randomNumber() << endl; //for test purposes
    // cout << mydrndom(10) << endl; //for test purposes
    // cout << mydrndom(20) << endl; //for test purposes
    // cout << mydrndom(21) << endl; //for test purposes
    // cout << mydrndom(11) << endl; //for test purposes
    // cout << mydrndom(32) << endl; //for test purposes
    // cout << mydrndom(12) << endl; //for test purposes
    // cout << "Processing events: " << endl;
    // while(!eventQueue.isEmpty()){
    //     Event* event = eventQueue.getEvent();
    //     cout << "Time: " << event -> timeStamp
    //         << " Process ID: " << event -> process -> processId
    //         << " State: " << (int)event -> oldState << endl;
    //     delete event;
    // }
    
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
    process->remainingCpuTime = totalCpuTime;
    process->cpuBurst = cpuBurst;
    process->ioBurst = ioBurst;
    process->state_ts = arrivalTime;
    process->cpuWaitingTime = 0;
    process->ioTime = 0;
    process-> newState = ProcessStates::CREATED;

    return process;
}

int get_randomNumber(){
    string line;
    int randomNumber;
    if (!getline(randomNumbers, line)) {
        // If end of file is reached, reset to the beginning
        randomNumbers.clear(); // Clear EOF flag
        randomNumbers.seekg(0, ios::beg); // Move to the beginning of the file
        // Skip the first line again
        getline(randomNumbers, line);
        getline(randomNumbers, line);
    }
    istringstream iss(line);
    iss >> randomNumber;
    return randomNumber;
}

int mydrndom(int burst){
    return 1+(get_randomNumber()%burst);
}

void simulationLoop(){
    Event* currentEvent;
    
    while(currentEvent = eventQueue.getEvent()){
        Process* currentProcess = currentEvent->process;
        CURRENT_RUNNING_PROCESS = currentProcess;
        Transition currentTransition = currentEvent->transition;
        CURRENT_TIME = currentEvent->timeStamp;
        int timeInPrevState = CURRENT_TIME - currentProcess->state_ts;

        switch(currentTransition){
            case Transition::TRANS_TO_READY:{
                currentProcess->newState = ProcessStates::READY;
                currentProcess->oldState = ProcessStates::CREATED;
                scheduler.add_process(currentProcess);
                CALL_SCHEDULER = true;
                break;
            }
            
            case Transition::TRANS_TO_RUN: {
                int randomCpuBurst = mydrndom(currentProcess->cpuBurst);
                if(currentProcess->remainingCpuTime > 0){
                    int actualBurst = min(randomCpuBurst, currentProcess->remainingCpuTime);
                    currentProcess->finishTime += actualBurst;
                    currentProcess->remainingCpuTime -= actualBurst;
                    currentProcess->newState = ProcessStates::BLOCK;
                    currentProcess->oldState = ProcessStates::RUNNING;
                    currentProcess->state_ts = actualBurst;
                    Event* runEvent = new Event(CURRENT_TIME + actualBurst, currentProcess, Transition::TRANS_TO_BLOCK);
                    eventQueue.insertEvent(runEvent);
                }else{
                    currentProcess->newState = ProcessStates::TERMINATED;
                    currentProcess->oldState = ProcessStates::RUNNING;
                    Event* runEvent = new Event(CURRENT_TIME, currentProcess, Transition::TRANS_TO_TERMINATE);
                    eventQueue.insertEvent(runEvent);
                    CURRENT_RUNNING_PROCESS = nullptr;
                }
                
                break;
            }
            
            case Transition::TRANS_TO_BLOCK: {
                int randomIoBurst = mydrndom(currentProcess->ioBurst);
                currentProcess->ioTime += randomIoBurst;
                currentProcess->finishTime += randomIoBurst;
                currentProcess->remainingCpuTime -= randomIoBurst;
                currentProcess->state_ts = randomIoBurst;
                currentProcess->newState = ProcessStates::READY;
                currentProcess->oldState = ProcessStates::BLOCK;
                Event* runEvent = new Event(CURRENT_TIME + randomIoBurst, currentProcess, Transition::TRANS_TO_RUN);
                eventQueue.insertEvent(runEvent);
                CALL_SCHEDULER = true;
                break;
            }
            
            case Transition::TRANS_TO_PREEMPT: {
                
                CALL_SCHEDULER = true;
                break;
            }
            case Transition::TRANS_TO_TERMINATE:{
                
                break;
            }
        }
        delete currentEvent; // Free the memory of the event

        if(CALL_SCHEDULER){
            CALL_SCHEDULER = false;
            if(CURRENT_RUNNING_PROCESS == nullptr){
                CURRENT_RUNNING_PROCESS = scheduler.get_next_process();
            }else if(CURRENT_RUNNING_PROCESS != nullptr){
                Event* runEvent = new Event(CURRENT_TIME, CURRENT_RUNNING_PROCESS, Transition::TRANS_TO_RUN);
                eventQueue.insertEvent(runEvent);
            }
        }
    }
}

void printOutcome(){
    double sumTurnaroundTime = 0;
    double sumCpuWaitingTime = 0;
    int processCount = outcomeProcesses.size();
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
