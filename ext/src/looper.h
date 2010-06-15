#ifndef LOOPER_H
#define LOOPER_H

#define LOOPER_SEQ_MAX_STEPS 1024

#define LOOPER_UNDEF    0
#define LOOPER_SEQ      1
#define LOOPER_SEQ_MONO 2

struct looper_widget {
  struct looper_widget *next;
  struct looper_widget *prev;
  
  struct alsa_midi_seq_client *client;
  int port_id;
  int channel;

  int type;
  scale_t *scale;

  int   seq_len;
  ev_t *seq_ev;
};
typedef struct looper_widget looper_t;

#define GET_LOOPER_STRUCT(obj)            \
  looper_t *looper;                       \
  Data_Get_Struct(obj, looper_t, looper);

#define GET_LOOPER GET_LOOPER_STRUCT(self)

#define LOOPER_EV_AT_POS(looper, pos) \
  &(looper->seq_ev[pos])

#define LOOPER_EV_AT(looper, beat) \
  LOOPER_EV_AT_POS(looper, ((beat) % (looper)->seq_len))

void Init_aMIDI_Looper();

#endif /*LOOPER_H*/
