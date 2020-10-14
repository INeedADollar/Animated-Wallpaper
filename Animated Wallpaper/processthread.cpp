#include "processthread.h"
#include <QApplication>
#include <QMessageBox>

ProcessThread::ProcessThread(DWORD pId, QWidget *parent) : QThread(parent)
{
    setPID(pId);
}

void ProcessThread::setPID(DWORD pId){
    pID = pId;

    if(pauseExecution)
        pauseExecution = false;
}

BOOL ProcessThread::waitForProcessToStop(DWORD pid)
{
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if(!process){
        emit processIdInvalid();
        return FALSE;
    }

    DWORD ret = WaitForSingleObject(process, INFINITE);
    CloseHandle(process);
    return ret == WAIT_OBJECT_0;
}

void ProcessThread::run(){
    while(waitForProcessToStop(pID)){
       if(!pauseExecution){
           pauseExecution = true;
           emit processClosed();
       }
    }
}
