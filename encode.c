#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "encode.h"
#include "common.h"


/* Check whether user selected encoding or decoding */
OperationType check_operation_type(char *argv[])
{
    if (argv[1] == NULL)
        return e_unsupported;

    if (strcmp(argv[1], "-e") == 0)
        return e_encode;

    if (strcmp(argv[1], "-d") == 0)
        return e_decode;

    return e_unsupported;
}


/* Read and validate encoding arguments */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (argv[2] == NULL || argv[3] == NULL)
        return e_failure;

    if (strstr(argv[2], ".bmp") == NULL)
        return e_failure;

    encInfo->src_image_fname = argv[2];
    encInfo->secret_fname = argv[3];

    if (argv[4] != NULL)
        encInfo->stego_image_fname = argv[4];
    else
        encInfo->stego_image_fname = "steged_img.bmp";

    return e_success;
}


/* Open source image, secret file and output image */
Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");

    if (encInfo->fptr_src_image == NULL)
    {
        perror("Source image");
        return e_failure;
    }


    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");

    if (encInfo->fptr_secret == NULL)
    {
        perror("Secret file");

        fclose(encInfo->fptr_src_image);

        return e_failure;
    }


    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");

    if (encInfo->fptr_stego_image == NULL)
    {
        perror("Stego image");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);

        return e_failure;
    }

    return e_success;
}


/* Get BMP image capacity */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width;
    uint height;

    fseek(fptr_image, 18, SEEK_SET);

    fread(&width, sizeof(uint), 1, fptr_image);
    fread(&height, sizeof(uint), 1, fptr_image);

    return width * height * 3;
}


/* Get size of a file */
uint get_file_size(FILE *fptr)
{
    long current_position;
    long file_size;

    current_position = ftell(fptr);

    fseek(fptr, 0, SEEK_END);

    file_size = ftell(fptr);

    fseek(fptr, current_position, SEEK_SET);

    return (uint)file_size;
}


/* Check whether image has enough space */
Status check_capacity(EncodeInfo *encInfo)
{
    uint required_bits;

    required_bits =
        (strlen(MAGIC_STRING)
        + strlen(encInfo->extn_secret_file)
        + sizeof(int)
        + encInfo->size_secret_file) * 8;

    if (required_bits <= encInfo->image_capacity)
        return e_success;

    return e_failure;
}


/* Copy BMP header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];

    fseek(fptr_src_image, 0, SEEK_SET);

    if (fread(buffer, 1, 54, fptr_src_image) != 54)
        return e_failure;

    if (fwrite(buffer, 1, 54, fptr_dest_image) != 54)
        return e_failure;

    return e_success;
}


/* Encode one byte into 8 image bytes */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    int i;

    for (i = 0; i < 8; i++)
    {
        image_buffer[i] =
            (image_buffer[i] & 0xFE) |
            ((data >> i) & 1);
    }

    return e_success;
}


/* Encode multiple bytes into image */
Status encode_data_to_image(char *data,
                            int size,
                            FILE *fptr_src_image,
                            FILE *fptr_stego_image)
{
    char image_buffer[8];
    int i;

    for (i = 0; i < size; i++)
    {
        if (fread(image_buffer, 1, 8, fptr_src_image) != 8)
            return e_failure;

        encode_byte_to_lsb(data[i], image_buffer);

        if (fwrite(image_buffer, 1, 8, fptr_stego_image) != 8)
            return e_failure;
    }

    return e_success;
}


/* Encode magic string */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    return encode_data_to_image(
        (char *)magic_string,
        strlen(magic_string),
        encInfo->fptr_src_image,
        encInfo->fptr_stego_image
    );
}


/* Encode secret file extension */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    return encode_data_to_image(
        (char *)file_extn,
        strlen(file_extn),
        encInfo->fptr_src_image,
        encInfo->fptr_stego_image
    );
}


/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    int size = (int)file_size;

    return encode_data_to_image(
        (char *)&size,
        sizeof(int),
        encInfo->fptr_src_image,
        encInfo->fptr_stego_image
    );
}


/* Encode secret file contents */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

    rewind(encInfo->fptr_secret);

    while (fread(&ch, 1, 1, encInfo->fptr_secret) == 1)
    {
        if (encode_data_to_image(
                &ch,
                1,
                encInfo->fptr_src_image,
                encInfo->fptr_stego_image) == e_failure)
        {
            return e_failure;
        }
    }

    return e_success;
}


/* Copy remaining image data */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char buffer[1024];
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), fptr_src)) > 0)
    {
        if (fwrite(buffer, 1, bytes, fptr_dest) != bytes)
            return e_failure;
    }

    return e_success;
}


/* Main encoding process */
Status do_encoding(EncodeInfo *encInfo)
{
    char *dot;
    int extension_length;

    /* Open all required files */
    if (open_files(encInfo) == e_failure)
        return e_failure;


    /* Find image capacity */
    encInfo->image_capacity =
        get_image_size_for_bmp(encInfo->fptr_src_image);


    /* Find secret file size */
    encInfo->size_secret_file =
        get_file_size(encInfo->fptr_secret);


    /* Find secret file extension */
    dot = strrchr(encInfo->secret_fname, '.');

    if (dot == NULL)
    {
        printf("ERROR: Secret file has no extension\n");
        return e_failure;
    }

    extension_length = strlen(dot);

    if (extension_length >= MAX_FILE_SUFFIX)
    {
        printf("ERROR: Secret file extension too long\n");
        return e_failure;
    }

    strcpy(encInfo->extn_secret_file, dot);


    /* Check whether image has enough capacity */
    if (check_capacity(encInfo) == e_failure)
    {
        printf("ERROR: Insufficient image capacity\n");
        return e_failure;
    }


    /* Copy BMP header */
    if (copy_bmp_header(
            encInfo->fptr_src_image,
            encInfo->fptr_stego_image) == e_failure)
        return e_failure;


    /* Encode magic string */
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
        return e_failure;


    /* Encode secret file extension */
    if (encode_secret_file_extn(
            encInfo->extn_secret_file,
            encInfo) == e_failure)
        return e_failure;


    /* Encode secret file size */
    if (encode_secret_file_size(
            encInfo->size_secret_file,
            encInfo) == e_failure)
        return e_failure;


    /* Encode secret file contents */
    if (encode_secret_file_data(encInfo) == e_failure)
        return e_failure;


    /* Copy remaining image data */
    if (copy_remaining_img_data(
            encInfo->fptr_src_image,
            encInfo->fptr_stego_image) == e_failure)
        return e_failure;


    /* Close all files */
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    return e_success;
}