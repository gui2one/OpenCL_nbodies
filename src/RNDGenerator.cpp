#include "RNDGenerator.h"

RNDGenerator *RNDGenerator::s_Instance = nullptr;
RNDGenerator *RNDGenerator::GetInstance()
{
    if (s_Instance == nullptr)
    {
        s_Instance = new RNDGenerator();
    }

    return s_Instance;
}

RNDGenerator::RNDGenerator()
{
    // std::cout << "RNDGenerator !!!!" << std::endl;
}

void RNDGenerator::Seed(int seed)
{
    gen.seed(seed);
}
float RNDGenerator::Float()
{
    return static_cast<float>(gen()) / static_cast<float>(gen.max());
}
float RNDGenerator::Float(float max)
{
    auto lerp = Float() * max;
    return lerp;
}
float RNDGenerator::Float(float min, float max)
{
    auto lerp = Float() * (max - min) + min;
    return lerp;
}

uint32_t RNDGenerator::Int()
{
    return gen();
}
uint32_t RNDGenerator::Int(uint32_t max)
{
    auto lerp = Int() / (gen.max() / max + 1);
    return lerp;
}
uint32_t RNDGenerator::Int(uint32_t min, uint32_t max)
{
    auto lerp = Int() / (gen.max() / (max + 1 - min)) + min;
    return lerp;
}