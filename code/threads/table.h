
#ifndef TABLE_H
#define TABLE_H
#include "synch.h"

class Table
{
    public:
      // Create a table holding the specified number of entries
      Table(int size);
      ~Table();

      int Alloc(void *object);

      void *Get(int index);

      void Release(int index);

    private:
      Lock *access_lock;
      void **backing;
      int last_element;
      int max_elements;
};

extern Table *threadlist;

#endif // TABLE_H

