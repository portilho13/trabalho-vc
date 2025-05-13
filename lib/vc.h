//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2024/2025
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char* data;
	int width, height;
	int channels;			// Bin�rio/Cinzentos=1; RGB=3
	int levels;				// Bin�rio=2; Cinzentos [1,256]; RGB [1,256]
	int bytesperline;		// width * channels
} IVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC* vc_read_image(char* filename);
int vc_write_image(char* filename, IVC* image);

int vc_rgb_to_gray(IVC* src, IVC *dst);

int vc_gray_to_bin(IVC* src, IVC* dst);

int vc_binary_dilate(IVC* src, IVC* dst, int kernel);

int vc_binary_erode(IVC* src, IVC* dst, int kernel);

int vc_opening(IVC* src, IVC* dst, int kernel);

int vc_closing(IVC* src, IVC* dst, int kernel);

unsigned char max(unsigned char a, unsigned char b, unsigned char c);

int vc_rgb_to_hsv(IVC* src, IVC* dst);

unsigned char min(unsigned char a, unsigned char b, unsigned char c);

float calculate_hue(float r, float g, float b, float max, float min);

int vc_hsv_to_bin(IVC* src, IVC* dst, int h_min, int h_max);

int vc_hsv_to_bin_extended(IVC* src, IVC* dst,
	int h_min, int h_max,
	int s_min, int s_max,
	int v_min, int v_max);

int diff_bin_images(IVC* src1, IVC* src2, IVC* dst);

int vc_binary_blob_labelling(IVC* src, IVC* dst);
