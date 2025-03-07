#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

vector<string> outputTable;
ifstream randomNumbers;

class Process {
    public:
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
};

Process get_processObj(string lineOfProcess);
int get_randomNumber();

int main(int argc, char *argv[]) {
    
    string inputFile = argv[1];
    string rfile = argv[2];
    ifstream readFile (inputFile);
    string lineOfProcess;
    Process currentProcess;
    string stringTotalRandomNumbers;
    randomNumbers.open(rfile);
    getline(randomNumbers, stringTotalRandomNumbers);
    while(getline(readFile, lineOfProcess)){
        currentProcess = get_processObj(lineOfProcess);
        // cout << currentProcess.arrivalTime << currentProcess.totalCpuTime << currentProcess.cpuBurst << currentProcess.ioBurst << endl; //for test purposes

    }
    // cout << get_randomNumber() << endl; //for test purposes
    // cout << get_randomNumber() << endl; //for test purposes
    // cout << get_randomNumber() << endl; //for test purposes
    // cout << get_randomNumber() << endl; //for test purposes

    
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
