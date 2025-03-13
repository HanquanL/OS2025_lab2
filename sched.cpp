#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
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

};

enum class ProcessStates{
    CREATED,
    READY,
    RUNNING,
    BLOCK,
    TERMINATED  
};

class Event {
    public:
        int timeStamp;
        Process* process;
        ProcessStates newState;
        ProcessStates oldState;
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

Process* get_processObj(string lineOfProcess);
int get_randomNumber();
int mydrndom(int burst);
void simulationLoop();
int globalProcessId = 0;
EventQueue eventQueue;

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
        eventQueue.insertEvent(newEvent);
        // cout << currentProcess.arrivalTime << currentProcess.totalCpuTime << currentProcess.cpuBurst << currentProcess.ioBurst << endl; //for test purposes
    }
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
    while(currentEvent = eventQueue.getEvent()){
        Process* currentProcess = currentEvent->process;
        int currentTime = currentEvent->timeStamp;
        ProcessStates currentState = currentEvent->newState;
        delete currentEvent;

    }
}
