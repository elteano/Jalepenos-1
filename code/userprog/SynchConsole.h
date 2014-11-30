// synchconsole.h
// 	To support system calls that access the console device
//	providing the abstraction of synchronous access to the console
//	


#include "copyright.h"

#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "console.h"
#include "synch.h"


class SynchConsole {
public:
    SynchConsole(char* name);    		// Initialize a synchronous console,
    // by initializing the raw Console.
    ~SynchConsole();			// De-allocate the synch console data

    void ReadConsole(char* data);
    void WriteConsole(char* data);

    void ReadAvail();
    void WriteDone();

private:
    Console *console;		  		// Raw console device
    Semaphore *semaphore; 		// To synchronize requesting thread
    // with the interrupt handler
    Lock *lock;		  		// Only one read/write request
    // can be sent to the disk at a time
};

#endif // SYNCHCONSOLE_H
