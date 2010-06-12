#ifndef ALSA_CLIENT_H
#define ALSA_CLIENT_H

#define EV_FIFO_SIZE 1024
#define RT_WRK "AlsaMIDI::Client::RT> "
#include <pthread.h>

#define GETTIME_CLOCK CLOCK_MONOTONIC
#define  NSLEEP_CLOCK CLOCK_MONOTONIC

struct alsa_midi_seq_client {
  snd_seq_t *seq;
  char      *name;

  pthread_t      thread;
  pthread_attr_t thread_attr;
  int            thread_running;
  int            thread_exit_status;
  ts_t           thread_period;
  ev_t          *thread_delay_pool;

  fifo_t *ev_free;
  fifo_t *ev_tx;
  fifo_t *ev_return;

  int client_id;

  // meter
  int clocks_per_beat;
  int beats_per_measure;
  int clock;
  int beat;
  int measure;

  // tempo
  int bpm;
  int clocks_per_minute;
  float clocks_per_second;

};
typedef struct alsa_midi_seq_client client_t;

#define GET_CLIENT_STRUCT(obj)            \
  client_t *client;                       \
  Data_Get_Struct(obj, client_t, client);

#define GET_CLIENT GET_CLIENT_STRUCT(self)

void Init_aMIDI_Client();

#endif /*ALSA_CLIENT_H*/
