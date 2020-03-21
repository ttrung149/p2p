#include "../src/messages.h"

int main() 
{
    RegisterMsg *reg_msg = create_register_msg(50, 
        "file.txt", "localhost", 9065, 
        "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9");
    RegisterMsg register_msg;
    parse_register_msg((char *)reg_msg, register_msg);
    free(reg_msg);

    ErrFileNotFoundMsg *err_msg = create_err_file_not_found_msg();
    ErrFileNotFoundMsg err_file_not_found_msg;
    parse_err_file_not_found_msg((char *)err_msg, err_file_not_found_msg);
    free(err_msg);
}