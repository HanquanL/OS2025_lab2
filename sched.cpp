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
    get_randomNumber();
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
    }
    // while(getline(readFile, lineOfProcess)){
    //     Process* currentProcess = get_processObj(lineOfProcess);
    //     outcomeProcesses.push_back(currentProcess);
    //     Event* newEvent = new Event(currentProcess->arrivalTime, currentProcess, Transition::TRANS_TO_READY);
    //     eventQueue->insertEvent(newEvent);
    //     // cout << currentProcess.arrivalTime << currentProcess.totalCpuTime << currentProcess.cpuBurst << currentProcess.ioBurst << endl; //for test purposes
    // }
    readInputFile(eventQueue, inputFile);
    //simulationLoop();
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
void simulationLoop(EventQueue* eventQueue, Scheduler* scheduler, int verbose){
    Event* currentEvent;
    Process* CURRENT_RUNNING_PROCESS = nullptr;
    int CURRENT_TIME = 0;
    while(currentEvent = eventQueue->getEvent()){
        CURRENT_TIME = currentEvent->get_timestamp();
        Process *proc = currentEvent->get_process(); //this is the process the event works on
        Transition transaction = currentEvent->get_transition();
        bool CALL_SCHEDULER = false;
        int timeInPrevState = CURRENT_TIME - proc->state_ts;
        eventQueue->removeEvent();
        switch(transaction){
            case Transition::TRANS_TO_READY:{
                ProcessStates preState = proc->newState;
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

                proc->newState = ProcessStates::READY;
                proc->state_ts = CURRENT_TIME;

                scheduler->add_process(proc);
                CALL_SCHEDULER = true;
                break;
            }
            case Transition::TRANS_TO_RUN:{
                int actual_cpu_burst;
                ProcessStates preState = proc->newState;
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
                        cout<< CURRENT_TIME << " " << proc->processId << " " << timeInPrevState << ": " <<processStateToString(proc->newState) <<" -> RUNNG cb=" << actual_cpu_burst << " rem=" 
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
                        cout<< CURRENT_TIME << " " << proc->processId << " " << timeInPrevState << ": " <<processStateToString(proc->newState) <<" -> RUNNG cb=" << scheduler->quantum << " rem=" 
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
                int actual_io_burst = mydrndom(proc->ioBurst);
                proc->cuurentPriority = proc->priority;
                if(verbose){
                    cout << CURRENT_TIME << " " << proc->processId << " " << timeInPrevState << ": " << processStateToString(proc->newState) << " -> BLOCK ib=" 
                    << actual_io_burst << " rem=" << proc->copy_totalCpuTime << "\n";
                }
                proc->state_ts = CURRENT_TIME;
                proc->newState = ProcessStates::BLOCK;
                proc->ioTime += actual_io_burst;
                // if(CURRENT_TIME < scheduler-> blockTill && CURRENT_TIME + actual_io_burst >= scheduler->blockTill){
                //     scheduler->non_overleap_io = CURRENT_TIME + actual_io_burst - scheduler->blockTill;
                // }else if(CURRENT_TIME + actual_io_burst > scheduler->blockTill){
                //     scheduler->non_overleap_io += actual_io_burst;
                // }
                // scheduler->blockTill = max(scheduler->blockTill, CURRENT_TIME + actual_io_burst);

                eventQueue->insertEvent(new Event(CURRENT_TIME + actual_io_burst, proc, Transition::TRANS_TO_READY));
                CALL_SCHEDULER = true;
                if(CURRENT_RUNNING_PROCESS != nullptr && proc->processId == CURRENT_RUNNING_PROCESS->processId){
                    CURRENT_RUNNING_PROCESS = nullptr;
                }
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

void printOutcome(Scheduler* scheduler){
    double sumTurnaroundTime = 0;
    double sumCpuWaitingTime = 0;
    double sumIoTime = 0;
    double sumCpuTime = 0;
    double size = scheduler->sizeOfExpQ();
    int lastFinishTime = 0;
    int priority;
    cout << scheduler->name << endl;
    for(int i = 0; i < size; i++){
        for(auto it = scheduler->expiredQueue.begin(); it != scheduler->expiredQueue.end(); ++it){
            if((*it)->processId == i){
                if((*it)->finishTime > lastFinishTime){
                    lastFinishTime = (*it)->finishTime;
                }
            }
            sumCpuTime += (*it)->totalCpuTime;
            sumIoTime += (*it)->ioTime;
            sumTurnaroundTime += (*it)->finishTime - (*it)->arrivalTime;
            sumCpuWaitingTime += (*it)->cpuWaitingTime;
            if(ifPrio){
                priority = (*it)->priority+1;
            }else{
                priority = (*it)->priority+2;
            }
            printf("%04d: %4d %4d %4d %4d %4d | %5d %5d %5d %5d\n",
                (*it)->processId, (*it)->arrivalTime, (*it)->totalCpuTime, (*it)->cpuBurst, (*it)->ioBurst, (*it)->priority,
                (*it)->finishTime, ((*it)->finishTime - (*it)->arrivalTime), (*it)->ioTime, (*it)->cpuWaitingTime);
        }
        
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
