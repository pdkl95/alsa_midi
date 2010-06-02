module AlsaMIDI
  class Port
    include ColorDebugMessages
    attr_reader :client, :port_id, :name

    class << self
      def port_name(client, name)
        name = name.next while client.find_port_by_name(name)
        name
      end

      def default_name
        "port_00"
      end
    end

    def type_name
      @type_name ||= self.class.to_s.gsub(/.*::/,'')
    end

    def initialize(client_obj, opt={})
      @port_id = nil
      @client  = client_obj
      @name    = Port.port_name(@client,
                                opt[:name] || self.class.default_name)
      setup!
      info "New port: #{inspect}"
    end

    def inspect
      "#<Port::#{type_name} id=#{@port_id}, name=#{name.inspect}>"
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
