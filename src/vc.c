//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2024/2025
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "vc.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem�ria para uma imagem
IVC* vc_image_new(int width, int height, int channels, int levels)
{
	IVC* image = (IVC*)malloc(sizeof(IVC));

	if (image == NULL) return NULL;
	if ((levels <= 0) || (levels > 256)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char*)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar mem�ria de uma imagem
IVC* vc_image_free(IVC* image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char* netpbm_get_token(FILE* file, char* tok, int len)
{
	char* t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)));
		if (c != '#') break;
		do c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF) break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#') ungetc(c, file);
	}

	*t = 0;

	return tok;
}


long int unsigned_char_to_bit(unsigned char* datauchar, unsigned char* databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char* p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char* databit, unsigned char* datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char* p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}


IVC* vc_read_image(char* filename)
{
	FILE* file = NULL;
	IVC* image = NULL;
	unsigned char* tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 256;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0) { channels = 1; levels = 2; }	// Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 2) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 256)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels+1);
			if (image == NULL) return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}


int vc_write_image(char* filename, IVC* image)
{
	FILE* file = NULL;
	unsigned char* tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL) return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 2)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

			fprintf(file, "%s \n%d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s \n%d %d \n%d\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height, image->levels - 1);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}

#define RED 0
#define GREEN 1
#define BLUE 2

#define RED 0
#define GREEN 1
#define BLUE 2

int calculate_max(unsigned int r, unsigned int g, unsigned int b) {
    return (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
}

int calculate_min(unsigned int r, unsigned int g, unsigned int b) {
    return (r < g) ? ((r < b) ? r : b) : ((g < b) ? g : b);
}

float calculate_hue(float r, float g, float b, float max, float min) {
    float hue = 0.0;
    if (max == min) return 0.0; // Undefined hue for grayscale colors

    if (max == r) {
        hue = 60.0 * ((g - b) / (max - min));
        if (hue < 0) hue += 360.0; // Ensure positive hue
    } else if (max == g) {
        hue = 120.0 + 60.0 * ((b - r) / (max - min));
    } else {
        hue = 240.0 + 60.0 * ((r - g) / (max - min));
    }

    return hue;
}

int vc_rgb_to_hsv(IVC *src, IVC *dest) {
    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            int pos = y * src->bytesperline + x * src->channels;

            unsigned char r = src->data[pos + RED];
            unsigned char g = src->data[pos + GREEN];
            unsigned char b = src->data[pos + BLUE];

            int max = calculate_max(r, g, b);
            int min = calculate_min(r, g, b);

            float max_f = max / 255.0;
            float min_f = min / 255.0;
            float r_f = r / 255.0;
            float g_f = g / 255.0;
            float b_f = b / 255.0;

            float hue = calculate_hue(r_f, g_f, b_f, max_f, min_f);
            float sat = (max == 0) ? 0 : (max - min) / (float) max;
            float val = max_f;

            dest->data[pos] = (unsigned char)(hue / 360.0 * 255.0);
            dest->data[pos + 1] = (unsigned char)(sat * 255.0);
            dest->data[pos + 2] = (unsigned char)(val * 255.0);
        }
    }
    return 0;
}

#define POS_DECLIVE 4
#define NEG_DECLIVE -4


float transform_red(float val) {
	if (val < 127.5) {
		return 0.0;
	} else if (val > 191.25) {
		return 255.0;
	} else {
		return (val * POS_DECLIVE - 510);
	}
	
	return 0.0;
}

float transform_blue(float val) {
	if (val < 63.75) {
		return 255.0;
	} else if (val > 127.5) {
		return 0.0;
	} else {
		return (val * NEG_DECLIVE + 510);
	}
	
	return 0.0;
}

float transform_green(float val) {
	if (val < 63.75) {
		return (val * POS_DECLIVE);
	} else if (val > 191.25) {
		return (val * NEG_DECLIVE + 1020);
	} else {
		return 255.0;
	}
	
	return 0.0;
}

int vc_scale_gray_to_color_palette(IVC *src, IVC *dst) {
	for (int y = 0; y < src->height; y++) {
		for(int x = 0; x < src->width; x++) {
			int pos = y * src->bytesperline + x * src->channels;
			int pos2 = y * dst->bytesperline + x * dst->channels;
			float data = (float)src->data[pos];
			float r = transform_red(data);
			float g = transform_green(data);
			float b = transform_blue(data);

			printf("Data: %f R %f G %f B %f\n", data, r, g, b);

			dst->data[pos2 + RED] = r;
			dst->data[pos2 + GREEN] = g;
			dst->data[pos2 + BLUE] = b;
		}
	}

	return 0;
}

int vc_grey_to_binary(IVC *src, IVC *dst, int threshold) {
    for (int y = 0; y < src->height; y++) {
        for(int x = 0; x < src->width; x++) {
            int pos_src = y * src->bytesperline + x * src->channels;
            int pos_dst = y * dst->bytesperline + x * dst->channels;

            if (src->data[pos_src] <= threshold) { // Change this if wrong
				dst->data[pos_dst] = 255;
			} else {
				dst->data[pos_dst] = 0;
			}
        }
    }

    return 0;
}


int vc_gray_to_binary_global_mean(IVC *src, IVC *dst) {
	long int pix_int = 0;
	for (int y = 0; y < src->height; y++) {
		for(int x = 0; x < src->width; x++) {
			int pos = y * src->bytesperline + x * src->channels;
			pix_int += src->data[pos];
		}
	}
	float threshold = (float)(pix_int / (src->height * src->width));
	for (int y = 0; y < src->height; y++) {
		for(int x = 0; x < src->width; x++) {
			int pos = y * src->bytesperline + x * src->channels;
	
			if (src->data[pos] >= threshold) {
				dst->data[pos] = 0;
			} else {
				dst->data[pos] = 1;
			}
		}
	}
	
	return 0;
}

int vc_binary_dilate(IVC* src, IVC* dst, int kernel) {
    int offSet = (kernel - 1) / 2;
    for (int y = 0; y < src->height; y++) {
        for(int x = 0; x < src->width; x++) {
            int pos = y * src->bytesperline + x * src->channels;
            if (src->data[pos] == 255) {
                dst->data[pos] = 255;
                continue;
            }
            int isWhite = 0;
            for (int kY = -offSet; kY < offSet; kY++) {
                for (int kX = -offSet; kX < offSet; kX++) {
                    if((x + kX) >= 0 && (x + kX) < src->width && (y + kY) >= 0 && (y + kY) < src->height) { 
                        int kPos = (y + kY) * src->bytesperline + (x + kX) * src->channels;
                        if (src->data[kPos] == 255) {
                            isWhite = 1;
                        }
                    }
                }
            }

            if (isWhite == 1) {
                dst->data[pos] = 255;
            } else {
                dst->data[pos] = 0;
            }
        }
    }
    return 0;
}

int vc_binary_erosion(IVC* src, IVC* dst, int kernel) {
    int offSet = (kernel - 1) / 2;

    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            int pos = y * src->bytesperline + x * src->channels;

            int isWhite = 1; 
            for (int kY = -offSet; kY <= offSet; kY++) {
                for (int kX = -offSet; kX <= offSet; kX++) {
                    if ((x + kX) >= 0 && (x + kX) < src->width && (y + kY) >= 0 && (y + kY) < src->height) {
                        int kPos = (y + kY) * src->bytesperline + (x + kX) * src->channels;

                        if (src->data[kPos] != 255) {
                            isWhite = 0;
                            break; 
                        }
                    }
                }
                if (isWhite == 0) break;
            }

            if (isWhite == 1) {
                dst->data[pos] = 255;
            } else {
                dst->data[pos] = 0;
            }
        }
    }

    return 0;
}



int vc_rgb_to_gray(IVC *src, IVC* dst) {
	for (int y = 0; y < src->height; y++) {
		for(int x = 0; x < src->width; x++) {
			int pos_src = y * src->bytesperline + x * src->channels;
			int pos_dst = y * dst->bytesperline + x * dst->channels;

			float r = (float) src->data[pos_src];
			float g = (float) src->data[pos_src + 1];
			float b = (float) src->data[pos_src + 2];

			unsigned char gray = (unsigned char) ((r * 0.299f) + (g * 0.587f) + (b * 0.114f));

			dst->data[pos_dst] = gray;
            dst->data[pos_dst + 1] = gray;
            dst->data[pos_dst + 2] = gray;
		}
	}
	
	return 0;
}
