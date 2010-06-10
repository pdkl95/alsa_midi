#ifndef SCALE_H
#define SCALE_H

#define MIDI_MIDDLE_A 57
#define SCALE_SIZE 7

struct scale_with_mode {
  int key_offset;
  int offset[SCALE_SIZE];
};
typedef struct scale_with_mode scale_t;

#define GET_SCALE_STRUCT(obj)           \
  scale_t *scale;                       \
  Data_Get_Struct(obj, scale_t, scale);

#define GET_SCALE GET_SCALE_STRUCT(self)

int midi_note_in_scale(scale_t *scale, int offset, int octave);

void Init_aMIDI_Scale();

#endif /*SCALE_H*/
