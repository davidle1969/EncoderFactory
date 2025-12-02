#include <stdio.h>
#include <iostream> 
#include "library/EncoderFactory.h"

using namespace std;

int main ()
{
    EncoderFactory encoder;
    encoder.process("./config/config.yaml");

    printf("Hello, World!\n");
    cout<<"Hello, World2!\n";
    return 0;
}
