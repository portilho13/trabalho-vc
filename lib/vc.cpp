//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT CNICO DO C VADO E DO AVE
//                          2024/2025
//             ENGENHARIA DE SISTEMAS INFORM TICOS
//                    VIS O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun  es n o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "stdlib.h"
#include <opencv2/opencv.hpp>
#include <map>
#include "vc.h"


#if defined(WIN32)
#include <malloc.h>
#else
#include <stdlib.h>
#endif



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN  ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem ria para uma imagem
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


// Libertar mem ria de uma imagem
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
//    FUN  ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
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

			// Aloca mem ria para imagem
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

			// Aloca mem ria para imagem
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

			float r = (float)src->data[pos_src + RED];
			float g = (float)src->data[pos_src + GREEN];
			float b = (float)src->data[pos_src + BLUE];

			unsigned char gray = (unsigned char)((r * 0.299f) + (g * 0.587f) + (b * 0.114f));

			dst->data[pos_dst] = gray;
		}
	}

	return 0;
}


int vc_gray_to_bin(IVC* src, IVC* dst) {
	if (src->channels != 1 || dst->channels != 3) return -1;


	memset(dst->data, 0, dst->height * dst->bytesperline);

	for (int y = 0; y < src->height; y++) {
		for (int x = 0; x < src->width; x++) {
			int pos_src = y * src->bytesperline + x * src->channels;
			int pos_dst = y * dst->bytesperline + x * dst->channels;

			unsigned char gray = src->data[pos_src];

			if (gray <= THRESHOLDING) {
				dst->data[pos_dst + RED] = 255;
				dst->data[pos_dst + GREEN] = 255;
				dst->data[pos_dst + BLUE] = 255;
			}
			else {
				dst->data[pos_dst + RED] = 0;
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
				dst->data[pos_dst + RED] = 255;
				dst->data[pos_dst + GREEN] = 255;
				dst->data[pos_dst + BLUE] = 255;

			}
			else {
				dst->data[pos_dst + RED] = 0;
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
				dst->data[pos + RED] = 255;
				dst->data[pos + GREEN] = 255;
				dst->data[pos + BLUE] = 255;

			}
			else {
				dst->data[pos + RED] = 0;
				dst->data[pos + GREEN] = 0;
				dst->data[pos + BLUE] = 0;
			}
		}
	}

	return 1;
}

int find(int* parent, int x) {
	if (parent[x] != x)
		parent[x] = find(parent, parent[x]);
	return parent[x];
}

void union_sets(int* parent, int a, int b) {
	int rootA = find(parent, a);
	int rootB = find(parent, b);
	if (rootA != rootB)
		parent[rootB] = rootA;
}

void compute_histogram(IVC* gray,
	int min_x, int max_x, int min_y, int max_y,
	int* hist)
{
	for (int i = 0; i < HIST_SIZE; ++i) hist[i] = 0;

	for (int y = min_y; y <= max_y; ++y)
		for (int x = min_x; x <= max_x; ++x) {
			int pos = y * gray->bytesperline + x * gray->channels;
			hist[gray->data[pos]]++;
		}
}

void compute_histogram_hsv(IVC* hsv,
	int min_x, int max_x, int min_y, int max_y,
	int* hist_h, int* hist_s, int* hist_v)
{
	for (int i = 0; i < HIST_SIZE; ++i) {
		hist_h[i] = hist_s[i] = hist_v[i] = 0;
	}

	for (int y = min_y; y <= max_y; ++y)
		for (int x = min_x; x <= max_x; ++x) {
			int pos = y * hsv->bytesperline + x * hsv->channels;
			unsigned char h = hsv->data[pos];
			unsigned char s = hsv->data[pos + 1];
			unsigned char v = hsv->data[pos + 2];

			hist_h[h]++;  hist_s[s]++;  hist_v[v]++;
		}
}

static inline float histogram_mean(const int* hist, int size)
{
	long sum = 0;  int total = 0;
	for (int i = 0; i < size; ++i) {
		sum += (long)i * hist[i];
		total += hist[i];
	}
	return (total == 0) ? 0.0f : (float)sum / (float)total;
}

Object* vc_binary_blob_labelling(
	IVC* bin,
	IVC* gray,
	IVC* hsv,
	int* n_objects)
{
	if (!bin || !gray || !hsv || !n_objects) return NULL;
	if (bin->width != gray->width || bin->height != gray->height) return NULL;
	if (bin->width != hsv->width || bin->height != hsv->height) return NULL;
	if (bin->channels != 3 || gray->channels != 1 || hsv->channels != 3) return NULL;

	const int width = bin->width;
	const int height = bin->height;
	const int size = width * height;

	int* labels = (int*)calloc(size, sizeof(int));
	int* parent = (int*)malloc((size / 2) * sizeof(int));
	if (!labels || !parent) {
		free(labels); free(parent);
		return NULL;
	}
	for (int i = 0; i < size / 2; ++i) parent[i] = i;

	int next_label = 1;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int idx = y * width + x;
			int pos = y * bin->bytesperline + x * bin->channels;

			if (bin->data[pos] != 255 || bin->data[pos + 1] != 255 || bin->data[pos + 2] != 255)
				continue;

			int l = (x > 0) ? labels[idx - 1] : 0;
			int u = (y > 0) ? labels[idx - width] : 0;

			if (!l && !u) labels[idx] = next_label++;
			else if (l && !u) labels[idx] = l;
			else if (!l && u) labels[idx] = u;
			else {
				labels[idx] = (l < u) ? l : u;
				int a = l, b = u;
				while (parent[a] != a) a = parent[a];
				while (parent[b] != b) b = parent[b];
				if (a != b) parent[b] = a;
			}
		}
	}

	// Resolve equivalences
	for (int i = 0; i < size; ++i) {
		if (labels[i]) {
			int r = labels[i];
			while (parent[r] != r) r = parent[r];
			labels[i] = r;
		}
	}

	// Remap labels to contiguous numbering
	int* map = (int*)calloc(next_label, sizeof(int));
	int new_label = 1;
	for (int i = 0; i < size; ++i)
	{
		if (labels[i]) {
			int r = labels[i];
			if (!map[r]) map[r] = new_label++;
			labels[i] = map[r];
		}
	}
	int nlab = new_label;

	int* min_x = (int*)malloc(nlab * sizeof(int));
	int* max_x = (int*)malloc(nlab * sizeof(int));
	int* min_y = (int*)malloc(nlab * sizeof(int));
	int* max_y = (int*)malloc(nlab * sizeof(int));
	int* area = (int*)calloc(nlab, sizeof(int));
	int* sx = (int*)calloc(nlab, sizeof(int));
	int* sy = (int*)calloc(nlab, sizeof(int));

	if (!min_x || !max_x || !min_y || !max_y || !area || !sx || !sy) {
		free(labels); free(parent); free(map);
		free(min_x); free(max_x); free(min_y); free(max_y);
		free(area); free(sx); free(sy);
		return NULL;
	}

	for (int i = 0; i < nlab; ++i) {
		min_x[i] = width; min_y[i] = height; max_x[i] = 0; max_y[i] = 0;
	}

	// Calculate bounding boxes and area sums
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			int idx = y * width + x;
			int l = labels[idx];
			if (!l) continue;
			area[l]++;
			sx[l] += x; sy[l] += y;
			if (x < min_x[l]) min_x[l] = x;
			if (x > max_x[l]) max_x[l] = x;
			if (y < min_y[l]) min_y[l] = y;
			if (y > max_y[l]) max_y[l] = y;
		}
	}

	Object* obj = (Object*)malloc((nlab - 1) * sizeof(Object));
	int count = 0;

	int hist_g[HIST_SIZE];
	int hist_h[HIST_SIZE], hist_s[HIST_SIZE], hist_v[HIST_SIZE];
	static unsigned long long uid_counter = 0;

	for (int l = 1; l < nlab; ++l)
	{
		// Remove Noise Objects
		if (area[l] < 4000) continue;
		int w = max_x[l] - min_x[l];
		int h = max_y[l] - min_y[l];
		if (w < 10 || h < 10) continue;

		int* border_idx = (int*)malloc(area[l] * sizeof(int));
		if (!border_idx) continue;
		int border_count = 0;

		for (int y = min_y[l]; y <= max_y[l]; ++y)
		{
			for (int x = min_x[l]; x <= max_x[l]; ++x)
			{
				int idx = y * width + x;
				if (labels[idx] != l) continue;
				if (x == 0 || labels[idx - 1] != l ||
					x == width - 1 || labels[idx + 1] != l ||
					y == 0 || labels[idx - width] != l ||
					y == height - 1 || labels[idx + width] != l)
				{
					border_idx[border_count++] = idx;
				}
			}
		}

		int per = border_count;
		float circ = (per > 0) ? (4.0f * PI * area[l]) / (per * per) : 0.0f;
		float cx = (float)sx[l] / area[l];
		float cy = (float)sy[l] / area[l];
		float diam = sqrtf((4.0f * area[l]) / PI);

		compute_histogram(gray, min_x[l], max_x[l], min_y[l], max_y[l], hist_g);
		float m_gray = histogram_mean(hist_g, HIST_SIZE);

		compute_histogram_hsv(hsv, min_x[l], max_x[l], min_y[l], max_y[l], hist_h, hist_s, hist_v);
		float m_hue = histogram_mean(hist_h, HIST_SIZE);
		float m_sat = histogram_mean(hist_s, HIST_SIZE);
		float m_val = histogram_mean(hist_v, HIST_SIZE);

		Object* o = &obj[count++];
		o->label = l;
		o->area = area[l];
		o->perimeter = per;
		o->id = ++uid_counter;
		o->circularity = circ;
		o->xc = (int)(cx + 0.5f);
		o->yc = (int)(cy + 0.5f);
		o->diameter = diam;
		o->mean_gray = m_gray;
		o->mean_hue = m_hue;
		o->mean_sat = m_sat;
		o->mean_val = m_val;
		o->min_x = min_x[l]; o->max_x = max_x[l];
		o->min_y = min_y[l]; o->max_y = max_y[l];
		o->type = COIN_TYPE::UNKNOWN;
		o->border_idx = border_idx;
		o->n_border_idx = border_count;

		if (o->area >= TWO_EURO_MIN_AREA && o->area <= TWO_EURO_MAX_AREA &&
			o->mean_gray >= TWO_EURO_MIN_GRAY && o->mean_gray <= TWO_EURO_MAX_GRAY &&
			o->mean_hue >= TWO_EURO_MIN_HUE && o->mean_hue <= TWO_EURO_MAX_HUE &&
			o->mean_sat >= TWO_EURO_MIN_SAT && o->mean_sat <= TWO_EURO_MAX_SAT &&
			o->mean_val >= TWO_EURO_MIN_VAL && o->mean_val <= TWO_EURO_MAX_VAL &&
			o->diameter >= TWO_EURO_MIN_DIAMETER && o->diameter <= TWO_EURO_MAX_DIAMETER) {
			o->type = COIN_TYPE::TWO_EUROS;
		}
		else if (o->area >= ONE_EURO_MIN_AREA && o->area <= ONE_EURO_MAX_AREA &&
			o->mean_gray >= ONE_EURO_MIN_GRAY && o->mean_gray <= ONE_EURO_MAX_GRAY &&
			o->mean_hue >= ONE_EURO_MIN_HUE && o->mean_hue <= ONE_EURO_MAX_HUE &&
			o->mean_sat >= ONE_EURO_MIN_SAT && o->mean_sat <= ONE_EURO_MAX_SAT &&
			o->mean_val >= ONE_EURO_MIN_VAL && o->mean_val <= ONE_EURO_MAX_VAL &&
			o->diameter >= ONE_EURO_MIN_DIAMETER && o->diameter <= ONE_EURO_MAX_DIAMETER) {
			o->type = COIN_TYPE::ONE_EURO;
		}
		else if (o->area >= FIFTY_CENT_MIN_AREA && o->area <= FIFTY_CENT_MAX_AREA &&
			o->mean_gray >= FIFTY_CENT_MIN_GRAY && o->mean_gray <= FIFTY_CENT_MAX_GRAY &&
			o->mean_hue >= FIFTY_CENT_MIN_HUE && o->mean_hue <= FIFTY_CENT_MAX_HUE &&
			o->mean_sat >= FIFTY_CENT_MIN_SAT && o->mean_sat <= FIFTY_CENT_MAX_SAT &&
			o->mean_val >= FIFTY_CENT_MIN_VAL && o->mean_val <= FIFTY_CENT_MAX_VAL &&
			o->diameter >= FIFTY_CENT_MIN_DIAMETER && o->diameter <= FIFTY_CENT_MAX_DIAMETER) {
			o->type = COIN_TYPE::FIFTY_CENTS;
		}
		else if (o->area >= TWENTY_CENT_MIN_AREA && o->area <= TWENTY_CENT_MAX_AREA &&
			o->mean_gray >= TWENTY_CENT_MIN_GRAY && o->mean_gray <= TWENTY_CENT_MAX_GRAY &&
			o->mean_hue >= TWENTY_CENT_MIN_HUE && o->mean_hue <= TWENTY_CENT_MAX_HUE &&
			o->mean_sat >= TWENTY_CENT_MIN_SAT && o->mean_sat <= TWENTY_CENT_MAX_SAT &&
			o->mean_val >= TWENTY_CENT_MIN_VAL && o->mean_val <= TWENTY_CENT_MAX_VAL &&
			o->diameter >= TWENTY_CENT_MIN_DIAMETER && o->diameter <= TWENTY_CENT_MAX_DIAMETER) {
			o->type = COIN_TYPE::TWENTY_CENTS;
		}
		else if (o->area >= TEN_CENT_MIN_AREA && o->area <= TEN_CENT_MAX_AREA &&
			o->mean_gray >= TEN_CENT_MIN_GRAY && o->mean_gray <= TEN_CENT_MAX_GRAY &&
			o->mean_hue >= TEN_CENT_MIN_HUE && o->mean_hue <= TEN_CENT_MAX_HUE &&
			o->mean_sat >= TEN_CENT_MIN_SAT && o->mean_sat <= TEN_CENT_MAX_SAT &&
			o->mean_val >= TEN_CENT_MIN_VAL && o->mean_val <= TEN_CENT_MAX_VAL &&
			o->diameter >= TEN_CENT_MIN_DIAMETER && o->diameter <= TEN_CENT_MAX_DIAMETER) {
			o->type = COIN_TYPE::TEN_CENTS;
		}
		else if (o->area >= FIVE_CENT_MIN_AREA && o->area <= FIVE_CENT_MAX_AREA &&
			o->mean_gray >= FIVE_CENT_MIN_GRAY && o->mean_gray <= FIVE_CENT_MAX_GRAY &&
			o->mean_hue >= FIVE_CENT_MIN_HUE && o->mean_hue <= FIVE_CENT_MAX_HUE &&
			o->mean_sat >= FIVE_CENT_MIN_SAT && o->mean_sat <= FIVE_CENT_MAX_SAT &&
			o->mean_val >= FIVE_CENT_MIN_VAL && o->mean_val <= FIVE_CENT_MAX_VAL &&
			o->diameter >= FIVE_CENT_MIN_DIAMETER && o->diameter <= FIVE_CENT_MAX_DIAMETER) {
			o->type = COIN_TYPE::FIVE_CENTS;
		}
		else if (o->area >= TWO_CENT_MIN_AREA && o->area <= TWO_CENT_MAX_AREA &&
			o->mean_gray >= TWO_CENT_MIN_GRAY && o->mean_gray <= TWO_CENT_MAX_GRAY &&
			o->mean_hue >= TWO_CENT_MIN_HUE && o->mean_hue <= TWO_CENT_MAX_HUE &&
			o->mean_sat >= TWO_CENT_MIN_SAT && o->mean_sat <= TWO_CENT_MAX_SAT &&
			o->mean_val >= TWO_CENT_MIN_VAL && o->mean_val <= TWO_CENT_MAX_VAL &&
			o->diameter >= TWO_CENT_MIN_DIAMETER && o->diameter <= TWO_CENT_MAX_DIAMETER) {
			o->type = COIN_TYPE::TWO_CENTS;
		}
		else if (o->area >= ONE_CENT_MIN_AREA && o->area <= ONE_CENT_MAX_AREA &&
			o->mean_gray >= ONE_CENT_MIN_GRAY && o->mean_gray <= ONE_CENT_MAX_GRAY &&
			o->mean_hue >= ONE_CENT_MIN_HUE && o->mean_hue <= ONE_CENT_MAX_HUE &&
			o->mean_sat >= ONE_CENT_MIN_SAT && o->mean_sat <= ONE_CENT_MAX_SAT &&
			o->mean_val >= ONE_CENT_MIN_VAL && o->mean_val <= ONE_CENT_MAX_VAL &&
			o->diameter >= ONE_CENT_MIN_DIAMETER && o->diameter <= ONE_CENT_MAX_DIAMETER) {
			o->type = COIN_TYPE::ONE_CENT;
		}

	}

	*n_objects = count;

	free(labels); free(parent); free(map);
	free(min_x); free(max_x); free(min_y); free(max_y);
	free(area); free(sx); free(sy);

	return obj;
}

cv::Mat IVCtoMat(IVC* img) {
	return cv::Mat(img->height, img->width, CV_8UC3, img->data, img->bytesperline);
}

int vc_draw_labels(IVC* dst, Object* objects, int n_objects) {
	cv::Mat matDst = IVCtoMat(dst);

	for (int i = 0; i < n_objects; i++) {
		Object* object = &objects[i];

		if (object->circularity >= 0.96f && object->circularity <= 1.28f) {
			if (DEBUG) {
				printf("Object UID %llu Label %d | Area: %d | Perimeter: %d | Diameter: %.2f | Circularity: %.3f | Centroid: (%d,%d) | Box: (%d,%d)–(%d,%d) | Mean Gray: %.2f | HSV: H=%.2f S=%.2f V=%.2f | Type: %s\n",
					object->id,
					object->label,
					object->area,
					object->perimeter,
					object->diameter,
					object->circularity,
					object->xc, object->yc,
					object->min_x, object->min_y, object->max_x, object->max_y,
					object->mean_gray,
					object->mean_hue, object->mean_sat, object->mean_val,
					coinTypeToString(object->type));
			}

			// Draw bounding box (white)
			for (int x = object->min_x; x <= object->max_x; x++) {
				if (x < 0 || x >= dst->width) continue;

				int top = object->min_y * dst->bytesperline + x * dst->channels;
				int bottom = object->max_y * dst->bytesperline + x * dst->channels;

				if (top >= 0 && bottom < dst->height * dst->bytesperline) {
					dst->data[top + RED] = 255;
					dst->data[top + GREEN] = 255;
					dst->data[top + BLUE] = 255;
					dst->data[bottom + RED] = 255;
					dst->data[bottom + GREEN] = 255;
					dst->data[bottom + BLUE] = 255;
				}
			}
			for (int y = object->min_y; y <= object->max_y; y++) {
				if (y < 0 || y >= dst->height) continue;

				int left = y * dst->bytesperline + object->min_x * dst->channels;
				int right = y * dst->bytesperline + object->max_x * dst->channels;

				if (left >= 0 && right < dst->height * dst->bytesperline) {
					dst->data[left + RED] = 255;
					dst->data[left + GREEN] = 255;
					dst->data[left + BLUE] = 255;
					dst->data[right + RED] = 255;
					dst->data[right + GREEN] = 255;
					dst->data[right + BLUE] = 255;
				}
			}

			// Draw border pixels in red
			for (int b = 0; b < object->n_border_idx; b++) {
				int idx = object->border_idx[b];
				int pixelPos = idx * dst->channels;
				if (pixelPos >= 0 && pixelPos + 2 < dst->width * dst->height * dst->channels) {
					dst->data[pixelPos + RED] = 0;
					dst->data[pixelPos + GREEN] = 0;
					dst->data[pixelPos + BLUE] = 255;
				}
			}

			// Draw centroid cross (white)
			int cx = object->xc;
			int cy = object->yc;
			int size = 3;
			for (int dy = -size; dy <= size; dy++) {
				int y = cy + dy;
				if (y >= 0 && y < dst->height) {
					int pos = y * dst->bytesperline + cx * dst->channels;
					dst->data[pos + RED] = 255;
					dst->data[pos + GREEN] = 255;
					dst->data[pos + BLUE] = 255;
				}
			}
			for (int dx = -size; dx <= size; dx++) {
				int x = cx + dx;
				if (x >= 0 && x < dst->width) {
					int pos = cy * dst->bytesperline + x * dst->channels;
					dst->data[pos + RED] = 255;
					dst->data[pos + GREEN] = 255;
					dst->data[pos + BLUE] = 255;
				}
			}

			// Draw text labels (type, id, and metrics)
			std::string typeText = coinTypeToString(object->type);
			std::string idText = "ID: " + std::to_string(object->label);
			std::string areaText = "A: " + std::to_string(object->area);
			std::string periText = "P: " + std::to_string(object->perimeter);
			std::string diaText = "D: " + std::to_string((int)object->diameter);
			std::string circText = "C: " + std::to_string(object->circularity).substr(0, 5);

			int fontFace = cv::FONT_HERSHEY_SIMPLEX;
			double fontScale = 0.4;
			int thickness = 1;

			int base_x = object->max_x + 5;
			int base_y = object->min_y;
			if (base_y < 10) base_y = object->min_y + 15;

			cv::putText(matDst, typeText, cv::Point(base_x, base_y),
				fontFace, fontScale, cv::Scalar(255, 255, 255), thickness);
			cv::putText(matDst, idText, cv::Point(base_x, base_y + 12),
				fontFace, fontScale, cv::Scalar(0, 255, 0), thickness);
			cv::putText(matDst, areaText, cv::Point(base_x, base_y + 24),
				fontFace, fontScale, cv::Scalar(255, 255, 255), thickness);
			cv::putText(matDst, periText, cv::Point(base_x, base_y + 36),
				fontFace, fontScale, cv::Scalar(255, 255, 255), thickness);
			cv::putText(matDst, diaText, cv::Point(base_x, base_y + 48),
				fontFace, fontScale, cv::Scalar(255, 255, 255), thickness);
			cv::putText(matDst, circText, cv::Point(base_x, base_y + 60),
				fontFace, fontScale, cv::Scalar(255, 255, 255), thickness);
		}
	}
	return 0;
}



void vc_track_coins(Object* currentFrameObjects, int numObjects, std::vector<TrackedCoin>& trackedCoins, int& nextCoinId) {

	for (auto& coin : trackedCoins) {
		coin.updated = false;
	}

	for (int i = 0; i < numObjects; i++) {
		Object& obj = currentFrameObjects[i];
		int currentXc = obj.xc;
		int currentYc = obj.yc;
		float currentArea = obj.area;

		bool matched = false;
		int bestMatchIdx = -1;
		float bestDistance = FLT_MAX;

		for (size_t j = 0; j < trackedCoins.size(); j++) {
			if (trackedCoins[j].updated) continue;

			// Calculate Euclidean distance between centers
			float dx = currentXc - trackedCoins[j].xc;
			float dy = currentYc - trackedCoins[j].yc;
			float distance = sqrt(dx * dx + dy * dy);

			// Area difference ratio
			float areaDiff = std::abs(currentArea - trackedCoins[j].area) / trackedCoins[j].area;

			// Dynamic distance threshold: half the current coin radius
			float radius = std::sqrt(trackedCoins[j].area / CV_PI);
			float maxDistanceThreshold = 0.6f * radius;

			if (distance < maxDistanceThreshold) {
				bestDistance = distance;
				bestMatchIdx = j;
				matched = true;
			}
		}

		if (matched) {
			// Track already Exists, Update It
			trackedCoins[bestMatchIdx].xc = currentXc;
			trackedCoins[bestMatchIdx].yc = currentYc;
			trackedCoins[bestMatchIdx].area = currentArea;
			trackedCoins[bestMatchIdx].updated = true;
			trackedCoins[bestMatchIdx].framesSinceLastSeen = 0;
			if (obj.min_y > 10) {
				trackedCoins[bestMatchIdx].type = obj.type;
			}
			trackedCoins[bestMatchIdx].min_y = obj.min_y;


			obj.label = trackedCoins[bestMatchIdx].id;
		}
		else {
			// Create new track
			TrackedCoin newCoin;
			newCoin.id = nextCoinId++;
			newCoin.xc = currentXc;
			newCoin.yc = currentYc;
			newCoin.area = currentArea;
			newCoin.updated = true;
			newCoin.framesSinceLastSeen = 0;
			newCoin.type = obj.type;
			newCoin.min_y = obj.min_y;
			trackedCoins.push_back(newCoin);

			obj.label = newCoin.id;
		}
	}
}


std::map<COIN_TYPE, int> countCoinsByType(const std::vector<TrackedCoin>& trackedCoins) {
	std::map<COIN_TYPE, int> counts;

	// Initialize counts to zero
	for (int t = ONE_CENT; t <= TWO_EUROS; t++) {
		counts[(COIN_TYPE)t] = 0;
	}
	counts[UNKNOWN] = 0;

	for (const auto& coin : trackedCoins) {
		counts[coin.type]++;
	}

	return counts;
}




const char* coinTypeToString(COIN_TYPE type) {
	switch (type) {
	case ONE_CENT:    return "UM CENTIMO";
	case TWO_CENTS:   return "DOIS CENTIMOS";
	case FIVE_CENTS:  return "CINCO CENTIMOS";
	case TEN_CENTS:   return "DEZ CENTIMOS";
	case TWENTY_CENTS:return "VINTE CENTIMOS";
	case FIFTY_CENTS: return "CINQUENTA CENTIMOS";
	case ONE_EURO:    return "UM EURO";
	case TWO_EUROS:   return "DOIS EUROS";
	default:         return "UNKNOWN";
	}
}