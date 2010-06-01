#include "alsa_midi.h"
#include "pattern.h"

VALUE aMIDI_cPattern;

void Init_alsa_midi_pattern()
{
  aMIDI_cPattern = rb_define_class_under(aMIDI_mod, "Pattern", rb_cObject);
}
