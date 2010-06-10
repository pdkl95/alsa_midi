#include "alsa_midi_seq.h"

aMIDI_inline int midi_note_from_ev(ev_t *ev)
{
  if (ev->flags & EV_FLAG_TRANSPOSE) {
    return midi_note_in_scale(ev->scale, ev->note_offset, ev->note_octave);
  } else {
    return ev->note;
  }
}

static VALUE Ev_get_etype(VALUE self)
{
  GET_EV;
  VALUE ret;
  switch(ev->type) {
  case EV_NOTE:
    ret = SYM("note");
    break;
  default:
    ret = SYM("unknown");
    break;
  }
  return ret;
}

static VALUE Ev_get_etype_id(VALUE self)
{
  return INT2NUM(EV_NULL);
}

static VALUE EvNote_get_etype_id(VALUE self)
{
  return INT2NUM(EV_NOTE);
}

STD_INT_ACCESSOR(Ev, ev_t, port_id);
STD_INT_ACCESSOR(Ev, ev_t, channel);
STD_INT_ACCESSOR(Ev, ev_t, note_offset);
STD_INT_ACCESSOR(Ev, ev_t, note_octave);
STD_INT_ACCESSOR(Ev, ev_t, velocity);

static void Ev_new_preinit(VALUE self, ev_t *ev)
{
  VALUE etype = rb_funcall(self, rb_intern("etype_id"), 0);
  ev->type = NUM2INT(etype);
}

STD_FREE(Ev, ev_t);
STD_NEW_PREINIT(Ev, ev_t);

void Init_aMIDI_Ev()
{
  CLASS_NEW(Ev);
  GETTER(Ev, etype);
  GETTER(Ev, etype_id);
  GETTER(EvNote, etype_id);

  ACCESSOR(Ev, port_id);
  ACCESSOR(Ev, channel);
  ACCESSOR(Ev, note_offset);
  ACCESSOR(Ev, note_octave);
  ACCESSOR(Ev, velocity);
}
