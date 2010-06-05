#include "alsa_midi_seq.h"

static VALUE Client_set_name(VALUE self, VALUE new_name)
{
  GET_CLIENT;
  client->name = StringValuePtr(new_name);
  DEBUG_STR("ALSA client name: \"%s\"", client->name);
  snd_seq_set_client_name(client->handle, client->name);
  return new_name;
}

static VALUE Client_get_name(VALUE self)
{
  GET_CLIENT;
  return client->name ? rb_str_new2(client->name) : Qnil;
}

static VALUE Client_set_ppq(VALUE self, VALUE new_ppq)
{
  GET_CLIENT;
  client->ppq = NUM2INT(new_ppq);
  return new_ppq;
}

static VALUE Client_get_ppq(VALUE self)
{
  GET_CLIENT;
  return INT2NUM(client->ppq);
}

static VALUE Client_set_bpm(VALUE self, VALUE new_bpm)
{
  GET_CLIENT;
  client->bpm = NUM2INT(new_bpm);
  return new_bpm;
}

static VALUE Client_get_bpm(VALUE self)
{
  GET_CLIENT;
  return INT2NUM(client->bpm);
}

static VALUE Client_get_client_id(VALUE self)
{
  GET_CLIENT;
  return INT2NUM(client->client_id);
}

static void Client_free(alsa_midi_client_t *client)
{
  free(client);
}

static VALUE Client_alloc(VALUE klass)
{
  alsa_midi_client_t *client;
  VALUE self = Data_Make_Struct(klass, alsa_midi_client_t,
                                NULL, Client_free, client);

  client->name = DEFAULT_CLIENT_NAME;
  client->bpm  = DEFAULT_BPM;
  client->ppq  = DEFAULT_PPQ;

  if (snd_seq_open(&(client->handle), "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    rb_raise(aMIDI_AlsaError, "Error opening ALSA sequencer.");
    exit(1);
  }
  client->client_id = snd_seq_client_id(client->handle);

  return self;
}

void Init_aMIDI_Client()
{
  CUSTOM_ALLOC(Client);

  ACCESSOR(Client, name);
  ACCESSOR(Client, bpm);
  ACCESSOR(Client, ppq);

  GETTER(Client, client_id);
}
