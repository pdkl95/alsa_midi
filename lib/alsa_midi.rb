def alsa_midi_require(name)
  begin
    require name
  rescue LoadError => err
    raise "Error loading required libarary \"#{name}\": #{err}"
  rescue => err
    raise "Unknown error when requiring \"#{name}\": #{err}"
  end
end

# external tools
alsa_midi_require 'rubygems'
alsa_midi_require 'color_debug_messages'
alsa_midi_require 'facets/kernel/returning'

module AlsaMIDI
  DEFAULT_CLIENT_OPT = {
    :clocks_per_beat   => 128,
    :beats_per_measure => 4,

    :name => "AlsaMIDI",
    :tx   => 1,
    :rx   => 0,
    :bpm  => 120
  }
end

# pull in the C backend
alsa_midi_require(File.dirname(__FILE__) + '/../ext/alsa_midi_seq')

# pull in the reset of the libarary as (easier...) ruby code
require 'alsa_midi/error'
require 'alsa_midi/scale'
require 'alsa_midi/looper'
require 'alsa_midi/port'
require 'alsa_midi/client'
