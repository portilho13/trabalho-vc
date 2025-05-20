//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2024/2025
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG

#include <opencv2/opencv.hpp>

#define DEBUG 0

#define VIDEO_FRAME_POS 0

#define VIDEO_ID 2

#define HIST_SIZE 256
#define PI        3.14159265358979323846

#if VIDEO_ID == 1
#define VIDEO "video1.mp4"
#define THRESHOLDING 105
#define OPENING_KERNEL_SIZE 11
#define CLOSING_KERNEL_SIZE 11

/* ---------- 1 ¢ ---------- */
#define ONE_CENT_MIN_AREA   8800
#define ONE_CENT_MAX_AREA   13000
#define ONE_CENT_MIN_GRAY   50
#define ONE_CENT_MAX_GRAY   90
#define ONE_CENT_MIN_HUE    135
#define ONE_CENT_MAX_HUE    152
#define ONE_CENT_MIN_SAT    90
#define ONE_CENT_MAX_SAT    138
#define ONE_CENT_MIN_VAL    60
#define ONE_CENT_MAX_VAL    115
#define ONE_CENT_MIN_DIAMETER 105
#define ONE_CENT_MAX_DIAMETER 123

/* ---------- 2 ¢ ---------- */
#define TWO_CENT_MIN_AREA   13200
#define TWO_CENT_MAX_AREA   15500
#define TWO_CENT_MIN_GRAY   50
#define TWO_CENT_MAX_GRAY   90
#define TWO_CENT_MIN_HUE    135
#define TWO_CENT_MAX_HUE    145
#define TWO_CENT_MIN_SAT    100
#define TWO_CENT_MAX_SAT    143
#define TWO_CENT_MIN_VAL    90
#define TWO_CENT_MAX_VAL    100
#define TWO_CENT_MIN_DIAMETER 121
#define TWO_CENT_MAX_DIAMETER 140

/* ---------- 5 ¢ ---------- */
#define FIVE_CENT_MIN_AREA  14000
#define FIVE_CENT_MAX_AREA  24000
#define FIVE_CENT_MIN_GRAY  68
#define FIVE_CENT_MAX_GRAY  78
#define FIVE_CENT_MIN_HUE   142
#define FIVE_CENT_MAX_HUE   147
#define FIVE_CENT_MIN_SAT   107
#define FIVE_CENT_MAX_SAT   126
#define FIVE_CENT_MIN_VAL   87
#define FIVE_CENT_MAX_VAL   100
#define FIVE_CENT_MIN_DIAMETER 141
#define FIVE_CENT_MAX_DIAMETER 160

/* ---------- 10 ¢ ---------- */
#define TEN_CENT_MIN_AREA   14900
#define TEN_CENT_MAX_AREA   16550
#define TEN_CENT_MIN_GRAY   97
#define TEN_CENT_MAX_GRAY   112
#define TEN_CENT_MIN_HUE    128
#define TEN_CENT_MAX_HUE    135
#define TEN_CENT_MIN_SAT    77
#define TEN_CENT_MAX_SAT    105
#define TEN_CENT_MIN_VAL    114
#define TEN_CENT_MAX_VAL    125
#define TEN_CENT_MIN_DIAMETER 135
#define TEN_CENT_MAX_DIAMETER 150

/* ---------- 20 ¢ ---------- */
#define TWENTY_CENT_MIN_AREA 12700
#define TWENTY_CENT_MAX_AREA 22000
#define TWENTY_CENT_MIN_GRAY 68
#define TWENTY_CENT_MAX_GRAY 92
#define TWENTY_CENT_MIN_HUE  128
#define TWENTY_CENT_MAX_HUE  135
#define TWENTY_CENT_MIN_SAT  85
#define TWENTY_CENT_MAX_SAT  126
#define TWENTY_CENT_MIN_VAL  80
#define TWENTY_CENT_MAX_VAL  115
#define TWENTY_CENT_MIN_DIAMETER 127
#define TWENTY_CENT_MAX_DIAMETER 170

/* ---------- 50 ¢ ---------- */
#define FIFTY_CENT_MIN_AREA 19800
#define FIFTY_CENT_MAX_AREA 25600
#define FIFTY_CENT_MIN_GRAY 95
#define FIFTY_CENT_MAX_GRAY 110
#define FIFTY_CENT_MIN_HUE  130
#define FIFTY_CENT_MAX_HUE  134.5
#define FIFTY_CENT_MIN_SAT  79
#define FIFTY_CENT_MAX_SAT  103
#define FIFTY_CENT_MIN_VAL  115
#define FIFTY_CENT_MAX_VAL  125
#define FIFTY_CENT_MIN_DIAMETER 173
#define FIFTY_CENT_MAX_DIAMETER 181


/* ---------- 1 € ---------- */
#define ONE_EURO_MIN_AREA   17700
#define ONE_EURO_MAX_AREA   22700
#define ONE_EURO_MIN_GRAY   66
#define ONE_EURO_MAX_GRAY   92
#define ONE_EURO_MIN_HUE    120
#define ONE_EURO_MAX_HUE    133
#define ONE_EURO_MIN_SAT    46
#define ONE_EURO_MAX_SAT    65
#define ONE_EURO_MIN_VAL    60
#define ONE_EURO_MAX_VAL    100
#define ONE_EURO_MIN_DIAMETER 150
#define ONE_EURO_MAX_DIAMETER 170

/* ---------- 2 € ---------- */
#define TWO_EURO_MIN_AREA   22500
#define TWO_EURO_MAX_AREA   29000
#define TWO_EURO_MIN_GRAY   88
#define TWO_EURO_MAX_GRAY   99
#define TWO_EURO_MIN_HUE    115
#define TWO_EURO_MAX_HUE    130
#define TWO_EURO_MIN_SAT    39
#define TWO_EURO_MAX_SAT    66
#define TWO_EURO_MIN_VAL    95
#define TWO_EURO_MAX_VAL    105
#define TWO_EURO_MIN_DIAMETER 169
#define TWO_EURO_MAX_DIAMETER 190



#elif VIDEO_ID == 2
#define VIDEO "video2.mp4"
#define THRESHOLDING 120

#define OPENING_KERNEL_SIZE 11
#define CLOSING_KERNEL_SIZE 11
/* ---------- 1 ¢ ---------- */
#define ONE_CENT_MIN_AREA   8800
#define ONE_CENT_MAX_AREA   12000
#define ONE_CENT_MIN_GRAY   50
#define ONE_CENT_MAX_GRAY   95
#define ONE_CENT_MIN_HUE    127
#define ONE_CENT_MAX_HUE    152
#define ONE_CENT_MIN_SAT    75
#define ONE_CENT_MAX_SAT    142
#define ONE_CENT_MIN_VAL    60
#define ONE_CENT_MAX_VAL    119
#define ONE_CENT_MIN_DIAMETER 105
#define ONE_CENT_MAX_DIAMETER 125

/* ---------- 2 ¢ ---------- */
#define TWO_CENT_MIN_AREA   12001
#define TWO_CENT_MAX_AREA   15500
#define TWO_CENT_MIN_GRAY   67
#define TWO_CENT_MAX_GRAY   85
#define TWO_CENT_MIN_HUE    134
#define TWO_CENT_MAX_HUE    145
#define TWO_CENT_MIN_SAT    112
#define TWO_CENT_MAX_SAT    134
#define TWO_CENT_MIN_VAL    88
#define TWO_CENT_MAX_VAL    107
#define TWO_CENT_MIN_DIAMETER 123
#define TWO_CENT_MAX_DIAMETER 147

/* ---------- 5 ¢ ---------- */
#define FIVE_CENT_MIN_AREA  15500
#define FIVE_CENT_MAX_AREA  19850
#define FIVE_CENT_MIN_GRAY  62
#define FIVE_CENT_MAX_GRAY  95
#define FIVE_CENT_MIN_HUE   132
#define FIVE_CENT_MAX_HUE   147
#define FIVE_CENT_MIN_SAT   97
#define FIVE_CENT_MAX_SAT   149
#define FIVE_CENT_MIN_VAL   87
#define FIVE_CENT_MAX_VAL   117
#define FIVE_CENT_MIN_DIAMETER 145
#define FIVE_CENT_MAX_DIAMETER 165

/* ---------- 10 ¢ ---------- */
#define TEN_CENT_MIN_AREA   14900
#define TEN_CENT_MAX_AREA   17500
#define TEN_CENT_MIN_GRAY   72
#define TEN_CENT_MAX_GRAY   120
#define TEN_CENT_MIN_HUE    122
#define TEN_CENT_MAX_HUE    140
#define TEN_CENT_MIN_SAT    75
#define TEN_CENT_MAX_SAT    122
#define TEN_CENT_MIN_VAL    90
#define TEN_CENT_MAX_VAL    135
#define TEN_CENT_MIN_DIAMETER 133
#define TEN_CENT_MAX_DIAMETER 150

/* ---------- 20 ¢ ---------- */
#define TWENTY_CENT_MIN_AREA 17000
#define TWENTY_CENT_MAX_AREA 22400
#define TWENTY_CENT_MIN_GRAY 68
#define TWENTY_CENT_MAX_GRAY 130
#define TWENTY_CENT_MIN_HUE  114
#define TWENTY_CENT_MAX_HUE  134
#define TWENTY_CENT_MIN_SAT  74
#define TWENTY_CENT_MAX_SAT  129
#define TWENTY_CENT_MIN_VAL  80
#define TWENTY_CENT_MAX_VAL  145
#define TWENTY_CENT_MIN_DIAMETER 150
#define TWENTY_CENT_MAX_DIAMETER 170

/* ---------- 50 ¢ ---------- */
#define FIFTY_CENT_MIN_AREA      22401
#define FIFTY_CENT_MAX_AREA      25300
#define FIFTY_CENT_MIN_GRAY      88
#define FIFTY_CENT_MAX_GRAY      97
#define FIFTY_CENT_MIN_HUE       124.0
#define FIFTY_CENT_MAX_HUE       127.6
#define FIFTY_CENT_MIN_SAT       90
#define FIFTY_CENT_MAX_SAT       113
#define FIFTY_CENT_MIN_VAL       106
#define FIFTY_CENT_MAX_VAL       112
#define FIFTY_CENT_MIN_DIAMETER  173
#define FIFTY_CENT_MAX_DIAMETER  179.3

/* ---------- 1 € ---------- */
#define ONE_EURO_MIN_AREA   17700
#define ONE_EURO_MAX_AREA   24500
#define ONE_EURO_MIN_GRAY   66
#define ONE_EURO_MAX_GRAY   120
#define ONE_EURO_MIN_HUE    90
#define ONE_EURO_MAX_HUE    125
#define ONE_EURO_MIN_SAT    30
#define ONE_EURO_MAX_SAT    60
#define ONE_EURO_MIN_VAL    100
#define ONE_EURO_MAX_VAL    130
#define ONE_EURO_MIN_DIAMETER 150
#define ONE_EURO_MAX_DIAMETER 175

/* ---------- 2 € ---------- */
#define TWO_EURO_MIN_AREA     25201
#define TWO_EURO_MAX_AREA     28600
#define TWO_EURO_MIN_GRAY     94
#define TWO_EURO_MAX_GRAY     105
#define TWO_EURO_MIN_HUE      110
#define TWO_EURO_MAX_HUE      130
#define TWO_EURO_MIN_SAT      38
#define TWO_EURO_MAX_SAT      60
#define TWO_EURO_MIN_VAL      100
#define TWO_EURO_MAX_VAL      115
#define TWO_EURO_MIN_DIAMETER 179
#define TWO_EURO_MAX_DIAMETER 191

#endif



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

enum RGB {
	RED,
	GREEN,
	BLUE
};

enum COIN_TYPE {
	ONE_CENT,
	TWO_CENTS,
	FIVE_CENTS,
	TEN_CENTS,
	TWENTY_CENTS,
	FIFTY_CENTS,
	ONE_EURO,
	TWO_EUROS,
	UNKNOWN
};

typedef struct
{
	unsigned long long id;
	int   label;
	int   area;
	int   perimeter;
	float circularity;
	int   xc, yc;
	float diameter;
	float mean_gray, mean_hue, mean_sat, mean_val;
	COIN_TYPE type;
	int   min_x, max_x, min_y, max_y;
	int* border_idx;
	int   n_border_idx;
} Object;


struct TrackedCoin {
	int id;
	int xc, yc;
	float area;
	bool updated;
	int framesSinceLastSeen;
	int min_y;
	COIN_TYPE type;
};




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC* vc_read_image(char* filename);
int vc_write_image(char* filename, IVC* image);

int vc_rgb_to_gray(IVC* src, IVC* dst);

int vc_gray_to_bin(IVC* src, IVC* dst);

int vc_binary_dilate(IVC* src, IVC* dst, int kernel);

int vc_binary_erode(IVC* src, IVC* dst, int kernel);

int vc_opening(IVC* src, IVC* dst, int kernel);

int vc_closing(IVC* src, IVC* dst, int kernel);

int vc_rgb_to_hsv(IVC* src, IVC* dst);

int vc_hsv_to_bin(IVC* src, IVC* dst, int h_min, int h_max);

int vc_hsv_to_bin_extended(IVC* src, IVC* dst,
	int h_min, int h_max,
	int s_min, int s_max,
	int v_min, int v_max);

int diff_bin_images(IVC* src1, IVC* src2, IVC* dst);

Object* vc_binary_blob_labelling(IVC* bin, IVC* gray, IVC* hue, int* n_objects);

int vc_draw_labels(IVC* dst, Object* objects, int n_objects);

const char* coinTypeToString(COIN_TYPE type);

void vc_track_coins(Object* currentFrameObjects, int numObjects, std::vector<TrackedCoin>& trackedCoins, int& nextCoinId);


std::map<COIN_TYPE, int> countCoinsByType(const std::vector<TrackedCoin>& trackedCoins);