#!/usr/bin/env ruby
require 'mkmf'

LIB_NAME = 'alsa_midi_seq'

ALSA_PKG = 'alsa'          # for pkg-config
ALSA_H   = 'asoundlib.h'

fail "Cannot find pkg-config!" unless find_executable('pkg-config')
pkg = pkg_config(ALSA_PKG) or fail "Missing package #{ALSA_PKG} in pkg-config!"
header_dir = pkg[0].sub(/^-I/,'').sub(/\s*$/,'')
puts "using ALSA headers in: #{header_dir.inspect}"

['stdio.h', 'stdlib.h', 'unistd.h', 'pthread.h', 'time.h'
].each do |hdr|
  have_header(hdr) or fail "missing: basic headers #{hdr.inspect}"
end

have_library('pthread', 'pthread_attr_init') or fail "missing: pthreads!"

['clock_nanosleep', 'clock_gettime'].each do |func|
  have_func(func) or fail "missing POSIX high-resolution timer function: #{func}()!"
end
['CLOCK_MONOTONIC', 'TIMER_ABSTIME'].each do |m|
  have_macro(m, 'time.h') or fail "Missing time.h macro: #{m}"
end

if have_header('sched.h')
  have_macro('SCHED_FIFO', 'sched.h')
  have_func('sched_setscheduler')
  have_func('sched_get_priority_max')
  have_func('geteuid')
else
  warn "Missing: sched.h - realtime priority setting disabled!"
end

have_header(ALSA_H) or fail "missing: alsa header!"

['OPEN_DUPLEX',    'PORT_TYPE_APPLICATION',
 'PORT_CAP_READ',  'PORT_CAP_SUBS_READ',
 'PORT_CAP_WRITE', 'PORT_CAP_SUBS_WRITE'
].map do |m|
  "SND_SEQ_#{m}"
end.each do |m|
  have_macro(m, ALSA_H) or fail "Missing ALSA macro: #{m}"
end

['snd_seq_tick_time_t',
 #'snd_seq_t'
 #'snd_seq_queue_tempo_t'
].each do |t|
  have_type(t, ALSA_H) or fail "Missing ALSA type: #{t}"
end

create_makefile LIB_NAME, 'src'
