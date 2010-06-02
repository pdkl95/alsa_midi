require File.dirname(__FILE__) + '/../ext/alsa_midi'

module AlsaMIDI
  DEFAULT_CLIENT_NAME = "AlsaMIDI"
  DEFAULT_BPM         = 120
end

require 'alsa_midi/port'
require 'alsa_midi/client'
