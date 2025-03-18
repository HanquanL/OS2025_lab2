#ifndef shced_h
#define shced_h
#include <iostream>
#include <queue>
#include <list>

using namespace std;

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
inline string processStateToString(ProcessStates state) {
    switch(state) {
        case ProcessStates::CREATED:    return "CREATED";
        case ProcessStates::READY:      return "READY";
        case ProcessStates::RUNNING:    return "RUNNING";
        case ProcessStates::BLOCK:      return "BLOCK";
        case ProcessStates::TERMINATED: return "TERMINATED";
        default:                        return "UNKNOWN";
    }
}

inline string transitionToString(Transition trans) {
    switch(trans) {
        case Transition::TRANS_TO_READY:     return "TRANS_TO_READY";
        case Transition::TRANS_TO_RUN:       return "TRANS_TO_RUN";
        case Transition::TRANS_TO_BLOCK:     return "TRANS_TO_BLOCK";
        case Transition::TRANS_TO_PREEMPT:   return "TRANS_TO_PREEMPT";
        case Transition::TRANS_TO_TERMINATE: return "TRANS_TO_TERMINATE";
        default:                           return "UNKNOWN";
    }
}

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
        ProcessStates newState;
        ProcessStates oleState;
        int priority;
        int cuurentPriority;
        Process(int pid, int AT, int TC, int CB, int IO, int PRIO) {
            processId = pid;
            arrivalTime = AT;
            copy_arrivalTime = AT;
            totalCpuTime = TC;
            copy_totalCpuTime = TC;
            cpuBurst = CB;
            copy_cpuBurst = 0;
            ioBurst = IO;
            state_ts = AT;
            ioTime = 0;
            cpuWaitingTime = 0;
            newState = ProcessStates::CREATED;
            priority = PRIO;
            cuurentPriority = PRIO;
        }
};

class Event {
    private:
        int timeStamp;
        Process* process;
        Transition transition;
    public:
        Event() {}
        Event(int ts, Process* p, Transition trans) : timeStamp(ts), process(p), transition(trans) {}
        int get_timestamp() const{ return timeStamp; }
        Process* get_process() const{ return process; }
        Transition get_transition() { return transition; }
        ~Event() {}
};

class EventQueue{
    private:
        list<Event*> events;
    
    public:
        EventQueue() {}
        ~EventQueue(){
            for(auto it = events.begin(); it != events.end(); it++){
                delete *it;
            }
        }
        void insertEvent(Event* event){
            if(events.empty()){
                events.push_back(event);
                return;
            }
            for(auto it = events.begin(); it != events.end(); it++){
                if((*it)->get_timestamp() > event->get_timestamp()){
                    events.insert(it, event);
                    return;
                }
            }
            events.push_back(event);
        }

        Event* getEvent(){
            if(events.empty()){
                return nullptr;
            }
            return events.front();
        }

       void removeEvent(){
            if(events.empty()){
                return;
            }
            events.pop_front();
        }

        bool isEmpty(){
            return events.empty();
        }
};

class Scheduler{
        
    public:
        list<Process*> runQueue, expiredQueue;
        string name;
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
        
    public:
        list<Process*> runQueue, expiredQueue;
        string name = "FCFS";
        int quantum;
        FCFS_Scheduler(int qtm) : Scheduler(qtm) {
            this->quantum = qtm;
        }
        ~FCFS_Scheduler() {}
        void add_process(Process* process) override {
            runQueue.push_back(process);
        }
        void add_expired_process(Process* process) override {
            expiredQueue.push_back(process);
        }
        Process* get_next_process() override {
            // if(runQueue.empty()){
            //     return nullptr;
            // }
            // Process* nextProcess = runQueue.front();
            // runQueue.pop();
            // return nextProcess;
            Process* result = nullptr;
            while(!runQueue.empty()){
                
                if(runQueue.front()->copy_totalCpuTime == 0){
                    expiredQueue.push_back(runQueue.front());
                    runQueue.pop_front();
                    continue;
                }
                
                result = runQueue.front();
                runQueue.pop_front();
                break;
            }
            
            return result;
        }
        int sizeOfRunQ() override {
            return runQueue.size();
        }
        int sizeOfExpQ() override {
            return expiredQueue.size();
        }
        bool isEmpty() {
            return runQueue.empty();
        }
};
#endif