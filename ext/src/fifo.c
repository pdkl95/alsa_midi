#include "alsa_midi_seq.h"

void fifo_init(fifo_t *f, size_t item_sz, size_t item_cnt)
{
  f->item_size  = item_sz;
  f->item_count = item_cnt;
  f->buf = ALLOC_N(void *, item_cnt);
  f->rd = f->wr = 0;
}

void fifo_cleanup(fifo_t *f)
{
  xfree(f->buf);
}

int fifo_write_ex(fifo_t *f, void *p)
{
  int ret = fifo_write(f, p);
  if (ret == 0) {
    rb_raise(aMIDI_Error, "Internal FIFO write failed?! Make bigger FIFOs?");
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
