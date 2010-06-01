#ifndef PATTERN_H
#define PATTERN_H

#define MAX_EVENTS 32

struct midi_event {
  
};
typedef struct midi_event midi_ev_t;

struct midi_pattern {
  midi_ev_t events[MAX_EVENTS];
};

extern VALUE aMIDI_cPattern;

void Init_alsa_midi_pattern();

#endif /*PATTERN_H*/
