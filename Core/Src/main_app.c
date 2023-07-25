#include "main_app.h"
#include "cmsis_os.h"
#include "stm32f1xx_hal.h"
#include "usb_device.h"
#include "keyboard.h"
#include "keypad.h"
#include "led.h"
#include "adc.h"
#include "midi_device.h"
#include "sustain.h"
#include <stdbool.h>

#define ADC_POLL_RATE 20
#define ADC_PITCH_MIDDLE 1900
#define ADC_PITCH_RANGE 60
#define ADC_MODULATION_BOTTOM 1260
#define ADC_MODULATION_RANGE 30
#define ADC_VOLUME_RANGE 50

#define MIDI_SENSE_RATE 500

osThreadId _main_app_thread_handle = NULL;
static KEY_T _main_app_key = KEY_NONE;
static bool _main_app_key_change = false;

/**
 * @brief Keypad key handler
 *
 * @param key
 * @param state
 * @param param
 */
void key_callback_handler(KEY_T key, KEY_STATE_T state, void *param)
{
    if(state == KEY_PRESS)
    {
        _main_app_key = key;
        _main_app_key_change = true;
    }
}

/**
 * @brief Pull up the USB data+ line indicating to the host we are connected
 *
 */
static void signal_usb_ready(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
}

/**
 * @brief Spin the LED's
 *
 * @param cycles Cycles to spin
 */
void led_spin(uint8_t cycles)
{
    uint8_t led[] = { LED_1, LED_2, LED_4, LED_3 };

    for(uint8_t i=0; i<cycles; i++)
    {
        for(uint8_t j=0; j<sizeof(led); j++)
        {
            led_set(led[j], true);
            osDelay(75);
            led_set(led[j], false);
        }
    }
}

/**
 * @brief Main app task
 *
 * @param argument
 */
void main_app_task(void const * argument)
{
    uint32_t adc_tick = ADC_POLL_RATE;
    uint32_t sense_tick = MIDI_SENSE_RATE;
    int8_t octave_offset = 0;
    int8_t transpose_offset = 0;
    int16_t volume;
    int16_t volume_last = -1000;
    int16_t pitch;
    int16_t modulation;
    int16_t modulation_last = -1000;
    bool last_pitch_normal = true;
    bool last_sustain = false;

    // start usb
    MX_USB_DEVICE_Init();

    // signal to usb host we are ready
    signal_usb_ready();

    // prime the adc, eat the values
    volume = adc_get_volume();
    pitch = adc_get_pitch();
    modulation = adc_get_modulation();

    // we are bus powered, wait a short bit then start processing
    // assuming we are connected right away...
    led_spin(5);

    // loop
    while(1)
    {
        osDelay(10);

        // process key presses
        if(_main_app_key_change)
        {
            // octave+ pressed, shift the notes an octave higher
            if(_main_app_key == KEY_OCTAVE_PLUS)
            {
                uint8_t note = keyboard_get_start_note();
                if((note + KEYBOARD_NOTES_PER_OCTAVE + KEYBOARD_TOTAL_KEYS) <= 0x7F)
                {
                    octave_offset++;
                    keyboard_set_start_note(note + KEYBOARD_NOTES_PER_OCTAVE);
                }

                led_set(LED_1, octave_offset > 0);
                led_set(LED_2, octave_offset < 0);
            }

            // octave- pressed, shift the notes an octave lower
            if(_main_app_key == KEY_OCTAVE_MINUS)
            {
                int8_t note = (int8_t)keyboard_get_start_note();
                if((note - KEYBOARD_NOTES_PER_OCTAVE) >= 0)
                {
                    octave_offset--;
                    keyboard_set_start_note(note - KEYBOARD_NOTES_PER_OCTAVE);
                }

                led_set(LED_1, octave_offset > 0);
                led_set(LED_2, octave_offset < 0);
            }

            // transpose+ pressed, shift the notes a step higher
            if(_main_app_key == KEY_TRANSPOSE_PLUS)
            {
                uint8_t note = keyboard_get_start_note();
                if((note + 1 + KEYBOARD_TOTAL_KEYS) <= 0x7F)
                {
                    transpose_offset++;
                    keyboard_set_start_note(note + 1);
                }

                led_set(LED_3, transpose_offset > 0);
                led_set(LED_4, transpose_offset < 0);
            }

            // transpose- pressed, shift the notes a step lower
            if(_main_app_key == KEY_TRANSPOSE_MINUS)
            {
                int8_t note = (int8_t)keyboard_get_start_note();
                if((note - 1) >= 0)
                {
                    transpose_offset--;
                    keyboard_set_start_note(note - 1);
                }

                led_set(LED_3, transpose_offset > 0);
                led_set(LED_4, transpose_offset < 0);
            }

            _main_app_key_change = false;
        }

        if(adc_tick + ADC_POLL_RATE < osKernelSysTick())
        {
            // check pitch
            pitch = adc_get_pitch();
            if(pitch < (ADC_PITCH_MIDDLE - ADC_PITCH_RANGE) || pitch > (ADC_PITCH_MIDDLE + ADC_PITCH_RANGE))
            {
                pitch -= ADC_PITCH_MIDDLE;

                MIDI_DEVICE.pitch_wheel(KEYBOARD_CHANNEL, pitch*8);

                last_pitch_normal = false;
            }
            else
            {
                if(last_pitch_normal == false)
                {
                    MIDI_DEVICE.pitch_wheel(KEYBOARD_CHANNEL, 0);
                }

                last_pitch_normal = true;
            }

            // check modulation
            modulation = adc_get_modulation();
            if(modulation < (modulation_last - ADC_MODULATION_RANGE) || modulation > (modulation_last + ADC_MODULATION_RANGE))
            {
                modulation_last = modulation;

                modulation -= ADC_MODULATION_BOTTOM;

                MIDI_DEVICE.modulation_wheel(KEYBOARD_CHANNEL, modulation * 8);
            }

            // check volume
            volume = adc_get_volume();
            if(volume < (volume_last - ADC_VOLUME_RANGE) || volume > (volume_last + ADC_VOLUME_RANGE))
            {
                volume_last = volume;

                MIDI_DEVICE.volume(KEYBOARD_CHANNEL, volume * 4);
            }

            adc_tick = osKernelSysTick();
        }

        // push out a sense message every so often
        if(sense_tick + MIDI_SENSE_RATE < osKernelSysTick())
        {
            MIDI_DEVICE.sense();

            sense_tick = osKernelSysTick();
        }

        // monitor sustain pedal
        if(last_sustain != sustain_get())
        {
            last_sustain = sustain_get();
            MIDI_DEVICE.sustain(KEYBOARD_CHANNEL, last_sustain);
        }

        // process USB MIDI
        MIDI_DEVICE.task();
    }
}

/**
 * @brief Initialize main app
 *
 */
void main_app_init()
{
    keypad_register_callback(key_callback_handler, NULL);

    osThreadDef(defaultTask, main_app_task, osPriorityNormal, 0, 512);
    _main_app_thread_handle = osThreadCreate(osThread(defaultTask), NULL);
}
