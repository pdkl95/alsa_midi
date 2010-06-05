#ifndef ALSA_CLIENT_H
#define ALSA_CLIENT_H

struct alsa_midi_seq_client {
  snd_seq_t           *handle;
  snd_seq_tick_time_t  tick;
  char *name;

  int queue_id;
  int ppq; // Pulses Per Quarter
  int bpm;
  int tempo;
};
typedef struct alsa_midi_seq_client alsa_midi_client_t;

#define GET_CLIENT_STRUCT(obj)                       \
  alsa_midi_client_t *client;                        \
  Data_Get_Struct(obj, alsa_midi_client_t, client);

#define GET_CLIENT GET_CLIENT_STRUCT(self)

void Init_aMIDI_Client();

#endif /*ALSA_CLIENT_H*/
