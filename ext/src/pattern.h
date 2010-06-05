#ifndef PATTERN_H
#define PATTERN_H

#define MAX_EVENTS 32

struct midi_event {
  int x;
};
typedef struct midi_event ev_t;

struct midi_pattern {
  ev_t events[MAX_EVENTS];
};
typedef struct midi_pattern pat_t;

void Init_aMIDI_Pattern();

#endif /*PATTERN_H*/
