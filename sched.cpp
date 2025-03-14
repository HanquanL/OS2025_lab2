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

class Process {
    public:
        int processId;
        int arrivalTime;
        int totalCpuTime;
        int cpuBurst;
        int ioBurst;
        int finishTime;
        int state_ts;

};

enum class ProcessStates{
    CREATED,
    READY,
    RUNNING,
    BLOCK,
    TERMINATED  
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
        ProcessStates newState;
        ProcessStates oldState;
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
int globalProcessId = 0;
EventQueue eventQueue;
bool CALL_SCHEDULER = false;
Process* CURRENT_RUNNING_PROCESS = nullptr;

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
        Event* newEvent = new Event();
        newEvent->timeStamp = currentProcess->arrivalTime;
        newEvent->process = currentProcess;
        newEvent->newState = ProcessStates::CREATED;
        newEvent->transition = Transition::TRANS_TO_READY;
        eventQueue.insertEvent(newEvent);
        // cout << currentProcess.arrivalTime << currentProcess.totalCpuTime << currentProcess.cpuBurst << currentProcess.ioBurst << endl; //for test purposes
    }
    simulationLoop();
    // cout << get_randomNumber() << endl; //for test purposes
    // cout << get_randomNumber() << endl; //for test purposes
    // cout << get_randomNumber() << endl; //for test purposes
    // cout << get_randomNumber() << endl; //for test purposes
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
        Transition currentTransition = currentEvent->transition;
        int timeInPrevState = currentTime - currentProcess->state_ts;
        /* For test */
        // cout << "Time: " << currentTime 
        //     << " Process ID: " << currentProcess->processId 
        //     << " Transition: " << static_cast<int>(currentTransition) << endl;
        /* For test */
        delete currentEvent;

        switch(currentTransition){
            case Transition::TRANS_TO_READY:{
                // must come from BLOCEKD or CREATED
                // ADD TO RUN QUEUE, NO EVENT CREATED
                scheduler.add_process(currentProcess);
                CALL_SCHEDULER = true;
                break;
            }
            case Transition::TRANS_TO_PREEMPT:{
                // must come from RUNNING
                // ADD TO RUN QUEUE, no event is generated
                scheduler.add_process(currentProcess);
                CALL_SCHEDULER = true;
                break;
            }
            case Transition::TRANS_TO_RUN:{
                // create event for either preemption or blocking
                int rdCpuBurst = mydrndom(currentProcess->cpuBurst);
                if(rdCpuBurst >= currentProcess -> totalCpuTime){
                    eventQueue.insertEvent(new Event(currentTime + currentProcess -> totalCpuTime, currentProcess,Transition::TRANS_TO_TERMINATE));
                }else{
                    eventQueue.insertEvent(new Event(currentTime + rdCpuBurst, currentProcess, Transition::TRANS_TO_BLOCK));
                }
                break;
            }
            case Transition::TRANS_TO_BLOCK:{
                // create event for when process becomes READY again
                int rdIoBurst = mydrndom(currentProcess->ioBurst);
                eventQueue.insertEvent(new Event(currentTime + rdIoBurst, currentProcess, Transition::TRANS_TO_READY));
                CALL_SCHEDULER = true;
                break;
            }
        }

        if(CALL_SCHEDULER && eventQueue.get_next_event_time() != currentTime){
            CALL_SCHEDULER = false;
            if(CURRENT_RUNNING_PROCESS == nullptr){
                CURRENT_RUNNING_PROCESS = scheduler.get_next_process();
                if(CURRENT_RUNNING_PROCESS != nullptr){
                    Event* runEvent = new Event(currentTime, CURRENT_RUNNING_PROCESS, Transition::TRANS_TO_RUN);
                }
            }
        }
    }
}
