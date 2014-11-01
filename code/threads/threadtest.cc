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

void badConditionTest1(int param){
    DEBUG('t', "Attempt to signal a condition that isn't waiting\n");
    wGo.Signal(&l); // signaling a condition that we don't have the lock to
}
void badConditionTest2(int param){
    DEBUG('t', "Attempt to broadcast a condition that isn't waiting\n");
    wGo.Broadcast(&l);
}
void CondTest1(){
    Thread * t = new Thread("one");
    t->Fork(badConditionTest1, 0);
}
void CondTest2(){
    Thread * t = new Thread("one");
    t->Fork(badConditionTest2, 0);
}

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
        printf("%s: \"there is a reader... I'm going to bed...\"\n", currentThread->getName());
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
    printf("%s wants to read\n", currentThread->getName());
    while(wantToWrite){
        printf("Ikadsfalksdfasd\n");
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
        printf("%s: \"No more readers... I'm going to bed...\"\n", currentThread->getName());
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
    DEBUG('t', "First Acquiring lock, then releasing... \n");
    locktest2->Acquire();
    locktest2->Release();
    delete locktest2;
    DEBUG('t', "\tSUCCESS!!\n");
    DEBUG('t', "Attempting to Acquire a lock and then delete it...\n");
    locktest2 = new Lock("LockTest2-bad");
    locktest2->Acquire();
    delete locktest2;
    DEBUG('t', "\tFAILURE!\nWe have deleted a lock that is in use...\n");
    locktest2->Release();
}//
void LockTest2(){
    DEBUG('t', "Entering LockTest2");
    locktest2 = new Lock("LockTest2");
    Thread * t;
    t = new Thread("one");
    t->Fork(Lock2Thread1, 0);
}//

void LockTest3Thread(int param){
    locktest2->Acquire();
    locktest2->Acquire();    
}//
void LockTest3(){
    DEBUG('t', "Entering LockTest3\n");
    locktest2 = new Lock("locktest3");
    Thread * t;
    t = new Thread("one");
    t->Fork(LockTest3Thread, 0);
}
//----------------------------------------------------------------------
// Mailbox tests
//----------------------------------------------------------------------
Mailbox sndr("mailbox");

void MailboxSend(int param){
    DEBUG('t', "\"%s\" entering MailboxSend\n", currentThread->getName());
    printf("sender has sent : %d\n", param);   
    sndr.Send(param);
     
    DEBUG('t', "\"%s\" exiting MailboxSend\n", currentThread->getName());
}
void MailboxReceive(int param){
    int i = 0;
    //printf("%s entering Mailboxreceive\n" , currentThread->getName());
    sndr.Receive(&i);
    printf ("Receiver has received : %d\n", i);
}

void MailboxTest1(){
    //DebugInit("i");
    printf("MailboxTest1: Sender then Receiver\n");
    Thread * t;
    t = new Thread("Sender");
    t->Fork(MailboxSend, 1347);
    t = new Thread("Receiver");
    t->Fork(MailboxReceive, 0);
}

/*
 * Test     : RECEIVE -> SEND
 * Expected : 
 */
void MailboxTest2(){
    printf("MailboxTest2: Receiver then Sender\n");
    Thread * t;
    t = new Thread("Receiver");
    t->Fork(MailboxReceive, 0);
    t = new Thread("Sender");
    t->Fork(MailboxSend, 1337);
}
/*
 * Test     : RECEIVE -> RECEIVE -> SEND ->
 *          : RECEIVE -> SEND -> SEND -> 
 *          : SEND -> SEND -> SEND
 * Expected : 
 */
void MailboxTest3(){
    DEBUG('t', "Entering MailboxTest3");
    Thread * t;
    t = new Thread("Receiver1");
    t->Fork(MailboxReceive, 0);
    t = new Thread("Receiver2");
    t->Fork(MailboxReceive, 0);
    t = new Thread("Sender1");
    t->Fork(MailboxSend, 4);
    t = new Thread("Sender2");
    t->Fork(MailboxSend, 5);
    t = new Thread("Sender3");
    t->Fork(MailboxSend, 3);
}
void MailboxTest4(){
    DEBUG('t', "Entering MailboxTest3");
    Thread * t;
    t = new Thread("Receiver1");
    t->Fork(MailboxReceive, 0);
    t = new Thread("Receiver2");
    t->Fork(MailboxReceive, 0);
    t = new Thread("Receiver3");
    t->Fork(MailboxReceive, 0);
    t = new Thread("Sender1");
    t->Fork(MailboxSend, 4);
    t = new Thread("Sender2");
    t->Fork(MailboxSend, 5);
}

//----------------------------------------------------------------------
// Join tests
//  Tests for joining
//----------------------------------------------------------------------

void Joinee(int param){
    DEBUG('t', "Entering JoinStart with Thread: %s\n", currentThread->getName());
    printf("Joinee needs to poo\n");//change poo to something more appropriate when finished ;p
    for(int i = 0; i < 5; i++){
      printf("Joinee still pooping!\n");
      currentThread->Yield();
    }
    currentThread->Yield();
    printf("Joinee is Done\n");
    currentThread->Yield();
    DEBUG('t', "Exiting JoinStart with Thread: %s\n", currentThread->getName());
}

void Joiner(Thread * joinee){
    DEBUG('t', "Entering JoinStart with Thread: %s\n", currentThread->getName());
    currentThread->Yield();
    printf("Is you done\n");
    joinee->Join(); // claim that we need to wait until joinee is done
    printf("Joinee should be done pooping\n");
    currentThread->Yield();
    currentThread->Yield();
    DEBUG('t', "Exiting JoinStart with Thread: %s\n", currentThread->getName());
}

void JoinTest1(){
    printf("Sublime entrance of JoinTest1...\n");
    Thread *joiner = new Thread("Joiner", 0);
    Thread *joinee = new Thread("Joinee", 1);
    joiner->Fork((VoidFunctionPtr)Joiner, (int) joinee);
    joinee->Fork((VoidFunctionPtr)Joinee, 0);
    printf("Graceful exit of JoinTest1...\n");
}

void JoinTask2(int param){
    DEBUG('t', "Entering JoinStart with Thread: %s\n", currentThread->getName());
    printf("We're in the join thread\n");
    printf("Me first!\n");
    DEBUG('t', "Exiting JoinStart with Thread: %s\n", currentThread->getName());
}


void JoinStart2(int param){ // 
    Thread * th = new Thread("Child", 1);
    th->Fork(JoinTask2, 0);
    currentThread->Yield();
    th->Join();
    printf("I've completed my duty...\n");
}

void JoinTest2(){
    //--- this test will insure that the child will finish before the parent
    //--- in other words, we will force the Child to wait for the Parent
    printf("Immaculate entrance of JoinTest2...\n");
    Thread * t = new Thread("Parent");
    t->Fork(JoinStart2, 0);

    printf("Perfect exit of JoinTest2...\n");
}

//----------------------------------------------------------------------
// Priority tests
//----------------------------------------------------------------------

void PriorityYieldTask(int ignore)
{
    DEBUG('t', "Entering empty task with \"%s\" priority %d.\n",
            currentThread->getName(), currentThread->getPriority());
    currentThread->Yield();
    DEBUG('t', "Exiting empty task with \"%s\" priority %d.\n",
            currentThread->getName(), currentThread->getPriority());
}

void PriorityEmptyTask(int ignore)
{
    DEBUG('t', "Empty task on thread \"%s\" at priority %d.\n",
            currentThread->getName(), currentThread->getPriority());
}

void PriorityOrderTest()
{
    DEBUG('t', "Entering PriorityOrderYieldTest.\n");
    DEBUG('t', "Expected order is main->P1->P3->P5->main.\n");
    Thread * t1 = new Thread("P1");
    t1->setPriority(1);
    t1->Fork(PriorityEmptyTask, 0);
    Thread * t5 = new Thread("P5");
    t5->setPriority(5);
    t5->Fork(PriorityEmptyTask, 0);
    Thread * t3 = new Thread("P3");
    t3->setPriority(3);
    t3->Fork(PriorityEmptyTask, 0);
    currentThread->setPriority(8);
    DEBUG('t', "Main thread yielding to next thread as priority %d.\n",
            currentThread->getPriority());
    currentThread->Yield();
}

void PriorityOrderYieldTest()
{
    DEBUG('t', "Entering PriorityOrderYieldTest.\n");
    DEBUG('t', "Expected order is main->P1->P3->P1->P3->P5->main->P5.\n");
    Thread * t1 = new Thread("P1");
    t1->setPriority(1);
    t1->Fork(PriorityYieldTask, 0);
    Thread * t5 = new Thread("P5");
    t5->setPriority(5);
    t5->Fork(PriorityYieldTask, 0);
    Thread * t3 = new Thread("P3");
    t3->setPriority(3);
    t3->Fork(PriorityYieldTask, 0);
    currentThread->setPriority(8);
    DEBUG('t', "Main thread yielding to next thread as priority %d.\n",
            currentThread->getPriority());
    currentThread->Yield();
}

void PriorityLockTaskIce(int lock)
{
    ((Lock *) lock)->Acquire();
    currentThread->Yield();
    printf("Ice ");
    currentThread->Yield();
    ((Lock *) lock)->Release();
}

void PriorityLockTaskCream(int lock)
{
    ((Lock *) lock)->Acquire();
    currentThread->Yield();
    printf("cream ");
    ((Lock *) lock)->Release();
}

void PriorityLockTaskDonuts(int lock)
{
    ((Lock *) lock)->Acquire();
    currentThread->Yield();
    printf("donuts!\n");
    ((Lock *) lock)->Release();
}

void PriorityLockOrderTest()
{
    printf("Expected print:\nIce cream donuts!\n");
    Thread * p1 = new Thread("P1");
    Thread * p2 = new Thread("P2");
    Thread * p3 = new Thread("P3");
    p1->setPriority(1);
    p2->setPriority(2);
    p3->setPriority(3);
    Lock * lock = new Lock("Priority Order");
    p3->Fork(PriorityLockTaskIce, (int) lock);
    // Make sure that p3 has a chance to acquire the lock
    currentThread->Yield();
    p2->Fork(PriorityLockTaskDonuts, (int) lock);
    currentThread->Yield();
    p1->Fork(PriorityLockTaskCream, (int) lock);
}

//----------------------------------------------------------------------
// Whale tests
// 	Test the whale mating routines
//----------------------------------------------------------------------

Whale *whale;

void FemaleTest(int ignore)
{
    DEBUG('w', "Thread \"%s\" entering Female.\n", currentThread->getName());
    whale->Female();
    DEBUG('w', "Thread \"%s\" has mated as a female.\n",
            currentThread->getName());
}

void MaleTest(int ignore)
{
    DEBUG('w', "Thread \"%s\" entering Male.\n", currentThread->getName());
    whale->Male();
    DEBUG('w', "Thread \"%s\" has mated as a male.\n", currentThread->getName());
}

void MatchTest(int ignore)
{
    DEBUG('w', "Thread \"%s\" entering Match.\n", currentThread->getName());
    whale->Matchmaker();
    DEBUG('w', "Thread \"%s\" has made a match!\n", currentThread->getName());
}

void WhaleTest()
{
    whale = new Whale("Da Whale");
    Thread * wmale = new Thread("Male");
    Thread * wfemale = new Thread("Female");
    Thread * wmatch = new Thread("Matchmaker");
    wmale->Fork(MaleTest, 0);
    wfemale->Fork(FemaleTest, 0);
    wmatch->Fork(MatchTest, 0);
    currentThread->Yield();
}

void WhaleMaidenTest()
{
    whale = new Whale("Da Whale");
    Thread * wmale = new Thread("Male");
    Thread * wfemale = new Thread("Female");
    Thread * wfemale2 = new Thread("Extra Female");
    Thread * wmatch = new Thread("Matchmaker");
    wmale->Fork(MaleTest, 0);
    wfemale->Fork(FemaleTest, 0);
    wfemale2->Fork(FemaleTest, 0);
    wmatch->Fork(MatchTest, 0);
    currentThread->Yield();
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DebugInit("w");
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
        LockTest3();
        break;
    case 20:
        ReadersAndWriters();
        break;
    case 21:
        CondTest1();
        break;
    case 22:
        CondTest2();
        break;
    case 30:
        MailboxTest1();
        break;
    case 31:
        MailboxTest2();
        break;
    case 32:
        MailboxTest3();
        break;
    case 33:
        MailboxTest4();
        break;
    case 40:
        PriorityOrderTest();
        break;
    case 41:
        PriorityOrderYieldTest();
        break;
    case 42:
        PriorityLockOrderTest();
        break;
    case 50:
        JoinTest1();
        break;
    case 51:
        JoinTest2();
        break;
    case 60:
        WhaleTest();
        break;
    case 61:
        WhaleMaidenTest();
        break;
    default:
        printf("No test specified.\n");
        break;
    }
}

