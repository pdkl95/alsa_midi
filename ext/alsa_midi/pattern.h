#ifndef PATTERN_H
#define PATTERN_H

#define MAX_EVENTS 32

struct midi_event {
  
};
typedef struct midi_event midi_ev_t;

struct midi_pattern {
  midi_ev_t events[MAX_EVENTS];
};

void Init_aMIDI_Pattern();

#endif /*PATTERN_H*/
