#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "decode.h"
#include "common.h"


/* Read and validate decoding arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (argv[2] == NULL)
        return e_failure;

    if (strstr(argv[2], ".bmp") == NULL)
        return e_failure;

    decInfo->stego_image_fname = argv[2];

    if (argv[3] != NULL)
        decInfo->output_fname = argv[3];
    else
        decInfo->output_fname = "decoded.txt";

    return e_success;
}


/* Open stego image */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image =
        fopen(decInfo->stego_image_fname, "rb");

    if (decInfo->fptr_stego_image == NULL)
        return e_failure;

    return e_success;
}


/* Decode one byte from 8 image bytes */
char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;
    int i;

    for (i = 0; i < 8; i++)
    {
        data |= (image_buffer[i] & 1) << i;
    }

    return data;
}


/* Decode multiple bytes from image */
Status decode_data_from_image(char *data,
                              int size,
                              FILE *fptr_stego_image)
{
    char image_buffer[8];
    int i;

    for (i = 0; i < size; i++)
    {
        if (fread(image_buffer, 1, 8, fptr_stego_image) != 8)
            return e_failure;

        data[i] = decode_byte_from_lsb(image_buffer);
    }

    return e_success;
}


/* Check whether the image contains our magic string */
Status decode_magic_string(const char *magic_string,
                           DecodeInfo *decInfo)
{
    char decoded_magic[32];
    int magic_length;

    magic_length = strlen(magic_string);

    if (magic_length >= sizeof(decoded_magic))
        return e_failure;

    if (decode_data_from_image(
            decoded_magic,
            magic_length,
            decInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    decoded_magic[magic_length] = '\0';

    if (strcmp(decoded_magic, magic_string) == 0)
        return e_success;

    return e_failure;
}


/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char extension[4];

    if (decode_data_from_image(
            extension,
            4,
            decInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    memcpy(decInfo->extn_secret_file, extension, 4);

    /* Add string terminator */
    decInfo->extn_secret_file[4] = '\0';

    return e_success;
}


/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    int size;

    if (decode_data_from_image(
            (char *)&size,
            sizeof(int),
            decInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    decInfo->size_secret_file = size;

    return e_success;
}


/* Decode secret file contents */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    long i;
    char data;

    for (i = 0; i < decInfo->size_secret_file; i++)
    {
        if (decode_data_from_image(
                &data,
                1,
                decInfo->fptr_stego_image) == e_failure)
        {
            return e_failure;
        }

        if (fwrite(&data, 1, 1, decInfo->fptr_output) != 1)
        {
            return e_failure;
        }
    }

    return e_success;
}


/* Main decoding process */
Status do_decoding(DecodeInfo *decInfo)
{
    char output_file[256];

    /* Open stego image */
    if (open_decode_files(decInfo) == e_failure)
    {
        printf("ERROR: Unable to open stego image\n");
        return e_failure;
    }


    /* Skip BMP header */
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);


    /* Check magic string */
    if (decode_magic_string(MAGIC_STRING, decInfo) == e_failure)
    {
        printf("ERROR: Invalid stego image\n");

        fclose(decInfo->fptr_stego_image);

        return e_failure;
    }


    /* Decode secret file extension */
    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        fclose(decInfo->fptr_stego_image);

        return e_failure;
    }


    /* Decode secret file size */
    if (decode_secret_file_size(decInfo) == e_failure)
    {
        fclose(decInfo->fptr_stego_image);

        return e_failure;
    }


    /*
     * If user did not provide an extension,
     * add the original secret file extension.
     */
    if (strstr(decInfo->output_fname, ".") == NULL)
    {
        snprintf(output_file,
                 sizeof(output_file),
                 "%s%s",
                 decInfo->output_fname,
                 decInfo->extn_secret_file);

        decInfo->output_fname = output_file;
    }


    /* Open output file */
    decInfo->fptr_output =
        fopen(decInfo->output_fname, "wb");

    if (decInfo->fptr_output == NULL)
    {
        fclose(decInfo->fptr_stego_image);

        return e_failure;
    }


    /* Decode secret file data */
    if (decode_secret_file_data(decInfo) == e_failure)
    {
        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_output);

        return e_failure;
    }


    /* Close files */
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    return e_success;
}