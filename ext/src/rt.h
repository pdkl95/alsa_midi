#ifndef RT_H
#define RT_H

#include <pthread.h>

typedef struct timespec ts_t;

struct rt_worker {
  pthread_t      thread;
  pthread_attr_t attr;

  snd_seq_t *client_handle;

  int running;
  int exit_status;

  ts_t period;
  
  fifo_t midi_tx_fifo;
  fifo_t free_ev_fifo;
};
typedef struct rt_worker rt_t;

#define GET_RT_STRUCT(obj)         \
  rt_t *rt;                        \
  Data_Get_Struct(obj, rt_t, rt);

#define GET_RT GET_RT_STRUCT(self)


extern void Init_aMIDI_RT();

#define MIDI_EV_FIFO_SIZE 1024
#define MIDI_EV_FIFO(obj) \
  fifo_init(&obj, sizeof(snd_seq_event_t), MIDI_EV_FIFO_SIZE);

#endif /*RT_H*/
