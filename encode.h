#ifndef ENCODE_H
#define ENCODE_H

#include <stdio.h>
#include "types.h"

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 5

/* Information needed for encoding */
typedef struct _EncodeInfo
{
    char *src_image_fname;
    FILE *fptr_src_image;

    uint image_capacity;
    uint bits_per_pixel;

    char image_data[MAX_IMAGE_BUF_SIZE];

    char *secret_fname;
    FILE *fptr_secret;

    char extn_secret_file[MAX_FILE_SUFFIX];
    char secret_data[MAX_SECRET_BUF_SIZE];

    long size_secret_file;

    char *stego_image_fname;
    FILE *fptr_stego_image;

} EncodeInfo;


/* Function declarations */

OperationType check_operation_type(char *argv[]);

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo);

Status do_encoding(EncodeInfo *encInfo);

Status open_files(EncodeInfo *encInfo);

Status check_capacity(EncodeInfo *encInfo);

uint get_image_size_for_bmp(FILE *fptr_image);

uint get_file_size(FILE *fptr);

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);

Status encode_secret_file_data(EncodeInfo *encInfo);

Status encode_data_to_image(char *data,
                            int size,
                            FILE *fptr_src_image,
                            FILE *fptr_stego_image);

Status encode_byte_to_lsb(char data, char *image_buffer);

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

#endif