module AlsaMIDI
  class Port
    attr_reader :client, :port_id

    def initialize(client_obj)
      @client = client_obj
      setup!
    end
    
    class TX < Port
    end

    class RX < Port
    end
  end
end
