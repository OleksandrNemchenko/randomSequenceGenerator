
#include <iostream>
#include <randomSequenceGenerator.hpp>

void TestSequence(CRandomSequenceGenerator::EGeneratorType genType);
void TestGeneralAbilities();

int main(int argc, char* argv[])
{
    std::cout << "* Test random sequence generators" << std::endl;

    try
    {
        std::cout << "* Test general abilities : " << std::endl;
        TestGeneralAbilities();

        std::cout << "* GPU generator : " << std::endl;
        TestSequence(CRandomSequenceGenerator::GPU_GENERATOR);

        std::cout << "* CPU implementation" << std::endl;
        TestSequence(CRandomSequenceGenerator::CPU_GENERATOR);
    }
    catch (...)
    {
        std::cout << "*** Unhandled exception has been thrown" << std::endl;
    }
}