
#ifndef BACKINGSTOREAWFUL
#define BACKINGSTOREAWFUL

#include "translate.h"
#include "addrspace.h"
#include "filesys.h"

class AddrSpace;

class BackingStore {

  private:
  AddrSpace *space;
  OpenFile *file;

  public:
  BackingStore(int start_size);

  /* Write the virtual page referenced by pte to the backing store */
  /* Example invocation: PageOut(&machine->pageTable[virtualPage]) or */
  /*                     PageOut(&space->pageTable[virtualPage]) */
  void PageOut(TranslationEntry *pte);

  /* Read the virtual page referenced by pte from the backing store */
  void PageIn(TranslationEntry *pte);

};

#endif

