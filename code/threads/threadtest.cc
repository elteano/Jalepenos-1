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
    printf("A WRITER WANTS TO WRITE!\n");
    while(nReaders > 0){
        wGo.Wait(&l);
    }
    currentThread->Yield();
    printf("WRITER WRITING\n");
    wantToWrite = false;
    printf("WRITER FINISHED WRITING\n");
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
    printf("READING\n");
    l.Acquire();
    printf("READER FINISHED READING!\n");
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
    #ifdef DEBUG
    printf("L1 Holding Lock? %i\n", locktest1->isHeldByCurrentThread());
    #endif
    printf("L1:0\n");
    locktest1->Acquire();
    #ifdef DEBUG
    printf("L1 Holding Lock? %i\n", locktest1->isHeldByCurrentThread());
    #endif
    printf("L1:1\n");
    currentThread->Yield();
    printf("L1:2\n");
    locktest1->Release();
    #ifdef DEBUG
    printf("L1 Holding Lock? %i\n", locktest1->isHeldByCurrentThread());
    #endif
    printf("L1:3\n");
}

void
LockThread2(int param)
{
    printf("L2:0\n");
    #ifdef DEBUG
    printf("L2 Holding Lock? %i\n", locktest1->isHeldByCurrentThread());
    #endif
    locktest1->Acquire();
    #ifdef DEBUG
    printf("L2 Holding Lock? %i\n", locktest1->isHeldByCurrentThread());
    #endif
    printf("L2:1\n");
    currentThread->Yield();
    printf("L2:2\n");
    locktest1->Release();
    #ifdef DEBUG
    printf("L2 Holding Lock? %i\n", locktest1->isHeldByCurrentThread());
    #endif
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
        ReadersAndWriters();
        break;
    default:
        printf("No test specified.\n");
        break;
    }
}

