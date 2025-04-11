#include "video.h"
#include <iostream>
#include <string>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

extern "C" {
    #include "vc.h"
    #include "vc.c"
}

#define VIDEO "video1.mp4"
#define GRAY_TO_BIN_THRESHOLD 100


void vc_timer(void) {
	static bool running = false;
	static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

	if (!running) {
		running = true;
	}
	else {
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

		// Tempo em segundos.
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
		double nseconds = time_span.count();

		std::cout << "Tempo decorrido: " << nseconds << "segundos" << std::endl;
		std::cout << "Pressione qualquer tecla para continuar...\n";
		std::cin.get();
	}
}

int main(void) {
	cv::setNumThreads(20);
	// V�deo
	char videofile[20] = "video1.mp4";
	cv::VideoCapture capture;
    Video video;
	std::string str;
	int key = 0;

	capture.open(videofile);

	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o ficheiro de v�deo!\n";
		return 1;
	}

	/* N�mero total de frames no v�deo */
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	/* Frame rate do v�deo */
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	/* Resolu��o do v�deo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o v�deo */
	cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

	/* Inicia o timer */
	vc_timer();

	cv::Mat frame;


	IVC* img = vc_image_new(video.width, video.height, 3, 256);
	IVC* img_dst = vc_image_new(video.width, video.height, 3, 256);
	IVC* img_bin = vc_image_new(video.width, video.height, 1, 2);
	IVC* img_bin_2 = vc_image_new(video.width, video.height, 1, 2);
	IVC* img_bin_result = vc_image_new(video.width, video.height, 1, 2);
	IVC* img_bin_dilate = vc_image_new(video.width, video.height, 1, 2);
	IVC* img_bin_erosion = vc_image_new(video.width, video.height, 1, 2);

	IVC* img_bin_dilate_2 = vc_image_new(video.width, video.height, 1, 2);
	IVC* img_bin_erosion_2 = vc_image_new(video.width, video.height, 1, 2);

	IVC* img_to_hsv = vc_image_new(video.width, video.height, 3, 256);



	while (key != 'q') {
		/* Leitura de uma frame do v�deo */
		capture.read(frame);

		/* Verifica se conseguiu ler a frame */
		if (frame.empty()) break;

		/* N�mero da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);


		memcpy(img->data, frame.data, video.width * video.height * 3);

		vc_rgb_to_gray(img, img_dst);
		vc_grey_to_binary(img_dst, img_bin, GRAY_TO_BIN_THRESHOLD);
		// vc_rgb_to_hsv(img, img_to_hsv);
		// vc_hsv_binary(img_to_hsv, img_bin_2, 100);
		// vc_bin_diff(img_bin, img_bin_2, img_bin_result);
		
		vc_binary_dilate(img_bin, img_bin_dilate, 7);
		vc_binary_erosion(img_bin_dilate, img_bin_erosion, 7);
		vc_binary_erosion(img_bin_erosion, img_bin_erosion_2, 7);
		vc_binary_dilate(img_bin_erosion_2, img_bin_dilate_2, 7);

		//memcpy(frame.data, img_to_hsv->data, video.width * video.height * 3);



        // for (int y = 0; y < video.height; y++) {
        //     for (int x = 0; x < video.width; x++) {
        //         int pos_bin = y * img_dst->bytesperline + x;
        //         unsigned char pixel_value = img_dst->data[pos_bin];
        //         frame.data[pos_bin] = pixel_value;
        //         frame.data[pos_bin + 1] = pixel_value;
        //         frame.data[pos_bin + 2] = pixel_value;
        //     }
        // }

        // std::stringstream ss;
        // ss << "./images/" << video.nframe << ".jpg";
        // std::string filename = ss.str();

        // cv::String cv_filename = filename.c_str();

        // if (!cv::imwrite(cv_filename, frame)) {
        //     std::cerr << "Error saving image to " << cv_filename << std::endl;
        //     return -1;
        // }




        //Transform from binary to rgb
		for (int y = 0; y < video.height; y++) {
            for (int x = 0; x < video.width; x++) {
                int pos_bin = y * img_bin_erosion_2->bytesperline + x;
                int pos_frame = y * video.width * 3 + x * 3;
                unsigned char pixel_value = img_bin_erosion_2->data[pos_bin];
				int pos_rgb = (y * video.width + x) * 3;
				frame.data[pos_rgb] = pixel_value;
				frame.data[pos_rgb + 1] = pixel_value;    // Green channel
				frame.data[pos_rgb + 2] = pixel_value;    // Red channel
				
            }
        }




		/* Exemplo de inser��o texto na frame */
		str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

    
    
        //std::cout << "Saved frame " << video.nframe << " as " << cv_filename << std::endl;

		/* Exibe a frame */
		cv::imshow("VC - VIDEO", frame);

		/* Sai da aplica��o, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);
	}

	vc_image_free(img);
	vc_image_free(img_dst);
	vc_image_free(img_bin);
	vc_image_free(img_bin_2);
	vc_image_free(img_bin_result);
	vc_image_free(img_bin_dilate);
	vc_image_free(img_bin_erosion);
	vc_image_free(img_to_hsv);

	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de v�deo */
	capture.release();

	return 0;
}