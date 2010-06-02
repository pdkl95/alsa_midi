#include "alsa_midi.h"

VALUE aMIDI;
VALUE aMIDI_Base;
VALUE aMIDI_Event;
VALUE aMIDI_Pattern;
VALUE aMIDI_Port;
VALUE aMIDI_PortTX;
VALUE aMIDI_PortRX;
VALUE aMIDI_Client;
VALUE aMIDI_Looper;

VALUE aMIDI_AlsaError;
VALUE aMIDI_SeqError;

void Init_alsa_midi()
{
  aMIDI      = rb_define_module("AlsaMIDI");
  aMIDI_Base = rb_define_class_under(aMIDI, "Base", rb_cObject);
  INCLUDE(aMIDI_Base, ColorDebugMessages);

  aMIDI_Event   = rb_define_class_under(aMIDI,      "Event",   aMIDI_Base);
  aMIDI_Pattern = rb_define_class_under(aMIDI,      "Pattern", aMIDI_Base);
  aMIDI_Port    = rb_define_class_under(aMIDI,      "Port",    aMIDI_Base);
  aMIDI_PortTX  = rb_define_class_under(aMIDI_Port, "TX",      aMIDI_Port);
  aMIDI_PortRX  = rb_define_class_under(aMIDI_Port, "RX",      aMIDI_Port);
  aMIDI_Client  = rb_define_class_under(aMIDI,      "Client",  aMIDI_Base);
  aMIDI_Looper  = rb_define_class_under(aMIDI,      "Looper",  aMIDI_Base);

  aMIDI_AlsaError = rb_define_class_under(aMIDI, "AlsaError",
                                          rb_eRuntimeError);

  aMIDI_SeqError  = rb_define_class_under(aMIDI, "AlsaSequencerError",
                                          aMIDI_AlsaError);
  Init_aMIDI_Pattern();
  Init_aMIDI_Port();
  Init_aMIDI_Client();
  Init_aMIDI_Looper();
}
