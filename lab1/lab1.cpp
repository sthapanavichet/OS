#include<iostream>
#include<vector>
#include "pidUtil.h" 

using namespace std;


int main(void){

    vector<int> pids;
    string pidName;
    int pidNum;
    ErrStatus error;

    error =  GetAllPids(pids);
    if(error != 0) cout << GetErrorMsg(error) << endl;  // print out error if error returned by GetAllPids is not zero
    else {
	cout << "All Pids:" << endl;
    	for(int i : pids) {  // otherwise print out all Pids and their name
		GetNameByPid(i, pidName);
		if(error != 0) cout << GetErrorMsg(error) << endl;  // if any error is seen, print error
		else cout << "Pid Number: " << i << " Pid Name: " << pidName << endl;
	}
    }

    cout << endl;
    cout << "Name of Pid 1:" << endl;
    pidNum = 1;
    error = GetNameByPid(pidNum, pidName);  // Get name of Pid 1 and output if there is no error.
    if(error != 0) cout << GetErrorMsg(error) << endl;
    else cout << pidName << endl;
    
    cout << endl;
    cout << "Pid of \"lab1\":" << endl; 
    pidName = "lab1";
    error = GetPidByName(pidName, pidNum);  // Get Pid of lab1 and output if there is no error.
    if(error != 0) cout << GetErrorMsg(error) << endl;
    else cout << pidNum << endl;
   
    cout << endl;
    cout << "Pid of \"lab11\":" << endl;
    pidName = "lab11";
    error = GetPidByName(pidName, pidNum);  // Get Pid of lab11, error is expected here because there is no process named "lab11".
    if(error != 0) cout << GetErrorMsg(error) << endl;
    else cout << pidNum << endl;
    return 0;

}
