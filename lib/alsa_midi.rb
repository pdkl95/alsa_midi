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

module AlsaMIDI
  DEFAULT_CLIENT_OPT = {
    :name => "AlsaMIDI",
    :bpm  => 120,
    :tx   => 2,
    :rx   => 2,
    :queue_length => 64
  }
end

# pull in the C backend
alsa_midi_require(File.dirname(__FILE__) + '/../ext/alsa_midi_seq')

# pull in the reset of the libarary as (easier...) ruby code
require 'alsa_midi/port'
require 'alsa_midi/client'
