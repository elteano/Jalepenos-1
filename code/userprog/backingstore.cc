#include "addrspace.h"
#include "backingstore.h"
#include "openfile.h"
#include "machine.h"
#include "system.h"
#include "filesys.h"

BackingStore::BackingStore(int start_size)
{
  space = NULL;
  sprintf(&fname[0], "bs%u", (unsigned int) start_size);
  fileSystem->Create(fname, start_size);
  file = fileSystem->Open(fname);
}

void
BackingStore::PageOut(TranslationEntry *pte)
{
  pte->valid = FALSE;
  //--- check to see if the entry being written out is dirty
  //--- only increment the data if it's been modified

  DEBUG('y', "Dirty bit %d.\n", pte->dirty);
  if (pte->dirty)
  {
    DEBUG('y', "Dirty page, paging out.\n");
    stats->numPageOuts = stats->numPageOuts + 1;
    file->WriteAt(&machine->mainMemory[pte->physicalPage * PageSize],
        PageSize, pte->virtualPage * PageSize);
  }
  else
  {
    DEBUG('y', "No page out, clean page.\n");
  }
  memmanage->FreePage(pte->physicalPage);
}

void
BackingStore::PageIn(TranslationEntry *pte)
{
  pte->valid = TRUE;
  //--- always increment the number of numPageIns from here
  stats->numPageIns = stats->numPageIns + 1;

  file->ReadAt(&machine->mainMemory[pte->physicalPage * PageSize],
      PageSize, pte->virtualPage * PageSize);
  pte->dirty = FALSE;
}

BackingStore::~BackingStore()
{
  fileSystem->Remove(fname);
  delete file;
}
