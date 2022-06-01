
#include <Windows.h>

#include <stdio.h>
#include <io.h>
#include "glpci.h"
#include "png.h"

int  glpci_write_bmp(unsigned char *image, int imageWidth, int imageHeight, wchar_t *filename)
{
    unsigned char header[54] = {
        0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0,
        54, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 32, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0
    };

    long file_size = (long)imageWidth * (long)imageHeight * 4 + 54;
    header[2] = (unsigned char)(file_size &0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    long width = imageWidth;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) &0x000000ff;
    header[20] = (width >> 16) &0x000000ff;
    header[21] = (width >> 24) &0x000000ff;

    long height = imageHeight;
    header[22] = height &0x000000ff;
    header[23] = (height >> 8) &0x000000ff;
    header[24] = (height >> 16) &0x000000ff;
    header[25] = (height >> 24) &0x000000ff;

    wchar_t fname_bmp[MAX_PATH] = { 0 };
    wsprintf(fname_bmp, L"%s.bmp", filename);

    FILE *fp;
    if (!(fp = _wfopen(fname_bmp, L"wb")))
        return -1;

    fwrite(header, sizeof(unsigned char), 54, fp);

    for (int i=0; i<(imageWidth * imageHeight); i++)
    {
        int tmp      = image[i*4];
        image[i*4]   = image[i*4+2];
        image[i*4+2]   = tmp;
    }

    int l = 0;
    int r = imageHeight - 1;
    for(;l<r;l++,r--)
    {
        for(int j=0;j<imageWidth;j++)
        {
            for(int i=0;i<4;i++)
            {
                int tmp          = image[(l*imageWidth+j)*4+i];
                image[(l*imageWidth+j)*4+i] = image[(r*imageWidth+j)*4+i];
                image[(r*imageWidth+j)*4+i] = tmp;
            }
        }
    }

    fwrite(image, sizeof(unsigned char), (size_t)(long)imageWidth * imageHeight * 4, fp);

    fclose(fp);

    return 0;
}

int  glpci_write_png(const wchar_t* png_file_name,unsigned char* pixels,int width,int height,int bit_depth)
{
    png_structp png_ptr;
    png_infop   info_ptr;
    if(_waccess(png_file_name,0) == 0)
    {
        _wremove(png_file_name);
    }
    FILE *png_file = _wfopen(png_file_name, TEXT("wb"));
    if(!png_file)
    {
        return -1;
    }
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(png_ptr == NULL)
    {
        printf("Error:png_create_write_struct\n");
        fclose(png_file);
        return -1;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL)
    {
        printf("ERROR:png_create_info_struct/n");
        png_destroy_write_struct(&png_ptr, NULL);
        return 0;
    }
    png_init_io(png_ptr,png_file);
    png_set_IHDR(png_ptr,info_ptr,width,height,bit_depth,PNG_COLOR_TYPE_RGB_ALPHA,
        PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_BASE,PNG_FILTER_TYPE_BASE);
    png_colorp palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
    if (!palette)
    {
        fclose(png_file);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return -1;
    }
    png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);
    png_write_info(png_ptr, info_ptr);
    png_set_packing(png_ptr);
    png_bytepp rows = (png_bytepp)png_malloc(png_ptr, height * sizeof(png_bytep));

    for (int i = 0; i < height; ++i)
    {
        rows[i] = (png_bytep)(pixels + (i) * width * 4);
    }
    png_write_image(png_ptr, rows);
    delete[] rows;
    png_write_end(png_ptr, info_ptr);
    png_free(png_ptr, palette);
    palette=NULL;
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(png_file);
    return 0;
}