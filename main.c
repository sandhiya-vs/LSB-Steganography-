#include <stdio.h>
#include "types.h"
#include "encode.h"
#include "decode.h"

int main(int argc, char *argv[])
{
    OperationType operation;

    if (argc < 2)
    {
        printf("Usage:\n");
        printf("Encoding: ./lsb_steg -e input.bmp secret.txt output.bmp\n");
        printf("Decoding: ./lsb_steg -d steged_img.bmp output.txt\n");
        return 1;
    }

    operation = check_operation_type(argv);

    if (operation == e_encode)
    {
        EncodeInfo encInfo = {0};

        if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            printf("Invalid encoding arguments\n");
            return 1;
        }

        if (do_encoding(&encInfo) == e_failure)
        {
            printf("Encoding failed\n");
            return 1;
        }

        printf("Encoding completed successfully\n");
    }
    else if (operation == e_decode)
    {
        DecodeInfo decInfo = {0};

        if (read_and_validate_decode_args(argv, &decInfo) == e_failure)
        {
            printf("Invalid decoding arguments\n");
            return 1;
        }

        if (do_decoding(&decInfo) == e_failure)
        {
            printf("Decoding failed\n");
            return 1;
        }

        printf("Decoding completed successfully\n");
    }
    else
    {
        printf("Invalid operation\n");
        printf("Use -e for encoding or -d for decoding\n");
        return 1;
    }

    return 0;
}