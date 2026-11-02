#ifndef SPI_MAIN_H
#define SPI_MAIN_H

#include <stdint.h>
#include <stdbool.h>

/*
 * SPI slave transport (nRF52840 = SPIS2, host e.g. ESP32 = master, SPI mode 0, MSB first).
 *
 * Command:  host sends one transaction that starts with a regular Chameleon data frame
 *           (first byte 0x11). A frame may be split over several transactions.
 * Response: when the response is ready the IRQ pin goes high. The host then runs a
 *           "read" transaction (MOSI = all 0x00) and clocks the response frame out of MISO.
 *           The IRQ pin goes low when that transaction ends.
 * Event:    with HF14A_4_SET_HOST_TIMEOUT (apdu_push=1) a relayed ISO14443-4 APDU is delivered as an
 *           unsolicited HF14A_4_APDU_RECV frame the same way (IRQ high -> read). If IRQ is high,
 *           read first before sending the next command.
 * Pins are overridable at build time, e.g. CFLAGS += -DSPI_PIN_SCK=NRF_GPIO_PIN_MAP\(0,3\)
 */

#ifndef SPI_BUF_SIZE
#define SPI_BUF_SIZE 1040
#endif

void spi_slave_init(void);
void spi_slave_process(void);
bool spi_request_pending(void);
void spi_request_clear(void);
void spi_data_response(const void *p_buf, uint16_t length);
// Push relayed ISO14443-4 APDUs to the host as unsolicited HF14A_4_APDU_RECV frames (IRQ high).
// If IRQ is high the host must read before sending its next command.
void spi_set_apdu_push(bool enable);

#endif
