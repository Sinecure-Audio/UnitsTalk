#include <chrono>
#include <iostream>

int main() {
    // std::chrono::seconds timeInSeconds{3.2}; // doesn't compile- you can't assign a floating point chrono unit or value to an integral chrono unit.

    std::chrono::duration<float> timeInFractionalSeconds{3.2f}; // represents 3.2 seconds;

    std::chrono::duration<double, std::milli> timeInFractionalMilliSeconds{3.2}; // represents 3.2 milliseconds
    std::chrono::duration<double, std::ratio<1, 1000>> timeInRatioMilliSeconds{3.2}; // also represents 3.2 milliseconds

    std::chrono::duration<double, std::deca> timeInDecaSeconds{3.2}; // 3.2 deca seconds, which is 32 seconds
    std::chrono::duration<double, std::kilo> timeInKiloSeconds{3.2}; // 3.2 kilo seconds, which is 3,200 seconds;

    std::chrono::duration<double> manySeconds = timeInKiloSeconds; // can convert between different duration lengths
    std::cout << manySeconds.count() << std::endl; // prints 3,200

    return 0;
}

// try it at https://godbolt.org/z/EECnpQ