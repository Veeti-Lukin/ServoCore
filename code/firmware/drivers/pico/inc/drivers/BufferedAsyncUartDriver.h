#ifndef BUFFEREDASYNCUARTDRIVER_H
#define BUFFEREDASYNCUARTDRIVER_H

#include <hardware/irq.h>
#include <hardware/uart.h>

#include <cstdint>

#include "assert/assert.h"
#include "drivers/interfaces/BufferedSerialCommunicationInterface.h"
#include "utils/RingBuffer.h"

namespace drivers {

namespace uart_config {

enum class DataBits : uint8_t {
    five  = 5,
    six   = 6,
    seven = 7,
    eight = 8,
};

enum class StopBits : uint8_t {
    one = 1,
    two = 2,
};

enum class Parity {
    none = UART_PARITY_NONE,
    even = UART_PARITY_EVEN,
    odd  = UART_PARITY_ODD,
};

}  // namespace uart_config

//
//
//
//
//
//

/** @brief UART driver with buffered asynchronous communication support.
 *
 * The BufferedAsyncUartDriver template class offers a robust UART driver capable of asynchronous communication.
 * It uses software-based buffering for both transmission (TX) and reception (RX), which enables non-blocking operations
 *
 * @tparam tx_buffer_size Size of the TX buffer.
 * @tparam rx_buffer_size Size of the RX buffer.
 */

template <size_t tx_buffer_size, size_t rx_buffer_size>
class BufferedAsyncUartDriver final : public interfaces::BufferedSerialCommunicationInterface {
public:
    BufferedAsyncUartDriver(uart_inst_t* uart_instance, volatile utils::RingBuffer<tx_buffer_size>* tx_buffer,
                            volatile utils::RingBuffer<rx_buffer_size>* rx_buffer, uint32_t baud_rate,
                            uart_config::DataBits data_bits, uart_config::StopBits stop_bits,
                            uart_config::Parity parity);

    /** @brief Initialize the UART hardware and configure settings. */
    void init();
    /** @brief Deinitialize the UART hardware and reset settings. */
    void deInit();

    /**
     * Configure uart data format and baud rate
     * @param baud_rate target baud rate
     * @param data_bits amount of data bits in one "byte"
     * @param stop_bits amount of stop bits
     * @param parity    parity
     */
    void configure(uint32_t baud_rate, uart_config::DataBits data_bits, uart_config::StopBits stop_bits,
                   uart_config::Parity parity);

    /**
     *  @brief Transmit a null terminated string
     *  @param string Pointer to the null-terminated string to transmit.
     */
    void transmitString(const char* string);

    // public: SerialBufferedCommunicationInterface
    /**
     * @brief Transmit a single byte through the communication interface.
     * @param byte The byte to be transmitted.
     */
    void transmitByte(uint8_t byte) override;
    /**
     * @brief Transmit multiple bytes through the communication interface.
     * @param bytes A span of bytes to be transmitted.
     */
    void transmitBytes(std::span<uint8_t> bytes) override;

    /** @brief Wait until all buffered TX data is transmitted */
    void flushTx();

    /**
     * @brief Get the number of available bytes in the rx buffer.
     * This function checks how many bytes are currently available to be read from the buffer.
     * @return The number of available received bytes.
     */
    size_t getReceivedBytesAvailableAmount() override;
    /**
     * @brief Read a single byte from the rx buffer.
     * @return The byte read from the rx buffer.
     */
    uint8_t readReceivedByte() override;
    /**
     * @brief Read multiple bytes from the rx buffer.
     * @param bytes A span of bytes where the received data will be stored.
     * @return The number of bytes successfully read from the rx buffer.
     */
    size_t readReceivedBytes(std::span<uint8_t> buffer) override;

    /** @brief Handle TX interrupt for asynchronous transmission. */
    void handleTxInterrupt();
    /** @brief Handle RX interrupt for asynchronous reception. */
    void handleRxInterrupt();

    /**
     * @brief Get the NVIC interrupt number for the combined UART interrupt of this UART block.
     *
     * Picos UART block has multiple individual interrupt sources (RX, TX, error, modem-status, etc.)
     * which are logically OR-ed together into a single "combined" interrupt output (UARTINTR).  This
     * combined interrupt is what gets exposed via the NVIC as UART0_IRQ or UART1_IRQ depending on which
     * UART instance is used.
     *
     * @return NVIC interrupt number for the combined UART interrupt (UART0_IRQ or UART1_IRQ).
     * @note Asserts if called on an unsupported UART instance.
     */
    unsigned int getNvicCombinedUartInterruptNumber();

private:
    uart_inst_t* uart_instance_;

    volatile utils::RingBuffer<tx_buffer_size>* tx_ring_buffer_{};
    volatile utils::RingBuffer<rx_buffer_size>* rx_ring_buffer_{};

    uint32_t target_baud_rate_ = 115200;
    uint32_t output_baud_rate_ = 115200;

    struct FormatConfig {
        uart_config::DataBits data_bits;
        uart_config::StopBits stop_bits;
        uart_config::Parity   parity;
    } constructor_format_config_;
};

//
//
//
//
//
//

/// ------------------------ DEFINITIONS --------------------------------------

template <size_t tx_buffer_size, size_t rx_buffer_size>
BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::BufferedAsyncUartDriver(
    uart_inst_t* uart_instance, volatile utils::RingBuffer<tx_buffer_size>* tx_buffer,
    volatile utils::RingBuffer<rx_buffer_size>* rx_buffer, uint32_t baud_rate, uart_config::DataBits data_bits,
    uart_config::StopBits stop_bits, uart_config::Parity parity)
    : uart_instance_(uart_instance),
      tx_ring_buffer_(tx_buffer),
      rx_ring_buffer_(rx_buffer),
      target_baud_rate_(baud_rate),
      constructor_format_config_{data_bits, stop_bits, parity} {}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::init() {
    output_baud_rate_ = uart_init(uart_instance_, target_baud_rate_);
    // Fifo must be disabled because the tx_needs_data interrupt is never triggered otherwise.
    // Software is handling the buffering byte by byte so the FIFO is not even needed
    uart_set_fifo_enabled(uart_instance_, false);
    uart_set_format(uart_instance_, (unsigned int)constructor_format_config_.data_bits,
                    (unsigned int)constructor_format_config_.stop_bits,
                    (uart_parity_t)constructor_format_config_.parity);
    // enable TX and RX interrupt
    uart_set_irq_enables(uart_instance_, true, true);
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::deInit() {
    uart_deinit(uart_instance_);
    // Re enable fifo (default state)
    uart_set_fifo_enabled(uart_instance_, true);
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::configure(uint32_t              baud_rate,
                                                                        uart_config::DataBits data_bits,
                                                                        uart_config::StopBits stop_bits,
                                                                        uart_config::Parity   parity) {
    target_baud_rate_ = baud_rate;
    output_baud_rate_ = uart_set_baudrate(uart_instance_, target_baud_rate_);

    constructor_format_config_(data_bits, stop_bits, parity);
    uart_set_format(uart_instance_, (unsigned int)data_bits, (unsigned int)stop_bits, (uart_parity_t)parity);
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::transmitString(const char* string) {
    const char* string_iterator_pointer = string;

    while (*string_iterator_pointer != '\0') {
        transmitByte(*string_iterator_pointer);
        string_iterator_pointer++;
    }
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::transmitByte(uint8_t byte) {
    // Mutex: ensure the interrupt does not happen during this function
    // Disables the possibility of race cases
    uart_set_irqs_enabled(uart_instance_, true, false);

    const bool is_transmission_ongoing = !tx_ring_buffer_->isEmpty();
    bool       byte_fitted_to_buffer   = tx_ring_buffer_->push(byte);

    if (!byte_fitted_to_buffer) {
        // enable interrupt to let the uart take the next byte from the buffer
        uart_set_irqs_enabled(uart_instance_, true, true);

        while (!byte_fitted_to_buffer) {
            uart_set_irqs_enabled(uart_instance_, true, false);
            byte_fitted_to_buffer = tx_ring_buffer_->push(byte);
            uart_set_irqs_enabled(uart_instance_, true, true);
        }

        uart_set_irqs_enabled(uart_instance_, true, false);
    }

    if (!is_transmission_ongoing) {
        // move byte to uart tx register and start the transmission
        uart_putc_raw(uart_instance_, tx_ring_buffer_->peek());
    }

    uart_set_irqs_enabled(uart_instance_, true, true);
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::transmitBytes(std::span<uint8_t> bytes) {
    for (const uint8_t byte : bytes) {
        transmitByte(byte);
    }
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::flushTx() {
    // Mutex: ensure the interrupt does not happen during this function
    // Disables the possibility of race cases
    uart_set_irqs_enabled(uart_instance_, true, false);

    bool tx_buffer_is_empty = tx_ring_buffer_->isEmpty();

    while (!tx_buffer_is_empty) {
        // enable interrupt to let the uart take the next byte from the buffer
        uart_set_irqs_enabled(uart_instance_, true, true);
        tx_buffer_is_empty = tx_ring_buffer_->isEmpty();
        uart_set_irqs_enabled(uart_instance_, true, false);
    }

    // Wait until the last byte is fully sent
    while (!uart_is_writable(uart_instance_)) {
        __asm volatile("nop");
    }

    // leave the Tx interrupt off since next transmit will enable it again
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
size_t BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::getReceivedBytesAvailableAmount() {
    return rx_ring_buffer_->bytesAvailable();
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
uint8_t BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::readReceivedByte() {
    uart_set_irqs_enabled(uart_instance_, false, true);
    const uint8_t byte = rx_ring_buffer_->pop();
    uart_set_irqs_enabled(uart_instance_, true, true);

    return byte;
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
size_t BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::readReceivedBytes(std::span<uint8_t> buffer) {
    uart_set_irqs_enabled(uart_instance_, false, true);

    size_t i = 0;

    while (i < buffer.size()) {
        // check for trying to read more bytes than available
        if (i == rx_ring_buffer_->bytesAvailable()) break;

        buffer[i] = rx_ring_buffer_->pop();
    }

    uart_set_irqs_enabled(uart_instance_, true, true);
    return i;
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::handleTxInterrupt() {
    // Last byte from the buffer was transmitted
    if (tx_ring_buffer_->isEmpty()) {
        uart_set_irqs_enabled(uart_instance_, true, false);
        return;
    }

    // Remove the transmitted data byte from the buffer
    tx_ring_buffer_->pop();

    // If there are any bytes left to transmit move next byte to UART tx register
    if (!tx_ring_buffer_->isEmpty()) uart_putc_raw(uart_instance_, tx_ring_buffer_->peek());
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
void BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::handleRxInterrupt() {
    if (rx_ring_buffer_->isFull()) return;

    rx_ring_buffer_->push(uart_getc(uart_instance_));
}

template <size_t tx_buffer_size, size_t rx_buffer_size>
unsigned int BufferedAsyncUartDriver<tx_buffer_size, rx_buffer_size>::getNvicCombinedUartInterruptNumber() {
    // The RP2040/RP2350 combines all UART interrupt sources into a single NVIC line:
    //   - UARTRXINTR  (receive FIFO trigger)
    //   - UARTTXINTR  (transmit FIFO trigger)
    //   - UARTEINTR   (error conditions: framing, parity, break, overrun)
    //   - UARTMSINTR  (modem status changes)
    //   - UARTRTINTR (receive timeout)
    // These are OR-ed into UARTINTR, signalled on UART0_IRQ or UART1_IRQ.

    // Map of the nvic interrupt numbers for the combined interrupts
    // If there are more uart modules in the mcu they must be added here
    if (uart_instance_ == uart0) return UART0_IRQ;
    if (uart_instance_ == uart1) return UART1_IRQ;
    ASSERT_WITH_MESSAGE(false, "Unknown uart module, cant map to NVIC irq");
}

}  // namespace drivers

#endif  // BUFFEREDASYNCUARTDRIVER_H
