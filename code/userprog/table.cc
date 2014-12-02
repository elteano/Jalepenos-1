
#include "table.h"
#include "synch.h"

Table::Table(int size)
{
    backing = new void*[size];
    access_lock = new Lock("Table Lock");
    max_elements = size;
    last_element = 0;
}

Table::~Table()
{
    delete backing;
    delete access_lock;
}

int Table::Alloc(void *object)
{
    access_lock->Acquire();
    int next_element = last_element;
    while (next_element < max_elements + last_element)
    {
      if (backing[next_element] == NULL)
      {
        backing[next_element] = object;
        access_lock->Release();
        return last_element = next_element+1;
      }
      ++next_element;
    }
    return -1;
    access_lock->Release();
}

void *Table::Get(int index)
{
    return backing[index-1];
}

void Table::Release(int index)
{
    access_lock->Acquire();
    backing[index-1] = NULL;
    access_lock->Release();
}

Table *threadlist;

