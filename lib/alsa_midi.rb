require 'rubygems'
require 'color_debug_messages'

require File.dirname(__FILE__) + '/../ext/alsa_midi'

module AlsaMIDI
  DEFAULT_CLIENT_OPT = {
    :name => "AlsaMIDI",
    :bpm  => 120,
    :tx   => 2,
    :rx   => 2,
    :queue_length => 64
  }
end

require 'alsa_midi/port'
require 'alsa_midi/client'
