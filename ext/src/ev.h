#ifndef EV_H
#define EV_H

#define EV_MEM_FIFO (0x1)
#define EV_MEM_LOOP (0x2)

#define EV_FLAG_ACTIVE    0x01
#define EV_FLAG_TRANSPOSE 0x02

#define EV_TYPE_NOTEON  (0x1)
#define EV_TYPE_NOTEOFF (0x2)
#define EV_TYPE_NOTE    (EV_TYPE_NOTEON | EV_TYPE_NOTEOFF)

union ev_atomic {
  uint64_t raw;
  struct {
    uint16_t duration;  // in clocks
    uint8_t  note;
    int8_t   octave;
    uint8_t  velocity;
    uint8_t  flags;
    uint8_t  type;
    uint8_t  reserved;
  } field PACK_STRUCT;
};
typedef union ev_atomic ev_atomic_t;

// many things break if this is not an int64 that we can
// read and write atomicly. this is a hack to force the sizeof()
// check to happen at compile time.
char ev_atomic_t_must_be_8_bytes[1 - 2*(sizeof(ev_atomic_t) != 8)];

struct seq_event {
  // fields to be used ONLY by the worker thread
  struct seq_event *next;
  struct seq_event *prev;
  uint64_t alarm_clock;

  // flags that change across threads
  // MUST USE ATOMIC READ/WRITE!
  volatile ev_atomic_t atomic;

  // fields that do not change across threads
  int      port_id;
  uint8_t  channel;
  scale_t *scale;
  uint8_t  mem;
};
typedef struct seq_event ev_t;

#define GET_EV_STRUCT(obj)        \
  ev_t *ev;                       \
  Data_Get_Struct(obj, ev_t, ev);

#define GET_EV_A             \
  ev_atomic_t ev_a;          \
  ev_a.raw = ev->atomic.raw;

#define GET_EV         \
  GET_EV_STRUCT(self); \
  GET_EV_A;

#define EVa_HAS_FLAG(ev_a, ev_flag) ((ev_a)->field.flags & ev_flag)
#define EVa_IS(      ev_a, ev_type) ((ev_a).field.type & ev_type)
#define EV_HAS_FLAG( ev,   ev_flag) (&((ev)->atomic, ev_flag))
#define EV_IS(       ev,   ev_type) EVa_IS((ev)->atomic, ev_type)

#define EVa_ACTIVE(ev_a)      EVa_HAS_FLAG(ev_a, EV_FLAG_ACTIVE)
#define EVa_TRANSPOSE(ev_a)   EVa_HAS_FLAG(ev_a, EV_FLAG_TRANSPOSE)
#define EVa_IS_NOTEON(ev_a)   EVa_IS(      ev_a, EV_TYPE_NOTEON)
#define EVa_IS_NOTEOFF(ev_a)  EVa_IS(      ev_a, EV_TYPE_NOTEOFF)
#define EVa_IS_NOTE(ev_a)    (EVa_IS_NOTEON(ev_a) && EVa_IS_NOTEOFF(ev_a))

#define EV_ACTIVE(ev)     EVa_ACTIVE(&((ev)->atomic))
#define EV_TRANSPOSE(ev)  EVa_TRANSPOSE(&((ev)->atomic))
#define EV_IS_NOTEON(ev)  EVa_IS_NOTEON((ev)->atomic)
#define EV_IS_NOTEOFF(ev) EVa_IS_NOTEOFF((ev)->atomic)
#define EV_IS_NOTE(ev)    EVa_IS_NOTE((ev)->atomic)

uint8_t midi_note_from_ev_atomic(ev_atomic_t *ev_a, scale_t *scale);
uint8_t midi_note_from_ev(ev_t *ev);

void Init_aMIDI_Ev();

#define ev_on( ev) ATOMIC_OR_8( &ev->atomic.field.flags,  EV_FLAG_ACTIVE)
#define ev_off(ev) ATOMIC_AND_8(&ev->atomic.field.flags, ~EV_FLAG_ACTIVE)

#endif /*EV_H*/
