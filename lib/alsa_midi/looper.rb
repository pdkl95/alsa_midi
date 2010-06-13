module AlsaMIDI
  class Looper
    class Seq16
    end
    
    attr_reader :client, :port, :channel
    def initialize(c, p, ch)
      @client  = c
      @port    = p
      @channel = ch

      @port_id = @port.port_id
    end
  end
end
