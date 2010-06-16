module AlsaMIDI
  class Port
    include ColorDebugMessages
    attr_reader :client, :port_id, :name

    class << self
      def default_name
        "port_00"
      end
    end

    def client_id
      @client.client_id
    end

    def port_id_string
      "#{client_id}:#{@port_id}"
    end

    def type_name
      @type_name ||= self.class.to_s.gsub(/.*::/,'')
    end

    def generate_port_name(pname = @requested_port_name)
      pname = self.class.default_name unless pname
      pname = pname.next while client.find_port_by_name(name)
      pname
    end

    def port_name
      @port_name ||= generate_port_name
    end

    def initialize
      info "New port: #{inspect}"
    end

    def connected_clients
      returning [] do |list|
        each_connected do |cid, pid, idx|
          list.push([cid, pid])
        end
      end
    end

    def create_seq16!(midi_channel)
      Looper::Seq::Mono.new(@client, self, midi_channel)
    end

    def to_s
      list = connected_clients
      str = "Port::#{type_name}[#{name.inspect}, #{port_id_string}"
      if list.length > 0
        str += ' -> ('
        str += list.map do |c|
          "#{c[0]}:#{c[1]}"
        end.join(', ')
        str += ')'
      end
      str + ']'
    end

    def show_status!
      info to_s
    end

    def inspect
      "#<#{to_s}>"
    end

    class TX < Port
      class << self
        def default_name
          "out_00"
        end
      end
    end

    class RX < Port
      class << self
        def default_name
          "in_00"
        end
      end
    end
  end
end
