#include "alsa_midi.h"
#include "alsa_seq.h"
#include "looper.h"

VALUE aMIDI_cLooper;

void Init_alsa_midi_looper()
{
  aMIDI_cLooper = rb_define_class_under(aMIDI_mod, "Looper", rb_cObject);
}
