#include "alsa_midi_seq.h"

VALUE aMIDI;
VALUE aMIDI_Base;
VALUE aMIDI_Event;
VALUE aMIDI_Pattern;
VALUE aMIDI_Port;
VALUE aMIDI_PortTX;
VALUE aMIDI_PortRX;
VALUE aMIDI_Client;
VALUE aMIDI_Looper;

VALUE aMIDI_Error;
VALUE aMIDI_TimerError;
VALUE aMIDI_AlsaError;

void Init_alsa_midi_seq()
{
  aMIDI      = rb_define_module("AlsaMIDI");

  aMIDI_Base = rb_define_class_under(aMIDI, "Base", rb_cObject);
  INCLUDE(aMIDI_Base, ColorDebugMessages);

#define K(name) aMIDI_##name = rb_define_class_under(aMIDI, #name, aMIDI_Base)
  K(Event);
  K(Pattern);
  K(Port);
  K(Client);
  K(Looper);
#undef K

  aMIDI_PortTX  = rb_define_class_under(aMIDI_Port, "TX",      aMIDI_Port);
  aMIDI_PortRX  = rb_define_class_under(aMIDI_Port, "RX",      aMIDI_Port);

  aMIDI_Error = rb_define_class_under(aMIDI, "Error", rb_eRuntimeError);

#define E(name) \
  aMIDI_##name = rb_define_class_under(aMIDI, #name, aMIDI_Error);
  E(TimerError);
  E(AlsaError);
#undef E

  Init_aMIDI_Pattern();
  Init_aMIDI_Port();
  Init_aMIDI_Client();
  Init_aMIDI_Looper();
}
