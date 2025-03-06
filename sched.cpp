#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

vector<string> outputTable;

class Process {
    public:
        int arrivalTime;
        int totalCpuTime;
        int cpuBurst;
        int ioBurst;
};

Process get_processObj(string lineOfProcess);

int main(int argc, char *argv[]) {
    
    string inputFile = argv[1];
    string rfile = argv[2];
    ifstream readFile (inputFile);
    string lineOfProcess;
    Process currentProcess;
    while(getline(readFile, lineOfProcess)){
        currentProcess = get_processObj(lineOfProcess);
        cout << currentProcess.arrivalTime << currentProcess.totalCpuTime << currentProcess.cpuBurst << currentProcess.ioBurst << endl;
    }

    
    return 0;
}

Process get_processObj(string lineOfProcess) {
    int arrivalTime, totalCpuTime, cpuBurst, ioBurst;
    istringstream iss(lineOfProcess);
    iss >> arrivalTime >> totalCpuTime >> cpuBurst >> ioBurst;

    Process process;
    process.arrivalTime = arrivalTime;
    process.totalCpuTime = totalCpuTime;
    process.cpuBurst = cpuBurst;
    process.ioBurst = ioBurst;

    return process;
}

