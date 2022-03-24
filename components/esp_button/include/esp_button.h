/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/gpio.h>
#include <freertos/portmacro.h>

typedef void (* button_cb)(void *);
typedef void *button_handle_t;

typedef enum {
    BUTTON_ACTIVE_HIGH = 1,    /*!<button active level: high level*/
    BUTTON_ACTIVE_LOW = 0,     /*!<button active level: low level*/
} button_active_t;

typedef enum {
    BUTTON_CB_PUSH = 0,   /*!<button push callback event */
    BUTTON_CB_RELEASE,    /*!<button release callback event */
    BUTTON_CB_TAP,        /*!<button quick tap callback event(will not trigger if there already is a "PRESS" event) */
    BUTTON_CB_SERIAL,     /*!<button serial trigger callback event */
} button_cb_type_t;

/**
 * @brief Init button functions
 *
 * @param gpio_num GPIO index of the pin that the button uses
 * @param active_level button hardware active level.
 *        For "BUTTON_ACTIVE_LOW" it means when the button pressed, the GPIO will read low level.
 *
 * @return A button_handle_t handle to the created button object, or NULL in case of error.
 */
button_handle_t button_create(gpio_num_t gpio_num, button_active_t active_level);

/**
 * @brief Register a callback function for a serial trigger event.
 *
 * @param btn_handle handle of the button object
 * @param start_after_sec define the time after which to start serial trigger action
 * @param interval_tick serial trigger interval
 * @param cb callback function for "TAP" action.
 * @param arg Parameter for callback function
 * @note
 *        Button callback functions execute in the context of the timer service task.
 *        It is therefore essential that button callback functions never attempt to block.
 *        For example, a button callback function must not call vTaskDelay(), vTaskDelayUntil(),
 *        or specify a non zero block time when accessing a queue or a semaphore.
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Parameter error
 */
esp_err_t button_add_serial_cb(button_handle_t btn_handle, uint32_t start_after_sec, TickType_t interval_tick, button_cb cb, void *arg);

/**
 * @brief Register a callback function for a button_cb_type_t action.
 *
 * @param btn_handle handle of the button object
 * @param type callback function type
 * @param cb callback function for "TAP" action.
 * @param arg Parameter for callback function
 * @note
 *        Button callback functions execute in the context of the timer service task.
 *        It is therefore essential that button callback functions never attempt to block.
 *        For example, a button callback function must not call vTaskDelay(), vTaskDelayUntil(),
 *        or specify a non zero block time when accessing a queue or a semaphore.
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Parameter error
 */
esp_err_t button_add_tap_cb(button_handle_t btn_handle, button_cb_type_t type, button_cb cb, void *arg);

/**
 * @brief Callbacks invoked as timer events occur while button is pressed.
 *        Example: If a button is configured for 2 sec, 5 sec and 7 sec callbacks and if the button is pressed for 6 sec then 2 sec callback would be invoked at 2 sec event and 5 sec callback would be invoked at 5 sec event
 *
 * @param btn_handle handle of the button object
 * @param press_sec the callback function would be called if you press the button for a specified period of time
 * @param cb callback function for "PRESS and HOLD" action.
 * @param arg Parameter for callback function
 *
 * @note
 *        Button callback functions execute in the context of the timer service task.
 *        It is therefore essential that button callback functions never attempt to block.
 *        For example, a button callback function must not call vTaskDelay(), vTaskDelayUntil(),
 *        or specify a non zero block time when accessing a queue or a semaphore.
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Parameter error
 */
esp_err_t button_add_press_cb(button_handle_t btn_handle, uint32_t press_sec, button_cb cb, void *arg);

/**
 * @brief Single callback invoked according to the latest timer event on button release.
 *        Example: If a button is configured for 2 sec, 5 sec and 7 sec callbacks and if the button is released at 6 sec then only 5 sec callback would be invoked
 *
 * @param btn_handle handle of the button object
 * @param press_sec the callback function would be called if you press the button for a specified period of time
 * @param cb callback function for "PRESS and RELEASE" action.
 * @param arg Parameter for callback function
 *
 * @note
 *        Button callback functions execute in the context of the timer service task.
 *        It is therefore essential that button callback functions never attempt to block.
 *        For example, a button callback function must not call vTaskDelay(), vTaskDelayUntil(),
 *        or specify a non zero block time when accessing a queue or a semaphore.
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Parameter error
 */
esp_err_t button_add_release_cb(button_handle_t btn_handle, uint32_t press_sec, button_cb cb, void *arg);

/**
 * @brief Delete button object and free memory
 * @param btn_handle handle of the button object
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Parameter error
 */
esp_err_t button_delete(button_handle_t btn_handle);

/**
 * @brief Remove callback
 *
 * @param btn_handle The handle of the button object
 * @param type callback function event type
 *
 * @return
 *     - ESP_OK Success
 */
esp_err_t button_rm_cb(button_handle_t btn_handle, button_cb_type_t type);

#ifdef __cplusplus
}
#endif