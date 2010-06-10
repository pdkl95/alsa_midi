#include "alsa_midi_seq.h"

fifo_t *fifo_alloc(size_t num_items)
{
  fifo_t *f = ALLOC(fifo_t);
  f->size = num_items;
  f->buf = ALLOC_N(void *, f->size);
  f->rd = f->wr = 0;
  return f;
}

void fifo_free(fifo_t *f)
{
  xfree(f->buf);
  xfree(f);
}

int fifo_write_ex(fifo_t *f, void *p)
{
  int ret = fifo_write(f, p);
  if (ret == 0) {
    //rb_raise(aMIDI_Error, "Internal FIFO write failed?! Make bigger FIFOs?");
    rb_bug("Ingernal FIFO write failed?!\n"
           "***** This shouldn't happen! *****\n"
           "Increase the internal FIFO size!\n"
           "(probably EV_FIFO_SIZE in client.h)");
  }
  return ret;
}

int fifo_write(fifo_t *f, void *p)
{
  int next = FIFO_NEXT_WR(f);
  if (f->rd == next) {
    return 0; // full
  }

  f->buf[f->wr] = p;
  f->wr = next;
  return 1;
}

void *fifo_read(fifo_t *f)
{
  void *ret;
  if (f->rd == f->wr) {
    return NULL;
  }
  ret = f->buf[f->rd];
  f->rd = FIFO_NEXT_RD(f);
  return ret;
}
