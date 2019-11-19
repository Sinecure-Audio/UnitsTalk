#include "../Units/AmplitudeUnits.h"
#include <iostream>

int main() {
    const Amplitude<float> amplitude = .5f;
    const Decibel<float> decibel = amplitude;

    const Decibel<double> halfPowerDecibel = Amplitude<double>{std::sqrt(.5)};
    const Amplitude<double> halfPowermplitude = halfPowerDecibel;

    std::cout << decibel << '\n';// outputs -6.0206dB
    std::cout << halfPowermplitude << '\n';// outputs .707107
    std::cout << halfPowerDecibel+halfPowerDecibel << '\n';// outputs 0dB
    std::cout << -20.0_dB << std::endl;// outputs -20dB
    std::cout << -200.0_dB << std::endl;// outputs -inf dB

    return 0;
}

// Live example can be found at: https://godbolt.org/z/4buwWc
