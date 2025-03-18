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
        Process(int pid, int AT, int TC, int CB, int IO) {
            processId = pid;
            arrivalTime = AT;
            totalCpuTime = TC;
            cpuBurst = CB;
            ioBurst = IO;
            copy_arrivalTime = AT;
            copy_totalCpuTime = TC;
            copy_cpuBurst = 0;
            state_ts = AT;
            newState = ProcessStates::CREATED;
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
#endif