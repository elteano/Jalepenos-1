// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;

    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}
//----------------------------------------------------------------------
// ConditionTest1
//----------------------------------------------------------------------
Condition wGo("test");
Condition rGo("test");
Lock l("test");
int nReaders = 0;
bool wantToWrite = false;
void Writer(int);
void Reader(int);
void ReadersAndWriters(){
    Thread * t;
    Thread * t2;
    t = new Thread("one");
    t2 = new Thread("2.one");
    t->Fork(Reader, 0);
    t2->Fork(Reader, 0);
    t = new Thread("six");
    t->Fork(Reader, 0);
    t2 = new Thread("2.three");
    t2->Fork(Reader, 0);

    t = new Thread("two");
    t->Fork(Writer, 0);
    t = new Thread("three");
    t2 = new Thread("2.two");
    t->Fork(Reader, 0);
    t2->Fork(Reader, 0);
    t = new Thread("four");
    t->Fork(Reader, 0);
    t = new Thread("five");
    t->Fork(Writer, 0);
}
void Writer(int param){
    l.Acquire();
    wantToWrite = true;
    printf("WRITER %s WANTS TO WRITE!\n", currentThread->getName());
    while(nReaders > 0){
        wGo.Wait(&l);
    }
    currentThread->Yield();
    printf("WRITER %s WRITING\n", currentThread->getName());
    wantToWrite = false;
    printf("WRITER %s FINISHED WRITING\n", currentThread->getName());
    rGo.Broadcast(&l);
    l.Release();
}

void Reader(int param){
    l.Acquire();
    while(wantToWrite){
        rGo.Wait(&l);
    }
    nReaders++;
    l.Release();
    currentThread->Yield();
    printf("%s IS READING\n", currentThread->getName());
    l.Acquire();
    printf("%s HAS FINISHED READING!\n", currentThread->getName());
    nReaders--;
    printf("THERE ARE %d READERS LEFT...\n", nReaders);
    if(nReaders == 0){
        wGo.Signal(&l);
    }
    l.Release();
}
//----------------------------------------------------------------------
// LockTest1
//----------------------------------------------------------------------

Lock *locktest1 = NULL;

void
LockThread1(int param)
{
    DEBUG('t', "L1 Holding Lock? %i\n", locktest1->isHeldByCurrentThread());
    printf("L1:0\n");
    locktest1->Acquire();
    DEBUG('t', "L1 Holding Lock? %i\n", locktest1->isHeldByCurrentThread());
    printf("L1:1\n");
    currentThread->Yield();
    printf("L1:2\n");
    locktest1->Release();
    DEBUG('t', "L1 Holding Lock? %i\n", locktest1->isHeldByCurrentThread());
    printf("L1:3\n");
}

void
LockThread2(int param)
{
    printf("L2:0\n");
    
    DEBUG('t', "L2 Holding Lock? %i\n", locktest1->isHeldByCurrentThread());
    locktest1->Acquire();
    DEBUG('t', "L2 Holding Lock? %i\n", locktest1->isHeldByCurrentThread());
    printf("L2:1\n");
    currentThread->Yield();
    printf("L2:2\n");
    locktest1->Release();
    DEBUG('t', "L2 Holding Lock? %i\n", locktest1->isHeldByCurrentThread());
    printf("L2:3\n");
}

void
LockTest1()
{
    DEBUG('t', "Entering LockTest1");

    locktest1 = new Lock("LockTest1");

    Thread *t = new Thread("one");
    t->Fork(LockThread1, 0);
    t = new Thread("two");
    t->Fork(LockThread2, 0);
}

Lock * locktest2 = NULL; // testing the safety features
void Lock2Thread1(int param){
    locktest2->Acquire();
    delete locktest2;
    locktest2->Release();
}//
void LockTest2(){
    DEBUG('t', "Entering LockTest2");
    locktest2 = new Lock("LockTest2");
    Thread * t;
    t = new Thread("one");
    t->Fork(Lock2Thread1, 0);
}//

//----------------------------------------------------------------------
// Mailbox tests
//----------------------------------------------------------------------
Mailbox sndr("mailbox");

void MailboxSend(int param){
    DEBUG('t', "\"%s\" entering MailboxSend\n", currentThread->getName());
    sndr.Send(param);
    DEBUG('t', "\"%s\" exiting MailboxSend\n", currentThread->getName());
}

void MailboxTest1(){
    DEBUG('t', "Entering MailboxTest1");
    Thread * t;
    t = new Thread("One");
    t->Fork(MailboxSend, 0);
//    t = new Thread("Two");
//    t->Fork(MailboxSend, 2);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
        ThreadTest1();
        break;
    case 2:
        LockTest1();
        break;
    case 3:
        LockTest2();
        break;
    case 4:
        ReadersAndWriters();
        break;
    case 5:
        MailboxTest1();
        break;
    default:
        printf("No test specified.\n");
        break;
    }
}

