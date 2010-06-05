#include "alsa_midi_seq.h"

static void event_free(ev_t *ev)
{
  free(ev);
}

static void pattern_free(ev_t *pat)
{
  free(pat);
}

static VALUE event_alloc(VALUE klass)
{
  ev_t *ev;
  VALUE self = Data_Make_Struct(klass, ev_t,
                                NULL, event_free, ev);
  return self;
}

static VALUE pattern_alloc(VALUE klass)
{
  pat_t *pat;
  VALUE self = Data_Make_Struct(klass, pat_t,
                                NULL, pattern_free, pat);
  return self;
}

void Init_aMIDI_Pattern()
{
  rb_define_alloc_func(aMIDI_Event,    event_alloc);
  rb_define_alloc_func(aMIDI_Pattern, pattern_alloc);
}
