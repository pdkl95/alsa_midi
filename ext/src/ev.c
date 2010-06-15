#include "alsa_midi_seq.h"

aMIDI_inline uint8_t midi_note_from_ev_atomic(ev_atomic_t *ev_a, scale_t *scale)
{
  if (scale && EVa_TRANSPOSE(ev_a)) {
    return midi_note_in_scale(scale, ev_a->field.note, ev_a->field.octave);
  } else {
    return ev_a->field.note;
  }
}

aMIDI_inline uint8_t midi_note_from_ev(ev_t *ev)
{
  GET_EV_A;
  return midi_note_from_ev_atomic(&ev_a, ev->scale);
}

static VALUE Ev_get_etype(VALUE self)
{
  GET_EV;
  VALUE ret;
  switch(ev->atomic.field.type) {
  case EV_TYPE_NOTEON:
    ret = SYM("note_on");
    break;
  case EV_TYPE_NOTEOFF:
    ret = SYM("note_off");
    break;
  case EV_TYPE_NOTE:
    ret = SYM("note");
    break;
  default:
    ret = SYM("unknown");
    break;
  }
  return ret;
}
#define EV_ATOMIC_INT_GETTER(fname)         \
  static VALUE Ev_get_##fname(VALUE self) { \
    GET_EV;                                 \
    return INT2NUM(ev->atomic.field.fname); \
  }

#define EV_ATOMIC_INT_SETTER(fname)                       \
  static VALUE Ev_set_##fname(VALUE self, VALUE newval) { \
    int x = NUM2INT(newval);                              \
    GET_EV;                                               \
    ev->atomic.field.fname = x;                           \
    return newval;                                        \
  }

#define EV_ATOMIC_INT_ACCESSOR(fname) \
  EV_ATOMIC_INT_GETTER(fname);        \
  EV_ATOMIC_INT_SETTER(fname);

EV_ATOMIC_INT_ACCESSOR(note);
EV_ATOMIC_INT_ACCESSOR(octave);
EV_ATOMIC_INT_ACCESSOR(velocity);

STD_INT_ACCESSOR(Ev, ev_t, port_id);
STD_INT_ACCESSOR(Ev, ev_t, channel);

STD_FREE(Ev, ev_t);
STD_NEW(Ev, ev_t);

void Init_aMIDI_Ev()
{
  CLASS_NEW(Ev);

  ACCESSOR(Ev, port_id);
  ACCESSOR(Ev, channel);
  ACCESSOR(Ev, note);
  ACCESSOR(Ev, octave);
  ACCESSOR(Ev, velocity);
}
