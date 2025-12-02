#include <stdio.h>
#include <iostream> 
#include "include/EncoderFactory.h"

using namespace std;

int main ()
{
    EncoderFactory encoder;
    encoder.process("./config/config.yaml");

    printf("Hello, World!\n");
    cout<<"Hello, World2!\n";
    return 0;
}
