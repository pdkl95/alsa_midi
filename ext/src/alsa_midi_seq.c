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
VALUE aMIDI_LooperSeq;
VALUE aMIDI_LooperSeqMono;

VALUE aMIDI_Error;
VALUE aMIDI_ParamError;
VALUE aMIDI_CWorkerError;
VALUE aMIDI_AlsaError;

#ifndef ATOMIC_BUILTINS
#warning no atomic builtins! falling back to a pthread based mutex!

pthread_mutex_t aMIDI_global_mutex;

#define LOCKED_OP_FUNC(name, op, size)                    \
  void ATOMIC_##name##_##size(volatile uint##size##_t *x, \
                              uint##size##_t mask)        \
  { pthread_mutex_lock(&aMIDI_global_mutex);              \
    *x = *x op mask;                                      \
    pthread_mutex_unlock(&aMIDI_global_mutex);            \
  }
#define LOCKED_OP(name, op)     \
  LOCKED_OP_FUNC(name, op, 8);  \
  LOCKED_OP_FUNC(name, op, 16); \
  LOCKED_OP_FUNC(name, op, 64);
LOCKED_OP(AND, &)
LOCKED_OP(OR,  |)
LOCKED_OP(XOR, ^)
#undef LOCKED_OP
#undef LOCKED_OP_FUNC

#endif

void Init_alsa_midi_seq()
{
#ifndef ATOMIC_BUILTINS
  pthread_mutex_init(&aMIDI_global_mutex, NULL);
#endif

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
  SUB(Looper,    Seq);
  SUB(LooperSeq, Mono);
#undef SUB

  aMIDI_Error = rb_define_class_under(aMIDI, "Error", rb_eRuntimeError);

#define E(name) \
  aMIDI_##name = rb_define_class_under(aMIDI, #name, aMIDI_Error);
  E(ParamError);
  E(CWorkerError);
  E(AlsaError);
#undef E

  Init_aMIDI_Scale();
  Init_aMIDI_Ev();
  Init_aMIDI_Pat();
  Init_aMIDI_Port();
  Init_aMIDI_Client();
  Init_aMIDI_Looper();
}
