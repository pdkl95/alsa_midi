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
  int        client_id;

  pthread_t      thread;
  pthread_attr_t thread_attr;
  int8_t         thread_running;
  int8_t         thread_exit_status;
  ts_t           thread_period;
  ev_t          *thread_delay_pool;

  fifo_t *ev_free;
  fifo_t *ev_tx;
  fifo_t *ev_return;

  // auto-run looping widgets
  looper_t *looper_widgets;

  // meter
  uint8_t  clocks_per_beat;
  uint8_t  beats_per_measure;
  uint16_t clock;
  uint8_t  beat;
  uint8_t  measure;

  uint64_t clock_total;
  uint64_t beat_total;

  // tempo
  uint16_t bpm;
  uint16_t clocks_per_minute;
  float clocks_per_second;

};
typedef struct alsa_midi_seq_client client_t;

#define GET_CLIENT_STRUCT(obj)            \
  client_t *client;                       \
  Data_Get_Struct(obj, client_t, client);

#define GET_CLIENT GET_CLIENT_STRUCT(self)

void Init_aMIDI_Client();

#endif /*ALSA_CLIENT_H*/
