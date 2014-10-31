// synch.cc
//  Routines for synchronizing threads.  Three kinds of
//  synchronization routines are defined here: semaphores, locks
//    and condition variables (the implementation of the last two
//  are left to the reader).
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
//  Initialize a semaphore, so that it can be used for synchronization.
//
//  "debugName" is an arbitrary name, useful for debugging.
//  "initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
//  De-allocate semaphore, when no longer needed.  Assume no one
//  is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
//  Wait until semaphore value > 0, then decrement.  Checking the
//  value and decrementing must be done atomically, so we
//  need to disable interrupts before checking the value.
//
//  Note that Thread::Sleep assumes that interrupts are disabled
//  when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts

    while (value == 0) {      // semaphore not available
        queue->Append((void *)currentThread); // so go to sleep
        currentThread->Sleep();
    }
    value--;          // semaphore available,
    // consume its value

    (void) interrupt->SetLevel(oldLevel); // re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//  Increment semaphore value, waking up a waiter if necessary.
//  As with P(), this operation must be atomic, so we need to disable
//  interrupts.  Scheduler::ReadyToRun() assumes that threads
//  are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)    // make thread ready, consuming the V immediately
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
    DEBUG('t', "Attempting to delete Lock: ");
    ASSERT(!isLocked);
    delete queue;
    DEBUG('t', "Success...\n");
}//--- end destructor

void Lock::Acquire() {
    //--- turn interrupts off
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(!isHeldByCurrentThread());
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
/*    if(currThread != currentThread){
        DEBUG('t', "Current thread attempting to Release thread not owned!");
        return;
    }*/
    ASSERT(isHeldByCurrentThread()); // panic! We're releasing a thread we don't own

    Thread * thread;
    thread = (Thread *)queue->Remove();
    //--- check to see if the head is not NULL
    if(thread != NULL){
    //--- --- wake it up if it isn't
        scheduler->ReadyToRun(thread);
    }
    //--- change the lock state to reflect that the lock is free
    currThread = NULL; // show that no thread owns the lock
    isLocked = false;  // show that the Lock is not in use

    //--- turn interrupts back on
    (void) interrupt->SetLevel(oldLevel); //reset
}//--- end routine Release

bool Lock::isHeldByCurrentThread(){
    return currentThread == currThread;
}

Condition::Condition(char* debugName) {
    name = debugName;
    queue = new List();
    lock = new Lock(debugName);
}//--- end constructor

Condition::~Condition() {
    ASSERT(queue->Remove() == NULL);
    delete queue;
    delete lock;
}//--- end destructor

void Condition::Wait(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    
    ASSERT(conditionLock->isHeldByCurrentThread());
    
    conditionLock->Release();
    DEBUG('t', "Condition '%s': Lock surrendered...\n", getName());
    queue->Append((void *)currentThread);
    
    currentThread->Sleep();
    conditionLock->Acquire();
    
    (void) interrupt->SetLevel(oldLevel); //reset
}
void Condition::Signal(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    //--- PANIC!!!! We're signalling even though we don't have the lock
    ASSERT(conditionLock->isHeldByCurrentThread());
    
    Thread * thread;
    thread = (Thread *) queue->Remove();
    if(thread != NULL){
        scheduler->ReadyToRun(thread); // wake up the first thread in the queue
    }
    DEBUG('t', "Condition '%s': Signalled!\n", getName());
    (void) interrupt->SetLevel(oldLevel); //reset
}
void Condition::Broadcast(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    //--- PANIC!!!! Attempting to broadcast while not holding the lock!!
    ASSERT(conditionLock->isHeldByCurrentThread());

    Thread * thread;
    while((thread = (Thread*)queue->Remove()) != NULL){
        scheduler->ReadyToRun(thread); // wake up all the threads
    }

    DEBUG('t', "Condition '%s': Broadcasted!\n", getName());
    (void) interrupt->SetLevel(oldLevel); //reset
}

//-------------------------------------------------------
//-- Mailbox class definition
//-------------------------------------------------------

Mailbox::Mailbox(char * debugName){
    buffer = new List();
    lock = new Lock("Mailbox Lock");
    condLock = new Lock("Condition Lock");
    mailSnd = new Condition("Mail Send");
    mailRcv = new Condition("Mail Receive");
    mailCpyDone = new Condition("Mail Copy");
    copyComplete = false;
    numPendingSends = 0;
    numPendingRecs = 0;
    name = debugName;
}//--- end constructor

Mailbox::~Mailbox(){
    delete mailSnd;
    delete mailRcv;
    delete lock;
    delete buffer;
}//--- end destructor

void Mailbox::Send(int message){
/*
    lock->Acquire();
    ++numPendingSends;
    lock->Release();
    condLock->Acquire();
    if (numPendingRecs == 0) {
      mailRcv->Wait(condLock);
    } else {
      mailSnd->Signal(condLock);
    }
    if (!copyComplete)
      mailCpyDone->Wait(condLock);
    condLock->Release();
    lock->Acquire();
    --numPendingSends;
    lock->Release();*/

    
    lock->Acquire();
    DEBUG('t', "Trying to send\n");
    numPendingSends++;
    
    buffer->Append((void *)&message);
    while(numPendingRecs == 0){
      mailSnd->Wait(lock);
    }
    DEBUG('t', "Sending\n");

    mailRcv->Signal(lock);
    numPendingSends--;
    lock->Release();
}//--- end routine Send

void Mailbox::Receive(int * message){
    lock->Acquire();
    DEBUG('t', "Trying to receive\n");
    numPendingRecs++;
 
    while(numPendingSends == 0){
      mailRcv->Wait(lock);
    }
    DEBUG('t', "Receiving\n");

    *message =  *(int*) buffer->Remove();
    printf("*message = %d\n", *message);
    
    mailSnd->Signal(lock);
    numPendingRecs--;
    lock->Release();
    
}//--- end routine Receive

char * Mailbox::getName(){
    return name;
}//--- end routine getName


//-------------------------------------------------------
//-- Whale class definition
//-------------------------------------------------------
Whale::Whale(char * debugName){
    lock = new Lock("Whale Lock");
    condLock = new Lock("Condition Lock");
    maleSnd = new Condition("Male Send");

    femaleSnd = new Condition("Female Send");

    matchSnd = new Condition("Match Send");

    numPendingMale = 0;
    numPendingFemale = 0;
    numPendingMatch = 0;

    name = debugName;
}
Whale::~Whale(){
    delete maleSnd;
    delete femaleSnd;
    delete matchSnd;


    delete lock;
}

void Whale::Male(){
    lock->Acquire();
    numPendingMale++;

    while((numPendingFemale == 0) || 
          (numPendingMatch == 0)){
      maleSnd->Wait(lock);
    }
    matchSnd->Signal(lock);
    femaleSnd->Signal(lock);

    numPendingMale--;

    lock->Release();
}

void Whale::Female(){
    lock->Acquire();
    numPendingFemale++;

    while((numPendingMale == 0) || 
          (numPendingMatch == 0)){
      femaleSnd->Wait(lock);
    }
    matchSnd->Signal(lock);
    femaleSnd->Signal(lock);

    numPendingFemale--;

    lock->Release();
}

void Whale::Matchmaker(){
    lock->Acquire();
    numPendingMatch++;

    while((numPendingFemale == 0) || 
          (numPendingMale == 0)){
      matchSnd->Wait(lock);
    }
    femaleSnd->Signal(lock);
    maleSnd->Signal(lock);

    numPendingMatch--;

    lock->Release();
}

