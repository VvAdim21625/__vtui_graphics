#pragma once
#include <cstdint>

/**
 * @brief Инициализирует платформу рендера.
 * @return true при успехе, false в случае ошибки.
 */
bool vtui_pal_init(void);

/**
 * @brief Завершает работу платформы рендера.
 * @return true при успехе, false в случае ошибки.
 */
bool vtui_pal_shutdown(void);

/**
 * @brief Читает данные входного потока в буфер.
 * @param buffer Указатель на буфер для записи данных.
 * @param max_bytes Максимальный размер буфера в байтах.
 * @return true при успешном чтении/записи, false при ошибке.
 */
bool vtui_pal_write_input(uint8_t* buffer, int max_bytes);

/**
 * @brief Записывает данные в выходной поток.
 * @param buffer Указатель на буфер с данными.
 * @param bytes Количество байт для записи.
 * @return true при успехе, false в случае ошибки.
 */
bool vtui_pal_write_output(const uint8_t* buffer, int bytes);

/**
 * @brief Опрашивает событие.
 * @param out_event Указатель на переменную для хранения события.
 * @return true при успехе, false в случае ошибки.
 */
bool vtui_pal_poll_raw_event(void* out_record);
