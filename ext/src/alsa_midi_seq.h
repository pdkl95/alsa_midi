#ifndef ALSA_MIDI_H
#define ALSA_MIDI_H

#define DEFAULT_CLIENT_NAME "AlsaMIDILooper"
#define DEFAULT_BPM         120
#define DEFAULT_PPQ         128
#define SHUTDOWN_WAIT_TIME  2

#define aMIDI_inline inline
//#define aMIDI_inline

#include "ruby.h"
#include <stdio.h>
#include <stdlib.h>
#include <asoundlib.h>

extern VALUE aMIDI;
extern VALUE aMIDI_Base;
extern VALUE aMIDI_RT;
extern VALUE aMIDI_Event;
extern VALUE aMIDI_Pattern;
extern VALUE aMIDI_Port;
extern VALUE aMIDI_PortTX;
extern VALUE aMIDI_PortRX;
extern VALUE aMIDI_Client;
extern VALUE aMIDI_Looper;

extern VALUE aMIDI_Error;
extern VALUE aMIDI_TimerError;
extern VALUE aMIDI_AlsaError;

#define KLASS_UNDER(scope, name) rb_const_get(scope, rb_intern(#name))
#define KLASS(name) KLASS_UNDER(rb_cObject, name)
#define INCLUDE(klass, name) rb_include_module(klass, KLASS(name))

#define OBJ_GET_IV(obj, iv_name)      rb_iv_get(obj, "@" #iv_name)
#define OBJ_SET_IV(obj, iv_name, val) rb_iv_set(obj, "@" #iv_name, val)
#define     GET_IV(     iv_name)      OBJ_GET_IV(self, iv_name)
#define     SET_IV(     iv_name, val) OBJ_SET_IV(self, iv_name, val)

#define IV_STR(name)                         \
  VALUE name##_value = GET_IV(name);         \
  char *name = StringValuePtr(name##_value);

#define IV_INT(name)                         \
  VALUE name##_value = GET_IV(name);         \
  int   name = NUM2INT(name##_value);

#define DEBUG_MSG(obj, type, msg) \
  rb_funcall(obj, rb_intern(type), 1, msg)

#define DEBUG(msg) DEBUG_MSG(self, "debug", rb_str_new2(msg))
#define  INFO(msg) DEBUG_MSG(self,  "info", rb_str_new2(msg))

#define PRINTF(fmt, value)                     \
  rb_funcall(rb_mKernel, rb_intern("sprintf"), \
             2, rb_str_new2(fmt), value)

#define PRINTF2(fmt, v1, v2)                   \
  rb_funcall(rb_mKernel, rb_intern("sprintf"), \
             3, rb_str_new2(fmt), v1, v2)

#define DEBUG_MSG_VAL(obj, type, fmt, value)   \
  DEBUG_MSG(obj, type, PRINTF(fmt, value));

#define DEBUG_VAL(fmt, value) DEBUG_MSG_VAL(self, "debug", fmt, value)
#define  INFO_VAL(fmt, value) DEBUG_MSG_VAL(self, "info", fmt, value)
#define DEBUG_STR(fmt, value) DEBUG_VAL(fmt, rb_str_new2(value))
#define  INFO_STR(fmt, value)  INFO_VAL(fmt, rb_str_new2(value))
#define DEBUG_NUM(fmt, value) DEBUG_VAL(fmt, INT2NUM(value))
#define  INFO_NUM(fmt, value)  INFO_VAL(fmt, INT2NUM(value))

#define MIDI_CONST(name) rb_const_get(aMIDI, rb_intern(name))
#define NEW(klass_name)  rb_class_new_instance(0, NULL, MIDI_CONST(klass_name));

#define CUSTOM_ALLOC(klass) \
  rb_define_alloc_func(aMIDI_##klass, klass##_alloc);

#define F_DEF(klass, name, argc, name_extra, joiner) \
  rb_define_method(aMIDI_##klass,                    \
                   #name name_extra,                 \
                   klass##joiner##name,              \
                   argc)

#define FUNC(  klass, name, argc) F_DEF(klass, name, argc,  "",     _)
#define FUNC_X(klass, name, argc) F_DEF(klass, name, argc, "!",     _)
#define FUNC_Q(klass, name, argc) F_DEF(klass, name, argc, "?",     _)
#define GETTER(klass, name)       F_DEF(klass, name,    0,  "", _get_)
#define SETTER(klass, name)       F_DEF(klass, name,    1, "=", _set_)

#define ACCESSOR(klass, name) \
  GETTER(klass, name);        \
  SETTER(klass, name);

#include "fifo.h"
#include "rt.h"
#include "pattern.h"
#include "port.h"
#include "client.h"
#include "looper.h"

#endif /*ALSA_MIDI_H*/
