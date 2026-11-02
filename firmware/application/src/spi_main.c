#include "spi_main.h"

#include <string.h>

#include "nrfx_spis.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "dataframe.h"
#include "netdata.h"
#include "data_cmd.h"
#include "app_status.h"
#include "nfc_14a_4.h"

#define NRF_LOG_MODULE_NAME spi_main
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#ifndef SPI_PIN_SCK
#define SPI_PIN_SCK  NRF_GPIO_PIN_MAP(0, 3)
#endif
#ifndef SPI_PIN_MOSI
#define SPI_PIN_MOSI NRF_GPIO_PIN_MAP(0, 5)
#endif
#ifndef SPI_PIN_MISO
#define SPI_PIN_MISO NRF_GPIO_PIN_MAP(0, 7)
#endif
#ifndef SPI_PIN_CSN
#define SPI_PIN_CSN  NRF_GPIO_PIN_MAP(0, 14)
#endif
#ifndef SPI_PIN_IRQ
#define SPI_PIN_IRQ  NRF_GPIO_PIN_MAP(0, 16)
#endif

// SPIS0/1 share peripherals with the SPI0 reader driver, so use instance 2.
static const nrfx_spis_t m_spis = NRFX_SPIS_INSTANCE(2);

static uint8_t m_tx[SPI_BUF_SIZE];
static uint8_t m_rx[SPI_BUF_SIZE];
static uint8_t m_rx_pending[SPI_BUF_SIZE];
static volatile size_t m_rx_pending_len = 0;
static volatile bool m_response_ready = false;
static volatile bool m_request_pending = false;
static bool m_apdu_push = false;

static void spis_arm(void) {
    APP_ERROR_CHECK(nrfx_spis_buffers_set(&m_spis, m_tx, sizeof(m_tx), m_rx, sizeof(m_rx)));
}

static void spis_event_handler(nrfx_spis_evt_t const *p_event, void *p_context) {
    if (p_event->evt_type != NRFX_SPIS_XFER_DONE) {
        return;
    }
    if (p_event->rx_amount > 0 && m_rx[0] == NETDATA_FRAME_SOF) {
        // command transaction; drop it if the previous one was not consumed yet
        if (m_rx_pending_len == 0) {
            memcpy(m_rx_pending, m_rx, p_event->rx_amount);
            m_rx_pending_len = p_event->rx_amount;
        }
    } else if (m_response_ready) {
        // read transaction: response has been clocked out
        m_response_ready = false;
        memset(m_tx, 0, sizeof(m_tx));
        nrf_gpio_pin_clear(SPI_PIN_IRQ);
    }
    spis_arm();
}

void spi_slave_init(void) {
    nrf_gpio_cfg_output(SPI_PIN_IRQ);
    nrf_gpio_pin_clear(SPI_PIN_IRQ);

    nrfx_spis_config_t config = NRFX_SPIS_DEFAULT_CONFIG;
    config.sck_pin = SPI_PIN_SCK;
    config.mosi_pin = SPI_PIN_MOSI;
    config.miso_pin = SPI_PIN_MISO;
    config.csn_pin = SPI_PIN_CSN;
    config.mode = NRF_SPIS_MODE_0;
    config.orc = 0x00;
    config.def = 0x00;
    APP_ERROR_CHECK(nrfx_spis_init(&m_spis, &config, spis_event_handler, NULL));
    spis_arm();
    NRF_LOG_INFO("SPI slave ready");
}

void spi_set_apdu_push(bool enable) {
    m_apdu_push = enable;
}

static void spi_push_apdu(void) {
    static uint8_t apdu[NFC_14A_4_MAX_APDU];
    uint16_t len = 0;
    if (!m_apdu_push || m_response_ready || m_request_pending || m_rx_pending_len > 0) {
        return;
    }
    if (nfc_tag_14a_4_get_pending_apdu(apdu, &len)) {
        data_frame_tx_t *frame = data_frame_make(DATA_CMD_HF14A_4_APDU_RECV, STATUS_SUCCESS, len, apdu);
        if (frame != NULL) {
            spi_data_response(frame->buffer, frame->length);
        }
    }
}

void spi_slave_process(void) {
    if (m_rx_pending_len > 0) {
        m_request_pending = true;
        data_frame_receive(m_rx_pending, (uint16_t)m_rx_pending_len);
        m_rx_pending_len = 0;
    }
    spi_push_apdu();
}

bool spi_request_pending(void) {
    return m_request_pending;
}

void spi_request_clear(void) {
    m_request_pending = false;
}

void spi_data_response(const void *p_buf, uint16_t length) {
    if (length > sizeof(m_tx)) {
        NRF_LOG_ERROR("SPI response too large: %d", length);
        return;
    }
    memcpy(m_tx, p_buf, length);
    m_response_ready = true;
    nrf_gpio_pin_set(SPI_PIN_IRQ);
}
