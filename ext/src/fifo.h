#ifndef FIFO_H
#define FIFO_H

struct lockless_fifo {
  size_t size;
  void **buf;

  volatile int rd;
  volatile int wr;
};
typedef struct lockless_fifo fifo_t;

fifo_t *fifo_alloc(size_t num_items);
void fifo_free(fifo_t *f);

int fifo_write(fifo_t *f, void *p);
void *fifo_read(fifo_t *f);

#define FIFO_NEXT(f, x) ((x + 1) % f->size)
#define FIFO_NEXT_RD(f) FIFO_NEXT(f, f->rd)
#define FIFO_NEXT_WR(f) FIFO_NEXT(f, f->wr)

#define FIFO_EACH(f, p)             \
  while((p = fifo_read(f)) != NULL) \

#endif /*FIFO_H*/
