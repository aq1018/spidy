#include "Servo.hpp"

using namespace pwm;

struct Servo::Impl {
    Impl(PWM& pwm, uint32_t minDuty, uint32_t maxDuty) : pwm(pwm), minDuty(minDuty), maxDuty(maxDuty) {}
    ~Impl() = default;

    void attach(uint8_t pin) { pwm.attach(pin); }
    void detach() { pwm.detach(); }
    double get() { return dutyToAngle(pwm.read()); }
    void set(double angle) { pwm.write(angleToDuty(angle)); }
    void set(double angle, uint32_t duration) { pwm.fade(angleToDuty(angle), duration); }
    void wait() { pwm.wait(); }
    double dutyToAngle(uint32_t duty) { return duty * (double)(maxDuty - minDuty) / (double)180; }
    uint32_t angleToDuty(double angle) { return minDuty + uint32_t((double)(maxDuty - minDuty) * angle / (double)180); }

    PWM& pwm;
    uint32_t minDuty;
    uint32_t maxDuty;
};

Servo::Servo(PWM& pwm, uint32_t minDuty, uint32_t maxDuty) : pImpl(new Impl(pwm, minDuty, maxDuty)) {}

Servo::~Servo() = default;

void Servo::attach(uint8_t pin) {
    pImpl->attach(pin);
}

void Servo::detach() {
    pImpl->detach();
}

double Servo::get() {
    return pImpl->get();
}

void Servo::set(double angle) {
    pImpl->set(angle);
}

void Servo::set(double angle, uint32_t duration) {
    pImpl->set(angle, duration);
}

void Servo::wait() {
    pImpl->wait();
}
