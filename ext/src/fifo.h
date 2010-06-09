#ifndef FIFO_H
#define FIFO_H

struct lockless_fifo {
  size_t item_size;
  size_t item_count;

  void **buf;

  volatile int rd;
  volatile int wr;
};
typedef struct lockless_fifo fifo_t;

void fifo_init(fifo_t *f, size_t item_sz, size_t item_cnt);
void fifo_cleanup(fifo_t *f);

int fifo_write(fifo_t *f, void *p);
void *fifo_read(fifo_t *f);

#define FIFO_NEXT(f, x) ((x + 1) % f->item_count)
#define FIFO_NEXT_RD(f) FIFO_NEXT(f, f->rd)
#define FIFO_NEXT_WR(f) FIFO_NEXT(f, f->wr)

#define FIFO_EACH(f, p)             \
  while((p = fifo_read(f)) != NULL) \

#endif /*FIFO_H*/
