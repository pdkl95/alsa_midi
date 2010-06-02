#include "alsa_midi.h"
#include "port.h"
#include "client.h"

static alsa_midi_client_t *client_for(VALUE obj)
{
  GET_CLIENT_STRUCT(rb_iv_get(obj, "@client"));
  return client;
}

VALUE port_setup(VALUE self)
{
  alsa_midi_client_t *client = client_for(self);
  unsigned int caps = NUM2INT(rb_funcall(self, rb_intern("cap_flags"), 0));
  int ret = snd_seq_create_simple_port(client->handle, client->name, caps,
                                       SND_SEQ_PORT_TYPE_APPLICATION);
  if (ret < 0) {
    rb_raise(aMIDI_AlsaError, "Error creating sequencer READ port.");
  }
  
  rb_iv_set(self, "@port_id", ret);
  return INT2NUM(ret);
}

VALUE port_tx_cap_flags(VALUE self)
{
  return INT2NUM(SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE);
}

VALUE port_rx_cap_flags(VALUE self)
{
  return INT2NUM(SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ);
}

void Init_aMIDI_Port()
{
  rb_define_method(aMIDI_Port,   "setup!",    port_setup,        0);
  rb_define_method(aMIDI_PortTX, "cap_flags", port_tx_cap_flags, 0);
  rb_define_method(aMIDI_PortRX, "cap_flags", port_rx_cap_flags, 0);
}
