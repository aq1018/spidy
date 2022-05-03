#pragma once

#include <memory>
#include "PWM.hpp"

namespace pwm {

class Servo {
   public:
    Servo(PWM& pwm, uint32_t minDuty, uint32_t maxDuty);
    ~Servo();

    void attach(uint8_t pin);
    void detach();
    double get();
    void set(double angle);
    void set(double angle, uint32_t duration);
    void wait();

   private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}  // namespace pwm
