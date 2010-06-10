#include "alsa_midi_seq.h"

aMIDI_inline int midi_note_in_scale(scale_t *scale, int offset, int octave)
{
  return (MIDI_MIDDLE_A + scale->offset[offset] +
          scale->key_offset + (12 * octave));
}

static VALUE Scale_midi_note(int argc, VALUE *argv, VALUE self)
{
  GET_SCALE;
  int off, oct, note;
  VALUE offset, octave;
  rb_scan_args(argc, argv, "11", &offset, &octave);

  off = NUM2INT(offset);
  if (octave == Qnil) {
    oct = 0;
  } else {
    oct = NUM2INT(octave);
  }
  note = midi_note_in_scale(scale, off, oct);
  return INT2NUM(note);
}

static void Scale_new_postinit(VALUE self, scale_t *scale)
{
  IV_INT(key_offset);
  scale->key_offset = key_offset;

  VALUE off_list = GET_IV(offsets);
  int idx;
  for(idx=0; idx<SCALE_SIZE; idx++) {
    scale->offset[idx] = NUM2INT(rb_ary_entry(off_list, idx));
  }
}

STD_FREE(Scale, scale_t);
STD_NEW_POSTINIT(Scale, scale_t);

void Init_aMIDI_Scale()
{
  CLASS_NEW(Scale);
  FUNC(Scale, midi_note, -1);
}
