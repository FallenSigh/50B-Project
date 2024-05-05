#ifndef _AS608_H_
#define _AS608_H_

#include <stdint.h>
#include <driver_as608.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AS608_BASIC_DEFAULT_PORT                AS608_BOOL_TRUE                      /**< enable */
#define AS608_BASIC_DEFAULT_BAUD_RATE           6                                    /**< 57600 bps */
#define AS608_BASIC_DEFAULT_LEVEL               AS608_LEVEL_3                        /**< level 3 */
#define AS608_BASIC_DEFAULT_PACKET_SIZE         AS608_PACKET_SIZE_128_BYTES          /**< 128 bytes */
#define AS608_BASIC_DEFAULT_PASSWORD            0x00000000                           /**< 0x00000000 */
#define AS608_BASIC_DEFAULT_ADDRESS             0xFFFFFFFF                           /**< 0xFFFFFFFF */
#define AS608_BASIC_DEFAULT_FEATURE             AS608_BUFFER_NUMBER_1                /**< buffer number 1 */
#define AS608_BASIC_DEFAULT_TIMEOUT             10                                   /**< 10s */


uint8_t as608_basic_init(uint32_t addr);
uint8_t as608_basic_input_fingerprint(void (*callback)(uint16_t status, const char *const fmt, ...),
                                      uint16_t *score,
                                      uint16_t *page_number,
                                      as608_status_t *status);
uint8_t as608_basic_high_speed_verify(void (*callback)(uint16_t status, const char *const fmt, ...), uint16_t *found_page, uint16_t *score, as608_status_t *status);

#ifdef __cplusplus
}
#endif
#endif