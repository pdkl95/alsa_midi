#include "alsa_midi.h"
#include "alsa_seq.h"
#include "looper.h"

VALUE aMIDI_mod;
VALUE aMIDI_cBase;
VALUE aMIDI_eAlsaError;

VALUE alsa_midi_klass_base()
{
  return rb_const_get(aMIDI_mod, rb_intern("Base"));
}

VALUE alsa_midi_klass_base_new()
{
  return rb_class_new_instance(0, NULL, alsa_midi_klass_base());
}

static void base_free(seq_client_t *seq)
{
  free(seq);
}

static VALUE base_alloc(VALUE klass)
{
  seq_client_t *data;
  VALUE obj = Data_Make_Struct(klass, seq_client_t, NULL, base_free, data);
  data->ticks_per_quarter = DEFAULT_TICKS_PER_QUARTER;
  return obj;
}

void Init_alsa_midi()
{
  aMIDI_mod        = rb_define_module("AlsaMIDI");
  aMIDI_cBase      = rb_define_class_under(aMIDI_mod, "Base",      rb_cObject);
  aMIDI_eAlsaError = rb_define_class_under(aMIDI_mod, "AlsaError", rb_eRuntimeError);

  rb_define_alloc_func(aMIDI_cBase, base_alloc);

  Init_alsa_midi_seq();
  Init_alsa_midi_looper();
}
