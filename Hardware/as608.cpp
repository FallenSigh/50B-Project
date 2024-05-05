#include <as608.h>
#include <driver_as608.h>
#include <driver_as608_interface.h>
#include <delay.h>
#include <stm32f10x.h>

#ifdef __cplusplus
extern "C" {
#endif

static as608_handle_t gs_handle;        /**< as608 handle */
static uint32_t gs_addr = 0xffffffff;            /**< as608 address */

uint8_t as608_basic_init(uint32_t addr)
{
    uint8_t res;

    /* link interface function */
    DRIVER_AS608_LINK_INIT(&gs_handle, as608_handle_t);
    DRIVER_AS608_LINK_UART_INIT(&gs_handle, as608_interface_uart_init);
    DRIVER_AS608_LINK_UART_DEINIT(&gs_handle, as608_interface_uart_deinit);
    DRIVER_AS608_LINK_UART_READ(&gs_handle, as608_interface_uart_read);
    DRIVER_AS608_LINK_UART_WRITE(&gs_handle, as608_interface_uart_write);
    DRIVER_AS608_LINK_UART_FLUSH(&gs_handle, as608_interface_uart_flush);
    DRIVER_AS608_LINK_DELAY_MS(&gs_handle, as608_interface_delay_ms);
    DRIVER_AS608_LINK_DEBUG_PRINT(&gs_handle, as608_interface_debug_print);

    /* as608 init */
    res = as608_init(&gs_handle, addr);
    return res;
}

uint8_t as608_basic_input_fingerprint(void (*callback)(uint16_t status, const char *const fmt, ...),
                                      uint16_t *score,
                                      uint16_t *page_number,
                                      as608_status_t *status)
{
    uint8_t res;
    uint32_t timeout;

    /* max default time */
    timeout = AS608_BASIC_DEFAULT_TIMEOUT;

    /* wait your finger */
    while (timeout != 0)
    {
        /* run the callback */
        if (callback != NULL)
        {
            /* output */
            callback(0, "please put your finger on the sensor.\n");
        }

        /* get image */
        res = as608_get_image(&gs_handle, gs_addr, status);
        if (res != 0)
        {
            return 1;
        }
        if (*status == AS608_STATUS_OK)
        {
            /* generate feature */
            res = as608_generate_feature(&gs_handle, gs_addr, AS608_BUFFER_NUMBER_1, status);
            if (res != 0)
            {
                return 1;
            }
            if (*status == AS608_STATUS_OK)
            {
                /* run the callback */
                if (callback != NULL)
                {
                    /* output */
                    callback(1, "please put your finger on the sensor again.\n");
                }

                /* max default time */
                timeout = AS608_BASIC_DEFAULT_TIMEOUT;

                /* wait your finger */
                while (timeout != 0)
                {
                    /* get image */
                    res = as608_get_image(&gs_handle, gs_addr, status);
                    if (res != 0)
                    {
                        return 1;
                    }
                    if (*status == AS608_STATUS_OK)
                    {
                        /* generate feature */
                        res = as608_generate_feature(&gs_handle, gs_addr, AS608_BUFFER_NUMBER_2, status);
                        if (res != 0)
                        {
                            return 1;
                        }
                        if (*status == AS608_STATUS_OK)
                        {
                            /* run the callback */
                            if (callback != NULL)
                            {
                                /* output */
                                callback(2, "generate feature success.\n");
                            }

                            goto next;
                        }
                        else
                        {
                            /* run the callback */
                            if (callback != NULL)
                            {
                                /* output */
                                callback(-1, "error.\n");
                            }
                        }
                    }

                    /* delay 1000ms */
                    as608_interface_delay_ms(1000);

                    /* timeout-- */
                    timeout--;
                }

                /* check timeout */
                if (timeout == 0)
                {
                    return 2;
                }
            }
            else
            {
                /* run the callback */
                if (callback != NULL)
                {
                    /* output */
                    callback(-1, "error.\n");
                }
            }
        }

        /* delay 1000ms */
        as608_interface_delay_ms(1000);

        /* timeout-- */
        timeout--;
    }

    /* check timeout */
    if (timeout == 0)
    {
        return 2;
    }

    next:
    /* match feature */
    res = as608_match_feature(&gs_handle, gs_addr, score, status);
    if (res != 0)
    {
        return 1;
    }
    if (*status != AS608_STATUS_OK)
    {
        /* run the callback */
        if (callback != NULL)
        {
            /* output */
            callback(-1, "error.\n");
        }

        return 1;
    }

    /* combine feature */
    res = as608_combine_feature(&gs_handle, gs_addr, status);
    if (res != 0)
    {
        return 1;
    }
    if (*status != AS608_STATUS_OK)
    {
        /* run the callback */
        if (callback != NULL)
        {
            /* output */
            callback(-1, "error.\n");
        }

        return 1;
    }

    /* get valid template number */
    res = as608_get_valid_template_number(&gs_handle, gs_addr, page_number, status);
    if (res != 0)
    {
        return 1;
    }
    if (*status != AS608_STATUS_OK)
    {
        /* run the callback */
        if (callback != NULL)
        {
            /* output */
            callback(-1, "error.\n");
        }

        return 1;
    }

    /* store feature */
    res = as608_store_feature(&gs_handle, gs_addr, AS608_BUFFER_NUMBER_2, *page_number, status);
    if (res != 0)
    {
        return 1;
    }
    if (*status != AS608_STATUS_OK)
    {
        /* run the callback */
        if (callback != NULL)
        {
            /* output */
            callback(-1, "error.\n");
        }

        return 1;
    }

    return 0;
}

uint8_t as608_basic_high_speed_verify(void (*callback)(uint16_t status, const char *const fmt, ...), uint16_t *found_page, uint16_t *score, as608_status_t *status)
{
    uint8_t res;

    /* wait your finger */
    while (1)
    {
        // feed watchdog
    	IWDG_ReloadCounter();

        /* run the callback */
        if (callback != NULL)
        {
            /* output */
            callback(0, "please put your finger on the sensor.\n");
        }

        /* get image */
        uint32_t start = get_ticks();
        res = as608_get_image(&gs_handle, gs_addr, status);
        if (res != 0)
        {
            return 1;
        }
        if (*status == AS608_STATUS_OK)
        {
            callback(get_ticks() - start, "generating feature.\n");
            /* generate feature */
            res = as608_generate_feature(&gs_handle, gs_addr, AS608_BUFFER_NUMBER_1, status);
            if (res != 0)
            {
                return 1;
            }
            if (*status == AS608_STATUS_OK)
            {
                callback(get_ticks() - start, "generate feature success.\n");
                // high speed search
                res = as608_high_speed_search(&gs_handle, gs_addr, AS608_BUFFER_NUMBER_1, 0, 300, found_page, score, status);
                return res;
            }
            else
            {
                /* run the callback */
                if (callback != NULL)
                {
                    /* output */
                    callback(-1, "error.\n");
                }
            }
        }

        /* delay 1000ms */
        // as608_interface_delay_ms(1);

        /* timeout-- */
        // timeout--;
    }

    /* check timeout */
    // if (timeout == 0)
    // {
    //     callback(-1, "timeout.\n");
    //     return 2;
    // }

    return 0;
}

#ifdef __cplusplus
}
#endif
