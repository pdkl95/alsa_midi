require File.dirname(__FILE__) + '/../ext/alsa_midi'

module AlsaMIDI
  class Seqx
    def initializex
      @seq = AlsaMIDI::Base.new
    end
  end
end
