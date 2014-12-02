// synchconsole.cc
// 	To support system calls that access the console device
//	providing the abstraction of synchronous access to the console
//	
//	filesys/synchdisk.{cc/h} mirror

#include "synchconsole.h"



static void SynchConsoleRead(int i){
     SynchConsole *synchConsole = (SynchConsole *)i;
     synchConsole->ReadAvail();

}
static void SynchConsoleWrite(int i){
     SynchConsole *synchConsole = (SynchConsole *)i;
     synchConsole->WriteDone();

}
//----------------------------------------------------------------------
// SynchDisk::SynchConsole
// 	Initialize the synchronous interface to the physical disk, in turn
//	initializing the physical disk.
//
//	"name" -- UNIX file name to be used as storage for the disk data
//	   (usually, "CONSOLE")
//----------------------------------------------------------------------

SynchConsole::SynchConsole(char* name)
{
    semaphoreOutput = new Semaphore("synch console output", 0);
    semaphoreInput = new Semaphore("synch console input", 0);
    
    lockOutput = new Lock("synch console output lock");
    lockInput = new Lock("synch console input lock");

//    console = new Console(name, ConsoleRequestDone, (int) this);
//    console = new Console(null, null, console_read, console_write, (int) this);
    console = new Console(readFile, writeFile, SynchConsoleRead, SynchConsoleWrite, (int) this);
}

//----------------------------------------------------------------------
// SynchConsole::~SynchConsole
// 	De-allocate data structures needed for the synchronous console
//	abstraction.
//----------------------------------------------------------------------

SynchConsole::~SynchConsole()
{
    delete console;
    delete lockOutput;
    delete lockInput;

    delete semaphoreOutput;
    delete semaphoreInput;

}

//----------------------------------------------------------------------
// SynchConsole::ReadChar
// 	Read the contents of console into a buffer.  Return only
//	after the data has been read.
//
//	"data" -- the buffer to hold the contents of the disk sector
//----------------------------------------------------------------------

char
SynchConsole::ReadChar()
{
    lockInput->Acquire();		
    semaphoreInput->P();			// wait for interrupt

    char ch;
    ch = console->GetChar();

    lockInput->Release();

    return ch;
}

//----------------------------------------------------------------------
// SynchConsole::WriteChar
// 	Write the contents of a buffer into a disk sector.  Return only
//	after the data has been written.
//
//	"data" -- the new contents of the disk sector
//----------------------------------------------------------------------

void
SynchConsole::WriteChar( char* data)
{
    lockOutput->Acquire();			// only one disk I/O at a time

    console->PutChar(data);
    semaphoreOutput->P();			// wait for interrupt
    lockOutput->Release();
}


int
SynchConsole::ReadLine(char * data, int size){
    lockInput->Acquire();

    char c;
    int total_read = 0;

    while((c != '\n') && (num_read < size)){

        semaphoreInput->P();
        c = console->GetChar();
        
        *data = c;
        data++;
        total_read++;

    }

    lockInput->Release();

    return total_read;
}

void
SynchConsole::WriteLine(char * data){
    lockOutput->Acquire();
    while(*data != '\0'){
       console->PutChar(data[0]);
       data++;
       semaphoreOutput->P();
    }

    lockOutput->Release();

}

//----------------------------------------------------------------------
// SynchConsole::ReadAvail
// 	
//----------------------------------------------------------------------


void
SynchConsole::ReadAvail()
{
    semaphoreInput->V();
}

//----------------------------------------------------------------------
// SynchConsole::WriteDone
// 	
//----------------------------------------------------------------------

void
SynchConsole::WriteDone()
{
    semaphoreOutput->V();
}
