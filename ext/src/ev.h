#ifndef EV_H
#define EV_H

#define EV_NULL 0
#define EV_NOTE 1

#define EV_FLAG_NOTEON  0x0001
#define EV_FLAG_NOTEOFF 0x0002
#define EV_FLAG_NOTE    (EV_SUB_NOTEON | EV_SUB_NOTEOFF)
#define EV_FLAG_RAWNOTE 0x0010
#define EV_FLAG_TNOTE   0x0020
#define EV_MEM_RUBY 1
#define EV_MEM_FIFO 2

struct seq_event {
  int type;
  int flags;
  int mem;

  scale_t *scale;

  int port_id;
  int channel;

  int note;
  int note_offset;
  int note_octave;

  int velocity;
};
typedef struct seq_event ev_t;

#define GET_EV_STRUCT(obj)        \
  ev_t *ev;                       \
  Data_Get_Struct(obj, ev_t, ev);

#define GET_EV GET_EV_STRUCT(self)

int midi_note_from_ev(ev_t *ev);

void Init_aMIDI_Ev();



#endif /*EV_H*/
