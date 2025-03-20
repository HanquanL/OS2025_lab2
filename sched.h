#ifndef shced_h
#define shced_h
#include <iostream>
#include <queue>
#include <list>
#include <deque>
#include <vector>


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
        int turnarTime;
        int runTime;
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
            cuurentPriority = PRIO-1;
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
        void set_timestamp(int ts) { timeStamp = ts; }
        Process* get_process() const{ return process; }
        void set_process(Process* p) { process = p; }
        Transition get_transition() { return transition; }
        void set_transition(Transition ts) { transition = ts; }
        ~Event() {}
};

class EventQueue{
    
    public:
        deque<Event*> events;
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
        vector<deque<Process*>> activeQueues, expiredQueues;
        vector<deque<Process*>> *activeQueuePtr, *expiredQueuePtr;
        string name;
        int quantum;
        int maxprio;
        int blockTill = 0;
        int non_overleap_io = 0;
        bool isPreempt = false;
        Scheduler(int qtm) : quantum(qtm) {
            this->quantum = qtm;
        }
        virtual ~Scheduler() {}    // virtual destructor
        virtual void add_process(Process* process){};
        virtual void add_expired_process(Process* process){};
        virtual Process* get_next_process(){ return nullptr; };
        virtual bool unblock_preempt(Process* running, Process* unblocked){ return false; }
        virtual int sizeOfRunQ() { return runQueue.size(); }
        virtual int sizeOfExpQ() { return expiredQueue.size(); }
        virtual string getSchedulerName() { return name; }
        virtual const list<Process*>& getExpiredQueue() const { return expiredQueue; }
        virtual int getMaxPrio() { return 4; }
        virtual bool isPreemptivePriority() { return isPreempt; }
};

class FCFS_Scheduler : public Scheduler{
        
    public:
        list<Process*> runQueue, expiredQueue;
        string name;
        int quantum;
        int blockTill = 0;
        int non_overleap_io = 0;
        bool isPreempt = false;
        FCFS_Scheduler(int qtm) : Scheduler(qtm) {
            this->quantum = qtm;
            this->name = "FCFS";
        }
        ~FCFS_Scheduler() {}
        void add_process(Process* process) override {
            runQueue.push_back(process);
        }
        void add_expired_process(Process* process) override {
            expiredQueue.push_back(process);
        }
        Process* get_next_process() override {
            Process* process = nullptr;
            while(!runQueue.empty()){
                
                if(runQueue.front()->copy_totalCpuTime == 0){
                    expiredQueue.push_back(runQueue.front());
                    runQueue.pop_front();
                    continue;
                }
                
                process = runQueue.front();
                runQueue.pop_front();
                break;
            }
            
            return process;
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
        string getSchedulerName() override {
            return name;
        }
        const list<Process*>& getExpiredQueue() const override { return expiredQueue; }
};

class LCFS_Scheduler : public Scheduler{
    public:
        list<Process*> runQueue, expiredQueue;
        string name;
        int quantum;
        int blockTill = 0;
        int non_overleap_io = 0;
        bool isPreempt = false;
        LCFS_Scheduler(int qtm) : Scheduler(qtm) {
            this->quantum = qtm;
            this->name = "LCFS";
        }
        ~LCFS_Scheduler() {}
        void add_process(Process* process) override {
            runQueue.push_back(process);
        }
        void add_expired_process(Process* process) override {
            expiredQueue.push_back(process);
        }
        Process* get_next_process() override{
            Process* process = nullptr;
            while(!runQueue.empty()){
                
                if(runQueue.back()->copy_totalCpuTime == 0){
                    expiredQueue.push_back(runQueue.back());
                    runQueue.pop_back();
                    continue;
                }
                
                process = runQueue.back();
                runQueue.pop_back();
                break;
            }
            return process;
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
        string getSchedulerName() override {
            return name;
        }
        const list<Process*>& getExpiredQueue() const override { return expiredQueue; }
};

class SRTF_Scheduler : public Scheduler{
    public:
        list<Process*> runQueue, expiredQueue;
        string name;
        int quantum;
        int blockTill = 0;
        int non_overleap_io = 0;
        bool isPreempt = false;
        SRTF_Scheduler(int qtm) : Scheduler(qtm) {
            this->quantum = qtm;
            this->name = "SRTF";
        }
        ~SRTF_Scheduler() {}
        void add_process(Process* process) override {
            runQueue.push_back(process);
        }
        void add_expired_process(Process* process) override {
            expiredQueue.push_back(process);
        }
        Process* get_next_process() override {
            if(runQueue.empty()){
                return nullptr;
            }
            for(list<Process*>::iterator it=runQueue.begin(); it != runQueue.end();){
                if ((*it) -> copy_totalCpuTime == 0 ){
                    expiredQueue.push_back(*it);
                    it=runQueue.erase(it);
                }else{
                    ++it;
                }
            }
            int tempTC = runQueue.front()->copy_totalCpuTime;
            Process* process = runQueue.front();
            for(auto it = runQueue.begin(); it != runQueue.end(); it++){
                if((*it)->copy_totalCpuTime < tempTC){
                    tempTC = (*it)->copy_totalCpuTime;
                    process = *it;
                }
            }
            runQueue.remove(process);
            return process;

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
        string getSchedulerName() override {
            return name;
        }
        const list<Process*>& getExpiredQueue() const override { return expiredQueue; }
};

class RR_Scheduler : public Scheduler{
    public:
        list<Process*> runQueue, expiredQueue;
        string name;
        int quantum;
        int blockTill = 0;
        int non_overleap_io = 0;
        bool isPreempt = false;
        RR_Scheduler(int qtm) : Scheduler(qtm) {
            this->quantum = qtm;
            this->name = "RR " + to_string(qtm);
        }
        ~RR_Scheduler() {}
        void add_process(Process* process) override {
            runQueue.push_back(process);
        }
        void add_expired_process(Process* process) override {
            expiredQueue.push_back(process);
        }
        Process* get_next_process() override{
            Process* process = nullptr;
            while(!runQueue.empty()){
                
                if(runQueue.front()->copy_totalCpuTime == 0){
                    expiredQueue.push_back(runQueue.front());
                    runQueue.pop_front();
                    continue;
                }
                
                process = runQueue.front();
                runQueue.pop_front();
                break;
            }
            return process;
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
        string getSchedulerName() override {
            return name;
        }
        const list<Process*>& getExpiredQueue() const override { return expiredQueue; }
};

class PRIO_Scheduler : public Scheduler{
    public:
        //list<Process*> runQueue, expiredQueue;
        vector<deque<Process*>> activeQueues, expiredQueues;
        vector<deque<Process*>> *activeQueuePtr, *expiredQueuePtr;
        string name;
        int quantum;
        int maxprio;
        int blockTill = 0;
        int non_overleap_io = 0;
        bool isPreempt = false;
        int expiredProcessNumber = 0;
        PRIO_Scheduler(int qtm, int maxprio) : Scheduler(qtm) {
            this->quantum = qtm;
            this->maxprio = maxprio;
            this->name = "PRIO " + to_string(qtm);
            for(int i = 0; i < maxprio; i++){
                deque<Process*> activeQ, expiredQ;
                activeQueues.push_back(activeQ);
                expiredQueues.push_back(expiredQ);
            }
            activeQueuePtr = &activeQueues;
            expiredQueuePtr = &expiredQueues;
            this->expiredProcessNumber = 0;
        }
        ~PRIO_Scheduler() {}
        void add_process(Process* process) override {
            // if(process->cuurentPriority == -1){
            //     process->cuurentPriority = process->priority;
            //     add_expired_process(process);
            // }
            // runQueue.push_back(process);
            if(process->cuurentPriority == -1){
                process->cuurentPriority = process->priority-1;
                expiredQueuePtr->at(process->cuurentPriority).push_back(process);
                expiredProcessNumber++;
            }else{
                activeQueuePtr->at(process->cuurentPriority).push_back(process);
            }
        }
        void add_expired_process(Process* process) override {
            expiredQueue.push_back(process);
        }
        Process* get_next_process() override{
            Process *process = findProcess();
            if(expiredProcessNumber > 0 && process == nullptr){
                swapQueus();
                return findProcess();
            }
            return process;
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
        string getSchedulerName() override {
            return name;
        }
        //const list<Process*>& getExpiredQueue() const override { return expiredQueue; }
        void swapQueus(){
            vector<deque<Process*>> *temp = activeQueuePtr;
            activeQueuePtr = expiredQueuePtr;
            expiredQueuePtr = temp;
            expiredProcessNumber = 0;
        }

        Process* findProcess(){
            Process* process = nullptr;
            deque<Process*> tempQ;
            for(int i = activeQueuePtr->size() - 1; i >= 0; i--){
                if(!activeQueuePtr->at(i).empty()){
                    process = activeQueuePtr->at(i).front();
                    activeQueuePtr->at(i).pop_front();
                    return process;
                }
            }
            return process;
        }

        bool isPriority(){
            return true;
        }
};

class Pre_PRIO_Scheduler : public Scheduler{
    public:
        //list<Process*> runQueue, expiredQueue;
        vector<deque<Process*>> activeQueues, expiredQueues;
        vector<deque<Process*>> *activeQueuePtr, *expiredQueuePtr;
        string name;
        int quantum;
        int maxprio;
        int blockTill = 0;
        int non_overleap_io = 0;
        bool isPreempt = false;
        int expiredProcessNumber;
        Pre_PRIO_Scheduler(int qtm, int maxprio) : Scheduler(qtm) {
            this->quantum = qtm;
            this->maxprio = maxprio;
            this->name = "PREPRIO " + to_string(qtm);
            this->isPreempt = true;
            for(int i = 0; i < maxprio; i++){
                deque<Process*> activeQ, expiredQ;
                activeQueues.push_back(activeQ);
                expiredQueues.push_back(expiredQ);
            }
            activeQueuePtr = &activeQueues;
            expiredQueuePtr = &expiredQueues;
            this->expiredProcessNumber = 0;
        }
        ~Pre_PRIO_Scheduler() {}
        Process* get_next_process() override{
            Process *process = findProcess();
            if(expiredProcessNumber > 0 && process == nullptr){
                swapQueus();
                return findProcess();
            }
            return process;
        }
        void add_process(Process* process){
            // if(process->cuurentPriority == -1){
            //     process->cuurentPriority = process->priority;
            //     add_expired_process(process);
            // }
            // runQueue.push_back(process);
            if(process->cuurentPriority == -1){
                process->cuurentPriority = process->priority-1;
                expiredQueuePtr->at(process->cuurentPriority).push_back(process);
                expiredProcessNumber++;
            }else{
                activeQueuePtr->at(process->cuurentPriority).push_back(process);
            }
        }
        void add_expired_process(Process* process) override {
            expiredQueue.push_back(process);
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
        string getSchedulerName() override {
            return name;
        }
        const list<Process*>& getExpiredQueue() const override { return expiredQueue; }

        void swapQueus(){
            vector<deque<Process*>> *temp = activeQueuePtr;
            activeQueuePtr = expiredQueuePtr;
            expiredQueuePtr = temp;
            expiredProcessNumber = 0;
        }

        Process* findProcess(){
            Process* process = nullptr;
            deque<Process*> tempQ;
            for(int i = activeQueuePtr->size() - 1; i >= 0; i--){
                if(!activeQueuePtr->at(i).empty()){
                    process = activeQueuePtr->at(i).front();
                    activeQueuePtr->at(i).pop_front();
                    return process;
                }
            }
            return process;
        }

        bool isPriority(){
            return true;
        }
        bool isPreemptivePriority(){
            return isPreempt;
        }
};
#endif