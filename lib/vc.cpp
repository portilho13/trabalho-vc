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
#include <math.h>
#include "stdlib.h"

#if defined(WIN32)
    #include <malloc.h>
#else
    #include <stdlib.h>
#endif

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
			image = vc_image_new(width, height, channels, levels + 1);
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

int vc_rgb_to_gray(IVC* src, IVC* dst) {
	for (int y = 0; y < src->height; y++) {
		for (int x = 0; x < src->width; x++) {
			int pos_src = y * src->bytesperline + x * src->channels;
			int pos_dst = y * dst->bytesperline + x * dst->channels;

			float r = (float)src->data[pos_src];
			float g = (float)src->data[pos_src + 1];
			float b = (float)src->data[pos_src + 2];

			unsigned char gray = (unsigned char)((r * 0.299f) + (g * 0.587f) + (b * 0.114f));

			dst->data[pos_dst] = gray;
		}
	}

	return 0;
}

#define THRESHOLDING 110
#define RED 0
#define GREEN 1
#define BLUE 2

int vc_gray_to_bin(IVC* src, IVC* dst) {
	if (src->channels != 1 || dst->channels != 3) return -1;

	
	memset(dst->data, 0, dst->height * dst->bytesperline);

	for (int y = 0; y < src->height; y++) {
		for (int x = 0; x < src->width; x++) {
			int pos_src = y * src->bytesperline + x * src->channels;
			int pos_dst = y * dst->bytesperline + x * dst->channels;

			unsigned char gray = src->data[pos_src];
			
			if (gray <= THRESHOLDING) {
				dst->data[pos_dst] = 255;
				dst->data[pos_dst + GREEN] = 255;
				dst->data[pos_dst + BLUE] = 255;
			} else {
				dst->data[pos_dst] = 0;
				dst->data[pos_dst + GREEN] = 0;
				dst->data[pos_dst + BLUE] = 0;
			}

		}
	}
	return 0;
}

int vc_binary_dilate(IVC* src, IVC* dst, int kernel) {
	if (!src || !dst || src->channels != 3 || dst->channels != 3) return -1;

	memset(dst->data, 0, src->height * src->bytesperline);
	int offset = (kernel - 1) / 2;

	for (int y = offset; y < src->height - offset; y++) {
		for (int x = offset; x < src->width - offset; x++) {
			int foundWhite = 0;

			for (int ky = -offset; ky <= offset && !foundWhite; ky++) {
				for (int kx = -offset; kx <= offset; kx++) {
					int kpos = (y + ky) * src->bytesperline + (x + kx) * src->channels;

					if (src->data[kpos] == 255 && src->data[kpos + 1] == 255 && src->data[kpos + 2] == 255) {
						foundWhite = 1;
						break;
					}
				}
			}

			if (foundWhite) {
				int pos = y * dst->bytesperline + x * dst->channels;
				dst->data[pos + RED] = 255;
				dst->data[pos + GREEN] = 255;
				dst->data[pos + BLUE] = 255;
			}
		}
	}

	return 0;
}



int vc_binary_erode(IVC* src, IVC* dst, int kernel) {
	if (!src || !dst || src->channels != 3 || dst->channels != 3) return -1;

	memset(dst->data, 0, src->height * src->bytesperline);
	int offset = (kernel - 1) / 2;

	for (int y = offset; y < src->height - offset; y++) {
		for (int x = offset; x < src->width - offset; x++) {
			int allWhite = 1;

			for (int ky = -offset; ky <= offset && allWhite; ky++) {
				for (int kx = -offset; kx <= offset; kx++) {
					int kpos = (y + ky) * src->bytesperline + (x + kx) * src->channels;

					if (!(src->data[kpos] == 255 && src->data[kpos + 1] == 255 && src->data[kpos + 2] == 255)) {
						allWhite = 0;
						break;
					}
				}
			}

			if (allWhite) {
				int pos = y * dst->bytesperline + x * dst->channels;
				dst->data[pos + RED] = 255;
				dst->data[pos + GREEN] = 255;
				dst->data[pos + BLUE] = 255;
			}
		}
	}

	return 0;
}



int vc_opening(IVC* src, IVC* dst, int kernel) {
	IVC* temp = vc_image_new(src->width, src->height, src->channels, src->levels);

	vc_binary_erode(src, temp, kernel);
	
	vc_binary_dilate(temp, dst, kernel);


	return 0;
}

int vc_closing(IVC* src, IVC* dst, int kernel) {
	IVC* temp = vc_image_new(src->width, src->height, src->channels, src->levels);

	vc_binary_dilate(src, temp, kernel);

	vc_binary_erode(temp, dst, kernel);

	return 0;
}


int vc_rgb_to_hsv(IVC* src, IVC* dst) {
	if (src == NULL || dst == NULL) return 0;
	if (src->width != dst->width || src->height != dst->height) return 0;
	if (src->channels != 3 || dst->channels != 3) return 0;

	for (int y = 0; y < src->height; y++) {
		for (int x = 0; x < src->width; x++) {
			int pos_src = y * src->bytesperline + x * src->channels;
			int pos_dst = y * dst->bytesperline + x * dst->channels;

			// Normalize RGB values to [0,1]
			float r = src->data[pos_src + RED] / 255.0f;
			float g = src->data[pos_src + GREEN] / 255.0f;
			float b = src->data[pos_src + BLUE] / 255.0f;

			float maxVal = fmaxf(fmaxf(r, g), b);
			float minVal = fminf(fminf(r, g), b);
			float delta = maxVal - minVal;

			float h, s, v;

			// Calculate Hue
			if (delta == 0)
				h = 0;
			else if (maxVal == r)
				h = 60 * fmodf(((g - b) / delta), 6);
			else if (maxVal == g)
				h = 60 * (((b - r) / delta) + 2);
			else
				h = 60 * (((r - g) / delta) + 4);

			if (h < 0)
				h += 360;

			// Calculate Saturation
			if (maxVal == 0)
				s = 0;
			else
				s = delta / maxVal;

			v = maxVal;

			dst->data[pos_dst + RED] = (unsigned char)(h / 360.0f * 255.0f);
			dst->data[pos_dst + GREEN] = (unsigned char)(s * 255.0f);
			dst->data[pos_dst + BLUE] = (unsigned char)(v * 255.0f);
		}
	}
	return 1;
}

int vc_hsv_to_bin(IVC* src, IVC* dst, int h_min, int h_max) {
	if (src == NULL || dst == NULL) return 0;
	if (src->width != dst->width || src->height != dst->height) return 0;
	if (src->channels != 3 || dst->channels != 1) return 0;

	for (int y = 0; y < src->height; y++) {
		for (int x = 0; x < src->width; x++) {
			int pos_src = y * src->bytesperline + x * src->channels;
			int pos_dst = y * dst->bytesperline + x * dst->channels;

			unsigned char h = src->data[pos_src + 0]; // Hue in channel 0

			if (h_min <= h_max) {
				if (h >= h_min && h <= h_max) {
					dst->data[pos_dst] = 1;
				}
				else {
					dst->data[pos_dst] = 0;
				}
			}
			else {
				if (h >= h_min || h <= h_max) {
					dst->data[pos_dst] = 1;
				}
				else {
					dst->data[pos_dst] = 0;
				}
			}
		}
	}

	return 1;  // Return 1 for success
}

int vc_hsv_to_bin_extended(IVC* src, IVC* dst,
	int h_min, int h_max,
	int s_min, int s_max,
	int v_min, int v_max) {
	if (src == NULL || dst == NULL) return 0;
	if (src->width != dst->width || src->height != dst->height) return 0;
	if (src->channels != 3 || dst->channels != 3) return 0;

	for (int y = 0; y < src->height; y++) {
		for (int x = 0; x < src->width; x++) {
			int pos_src = y * src->bytesperline + x * src->channels;
			int pos_dst = y * dst->bytesperline + x * dst->channels;

			unsigned char h = src->data[pos_src + 0]; // Hue
			unsigned char s = src->data[pos_src + 1]; // Saturation
			unsigned char v = src->data[pos_src + 2]; // Value

			bool h_in_range = (h_min <= h_max) ?
				(h >= h_min && h <= h_max) :
				(h >= h_min || h <= h_max);

			bool s_in_range = (s >= s_min && s <= s_max);
			bool v_in_range = (v >= v_min && v <= v_max);

			if (h_in_range && s_in_range && v_in_range) {
				dst->data[pos_dst] = 255;
				dst->data[pos_dst + GREEN] = 255;
				dst->data[pos_dst + BLUE] = 255;

			}
			else {
				dst->data[pos_dst] = 0;
				dst->data[pos_dst + GREEN] = 0;
				dst->data[pos_dst + BLUE] = 0;
			}
		}
	}

	return 1;
}

int diff_bin_images(IVC* src1, IVC* src2, IVC* dst) {
	if (src1 == NULL || src2 == NULL || dst == NULL) return 0;
	if (src1->width != src2->width || src1->width != dst->width ||
		src1->height != src2->height || src1->height != dst->height) return 0;
	if (src1->channels != src2->channels || src1->channels != dst->channels) return 0;

	for (int y = 0; y < src1->height; y++) {
		for (int x = 0; x < src1->width; x++) {
			int pos = y * src1->bytesperline + x * src1->channels;

			if (src1->data[pos] != src2->data[pos]) {
				dst->data[pos] = 1;
			}
			else {
				dst->data[pos] = 0;
			}
		}
	}

	return 1;
}
unsigned char min4(unsigned char a, unsigned char b, unsigned char c, unsigned char d) {
	unsigned char min = 255;

	if (a != 0 && a < min) min = a;
	if (b != 0 && b < min) min = b;
	if (c != 0 && c < min) min = c;
	if (d != 0 && d < min) min = d;

	return (min == 255) ? 0 : -1; // All are 0
}

int find(int* parent, int x) {
    if (parent[x] != x)
        parent[x] = find(parent, parent[x]); // Path compression
    return parent[x];
}

void union_sets(int* parent, int a, int b) {
    int rootA = find(parent, a);
    int rootB = find(parent, b);
    if (rootA != rootB)
        parent[rootB] = rootA;
}

int vc_binary_blob_labelling(IVC* bin, IVC* rgb) {
    if (bin == NULL || rgb == NULL) return 0;
    if (bin->width != rgb->width || bin->height != rgb->height) return 0;
    if (rgb->channels != 3 || bin->channels != 3) return 0;

    int width = bin->width;
    int height = bin->height;
    int size = width * height;

    int* labels = (int*)calloc(size, sizeof(int));
    int* parent = (int*)malloc(sizeof(int) * (size / 2));
    if (!labels || !parent) return 0;

    int label = 1;
    for (int i = 0; i < (size / 2); i++) parent[i] = i;

    // First pass - 4-connectivity
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            int pos = y * bin->bytesperline + x * bin->channels;

            // Only consider white pixels as foreground
            if (bin->data[pos + 0] != 255 || bin->data[pos + 1] != 255 || bin->data[pos + 2] != 255)
                continue;

            int left = (x > 0) ? labels[idx - 1] : 0;
            int up   = (y > 0) ? labels[idx - width] : 0;

            if (left == 0 && up == 0) {
                labels[idx] = label++;
            } else if (left != 0 && up == 0) {
                labels[idx] = left;
            } else if (left == 0 && up != 0) {
                labels[idx] = up;
            } else {
                labels[idx] = (left < up) ? left : up;
                union_sets(parent, left, up);
            }
        }
    }

    // Second pass - resolve equivalences
    for (int i = 0; i < size; i++) {
        if (labels[i] != 0)
            labels[i] = find(parent, labels[i]);
    }

    // Relabeling to sequential values
    int* new_labels = (int*)calloc(label, sizeof(int));
    int new_label = 1;

    for (int i = 0; i < size; i++) {
        int lbl = labels[i];
        if (lbl != 0) {
            if (new_labels[lbl] == 0) new_labels[lbl] = new_label++;
            labels[i] = new_labels[lbl];
        }
    }

    // Prepare bounding boxes
    int* min_x = (int*)malloc(sizeof(int) * new_label);
    int* max_x = (int*)malloc(sizeof(int) * new_label);
    int* min_y = (int*)malloc(sizeof(int) * new_label);
    int* max_y = (int*)malloc(sizeof(int) * new_label);
    if (!min_x || !max_x || !min_y || !max_y) return 0;

    for (int i = 0; i < new_label; i++) {
        min_x[i] = width;  max_x[i] = 0;
        min_y[i] = height; max_y[i] = 0;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            int lbl = labels[idx];

            if (lbl > 0) {
                if (x < min_x[lbl]) min_x[lbl] = x;
                if (x > max_x[lbl]) max_x[lbl] = x;
                if (y < min_y[lbl]) min_y[lbl] = y;
                if (y > max_y[lbl]) max_y[lbl] = y;
            }
        }
    }

    // Draw boxes on RGB output
    for (int l = 1; l < new_label; l++) {
        int w = max_x[l] - min_x[l];
        int h = max_y[l] - min_y[l];

        if (w < 5 || h < 5) continue; // optional: skip small blobs

        for (int x = min_x[l]; x <= max_x[l]; x++) {
            int top    = min_y[l] * rgb->bytesperline + x * rgb->channels;
            int bottom = max_y[l] * rgb->bytesperline + x * rgb->channels;

            rgb->data[top + 0] = 255;
            rgb->data[top + 1] = 255;
            rgb->data[top + 2] = 255;

            rgb->data[bottom + 0] = 255;
            rgb->data[bottom + 1] = 255;
            rgb->data[bottom + 2] = 255;
        }
        for (int y = min_y[l]; y <= max_y[l]; y++) {
            int left  = y * rgb->bytesperline + min_x[l] * rgb->channels;
            int right = y * rgb->bytesperline + max_x[l] * rgb->channels;

            rgb->data[left + 0] = 255;
            rgb->data[left + 1] = 255;
            rgb->data[left + 2] = 255;

            rgb->data[right + 0] = 255;
            rgb->data[right + 1] = 255;
            rgb->data[right + 2] = 255;
        }
    }

    int object_count = new_label - 1;

    // Cleanup
    free(labels);
    free(parent);
    free(new_labels);
    free(min_x); free(max_x); free(min_y); free(max_y);

    return object_count;
}
