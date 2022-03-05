#pragma once
#include <iostream>

#include <random>
class RNDGenerator
{
public:
    std::mt19937 gen;

public:
    static RNDGenerator *GetInstance();

    RNDGenerator(const RNDGenerator &other) = delete;
    RNDGenerator(RNDGenerator &other) = delete;
    void operator=(const RNDGenerator &other) = delete;

    void Seed(int seed);
    float Float();
    float Float(float max);
    float Float(float min, float max);

    uint32_t Int();
    uint32_t Int(uint32_t max);
    uint32_t Int(uint32_t min, uint32_t max);

private:
    static RNDGenerator *s_Instance;

    std::random_device rd;

protected:
    RNDGenerator();
};