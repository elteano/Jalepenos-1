// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    while (value == 0) { 			// semaphore not available
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    }
    value--; 					// semaphore available,
    // consume its value

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
    isLocked = false;
    name = debugName;
    queue = new List();
    currThread = NULL;
}//--- end constructor

Lock::~Lock() {
    delete queue;
}//--- end destructor

void Lock::Acquire() {
    //--- turn interrupts off
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    while(isLocked){
        queue->Append((void *)currentThread);
        currentThread->Sleep();
    }
    isLocked = true;
    currThread = currentThread; //--- show that the current thread has the lock
    (void) interrupt->SetLevel(oldLevel); //--- restore old state
}//--- end routine Aquire

void Lock::Release() {
    //--- turn interrupts off
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    //--- get the head of the queue
    Thread * thread;
    thread = (Thread *)queue->Remove();
    //--- check to see if the head is not NULL
    if(thread != NULL){
    //--- --- wake it up if it isn't
        scheduler->ReadyToRun(thread);
    }
    //--- change the lock state to reflect that the lock is free
    currThread = NULL;
    isLocked = false;

    //--- turn interrupts back on
    (void) interrupt->SetLevel(oldLevel); //reset
}//--- end routine Release

bool Lock::isHeldByCurrentThread(){
    if(currentThread == currThread){
        return true;
    }
    return false;
}

Condition::Condition(char* debugName) {
    name = debugName;
    queue = new List();
    lock = new Lock(debugName);
}//--- end constructor

Condition::~Condition() { 
    delete queue;
    delete lock;
}//--- end destructor

void Condition::Wait(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    conditionLock->Release();
    queue->Append((void *)currentThread);
    currentThread->Sleep();
    conditionLock->Acquire();
    (void) interrupt->SetLevel(oldLevel); //reset
}
void Condition::Signal(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    Thread * thread;
    thread = (Thread *) queue->Remove();
    if(thread != NULL){
        scheduler->ReadyToRun(thread);
    }
    (void) interrupt->SetLevel(oldLevel); //reset
}
void Condition::Broadcast(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    Thread * thread;
    while((thread = (Thread*)queue->Remove()) != NULL){
        scheduler->ReadyToRun(thread);
    }
    (void) interrupt->SetLevel(oldLevel); //reset
}
