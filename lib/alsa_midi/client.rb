module AlsaMIDI
  class Client
    class PortList < Array
      attr_reader :client, :type, :port_klass

      def initialize(client_obj, list_type, create_klass)
        @client     = client_obj
        @type       = list_type
        @port_klass = create_klass
      end

      def find_by_name(name)
        each do |port|
          return port if port.name == name
        end
        nil
      end

      def to_s
        ['[', map { |x| x.name }.join(', '), ']'].join
      end

      def create_port!(name)
        push(port_klass.new(client, name))
      end

      def create!(port_names)
        names = case port_names.class.to_s
                when 'Fixnum'   then [nil] * port_names
                when 'NilClass' then []
                else                 port_names
                end
        
        if names.is_a? Array
          names.each do |name|
            create_port! name
          end
        else
          raise ::AlsaMIDI::Error::Param, {
            :param    => "ALSA Port name",
            :expected => [Array, Fixnum],
            :given    => port_names
          }
        end
      end
    end
    
    attr_accessor :opt
    attr_reader :tx_ports, :rx_ports

    def initialize(opt={})
      @opt = ::AlsaMIDI::DEFAULT_CLIENT_OPT.merge(opt)
      @tx_ports = PortList.new(self, :tx, ::AlsaMIDI::Port::TX)
      @rx_ports = PortList.new(self, :rx, ::AlsaMIDI::Port::RX)

      self.name              = @opt[:name]
      self.clocks_per_beat   = @opt[:clocks_per_beat]
      self.beats_per_measure = @opt[:beats_per_measure]
      self.bpm               = @opt[:bpm]

      @tx_ports.create! opt[:tx]
      @rx_ports.create! opt[:rx]
    end

    def inspect_params
      [ "name=#{name.inspect}",
        "clock=#{clocks_per_beat}:#{beats_per_measure}",
        "tx=[#{tx_ports}]",
        "rx=[#{rx_ports}]" ]
    end

    def inspect
      "#<AlsaMIDI::Client #{inspect_params.join(', ')}]>"
    end

    def to_s
      "AlsaMIDI::Client\n" + inspect_params.map do |str|
        "    -- #{str}"
      end.join("\n")
    end
    
    def to_details
      <<CLIENT_TO_S
#{to_s}
  tx_ports: #{tx_ports}
  rx_ports: #{rx_ports}
CLIENT_TO_S
    end
    
    def each_port(&block)
      tx_ports.each(&block)
      rx_ports.each(&block)
    end

    def ports
      @ports_tx + @ports_rx
    end

    def find_port_by_name(name)
      tx_ports.find_by_name(name) || rx_ports.find_by_name(name)
    end

    def tempo_changed!
      info "NEW TEMPO --> #{tempo} us/tick, #{bpm} bpm"
    end
  end
end
