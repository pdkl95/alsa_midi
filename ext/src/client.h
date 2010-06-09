#ifndef ALSA_CLIENT_H
#define ALSA_CLIENT_H

#include <pthread.h>

typedef struct timespec ts_t;

struct alsa_midi_seq_client {
  snd_seq_t *seq;
  char      *name;

  pthread_t      thread;
  pthread_attr_t thread_attr;
  int            thread_running;
  int            thread_exit_status;
  ts_t           thread_period;

  fifo_t *ev_free;
  fifo_t *ev_tx;

  int client_id;
  int ppq; // Pulses Per Quarter
  int bpm;
  int tempo;
};
typedef struct alsa_midi_seq_client client_t;

#define GET_CLIENT_STRUCT(obj)            \
  client_t *client;                       \
  Data_Get_Struct(obj, client_t, client);

#define GET_CLIENT GET_CLIENT_STRUCT(self)

void Init_aMIDI_Client();

#define EV_FIFO_SIZE 1024

#endif /*ALSA_CLIENT_H*/
