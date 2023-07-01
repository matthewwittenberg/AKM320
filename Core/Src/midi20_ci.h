#ifndef SRC_MIDI20_CI_H_
#define SRC_MIDI20_CI_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    MIDI20_CI_RESULT_SUCCESS,
    MIDI20_CI_RESULT_UNKNOWN_PROPERTY,
} MIDI20_CI_RESULT_T;

typedef void (*ci_process_callback)(uint8_t *pmessage, uint32_t length);

void midi20_ci_init();
void midi20_ci_process(uint8_t *pmessage, uint32_t length, ci_process_callback callback);

#endif
