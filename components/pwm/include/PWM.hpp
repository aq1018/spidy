#pragma once

#include <memory>

namespace pwm {
class PWM {
   public:
    enum Channel {
        // high speed channels
        ChannelHS1 = 0,  // timer 1
        ChannelHS2,      // timer 1
        ChannelHS3,      // timer 2
        ChannelHS4,      // timer 2
        ChannelHS5,      // timer 3
        ChannelHS6,      // timer 3
        ChannelHS7,      // timer 4
        ChannelHS8,      // timer 4

        // low speed channels
        ChannelLS1,  // timer 1
        ChannelLS2,  // timer 1
        ChannelLS3,  // timer 2
        ChannelLS4,  // timer 2
        ChannelLS5,  // timer 3
        ChannelLS6,  // timer 3
        ChannelLS7,  // timer 4
        ChannelLS8,  // timer 4
    };

    PWM(Channel channel, double freqency, uint8_t resolution);
    ~PWM();

    void changeFrequency(double freqency, uint8_t resolution);
    void attach(uint8_t pin);
    void detach();
    uint32_t read();
    void write(uint32_t duty);
    void fade(uint32_t duty, uint32_t duration);
    void wait();

    static void init();

   private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}  // namespace pwm
