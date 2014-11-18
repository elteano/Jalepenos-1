
#include "memmanage.h"

MemoryManager::MemoryManager(int numPages)
{
  mNumPages  numPages;
  bmap = BitMap(numPages);
}

int MemoryManager::AllocPage()
{
  for (int i = 0; i < mNumPages; ++i) {
    if (!bmap.Test(i)) {
      bmap.Mark(i);
      return i;
    }
  }
  return -1;
}

void MemoryManager::FreePage(int physPageNum)
{
  bmap.Clear(physPageNum);
}

bool MemoryManager::PageIsAllocated(int physPageNum)
{
  return bmap.Test(physPageNum);
}

