#include "alsa_midi.h"
#include "alsa_seq.h"

VALUE aMIDI_cSeq;
VALUE aMIDI_eSeqError;

VALUE alsa_midi_klass_seq()
{
  return rb_const_get(aMIDI_mod, rb_intern("Seq"));
}
VALUE alsa_midi_klass_seq_new()
{
  return rb_class_new_instance(0, NULL, alsa_midi_klass_seq());
}

static VALUE seq_open(VALUE self)
{
  SEQ_BASE;
  VALUE cname_obj = rb_iv_get(self, "@client_name");
  char *cname = StringValuePtr(cname_obj);
  
  if (snd_seq_open(&(seq->handle), "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(seq->handle, cname);
  if ((seq->port_out_id = snd_seq_create_simple_port(seq->handle, cname,
            SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {

    fprintf(stderr, "Error creating sequencer READ port.\n");
    exit(1);
  }
  if ((seq->port_in_id = snd_seq_create_simple_port(seq->handle, cname,
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer WRITE port.\n");
    exit(1);
  }
  printf("seq open successful!\n");

  seq->queue_id = snd_seq_alloc_queue(seq->handle);
  snd_seq_set_client_pool_output(seq->handle, (seq->seq_len << 1) + 4);

  printf("allocated seq-queue!\n");
  return self;
}

static VALUE seq_set_tempo(VALUE self, VALUE new_bpm)
{
  SEQ_BASE;
  seq->bpm = NUM2INT(new_bpm);

  snd_seq_queue_tempo_t *queue_tempo;
  snd_seq_queue_tempo_malloc(&queue_tempo);

  double tpq = (double)(seq->ticks_per_quarter);
  double bpm = (double)(seq->bpm);
  seq->tempo = (int)(6e7 / (bpm * tpq * tpq));
  printf("New Tempo: %d bpm, %d ticks\n", seq->bpm, seq->tempo);

  snd_seq_queue_tempo_set_tempo(queue_tempo, seq->tempo);
  snd_seq_queue_tempo_set_ppq(queue_tempo, seq->ticks_per_quarter);
  snd_seq_set_queue_tempo(seq->handle, seq->queue_id, queue_tempo);

  snd_seq_queue_tempo_free(queue_tempo);
  return new_bpm;
}

static VALUE seq_shutdown(VALUE self)
{
  SEQ_BASE;
  seq_clear_queue(self);
  sleep(SHUTDOWN_WAIT_TIME);
  snd_seq_stop_queue(seq->handle, seq->queue_id, NULL);
  snd_seq_free_queue(seq->handle, seq->queue_id);
}

static VALUE seq_initialize(VALUE self)
{
  rb_iv_set(self, "@seq", alsa_midi_klass_base_new());
  rb_iv_set(self, "@client_name", rb_str_new2(DEFAULT_CLIENT_NAME));
  seq_open(self);
  seq_set_tempo(self, INT2NUM(120));
  return self;
}

void Init_alsa_midi_seq()
{
  aMIDI_cSeq      = rb_define_class_under(aMIDI_mod, "Seq",          rb_cObject);
  aMIDI_eSeqError = rb_define_class_under(aMIDI_mod, "AlsaSeqError", rb_eRuntimeError);

  rb_define_method(aMIDI_cSeq, "initialize", seq_initialize, 0);
  rb_define_method(aMIDI_cSeq, "open",       seq_open,       0);
  rb_define_method(aMIDI_cSeq, "tempo=",     seq_set_tempo,  1);
}
