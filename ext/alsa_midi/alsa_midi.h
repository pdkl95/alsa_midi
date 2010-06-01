#ifndef ALSA_MIDI_H
#define ALSA_MIDI_H

#define DEFAULT_CLIENT_NAME       "AlsaMIDILooper"
#define DEFAULT_TICKS_PER_QUARTER 128
#define SHUTDOWN_WAIT_TIME        2

#include "ruby.h"
#include <stdio.h>
#include <stdlib.h>
#include <asoundlib.h>

extern VALUE aMIDI_mod;
extern VALUE aMIDI_cBase;
extern VALUE aMIDI_eAlsaError;

struct midi_seq_client {
  snd_seq_t *handle;
  snd_seq_tick_time_t tick;

  int ticks_per_quarter;
  int queue_id;
  int port_out_id;
  int port_in_id;
  int transpose;
  int bpm0;
  int bpm;
  int tempo;
  int swing;
  int seq_len;
};
typedef struct midi_seq_client seq_client_t;

#define GET_BASE(obj)                             \
  seq_client_t *seq;                              \
  Data_Get_Struct(obj, seq_client_t, seq);

VALUE alsa_midi_klass_base();
VALUE alsa_midi_klass_base_new();

#endif /*ALSA_MIDI_H*/
