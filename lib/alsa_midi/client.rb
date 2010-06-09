module AlsaMIDI
  class Client
    attr_accessor :opt
    attr_reader :ports_tx, :ports_rx

    def initialize(opt={})
      @opt = AlsaMIDI::DEFAULT_CLIENT_OPT.merge(opt)
      @ports_tx = []
      @ports_rx = []

      self.name = @opt[:name]
      self.bpm  = @opt[:bpm]

      setup_ports :tx, AlsaMIDI::Port::TX
      setup_ports :rx, AlsaMIDI::Port::RX

      worker_start!
    end

    def inspect
      "#<AlsaMIDI::Client name=#{name.inspect}, bpm=#{bpm}, tx=[#{tx_port_str}], rx=[#{rx_port_str}]>"
    end

    def to_s
      "AlsaMIDI::Client[#{name.inspect}]{ #{bpm} bpm, #{@ports_tx.length} tx, #{@ports_rx.length} rx }"
    end
    
    def to_details
      <<CLIENT_TO_S
#{to_s}
  tx_ports: #{tx_port_str}
  rx_ports: #{rx_port_str}
CLIENT_TO_S
    end
    
    def port_str(list)
      ['[', list.map { |x| x.name }.join(', '), ']'].join
    end

    def tx_port_str
      port_str @ports_tx
    end

    def rx_port_str
      port_str @ports_rx
    end

    def each_tx_port
      @ports_tx.each { |port| yield(port) }
    end

    def each_rx_port
      @ports_rx.each { |port| yield(port) }
    end

    def each_port(&block)
      each_tx_port(&block)
      each_rx_port(&block)
    end

    def ports
      @ports_tx + @ports_rx
    end

    def find_port_by_name(name)
      each_port do |port|
        return port if port.name == name
      end
      nil
    end

    def create_tx_port(opt={})
      @ports_tx.push AlsaMIDI::Port::TX.new(self, opt)
    end

    def create_rx_port(opt={})
      @ports_rx.push AlsaMIDI::Port::RX.new(self, opt)
    end

    def tempo_changed!
      info "NEW TEMPO --> #{tempo} us/tick, #{bpm} bpm"
    end

    def setup_ports(type, klass, names = opt[type])
      names = [nil] * names unless names.class == Array
      names.each do |name|
        port_opt = @opt[:port_opts] || {}
        port_opt[:name] = name
        send("ports_#{type}").push(klass.new(self, port_opt))
      end
    end
  end
end
