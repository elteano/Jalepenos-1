// addrspace.cc
//  Routines to manage address spaces (executing user programs).
//
//  In order to run a user program, you must:
//
//  1. link with the -N -T 0 option
//  2. run coff2noff to convert the object file to Nachos format
//    (Nachos object code format is essentially just a simpler
//    version of the UNIX executable object code format)
//  3. load the NOFF file into the Nachos file system
//    (if you haven't implemented the file system yet, you
//    don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif

MemoryManager * memmanage;

//----------------------------------------------------------------------
// SwapHeader
//  Do little endian to big endian conversion on the bytes in the
//  object file header, in case the file was generated on a little
//  endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//  Create an address space to run a user program.
//  Load the program from a file "executable", and set everything
//  up so that we can start executing user instructions.
//
//  Assumes that the object code file is in NOFF format.
//
//  First, set up the translation from program memory to physical
//  memory.  For now, this is really simple (1:1), since we are
//  only uniprogramming, and we have a single unsegmented page table
//
//  "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
  // Do nothing =D
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//  Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete [] pageTable;
}


//----------------------------------------------------------------------
// AddrSpace::Initialize
//  Initialize an address space, knowing that we might fail.
//
//  Something something something comments dark side.
//----------------------------------------------------------------------

AddrSpace*
AddrSpace::Initialize(OpenFile *executable)
{
    AddrSpace* ret = new AddrSpace(NULL);
    ret->stored_executable = executable;

    NoffHeader noffH;
    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
            (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
           + UserStackSize; // we need to increase the size
    // to leave room for the stack
    ret->numPages = divRoundUp(size, PageSize);
    size = ret->numPages * PageSize;

    ASSERT(ret->numPages <= NumPhysPages);    // check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
          ret->numPages, size);
// first, set up the translation
    ret->pageTable = new TranslationEntry[ret->numPages];
    for (i = 0; i < ret->numPages; i++) {
        ret->pageTable[i].virtualPage = i;
        // Request page from memory manager
        ret->pageTable[i].physicalPage = -1;
        // No more need to ensure we have a valid page
        // Rest of initialization code is fine
        ret->pageTable[i].valid = FALSE;
        ret->pageTable[i].use = FALSE;
        ret->pageTable[i].dirty = FALSE;
        ret->pageTable[i].readOnly = FALSE;  // if the code segment was entirely on
        // a separate page, we could set its
        // pages to be read-only
    }

    return ret;

}


bool
AddrSpace::demandpage(int page_num)
{
  //1.4 add demandpage pagefault handler
  NoffHeader noffH;
  int page_addr = page_num * PageSize;
  //store stored_executable into noffH
  stored_executable->ReadAt((char *)&noffH, sizeof(noffH), 0);

  //does something idk if i need this here.
  if ((noffH.noffMagic != NOFFMAGIC) &&
      (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    SwapHeader(&noffH);
  ASSERT(noffH.noffMagic == NOFFMAGIC);
  pageTable[page_num].physicalPage = memmanage->AllocPage();

  if (pageTable[page_num].physicalPage < 0)
  {
    DEBUG('a', "got page %d.\n", pageTable[page_num].physicalPage);
    return false;
  }

  //1.4.2 if pagefault on data: read data from stored_executable
  if(noffH.code.virtualAddr <= page_addr
      && noffH.code.virtualAddr + noffH.code.size > page_addr){
    int physLocation = pageTable[page_num].physicalPage
      * PageSize;
    int offset = (page_num * PageSize) - noffH.code.virtualAddr;

    // Don't need to worry about being in the middle of a page, because the
    // entire page was requested...

    stored_executable->ReadAt(&(machine->mainMemory[physLocation]),
        PageSize, noffH.code.inFileAddr + offset);
  }
  else if(noffH.initData.virtualAddr <= page_addr
      && noffH.initData.virtualAddr + noffH.initData.size > page_addr){
    int physLocation = pageTable[page_num].physicalPage
      * PageSize;
    int offset = (page_num * PageSize) - noffH.initData.virtualAddr;

    stored_executable->ReadAt(&(machine->mainMemory[physLocation]),
        PageSize, noffH.initData.inFileAddr + offset);
  }
  //1.4.3 if pagefault on anything else: zero-fill it 
  else{
    // Zero out the current page
    bzero(&machine->mainMemory[page_addr],
        PageSize);
  }

  //1.5.1 mark PTE as valid
  pageTable[page_num].valid = TRUE;
  //1.5.2 restart execution of user program without incrementing PC
  return true;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//  Set the initial values for the user-level register set.
//
//  We write these directly into the "machine" registers, so
//  that we can immediately jump to user code.  Note that these
//  will be saved/restored into the currentThread->userRegisters
//  when this thread is context switched out.
//----------------------------------------------------------------------
void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//  On a context switch, save any machine state, specific
//  to this address space, that needs saving.
//
//  For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//  On a context switch, restore the machine state so that
//  this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}


//----------------------------------------------------------------------
// AddrSpace::ClearState
//  Delete the entire memory footprint of the application, allowing it to run 
//  while taking up no memory whatsoever.
//----------------------------------------------------------------------

void AddrSpace::ClearState()
{
    for (unsigned int page = 0; page < numPages; ++page)
    {
        memmanage->FreePage(pageTable[page].physicalPage);
    }
}

