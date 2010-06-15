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

#define FIFO_NEXT(x, size) ((x + 1) % size)

#define FIFO_FLUSH(f, p)            \
  while((p = fifo_read(f)) != NULL) \

#endif /*FIFO_H*/
