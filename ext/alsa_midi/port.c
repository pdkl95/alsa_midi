#include "alsa_midi.h"

static alsa_midi_client_t *client_for(VALUE self)
{
  GET_CLIENT_STRUCT(GET_IV(client));
  return client;
}

static VALUE port_setup(VALUE self)
{
  alsa_midi_client_t *client = client_for(self);
  unsigned int caps = NUM2INT(rb_funcall(self, rb_intern("cap_flags"), 0));
  IV_STR(name);
  int ret = snd_seq_create_simple_port(client->handle, name, caps,
                                       SND_SEQ_PORT_TYPE_APPLICATION);
  int retval = INT2NUM(ret);
  //DEBUG_NUM("@port_id = %d", ret);

  if (ret < 0) {
    rb_raise(aMIDI_AlsaError, "Error creating sequencer READ port.");
  }
  SET_IV(port_id, retval);
  return retval;
}

static VALUE port_tx_cap_flags(VALUE self)
{
  return INT2NUM(SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ);
}

static VALUE port_rx_cap_flags(VALUE self)
{
  return INT2NUM(SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE);
}

static void ev_add(ev_t *ev)
{
}

static VALUE port_ev_add(VALUE ev)
{
}

void Init_aMIDI_Port()
{
  rb_define_method(aMIDI_Port,   "setup!",    port_setup,        0);
  rb_define_method(aMIDI_PortTX, "cap_flags", port_tx_cap_flags, 0);
  rb_define_method(aMIDI_PortRX, "cap_flags", port_rx_cap_flags, 0);
  rb_define_method(aMIDI_Port,   "ev_add",    port_ev_add,       1);
}
