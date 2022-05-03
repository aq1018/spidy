#include "PWM.hpp"
#include "driver/ledc.h"
#include "esp32-hal-ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

using namespace pwm;

void PWM::init() {
    ledc_fade_func_install(0);
}

static bool handleFadeComplete(const ledc_cb_param_t* param, void* arg) {
    portBASE_TYPE taskAwoken = pdFALSE;

    if (param->event == LEDC_FADE_END_EVT) {
        SemaphoreHandle_t semaphore = (SemaphoreHandle_t)arg;
        xSemaphoreGiveFromISR(semaphore, &taskAwoken);
    }

    return (taskAwoken == pdTRUE);
}

static ledc_cbs_t callbacks = {
    .fade_cb = &handleFadeComplete,
};

struct PWM::Impl {
    Impl(uint8_t chan, double freq, uint8_t res) : _chan(chan), _freq(freq), _res(res) {
        _pin = 0;
        _period = (double)1000000 / _freq;
        _maxTicks = (double)(1 << _res);
        _ledcMode = ledc_mode_t(_chan / 8);
        _ledcChannel = ledc_channel_t(_chan % 8);
        _semaphore = xSemaphoreCreateBinary();
        registerCallback();
    };

    ~Impl() {
        detach();
        vSemaphoreDelete(_semaphore);
    };

    void changeFrequency(double freq, uint8_t res) {
        _freq = freq;
        _res = res;
        ledcChangeFrequency(_chan, _freq, _res);
    }

    void attach(uint8_t pin) {
        if (_pin > 0) {
            detach();
        }
        _pin = pin;
        ledcAttachPin(_pin, _chan);
    }

    void detach() {
        if (_pin > 0) {
            ledcDetachPin(_pin);
            _pin = 0;
        }
    }

    uint32_t read() { return ticksToMicroSeconds(ledcRead(_chan)); }
    void write(uint32_t duty) { ledcWrite(_chan, microSecondsToTicks(duty)); }

    void registerCallback() {
        ESP_ERROR_CHECK(ledc_cb_register(_ledcMode, _ledcChannel, &callbacks, (void*)_semaphore));
    }

    void fade(uint32_t duty, uint32_t duration) {
        ESP_ERROR_CHECK(ledc_set_fade_time_and_start(_ledcMode, _ledcChannel, duty, duration, LEDC_FADE_NO_WAIT));
    }

    void wait() { xSemaphoreTake(_semaphore, portMAX_DELAY); }

    uint32_t ticksToMicroSeconds(uint32_t ticks) { return uint32_t(_period * ((double)ticks / _maxTicks)); }
    uint32_t microSecondsToTicks(uint32_t duty) { return uint32_t(_maxTicks * ((double)duty / _period)); }

    uint8_t _chan;
    double _freq;
    uint8_t _res;
    uint8_t _pin;
    double _period;
    double _maxTicks;
    ledc_mode_t _ledcMode;
    ledc_channel_t _ledcChannel;
    SemaphoreHandle_t _semaphore;
};

PWM::PWM(Channel channel, double frequency, uint8_t resolution)
    : pImpl(new Impl((uint8_t)channel, frequency, resolution)) {}

PWM::~PWM() = default;

void PWM::changeFrequency(double frequency, uint8_t resolution) {
    pImpl->changeFrequency(frequency, resolution);
}

void PWM::attach(uint8_t pin) {
    pImpl->attach(pin);
}

void PWM::detach() {
    pImpl->detach();
}

uint32_t PWM::read() {
    return pImpl->read();
}

void PWM::write(uint32_t duty) {
    pImpl->write(duty);
}

void PWM::fade(uint32_t duty, uint32_t duration) {
    pImpl->fade(duty, duration);
}

void PWM::wait() {
    pImpl->wait();
}
