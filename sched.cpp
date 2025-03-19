#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <queue>
#include <algorithm>
#include <unistd.h>
#include "sched.h"

using namespace std;


ifstream randomNumbers;
vector<int> randvals;
int randomRange, randomOffset = 0;
int ioTime = 0;
int maxPriority = 4;
bool ifPrio = false;
bool verbose = false;
deque<Event*> eventQueue;
vector<Process*> outcomeProcesses;
void get_randomNumber();
int mydrndom(int burst);
void simulationLoop(Scheduler* scheduler, int verbose);
void printOutcome(Scheduler* scheduler);
void readInputFile(deque<Event*> *eventQueue, string inputFile);
int getActualAT(EventQueue* eventQueue, int pid);
void removeDuplicate(EventQueue* eventQueue, int pid);
void preemptRunningProcess(Process* currentRunningProcess, Process* proc, int currentTime, Scheduler* scheduler);
deque <Event*> :: iterator  findEvent(deque<Event*> *eventQueue, Process *process);
void insertSortedQ(deque<Event*> *eventQueue, Event *event);
Event* getEvent(deque<Event*> *eventQueue);
int getNextEventTime(deque<Event*> *eventQueue);
bool compareInterval(Process* i1, Process* i2);

int main(int argc, char *argv[]) {
    string inputFile = argv[argc-2];
    string rfile = argv[argc-1];
    string lineOfProcess;
    string schedulerType;
    int c;
    //EventQueue* eventQueue = new EventQueue();
    Scheduler* scheduler;
    randomNumbers.open(rfile);
  
    while((c = getopt(argc, argv, "v::s:")) != -1){
        switch(c){
            case 'v':
                verbose = true;
                break;
            case 's':
                schedulerType = optarg;
                break;
            default:
                break;
        }
    }
    switch (schedulerType.at(0)){
        case 'F':{
            scheduler = new FCFS_Scheduler(10000);
            break;
        }
        case 'L':{
            scheduler = new LCFS_Scheduler(10000);
            break;
        }
        case 'S':{
            scheduler = new SRTF_Scheduler(10000);
            break;
        }
        case 'R':{
            scheduler = new RR_Scheduler(atoi(schedulerType.substr(1).c_str()));
            break;
        }
        case 'P':{
            if(schedulerType.size() > 3){
                maxPriority = atoi(schedulerType.substr(3).c_str());
            }
            scheduler = new PRIO_Scheduler(atoi(schedulerType.substr(1).c_str()), maxPriority);
            ifPrio = true;
            break;
        }
        case 'E':{
            if(schedulerType.size() > 3){
                maxPriority = atoi(schedulerType.substr(3).c_str());
            }
            scheduler = new Pre_PRIO_Scheduler(atoi(schedulerType.substr(1).c_str()), maxPriority);
            ifPrio = true;
            break;
        }
    }
    get_randomNumber();
    // cout << mydrndom(10) << endl; //for test purposes
    // cout << mydrndom(10) << endl; //for test purposes
    // cout << mydrndom(10) << endl; //for test purposes
    // cout << mydrndom(10) << endl; //for test purposes
    // cout << mydrndom(10) << endl; //for test purposes
    readInputFile(&eventQueue, inputFile);
    // for(auto event : eventQueue){    //for test purposes
    //     cout << event->get_timestamp() << " " << event->get_process()->processId << " " << transitionToString(event->get_transition()) << endl;
    // }
    simulationLoop( scheduler, verbose);
    //cout << outcomeProcesses.size() << endl;
    printOutcome(scheduler);
    
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

void simulationLoop(Scheduler* scheduler, int verbose){
    Event* currentEvent;
    Process* CURRENT_RUNNING_PROCESS = nullptr;
    int overlap = 0;
    int prevIOTime = 0;
    int currentIOTime = 0; 
    while(currentEvent = getEvent(&eventQueue)){
        Process *proc = currentEvent->get_process(); //this is the process the event works on
        Event* tempEvent;
        int CURRENT_TIME = currentEvent->get_timestamp();
        int timeInPrevState = CURRENT_TIME - proc->state_ts;
        Transition transaction = currentEvent->get_transition();
        bool CALL_SCHEDULER = false;
        int random_cpu_burst = 0;
        int cpu_burst = 0;
        switch(transaction){
            case Transition::TRANS_TO_READY :{
                // must come from BLOCKED or CREATED
                // add to run queue, no event created
                if(verbose){
                    cout << CURRENT_TIME << " " << proc->processId << " " << timeInPrevState << ": " << processStateToString(proc->newState) << " -> READY\n";
                }
                proc->newState = ProcessStates::READY;
                scheduler->add_process(proc);
                preemptRunningProcess(CURRENT_RUNNING_PROCESS, proc, CURRENT_TIME, scheduler);
                proc->state_ts = CURRENT_TIME;
                CALL_SCHEDULER = true;
                break;
            }
            case Transition::TRANS_TO_RUN:{
                // create event for either preemption or blocking
                if(CURRENT_RUNNING_PROCESS->copy_cpuBurst > 0){
                    random_cpu_burst = CURRENT_RUNNING_PROCESS->copy_cpuBurst;
                    CURRENT_RUNNING_PROCESS->copy_cpuBurst = 0;
                }else{
                    random_cpu_burst = mydrndom(CURRENT_RUNNING_PROCESS->cpuBurst);
                }

                if(random_cpu_burst > CURRENT_RUNNING_PROCESS->copy_totalCpuTime){
                    random_cpu_burst = CURRENT_RUNNING_PROCESS->copy_totalCpuTime;
                }

                tempEvent = new Event();
                //tempEvent->get_process()->newState = ProcessStates::RUNNING;
                if(scheduler->quantum != -1){
                    if(random_cpu_burst > scheduler->quantum){
                        CURRENT_RUNNING_PROCESS->copy_cpuBurst = random_cpu_burst - scheduler->quantum;
                        cpu_burst = scheduler->quantum;
                        tempEvent->set_transition(Transition::TRANS_TO_PREEMPT);
                    }else{
                        CURRENT_RUNNING_PROCESS->copy_cpuBurst = 0;
                        tempEvent->set_transition(Transition::TRANS_TO_BLOCK);
                        cpu_burst = random_cpu_burst;
                    }
                }else{
                    cpu_burst = random_cpu_burst;
                    tempEvent->set_transition(Transition::TRANS_TO_BLOCK);
                }
                if(verbose){
                    cout << CURRENT_TIME << " " << CURRENT_RUNNING_PROCESS->processId << " " << timeInPrevState << ": " << processStateToString(CURRENT_RUNNING_PROCESS->newState) 
                    << " -> RUNNG cb=" << cpu_burst << " rem=" << CURRENT_RUNNING_PROCESS->copy_totalCpuTime << "\n";
                }

                CURRENT_RUNNING_PROCESS->runTime += cpu_burst;
                CURRENT_RUNNING_PROCESS->cpuWaitingTime += timeInPrevState;
                CURRENT_RUNNING_PROCESS = proc;
                CURRENT_RUNNING_PROCESS->copy_totalCpuTime -= cpu_burst;
                CURRENT_RUNNING_PROCESS->state_ts = CURRENT_TIME;

                tempEvent->set_timestamp(CURRENT_TIME + cpu_burst);
                tempEvent->set_process(CURRENT_RUNNING_PROCESS);
                //tempEvent->set_transition(Transition::
                insertSortedQ(&eventQueue, tempEvent);
                break;
            }
            case Transition::TRANS_TO_BLOCK:{
                //create an event for when process becomes READY again
                if(CURRENT_RUNNING_PROCESS->copy_totalCpuTime ==0){
                    tempEvent = new Event();
                    tempEvent->set_timestamp(currentEvent->get_timestamp());
                    tempEvent->set_process(proc);
                    tempEvent->set_transition(Transition::TRANS_TO_TERMINATE);
                    eventQueue.insert(eventQueue.begin(), tempEvent);
                    CURRENT_RUNNING_PROCESS = nullptr;
                    break;
                }
                CURRENT_RUNNING_PROCESS->cuurentPriority = CURRENT_RUNNING_PROCESS->priority-1;
                random_cpu_burst = mydrndom(CURRENT_RUNNING_PROCESS->ioBurst);
                CURRENT_RUNNING_PROCESS->ioTime += random_cpu_burst;

                if(CURRENT_TIME + random_cpu_burst > currentIOTime ){
                    ioTime += currentIOTime - (prevIOTime + overlap);
                    if(currentIOTime != 0){
                        overlap = currentIOTime - CURRENT_TIME;
                        if(overlap < 0){
                            overlap = 0;
                        }
                    }
                    currentIOTime = random_cpu_burst + CURRENT_TIME;
                    prevIOTime = CURRENT_TIME;
                }

                CURRENT_RUNNING_PROCESS->state_ts = CURRENT_TIME;

                if(verbose){
                    cout << CURRENT_TIME << " " << CURRENT_RUNNING_PROCESS->processId << " " << timeInPrevState << ": " << processStateToString(CURRENT_RUNNING_PROCESS->newState) 
                    << " -> BLOCK ib=" << random_cpu_burst << " rem=" << CURRENT_RUNNING_PROCESS->copy_totalCpuTime << "\n";
                }

                tempEvent = new Event();
                tempEvent->set_transition(Transition::TRANS_TO_READY);
                tempEvent->set_process(CURRENT_RUNNING_PROCESS);
                tempEvent->get_process()->newState = ProcessStates::BLOCK;
                tempEvent->get_process()->oleState = ProcessStates::RUNNING;
                tempEvent->set_timestamp(CURRENT_TIME + random_cpu_burst);
                insertSortedQ(&eventQueue, tempEvent);
                CALL_SCHEDULER = true;
                CURRENT_RUNNING_PROCESS = nullptr;
                break;
            }
            case Transition::TRANS_TO_PREEMPT:{
                // must come from RUNNING (preemption)
                // add to runqueue (no event is generated)
                if(verbose){
                    cout << CURRENT_TIME << " " << proc->processId << " " << timeInPrevState << ": "
                        << transitionToString(currentEvent->get_transition()) << " -> " << "READY  cb=" << CURRENT_RUNNING_PROCESS->copy_cpuBurst << " rem="
                        << CURRENT_RUNNING_PROCESS->copy_totalCpuTime << " prio=" << CURRENT_RUNNING_PROCESS->cuurentPriority << endl;
                }
                if(ifPrio){
                    proc->cuurentPriority--;
                }
                proc->state_ts = CURRENT_TIME;
                scheduler->add_process(proc);
                CURRENT_RUNNING_PROCESS = nullptr;
                CALL_SCHEDULER = true;
                break;
            }
            case Transition::TRANS_TO_TERMINATE:{
                if(verbose){
                    cout << CURRENT_TIME << " " << proc->processId << " " << timeInPrevState << ": " << "Done" << endl;
                }
                Process* finishedProcess = currentEvent->get_process();
                finishedProcess->finishTime = CURRENT_TIME;
                finishedProcess->turnarTime = CURRENT_TIME - finishedProcess->arrivalTime;
                finishedProcess->newState = ProcessStates::TERMINATED;
                outcomeProcesses.push_back(finishedProcess);
                CALL_SCHEDULER = true;
                break;
            }
        }
        if(CALL_SCHEDULER){
            if(getNextEventTime(&eventQueue) == CURRENT_TIME){
                continue;
            }
            CALL_SCHEDULER = false;
            if(CURRENT_RUNNING_PROCESS == nullptr){
                CURRENT_RUNNING_PROCESS = scheduler->get_next_process();
                if(CURRENT_RUNNING_PROCESS == nullptr){
                    continue;
                }
                Event* tempEvent = new Event();
                tempEvent->set_timestamp(CURRENT_TIME);
                tempEvent->set_process(CURRENT_RUNNING_PROCESS);
                tempEvent->set_transition(Transition::TRANS_TO_RUN);
                tempEvent->get_process()->newState = ProcessStates::READY;
                insertSortedQ(&eventQueue, tempEvent);

            }
            delete currentEvent;
            currentEvent = nullptr;
        }
    }
    ioTime += currentIOTime - (prevIOTime + overlap);
}
void printOutcome(Scheduler* scheduler){
    if(outcomeProcesses.empty()){
        return;
    }
    sort(outcomeProcesses.begin(), outcomeProcesses.end(), compareInterval);
    double sumTurnaroundTime = 0;
    double sumCpuWaitingTime = 0;
    double sumIoTime = 0;
    double sumCpuTime = 0;
    double size = outcomeProcesses.size();
    int lastFinishTime = outcomeProcesses[outcomeProcesses.size()-1]->finishTime;
    //int priority;
    cout << scheduler->getSchedulerName() << endl;
    for(int i = 0; i < outcomeProcesses.size(); i++){
        Process* proc = outcomeProcesses[i];
        if(proc->finishTime > lastFinishTime){
            lastFinishTime = proc->finishTime;
        }
        sumCpuTime += proc->totalCpuTime;
        //sumIoTime += proc->ioTime;
        sumTurnaroundTime += proc->finishTime - proc->arrivalTime;
        sumCpuWaitingTime += proc->cpuWaitingTime;
        //priority = ifPrio ? proc->priority+1 : proc->priority+2;
        printf("%04d: %4d %4d %4d %4d %4d | %5d %5d %5d %5d\n",
            proc->processId, proc->arrivalTime, proc->totalCpuTime, proc->cpuBurst, proc->ioBurst, proc->priority,
            proc->finishTime, (proc->finishTime - proc->arrivalTime), proc->ioTime, proc->cpuWaitingTime);
    }
    double avgTurnaroundTime = sumTurnaroundTime / size;
    double avgCpuWaitingTime = sumCpuWaitingTime / size;
    double cpuUtilization = 100.0 * (double) sumCpuTime / lastFinishTime;
    double ioUtilization = 100.0 * (double) ioTime / lastFinishTime;
    double throughput = 100.0 * (double) size / lastFinishTime;
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
        lastFinishTime, cpuUtilization, ioUtilization, avgTurnaroundTime, avgCpuWaitingTime, throughput);
}

void readInputFile(deque<Event*> *eventQueue, string inputFile){
    fstream file;
    file.open(inputFile);
    string lineOfProcess;
    int processId = 0;
    int arrivalTime, totalCpuTime, cpuBurst, ioBurst;
    while(file >> arrivalTime >> totalCpuTime >> cpuBurst >> ioBurst){
        //int prioiry = mydrndom(maxPriority) - 2;
        int prioiry = mydrndom(maxPriority);
        Process* currentProcess = new Process(processId, arrivalTime, totalCpuTime, cpuBurst, ioBurst, prioiry);
        Event* newEvent = new Event(currentProcess->arrivalTime, currentProcess, Transition::TRANS_TO_READY);
        insertSortedQ(eventQueue, newEvent);
        processId++;
    }
}

int getActualAT(EventQueue* eventQueue, int pid){
    int actualAT = 0;
    for(auto it = eventQueue->events.begin(); it != eventQueue->events.end(); it++){
        if((*it)->get_process()->processId == pid){
            actualAT = (*it)->get_timestamp();
        }
    }
    return actualAT;
}

void removeDuplicate(EventQueue* eventQueue, int pid){
    for(auto it = eventQueue->events.begin(); it != eventQueue->events.end();){
        if((*it)->get_process()->processId == pid){
            it = eventQueue->events.erase(it);
        }else{
            ++it;
        }
    }
}

void preemptRunningProcess(Process* currentRunningProcess, Process* proc, int currentTime, Scheduler* scheduler){
    if(scheduler->isPreempt && currentRunningProcess != nullptr && !eventQueue.empty()){
        deque<Event*> :: iterator it = findEvent(&eventQueue, currentRunningProcess);
        Event *event = *it;
        if(verbose){
            printf("---> PRIO preemption %d by %d ? %d TS=%d now=%d", currentRunningProcess->processId,
                    proc->processId, proc->cuurentPriority > currentRunningProcess->cuurentPriority,
                    event->get_timestamp(), currentTime);
        }
        if(proc->cuurentPriority > currentRunningProcess->cuurentPriority && (*it)->get_timestamp() >currentTime){
            if(verbose){
                cout << " --> YES" << endl;
            } 
            eventQueue.erase(it);
        }
        int diffTime = event->get_timestamp() - currentTime;
        currentRunningProcess->copy_cpuBurst += diffTime;
        currentRunningProcess->copy_totalCpuTime += diffTime;
        currentRunningProcess->runTime += diffTime;
        event->set_timestamp(currentTime);
        event->set_transition(Transition::TRANS_TO_PREEMPT);
        insertSortedQ(&eventQueue, event);
    }else{
        if(verbose){
            cout << " --> NO" << endl;
        }
    }
}

deque <Event*> :: iterator  findEvent(deque<Event*> *eventQueue, Process *process){
    deque<Event*> :: iterator it;
    for(it = eventQueue->begin(); it != eventQueue->end(); it++){
        if((*it)->get_process()->processId == process->processId){
            return it;
        }
    }
    return it;
}

void insertSortedQ(deque<Event*> *eventQueue, Event *event){
    if(eventQueue->empty()){
        eventQueue->push_back(event);
        return;
    }else{
        for(int i = 0; i < eventQueue->size(); i++){
            // if((*eventQueue)[i]->get_timestamp() > event->get_timestamp()){
            //     eventQueue->insert(eventQueue->begin() + i, event);
            //     return;
            // }
            if(eventQueue->at(i)->get_timestamp() > event->get_timestamp()){
                eventQueue->insert(eventQueue->begin() + i, event);
                return;
            }
        }
        eventQueue->push_back(event);
    }
    
}

Event* getEvent(deque<Event*> *eventQueue){
    if(eventQueue->empty()){
        return nullptr;
    }
    Event* event = eventQueue->front();
    eventQueue->pop_front();
    return event;
}

int getNextEventTime(deque<Event*> *eventQueue){
    if(eventQueue->empty())
        return -1;
    Event *evt = eventQueue->front();
    return evt->get_timestamp();
}

bool compareInterval(Process* i1, Process* i2)
{
    return (i1->processId < i2->processId);
}