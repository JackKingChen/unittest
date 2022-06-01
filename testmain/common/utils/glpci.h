
#pragma once;

int  glpci_write_bmp(unsigned char *image, int imageWidth, int imageHeight, wchar_t *filename);

int  glpci_write_png(const wchar_t* png_file_name,unsigned char* pixels,int width,int height,int bit_depth);
