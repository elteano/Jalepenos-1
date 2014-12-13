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
  file->WriteAt(&machine->mainMemory[pte->physicalPage * PageSize],
      PageSize, pte->virtualPage * PageSize);
  memmanage->FreePage(pte->physicalPage);
}

void
BackingStore::PageIn(TranslationEntry *pte)
{
  pte->valid = 1;
  file->ReadAt(&machine->mainMemory[pte->physicalPage * PageSize],
      PageSize, pte->virtualPage * PageSize);
}
