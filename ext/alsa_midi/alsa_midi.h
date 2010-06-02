#ifndef ALSA_MIDI_H
#define ALSA_MIDI_H

#define DEFAULT_CLIENT_NAME       "AlsaMIDILooper"
#define DEFAULT_BPM               120
#define DEFAULT_TICKS_PER_QUARTER 128
#define SHUTDOWN_WAIT_TIME        2

#include "ruby.h"
#include <stdio.h>
#include <stdlib.h>
#include <asoundlib.h>

extern VALUE aMIDI;
extern VALUE aMIDI_Pattern;
extern VALUE aMIDI_Port;
extern VALUE aMIDI_PortTX;
extern VALUE aMIDI_PortRX;
extern VALUE aMIDI_Client;
extern VALUE aMIDI_Looper;

extern VALUE aMIDI_AlsaError;
extern VALUE aMIDI_SeqError;

#define MIDI_CONST(name) rb_const_get(aMIDI, rb_intern(name))
#define NEW(klass_name)  rb_class_new_instance(0, NULL, MIDI_CONST(klass_name));

#endif /*ALSA_MIDI_H*/
