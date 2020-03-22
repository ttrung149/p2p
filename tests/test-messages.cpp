#include "../src/messages.h"

int main() 
{
    RegisterMsg *reg_msg = create_register_msg(50, 
        "file.txt", "localhost", 9065, 
        "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde\0");
    RegisterMsg parsed_register;
    parse_register_msg((char *)reg_msg, parsed_register);
    free(reg_msg);

    ErrFileNotFoundMsg *err_msg = create_err_file_not_found_msg();
    ErrFileNotFoundMsg parsed_err;
    parse_err_file_not_found_msg((char *)err_msg, parsed_err);
    free(err_msg);

    ReqPeerMsg *reqpeer_msg = create_reqpeer_msg("file.txt", "localhost", 9065);
    ReqPeerMsg parsed_reqpeer;
    parse_reqpeer_msg((char *)reqpeer_msg, parsed_reqpeer);
    free(reqpeer_msg);

    char *data_msg = create_data_msg(187, (char *)"Lorem ipsum dolor \
    sit amet, consectetur adipiscing elit, sed do eiusmod tempor \
    incididunt ut labore et dolore magna aliqua. Ut enim ad minim \
    veniam, quis nostrud exercitation");
    DataMsg parsed_data;
    parse_data_msg(data_msg, parsed_data);
    free(data_msg);
}