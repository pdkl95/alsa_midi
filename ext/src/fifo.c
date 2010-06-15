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
  int r, w, next;
  r = f->rd;
  w = f->wr;

  next = FIFO_NEXT(w, f->size);
  if (r == next) {
    return 0; // full
  }

  f->buf[w] = p;
  f->wr = next;
  return 1;
}

void *fifo_read(fifo_t *f)
{
  void *ret;
  int r, w;
  r = f->rd;
  w = f->wr;

  if (r == w) {
    return NULL;
  }

  ret = f->buf[r];
  r = FIFO_NEXT(r, f->size);
  f->rd = r;
  return ret;
}
