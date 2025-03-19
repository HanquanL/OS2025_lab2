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

vector<string> outputTable;
ifstream randomNumbers;
vector<int> randvals;
int randomRange, randomOffset = 0;
int maxPriority = 4;
bool ifPrio = false;
void get_randomNumber();
int mydrndom(int burst);
void simulationLoop(EventQueue* eventQueue, Scheduler* scheduler, int verbose);
void printOutcome(Scheduler* scheduler);
void readInputFile(EventQueue* evenQ, string inputFile);
int getActualAT(EventQueue* eventQueue, int pid);
void removeDuplicate(EventQueue* eventQueue, int pid);

int main(int argc, char *argv[]) {
    string inputFile = argv[argc-2];
    string rfile = argv[argc-1];
    string lineOfProcess;
    string schedulerType;
    bool verbose = false;
    int c;
    EventQueue* eventQueue = new EventQueue();
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
    // while(getline(readFile, lineOfProcess)){
    //     Process* currentProcess = get_processObj(lineOfProcess);
    //     outcomeProcesses.push_back(currentProcess);
    //     Event* newEvent = new Event(currentProcess->arrivalTime, currentProcess, Transition::TRANS_TO_READY);
    //     eventQueue->insertEvent(newEvent);
    //     // cout << currentProcess.arrivalTime << currentProcess.totalCpuTime << currentProcess.cpuBurst << currentProcess.ioBurst << endl; //for test purposes
    // }
    get_randomNumber();
    readInputFile(eventQueue, inputFile);
    simulationLoop(eventQueue, scheduler, verbose);
    printOutcome(scheduler);
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

void simulationLoop(EventQueue* eventQueue, Scheduler* scheduler, int verbose){
    Event* currentEvent;
    Process* CURRENT_RUNNING_PROCESS = nullptr;
    int CURRENT_TIME = 0;
    int breakTime = -1;
    int breakPrio = 0;
    list<Process*> tempList;
    while(currentEvent = eventQueue->getEvent()){
        CURRENT_TIME = currentEvent->get_timestamp();
        Process *proc = currentEvent->get_process(); //this is the process the event works on
        Transition transaction = currentEvent->get_transition();
        bool CALL_SCHEDULER = false;
        int timeInPrevState = CURRENT_TIME - proc->state_ts;
        eventQueue->removeEvent();
        ProcessStates preState;
        switch(transaction){
            case Transition::TRANS_TO_READY:{
                preState = proc->newState;
                if(proc->copy_totalCpuTime ==0){
                    if(verbose){
                        cout << CURRENT_TIME << " " << proc->processId << " " << timeInPrevState << ": " << "Done" << endl;
                    }
                    CALL_SCHEDULER = true;    //process is done so we need to call the scheduler to get next process
                    proc->finishTime = CURRENT_TIME;
                    proc->newState = ProcessStates::TERMINATED;
                    scheduler->add_expired_process(proc);
                    if(CURRENT_RUNNING_PROCESS != nullptr && proc->processId == CURRENT_RUNNING_PROCESS->processId){
                        CURRENT_RUNNING_PROCESS = nullptr;
                    }
                    break;
                }

                if(proc->newState == ProcessStates::BLOCK || proc->newState == ProcessStates::CREATED){
                    if(verbose){
                        cout<< CURRENT_TIME << " " << proc->processId << " " << timeInPrevState << ": " << processStateToString(proc->newState) << " -> READY\n";
                    }
                }else{
                    if(verbose){
                        cout<< CURRENT_TIME << " " << proc->processId << " " << timeInPrevState << ": " << processStateToString(proc->newState)<< " -> READY  cb=" << proc->copy_cpuBurst 
                        << " rem=" << proc->copy_totalCpuTime <<" prio=" << (proc->cuurentPriority+1)<< "\n";
                    }
                }

                if(CURRENT_RUNNING_PROCESS != nullptr && proc->processId == CURRENT_RUNNING_PROCESS->processId){
                   CURRENT_RUNNING_PROCESS = nullptr;
                }
                if(breakTime == CURRENT_TIME && scheduler->isPreempt && proc->newState == ProcessStates::BLOCK){
                    if(verbose){
                        cout << proc->processId << " " << proc->cuurentPriority << " " << breakPrio << endl;
                }

                proc->newState = ProcessStates::READY;
                proc->state_ts = CURRENT_TIME;

                if(CURRENT_RUNNING_PROCESS != nullptr && scheduler->isPreempt){
                    int actualAT = getActualAT(eventQueue, CURRENT_RUNNING_PROCESS->processId);

                    bool highPrio = (proc->priority > CURRENT_RUNNING_PROCESS->cuurentPriority+1);
                    if(proc->priority > CURRENT_RUNNING_PROCESS->cuurentPriority+1 && actualAT > CURRENT_TIME){
                        CURRENT_RUNNING_PROCESS->copy_arrivalTime = CURRENT_TIME;
                        if(actualAT != 0){
                            CURRENT_RUNNING_PROCESS->copy_cpuBurst += (actualAT - CURRENT_TIME);
                            CURRENT_RUNNING_PROCESS->copy_totalCpuTime += (actualAT - CURRENT_TIME);
                        }
                        if(CURRENT_RUNNING_PROCESS-> copy_cpuBurst > CURRENT_RUNNING_PROCESS -> copy_totalCpuTime){
                            CURRENT_RUNNING_PROCESS->copy_cpuBurst = CURRENT_RUNNING_PROCESS->copy_totalCpuTime;
                        }
                        removeDuplicate(eventQueue, CURRENT_RUNNING_PROCESS->processId);
                        if(verbose){
                            cout<< "---> PRIO preemption "<< CURRENT_RUNNING_PROCESS->processId<<" by "<<proc->processId
                        <<" ? "<< highPrio <<" TS="<<actualAT<<" now="<<CURRENT_TIME<<") --> YES\n";
                        }
                        eventQueue->insertEvent(new Event(actualAT, CURRENT_RUNNING_PROCESS, Transition::TRANS_TO_READY));
                        for(auto it = tempList.begin(); it != tempList.end(); it++){
                            (*it)->copy_arrivalTime = CURRENT_TIME;
                        }
                        tempList.clear();
                        breakTime = CURRENT_TIME;
                        CURRENT_RUNNING_PROCESS = nullptr;
                    }else{
                        if(verbose){
                            cout<< "---> PRIO preemption "<< CURRENT_RUNNING_PROCESS->processId<<" by "<<proc->processId
                        <<" ? "<<highPrio<<" TS="<<actualAT<<" now="<<CURRENT_TIME<<") --> NO\n";
                        }
                        if(CURRENT_TIME == actualAT){
                            proc->copy_arrivalTime = actualAT;
                            for(auto it = tempList.begin(); it != tempList.end(); it++){
                                (*it)->copy_arrivalTime = CURRENT_TIME;
                            }
                            tempList.clear();
                        }else{
                            tempList.push_back(proc);
                        }
                    }
                }

                scheduler->add_process(proc);
                CALL_SCHEDULER = true;
                break;
            }
            case Transition::TRANS_TO_RUN:{
                int actual_cpu_burst;
                preState = proc->newState;
                if(proc->copy_cpuBurst ==0){
                    actual_cpu_burst = mydrndom(proc->cpuBurst);
                    proc->copy_cpuBurst = actual_cpu_burst;
                }else{
                    actual_cpu_burst = proc->copy_cpuBurst;
                }

                if(actual_cpu_burst > proc->copy_totalCpuTime){
                    actual_cpu_burst = proc->copy_totalCpuTime;
                    proc->copy_cpuBurst = actual_cpu_burst;
                }

                proc->state_ts = CURRENT_TIME;
                proc->newState = ProcessStates::RUNNING;
                proc->cpuWaitingTime += timeInPrevState;
                
                if(ifPrio){
                    proc->cuurentPriority--;
                }

                if(actual_cpu_burst <= scheduler->quantum){
                    if(verbose){
                        cout<< CURRENT_TIME << " " << proc->processId << " " << timeInPrevState << ": " <<processStateToString(proc->newState) 
                        <<" -> RUNNG cb=" << actual_cpu_burst << " rem=" 
                        << proc->copy_totalCpuTime <<" prio=" << (proc->cuurentPriority+1)<< "\n";
                    }

                    proc->copy_totalCpuTime -= actual_cpu_burst;

                    if(proc->copy_totalCpuTime > 0){
                        eventQueue->insertEvent(new Event(CURRENT_TIME + actual_cpu_burst, proc, Transition::TRANS_TO_BLOCK));
                        proc->copy_cpuBurst = 0;
                    }else{
                        eventQueue->insertEvent(new Event(CURRENT_TIME + actual_cpu_burst, proc, Transition::TRANS_TO_READY));
                    }

                    CURRENT_TIME += actual_cpu_burst;
                }else{
                    if(verbose){
                        cout<< CURRENT_TIME << " " << proc->processId << " " << timeInPrevState << ": " <<processStateToString(proc->newState) 
                        <<" -> RUNNG cb=" << scheduler->quantum << " rem=" 
                        << proc->copy_totalCpuTime <<" prio=" << (proc->cuurentPriority+1)<< "\n";
                    }
                    proc->copy_totalCpuTime -= min(proc->copy_cpuBurst, scheduler->quantum);
                    proc->copy_cpuBurst -= min(proc->copy_cpuBurst, scheduler->quantum);

                    if(proc->copy_cpuBurst > 0){
                        eventQueue->insertEvent(new Event(CURRENT_TIME + scheduler->quantum, proc, Transition::TRANS_TO_READY));
                    }else{
                        if(proc->copy_totalCpuTime > 0){
                            eventQueue->insertEvent(new Event(CURRENT_TIME + scheduler->quantum, proc, Transition::TRANS_TO_BLOCK));
                        }else{
                            eventQueue->insertEvent(new Event(CURRENT_TIME + scheduler->quantum, proc, Transition::TRANS_TO_READY));
                        }
                    }

                    CURRENT_TIME += min(proc->copy_cpuBurst, scheduler->quantum);
                }

                CURRENT_RUNNING_PROCESS = proc;

                break;

            }
            case Transition::TRANS_TO_BLOCK:{
                //create an event for when process becomes READY again
                int actual_io_burst = mydrndom(proc->ioBurst);
                proc->cuurentPriority = proc->priority;
                if(verbose){
                    cout << CURRENT_TIME << " " << proc->processId << " " << timeInPrevState << ": " << processStateToString(proc->newState) 
                    << " -> BLOCK ib=" 
                    << actual_io_burst << " rem=" << proc->copy_totalCpuTime << "\n";
                }
                proc->state_ts = CURRENT_TIME;
                proc->newState = ProcessStates::BLOCK;
                proc->ioTime += actual_io_burst;
                if(CURRENT_TIME < scheduler-> blockTill && CURRENT_TIME + actual_io_burst >= scheduler->blockTill){
                    scheduler->non_overleap_io = CURRENT_TIME + actual_io_burst - scheduler->blockTill;
                }else if(CURRENT_TIME + actual_io_burst > scheduler->blockTill){
                    scheduler->non_overleap_io += actual_io_burst;
                }
                scheduler->blockTill = max(scheduler->blockTill, CURRENT_TIME + actual_io_burst);
                eventQueue->insertEvent(new Event(CURRENT_TIME + actual_io_burst, proc, Transition::TRANS_TO_READY));
                CALL_SCHEDULER = true;
                if(CURRENT_RUNNING_PROCESS != nullptr && proc->processId == CURRENT_RUNNING_PROCESS->processId){
                    CURRENT_RUNNING_PROCESS = nullptr;
                }
                break;
            }
            case Transition::TRANS_TO_PREEMPT:{
                // must come from RUNNING (preemption)
                // add to runqueue (no event is generated)
                CALL_SCHEDULER = true;
                break;
            }
        }
        currentEvent = nullptr;

        if(CALL_SCHEDULER){
            if(eventQueue->getEvent()){
                if(eventQueue->getEvent()->get_timestamp() <= CURRENT_TIME){    //get_next_event_time
                    continue;
                }
            }

            CALL_SCHEDULER = false;   //reset the flag

            if(CURRENT_RUNNING_PROCESS == nullptr){
                proc = scheduler->get_next_process();
                if(proc == nullptr)
                    continue;
                
                eventQueue->insertEvent(new Event(max(CURRENT_TIME,proc->arrivalTime), proc, Transition::TRANS_TO_RUN));
            }
            
        }
    }
}   
}

void printOutcome(Scheduler* scheduler){
    double sumTurnaroundTime = 0;
    double sumCpuWaitingTime = 0;
    double sumIoTime = 0;
    double sumCpuTime = 0;
    double size = scheduler->sizeOfExpQ();
    int lastFinishTime = 0;
    int priority;
    cout << scheduler->getSchedulerName() << endl;
    for(auto proc : scheduler->getExpiredQueue()){
        if(proc->finishTime > lastFinishTime){
            lastFinishTime = proc->finishTime;
        }
        sumCpuTime += proc->totalCpuTime;
        sumIoTime += proc->ioTime;
        sumTurnaroundTime += proc->finishTime - proc->arrivalTime;
        sumCpuWaitingTime += proc->cpuWaitingTime;
        priority = ifPrio ? proc->priority+1 : proc->priority+2;
        printf("%04d: %4d %4d %4d %4d %4d | %5d %5d %5d %5d\n",
            proc->processId, proc->arrivalTime, proc->totalCpuTime, proc->cpuBurst, proc->ioBurst, priority,
            proc->finishTime, (proc->finishTime - proc->arrivalTime), proc->ioTime, proc->cpuWaitingTime);
    }
    double avgTurnaroundTime = sumTurnaroundTime / size;
    double avgCpuWaitingTime = sumCpuWaitingTime / size;
    double cpuUtilization = 100.0 * (double) sumCpuTime / lastFinishTime;
    double ioUtilization = 100.0 * (double) sumIoTime / lastFinishTime;
    double throughput = 100.0 * (double) size / lastFinishTime;
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
        lastFinishTime, cpuUtilization, ioUtilization, avgTurnaroundTime, avgCpuWaitingTime, throughput);
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
        Event* newEvent = new Event(currentProcess->arrivalTime, currentProcess, Transition::TRANS_TO_READY);
        evenQ->insertEvent(newEvent);
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