// synchconsole.cc
// 	To support system calls that access the console device
//	providing the abstraction of synchronous access to the console
//	
//	filesys/synchdisk.{cc/h} mirror

#include "synchconsole.h"


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
    semaphore = new Semaphore("synch console", 0);
    lock = new Lock("synch console lock");
//    console = new Console(name, ConsoleRequestDone, (int) this);
    console = new Console(null, null, console_read, console_write, (int) this);
}

//----------------------------------------------------------------------
// SynchConsole::~SynchConsole
// 	De-allocate data structures needed for the synchronous console
//	abstraction.
//----------------------------------------------------------------------

SynchConsole::~SynchConsole()
{
    delete console;
    delete lock;
    delete semaphore;
}

//----------------------------------------------------------------------
// SynchConsole::ReadConsole
// 	Read the contents of console into a buffer.  Return only
//	after the data has been read.
//
//	"data" -- the buffer to hold the contents of the disk sector
//----------------------------------------------------------------------

void
SynchConsole::ReadConsole(char* data)
{
    lock->Acquire();		

//
    //console = new Console(null, null, (int) this);

    console->GetChar();
    semaphore->P();			// wait for interrupt
    lock->Release();
}

//----------------------------------------------------------------------
// SynchConsole::WriteConsole
// 	Write the contents of a buffer into a disk sector.  Return only
//	after the data has been written.
//
//	"data" -- the new contents of the disk sector
//----------------------------------------------------------------------

void
SynchConsole::WriteConsole( char* data)
{
    lock->Acquire();			// only one disk I/O at a time

//Write(buffer, i, ConsoleOutput);
    console->PutChar(data);
    semaphore->P();			// wait for interrupt
    lock->Release();
}

//----------------------------------------------------------------------
// SynchConsole::WriteDone
// 	
//----------------------------------------------------------------------

void
SynchConsole::ReadAvail()
{
   // semaphore->();
}



//----------------------------------------------------------------------
// SynchConsole::WriteDone
// 	
//----------------------------------------------------------------------

void
SynchConsole::WriteDone()
{
    semaphore->V();
}
