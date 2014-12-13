#include "addrspace.h"
#include "backingstore.h"
#include "openfile.h"
#include "machine.h"
#include "system.h"
#include "filesys.h"

BackingStore::BackingStore(int start_size)
{
  space = NULL;
  char fname[1024];
  sprintf(&fname[0], "bs%u", (unsigned int) start_size);
  fileSystem->Create(fname, start_size);
  file = fileSystem->Open(fname);
}

void
BackingStore::PageOut(TranslationEntry *pte)
{
  pte->valid = 0;
  //--- check to see if the entry being written out is dirty
  //--- only increment the data if it's been modified
  stats->numPageOuts = (pte->dirty)? stats->numPageOuts + 1 : stats->numPageOuts;

  file->WriteAt(&machine->mainMemory[pte->physicalPage * PageSize],
      PageSize, pte->virtualPage * PageSize);
  memmanage->FreePage(pte->physicalPage);
}

void
BackingStore::PageIn(TranslationEntry *pte)
{
  pte->valid = 1;
  //--- always increment the number of numPageIns from here
  stats->numPageIns = stats->numPageIns + 1;

  file->ReadAt(&machine->mainMemory[pte->physicalPage * PageSize],
      PageSize, pte->virtualPage * PageSize);
}
