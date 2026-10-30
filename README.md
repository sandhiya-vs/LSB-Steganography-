# LSB-Steganography-# Image Steganography in C

## Project Description
Developed an Image Steganography project using C to hide secret text messages inside BMP images using the **Least Significant Bit (LSB) technique**.

## Key Features
- Encode text messages into BMP images.
- Decode hidden messages from images.
- Command-line argument handling.
- Error handling and file operations.

## Technologies Used
- C Programming
- Data Structures
- Pointers and File Handling
- Bitwise Operations
- LSB Technique
- Ubuntu Linux and GCC

## Compilation
```bash
gcc main.c encode.c decode.c -o lsb_steg
```

## Sample Execution
```bash
./lsb_steg -e beautiful.bmp secret.txt steged_img.bmp
./lsb_steg -d steged_img.bmp decoded.txt
```

**Learning Outcome:** Gained practical knowledge of bitwise operations, file handling, and data hiding techniques in C.
