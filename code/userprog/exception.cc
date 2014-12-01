// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

void Destroy(){
    int status = machine->ReadRegister(4);

    currentThread->space->ClearState();
    printf("Process exiting with status %d.\n", status);
    currentThread->setExitStatus(0);
    currentThread->Finish();

    //remove process?
    //currentThread->Finish();
    //else interrupt->Halt();
   
}

void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt))
    {
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
    }
    else if ((which == SyscallException) && (type == SC_Exec))
    {
        DEBUG('a', "User program called Exec.\n");
        char input[1024];
        int addr = machine->ReadRegister(4);
        // Read file name from input
        int c;
        int v;
        for (c = 0; c < 1024; ++c)
        {
          machine->ReadMem(addr+c, 1, &v);
          input[c] = v;
          if (v == 0)
            break;
        }
        if (v != 0)
        {
          // Input too long! Abort.
          machine->WriteRegister(2, -1);
          printf("Exec input too long.");
          ASSERT(FALSE);
        }
        DEBUG('a', "User wants to execute %s.\n", input);
        // TODO: actually fork
        // TODO: write useful value
        machine->WriteRegister(2, 0);
    }
    else if ((which == SyscallException) && (type == SC_Exit))
    {
        DEBUG('a', "Exit, initiated by user program.\n");
        int status = machine->ReadRegister(4);
        currentThread->space->ClearState();
        printf("Process exiting with status %d.\n", status);
        currentThread->setExitStatus(0);
        currentThread->Finish();
    }
/* 
Other exceptions to check for : 
PageFaultException, ReadOnlyException, BusErrorException, AddressErrorException, OverflowException, IllegalInstrException
*/

    else if(which == PageFaultException){
        DEBUG('a', "PageFaultException : No valid translation found.\n");
        Destroy();
    }
    else if(which == ReadOnlyException){
        DEBUG('a', "ReadOnlyException : Write attempted to page marked \"read-only\".\n");
        Destroy();
    }
    else if(which == BusErrorException){
        DEBUG('a', "BusErrorException : Invalid physical address translation.\n");
        Destroy();
    }
    else if(which == AddressErrorException){
        DEBUG('a', "AddressErrorException : Unaligned reference to address space.\n");
        Destroy();
    }
    else if(which == OverflowException){
        DEBUG('a', "OverflowException : Integer overflow in add or sub.\n");
        Destroy();
    }
    else if(which == IllegalInstrException){
        DEBUG('a', "IllegalInstrException : Unimplemented or reserved instruction.\n");
        Destroy();
    }
    else
    {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
    // If we're here, it means nothing went wrong.
    // Increment the PC and continue.
    int pcAfter = machine->ReadRegister(NextPCReg) + 4;
    machine->WriteRegister(NextPCReg, pcAfter);
}


