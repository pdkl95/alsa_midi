module AlsaMIDI
  class Client
    attr_reader :ports_tx, :ports_rx

    def initialize
      @ports_tx = []
      @ports_rx = []
      name = AlsaMIDI::DEFAULT_CLIENT_NAME
      bpm  = AlsaMIDI::DEFAULT_BPM
      create_tx_port
    end

    def inspect
      "#<AlsaMIDI::Client name=#{name.inspect}, bpm=#{bpm}>"
    end

    def create_tx_port
      @ports_tx.push(AlsaMIDI::Port::TX.new(self))
    end

    def create_rx_port
      @ports_tx.push(AlsaMIDI::Port::RX.new(self))
    end
  end
end
