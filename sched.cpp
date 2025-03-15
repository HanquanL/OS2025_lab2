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
        int finishTime;
        int state_ts;    // the time when the process changed its state
        int cpuWaitingTime;    // the time the process has been waiting in the ready queue
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
EventQueue eventQueue;
Process* CURRENT_RUNNING_PROCESS = nullptr;
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
        //newEvent->newState = ProcessStates::CREATED;
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
    FCFS_Scheduler scheduler;
    while(currentEvent = eventQueue.getEvent()){
        Process* currentProcess = currentEvent->process;
        int currentTime = currentEvent->timeStamp;
        simulationFinishTime = currentTime;
        Transition currentTransition = currentEvent->transition;
        // int timeInPrevState = currentTime - currentProcess->state_ts;
        // currentProcess->state_ts = currentTime;
        int prevStateTime = currentProcess->state_ts;
        int timeInPrevState = currentTime - prevStateTime;
        /* For test */
        // cout << "Time: " << currentTime 
        //     << " Process ID: " << currentProcess->processId 
        //     << " Transition: " << (int)currentTransition << endl;
        /* For test */

        switch(currentTransition){
            case Transition::TRANS_TO_READY:{
                // must come from BLOCEKD or CREATED
                // ADD TO RUN QUEUE, NO EVENT CREATED
                //currentProcess->cpuWaitingTime += timeInPrevState;
                // If timeInPrevState is > 0, we assume the process was blocked and has finished I/O.
                if(timeInPrevState > 0) {
                    // Process was in I/O, so update I/O busy time.
                    // Decrement the count of processes in I/O.
                    ioActiveCount--;
                    // If no process is performing I/O now, record the I/O busy period.
                    if(ioActiveCount == 0) {
                        totalIoBusyTime += (currentTime - ioBusyStart);
                    }
                }else{
                    currentProcess->cpuWaitingTime += timeInPrevState;
                }
                currentProcess->state_ts = currentTime; // Update the state timestamp
                scheduler.add_process(currentProcess);
                CALL_SCHEDULER = true;
                break;
            }
            case Transition::TRANS_TO_PREEMPT:{
                // must come from RUNNING
                // ADD TO RUN QUEUE, no event is generated
                //currentProcess->cpuWaitingTime += timeInPrevState;
                int timeSpentRunning = timeInPrevState; // use saved timestamp
                totalCpuBusyTime += timeSpentRunning; // accumulate the CPU busy time
                currentProcess->totalCpuTime -= timeSpentRunning; // Update remaining CPU time
                currentProcess->state_ts = currentTime; // Update the state timestamp
                scheduler.add_process(currentProcess);
                CALL_SCHEDULER = true;
                break;
            }
            case Transition::TRANS_TO_RUN:{
                // create event for either preemption or blocking
                //int waitingTime = currentTime - currentProcess->state_ts;
                int waitingTime = timeInPrevState;
                currentProcess->cpuWaitingTime += waitingTime;
                currentProcess->state_ts = currentTime; // Update the state timestamp
                int rdCpuBurst = mydrndom(currentProcess->cpuBurst);
                int actualCpuBurst = 0;
                // check if the cpu burst is exceeding the remaining cpu time
                if(rdCpuBurst >= currentProcess -> totalCpuTime){
                    actualCpuBurst = currentProcess -> totalCpuTime; // Process will run for the remaining CPU time
                    eventQueue.insertEvent(new Event(currentTime + currentProcess -> totalCpuTime, currentProcess,Transition::TRANS_TO_TERMINATE));
                }else{
                    actualCpuBurst = rdCpuBurst;    // Process will run for rdCpuBurst and then block (or be preempted).
                    eventQueue.insertEvent(new Event(currentTime + rdCpuBurst, currentProcess, Transition::TRANS_TO_BLOCK));
                    //currentProcess->totalCpuTime -= actualCpuBurst; // Update remaining CPU time
                }
                totalCpuBusyTime += actualCpuBurst;    // accumulate the CPU busy time
                break;
            }
            case Transition::TRANS_TO_TERMINATE:{
                // Process finishes execution.
                currentProcess->finishTime = currentTime;
                currentProcess->state_ts = currentTime;
                // If this process was running, clear the running process pointer.
                if (CURRENT_RUNNING_PROCESS == currentProcess) {
                    CURRENT_RUNNING_PROCESS = nullptr;
                }
                break;
            }
            case Transition::TRANS_TO_BLOCK:{
                // create event for when process becomes READY again
                // When the first process enters I/O, mark the start of an I/O busy period.
                if(ioActiveCount == 0) {
                    ioBusyStart = currentTime;
                }
                ioActiveCount++; // Increment the count of processes doing I/O.
                int rdIoBurst = mydrndom(currentProcess->ioBurst);
                currentProcess->ioTime += rdIoBurst;
                eventQueue.insertEvent(new Event(currentTime + rdIoBurst, currentProcess, Transition::TRANS_TO_READY));
                currentProcess->state_ts = currentTime;
                CALL_SCHEDULER = true;
                break;
            }
        }
        delete currentEvent; // Free the memory of the event

        // if(CALL_SCHEDULER && eventQueue.get_next_event_time() != currentTime){
        //     CALL_SCHEDULER = false;
        //     if(CURRENT_RUNNING_PROCESS == nullptr){
        //         CURRENT_RUNNING_PROCESS = scheduler.get_next_process();
        //         if(CURRENT_RUNNING_PROCESS != nullptr){
        //             Event* runEvent = new Event(currentTime, CURRENT_RUNNING_PROCESS, Transition::TRANS_TO_RUN);
        //         }
        //     }
        // }
        if(CALL_SCHEDULER){
            if(eventQueue.get_next_event_time() == currentTime){
                continue;
            }
            CALL_SCHEDULER = false;
            if(CURRENT_RUNNING_PROCESS == nullptr){
                CURRENT_RUNNING_PROCESS = scheduler.get_next_process();
                if(CURRENT_RUNNING_PROCESS != nullptr){
                    Event* runEvent = new Event(currentTime, CURRENT_RUNNING_PROCESS, Transition::TRANS_TO_RUN);
                    eventQueue.insertEvent(runEvent);
                }
            }
        }
    }
}

void printOutcome(){
    double sumTurnaroundTime = 0;
    double sumCpuWaitingTime = 0;
    int processCount = outcomeProcesses.size();
    for(Process* process : outcomeProcesses){
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
        simulationFinishTime, cpuUtilization, ioUtilization, avgTurnaroundTime, avgCpuWaitingTime, throughput);
}
