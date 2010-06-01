require File.dirname(__FILE__) + '/../ext/alsa_midi'

class AlsaMIDI
  attr_reader :ports_tx, :ports_rx

  def initialize
    @ports_tx = []
    @ports_rx = []
  end

  def create_tx_port
    @ports_tx.push(Port::TX.new)
  end

  def create_rx_port
    @ports_tx.push(Port::RX.new)
  end
end
