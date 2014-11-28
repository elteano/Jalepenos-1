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
        ret->pageTable[i].physicalPage = memmanage->AllocPage();
        // Ensure that we were given a page
        ASSERT(ret->pageTable[i].physicalPage >= 0);
        // Rest of initialization code is fine
        ret->pageTable[i].valid = TRUE;
        ret->pageTable[i].use = FALSE;
        ret->pageTable[i].dirty = FALSE;
        ret->pageTable[i].readOnly = FALSE;  // if the code segment was entirely on
        // a separate page, we could set its
        // pages to be read-only
    }

    // Zero out all allocated memory
    for (unsigned int pageNum = 0; pageNum < ret->numPages; ++pageNum)
    {
      int physPage = ret->pageTable[pageNum].physicalPage;
      bzero(&machine->mainMemory[physPage * PageSize], PageSize);
    }

    // Copy code and data segments into memory
    unsigned int pageNum;
    unsigned int file_offset;
    int virt_addr;

    virt_addr = noffH.code.virtualAddr;

    ASSERT(virt_addr >= 0);

    pageNum = (unsigned) virt_addr / PageSize;
    file_offset = (unsigned) virt_addr % PageSize;

    // Copy all code information into physical memory
    unsigned int numWholeCodePages = noffH.code.size / PageSize;
    int firstWholePage = pageNum;
    int codeOverflow = noffH.code.size % PageSize - file_offset;
    if (codeOverflow < 0)
    {
      codeOverflow += PageSize;
    }
    // If we don't start at the beginning of a page, write the first partial
    if (file_offset > 0)
    {
      ++firstWholePage;
      int physLocation = pageNum * PageSize + file_offset;
      executable->ReadAt(&(machine->mainMemory[physLocation]),
          PageSize - file_offset, noffH.code.inFileAddr);
    }
    unsigned int cPage;
    for (cPage = 0; cPage < numWholeCodePages; ++cPage)
    {
      int physLocation = (pageNum + cPage) * PageSize;
      executable->ReadAt(&(machine->mainMemory[physLocation]),
          PageSize, firstWholePage + cPage * PageSize);
    }
    // Copy all remaining data into physical memory
    if (codeOverflow > 0)
    {
      int physLocation = (pageNum + cPage) * PageSize;
      executable->ReadAt(&(machine->mainMemory[physLocation]),
          codeOverflow, firstWholePage + cPage * PageSize);
    }

    virt_addr = noffH.initData.virtualAddr;

    ASSERT(virt_addr >= 0);

    pageNum = (unsigned) virt_addr / PageSize;
    file_offset = (unsigned) virt_addr % PageSize;

    // Copy all data information into physical memory
    unsigned int numWholeDataPages = noffH.initData.size / PageSize;
    firstWholePage = pageNum;
    int dataOverflow = noffH.initData.size % PageSize - file_offset;
    if (dataOverflow < 0)
    {
      dataOverflow += PageSize;
    }
    // If we don't start at the beginning of a page, write the first partial
    if (file_offset > 0)
    {
      ++firstWholePage;
      int physLocation = pageNum * PageSize + file_offset;
      executable->ReadAt(&(machine->mainMemory[physLocation]),
          PageSize - file_offset, noffH.initData.inFileAddr);
    }
    for (cPage = 0; cPage < numWholeDataPages; ++cPage)
    {
      int physLocation = (pageNum + cPage) * PageSize;
      executable->ReadAt(&(machine->mainMemory[physLocation]),
          PageSize, firstWholePage + cPage * PageSize);
    }
    if (dataOverflow > 0)
    {
      int physLocation = (pageNum + cPage) * PageSize;
      executable->ReadAt(&(machine->mainMemory[physLocation]),
          dataOverflow, firstWholePage + cPage * PageSize);
    }
    return ret;
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
