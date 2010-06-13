#include "alsa_midi_seq.h"

VALUE aMIDI;
VALUE aMIDI_Base;
VALUE aMIDI_Scale;
VALUE aMIDI_Ev;
VALUE aMIDI_EvNote;
VALUE aMIDI_Pat;
VALUE aMIDI_Port;
VALUE aMIDI_PortTX;
VALUE aMIDI_PortRX;
VALUE aMIDI_Client;
VALUE aMIDI_Looper;
VALUE aMIDI_LooperSeq16;

VALUE aMIDI_Error;
VALUE aMIDI_TimerError;
VALUE aMIDI_AlsaError;

void Init_alsa_midi_seq()
{
  aMIDI      = rb_define_module("AlsaMIDI");

  aMIDI_Base = rb_define_class_under(aMIDI, "Base", rb_cObject);
  INCLUDE(aMIDI_Base, ColorDebugMessages);

#define K(name) aMIDI_##name = rb_define_class_under(aMIDI, #name, aMIDI_Base)
  K(Scale);
  K(Ev);
  K(Pat);
  K(Port);
  K(Client);
  K(Looper);
#undef K

#define SUB(parent, klass)                                              \
  aMIDI_##parent##klass =                                               \
    rb_define_class_under(aMIDI_##parent, #klass, aMIDI_##parent);
  SUB(Ev, Note);
  SUB(Port, TX);
  SUB(Port, RX);
  SUB(Looper, Seq16);
#undef SUB

  aMIDI_Error = rb_define_class_under(aMIDI, "Error", rb_eRuntimeError);

#define E(name) \
  aMIDI_##name = rb_define_class_under(aMIDI, #name, aMIDI_Error);
  E(TimerError);
  E(AlsaError);
#undef E

  Init_aMIDI_Scale();
  Init_aMIDI_Ev();
  Init_aMIDI_Pat();
  Init_aMIDI_Port();
  Init_aMIDI_Client();
  Init_aMIDI_Looper();
}
