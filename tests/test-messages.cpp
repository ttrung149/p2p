#include "../src/messages.h"

int main() 
{
    RegisterMsg *msg = create_register_msg(REGISTER, 50, "file.txt", 
        "localhost", 9065, 
        "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9");

    RegisterMsg register_msg;
    parse_register_msg((char *)msg, register_msg);

    free(msg);
}