#include "sklaff.h"
#include "ext_globals.h"

void *my_malloc(bytes)
size_t bytes;
{
  void *tmpptr;
  int tmpmemptr;

  tmpmemptr = 0;
  tmpptr = (void *)malloc(bytes);
  if (tmpptr == NULL) output("Malloc failed!\n");
  while ((memstack[tmpmemptr] != 0L) && (tmpmemptr < 30000)) tmpmemptr++;
  if (tmpmemptr < 30000) {
    memstack[tmpmemptr] = (long)tmpptr;
    if (tmpmemptr == (mempointer + 1)) mempointer++;
    return tmpptr;
  }
  else output("Out of memstack!\n");
  exit(1);
}

void
my_free (void *ptr)
{
  int tmppotr;

  tmppotr = 0;
  while ((memstack[tmppotr] != (long)ptr) && (tmppotr < 29999)) tmppotr++;
  if (tmppotr == 29999) {
      output("\nFree without malloc problem, contact clumsy programmer!\n");
      return;
  }
  else memstack[tmppotr] = 0L;
  free(ptr);
  return;
}


