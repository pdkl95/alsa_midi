#ifndef ALSA_PORT_H
#define ALSA_PORT_H

#define PORT_STR(varname, client_id, port_id) \
  VALUE varname = PRINTF2("%d:%d",            \
                          INT2NUM(client_id), \
                          INT2NUM(port_id))

#define SUBS_CAP_MASK (SND_SEQ_PORT_CAP_SUBS_READ|SND_SEQ_PORT_CAP_SUBS_WRITE)
#define SUBS_CAP_ONLY(x) (x & SUBS_CAP_MASK)
#define SUBS_CAP SUBS_CAP_ONLY(find_caps(self))

void Init_aMIDI_Port();

#endif /*ALSA_Port_H*/
