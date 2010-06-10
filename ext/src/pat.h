#ifndef PAT_H
#define PAT_H

#define MAX_EVENTS 32

struct seq_pattern {
  ev_t events[MAX_EVENTS];
};
typedef struct seq_pattern pat_t;

#define GET_PAT_STRUCT(obj)         \
  pat_t *pat;                       \
  Data_Get_Struct(obj, pat_t, pat);

#define GET_PAT GET_PAT_STRUCT(self)

void Init_aMIDI_Pat();

#endif /*PAT_H*/
