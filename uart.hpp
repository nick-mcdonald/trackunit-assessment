
namespace UART {

    enum Parity {
        None,
        Odd,
        Even
    }

    enum StopBits {
        ...
    }

    enum BitOrder {
        LeastSignificant,
        MostSignificant
    }

    /*
    * Encapsulates all the configuration parameters for communication via UART
    */
    struct Configuration {
        const size_t baud_rate;
        const size_t data_bits_size;
        const Parity parity_type;
        const StopBits stop_bits;
        const BitOrder bit_order;
        uint32 timeout_ms;

        /*
        * If `buffer_size` is equal to zero, the connection must be `open`ed with a user initialized buffer.
        * If `buffer_size` is greater than zero, the `Device` will initialize and clean up it's own buffer.
        */
        size_t buffer_size = 0;
    }

    /* Using return codes/status instead of exceptions here.
    * Assuming this is a hard realtime system we don't want the device to interupt program execution.
    * But maybe my assumptions are off in which case exceptions can make this interface more readable and user friendly
    * as we won't have to remember to check return codes.
    */
    enum ErrorCode {
        Okay, // No error
        NoConnnection,
        Locked,
        Interupt,
        BufferUninitialized,
        ...
    }

    struct Status {
        ErrorCode code,
        bool has_error,

    }

    struct Response {
        Status status;
        std::vector<std::byte>* data;
    }

    using ReadCallback = typename std::function<void(Status, std::vector<std::byte>)>;

    class Device {
    public:
        /*
        * If interacting with the UART Device should be atomic, pass in a `lock` to a std::mutex or std::timed_mutex
        * config - The configuration params for this Device's IO as a UART::Configuration struct
        * device_id - Some platform/UART specific identifier for a device
        * lock - user supplied lock if IO needs to be synchronized, can be std::mutex or std::timed_mutex
        */
        Device(Configuration config, unit device_id, std::lockable* lock=nullptr) :
            config{config}, lock{lock} {
            if (config.buffer_size > 0) {
                this->buffer = new std::vector<std::byte>{}; // Maybe use a custom allocator or initial capacity
                this->buffer->reserve(DEFAULT_BUFFER_SIZE);
            }
            ... ctor Implementation ...
        };
        virtual ~Device() {
            // Not sure if it makes sense to allow child classes but marking dtor virtual here forn discussion
            // Would lean towards composition over inheritence
            if (this->is_open()) this->close();
        };

        /*
        * Different ways to open the device for communication
        * 1) open() -- Device manages it's own memory buffer for incoming communications
        * 2) open(buffer) -- Device uses the supplied std::vector to store incoming data
        * 3) open(callback) -- Device will call the supplied callback handler when new data is available
        */
        Status open() {...};
        Status open(std::vector<std::byte>& buffer) {...};
        Status open(ReadCallback callback) {...};

        /*
        * Cleans up any resources associated with the device.
        */
        Status close() {...};

        /*
        * If not using callbacks, read data
        */
        Response read() {
            // Acquire any necessary lock, if using a timed lock then block for required amount of time
            if (this->lock) {
                bool acquired = false;
                if(auto timed_lock = dynamic_cast<std::timed_lock>(this->lock)) {
                    acquired = timed_lock.try_lock_for(this->config.timeout);
                } else {
                    acquired = this->lock.try_lock();
                }
                if(!acquired) {
                    return {{ErrorCode::Locked, true}, nullptr};
                }
            }

            ... Implementation ...

            return {{ErrorCode::Okay, false}, this->buffer};
        };

        ErrorCode write(std::vector<std::byte>& data) {...};

    protected:
        Configuration config;
        std::lockable* lock;
        std::vector<std::byte>* buffer;
        ReadCallback callback;
        ... Implementation specific UART data members ...
    }

}
