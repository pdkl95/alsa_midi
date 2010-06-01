#ifndef ALSA_SEQ_H
#define ALSA_SEQ_H

extern VALUE aMIDI_cSeq;
extern VALUE aMIDI_eSeqError;

#define SEQ_BASE GET_BASE(rb_iv_get(self, "@seq"))

void Init_alsa_midi_seq();
VALUE alsa_midi_klass_seq();
VALUE alsa_midi_klass_seq_new();

#endif /*ALSA_SEQ_H*/
