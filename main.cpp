#include <iostream>
#include <string>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "lib/vc.h"
#include "stdlib.h"



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
	// V�deo
	char videofile[20] = VIDEO;
	cv::VideoCapture capture;
	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;
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

	IVC* image = vc_image_new(video.width, video.height, 3, 256);
	IVC* image_gray = vc_image_new(video.width, video.height, 1, 256);
	IVC* image_hsv = vc_image_new(video.width, video.height, 3, 256);
	IVC* image_bin = vc_image_new(video.width, video.height, 3, 256);
	IVC* image_closing = vc_image_new(video.width, video.height, 3, 256);
	IVC* image_opening = vc_image_new(video.width, video.height, 3, 256);
	IVC* image_labeled = vc_image_new(video.width, video.height, 3, 256);

	std::vector<TrackedCoin> trackedCoins;
	int nextCoinId = 1;
	int totalUniqueCoins = 0;


	std::vector<Object> filteredObjects;

	if (VIDEO_FRAME_POS > 0 && DEBUG) {
		capture.set(cv::CAP_PROP_POS_FRAMES, VIDEO_FRAME_POS);
	}
	cv::Mat frame;
	while (key != 'q') {


		/* Leitura de uma frame do v�deo */
		capture.read(frame);


		/* Verifica se conseguiu ler a frame */
		if (frame.empty()) break;

		filteredObjects.clear();

		/* N�mero da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(image->data, frame.data, video.width * video.height * 3);
		memcpy(image_labeled->data, frame.data, video.width* video.height * 3);

		vc_rgb_to_hsv(image, image_hsv);


		vc_rgb_to_gray(image, image_gray);

		vc_gray_to_bin(image_gray, image_bin);


		vc_closing(image_bin, image_closing, CLOSING_KERNEL_SIZE);

		vc_opening(image_closing, image_opening, OPENING_KERNEL_SIZE);


		int n;
		Object* allObjects = vc_binary_blob_labelling(image_opening, image_gray, image_hsv , &n);

		for (int i = 0; i < n; i++) { // Filter by circularity and if type != UNKNOWN
			Object& obj = allObjects[i];

			if (obj.circularity < 0.96f || obj.circularity > 1.28f) continue;
			if (obj.type == COIN_TYPE::UNKNOWN) continue;

			filteredObjects.push_back(obj);
		}

		vc_track_coins(filteredObjects.data(), filteredObjects.size(), trackedCoins, nextCoinId);
		auto coinCounts = countCoinsByType(trackedCoins);


		totalUniqueCoins = nextCoinId - 1;
		
		if (!filteredObjects.empty())
		{
			vc_draw_labels(image_labeled,
				filteredObjects.data(),
				static_cast<int>(filteredObjects.size()));
		}
		free(allObjects);

		IVC* image_to_display = image_labeled;

		if (image_to_display->channels == 1 && DEBUG) { // Display Gray Image for Debuging
			for (int y = 0; y < video.height; y++) {
				for (int x = 0; x < video.width; x++) {
					int pos_gray = y * video.width + x;
					int pos_rgb = (y * video.width + x) * 3;

					unsigned char value = image_to_display->data[pos_gray];

					frame.data[pos_rgb] = value;
					frame.data[pos_rgb + 1] = value;
					frame.data[pos_rgb + 2] = value;
				}
			}
		}
		else {
			memcpy(frame.data, image_to_display->data, video.width* video.height * 3);
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
		str = std::string("N. DE MOEDAS: ").append(std::to_string(totalUniqueCoins));
		cv::putText(frame, str, cv::Point(20, 125), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 125), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. 1 CENTIMO: ").append(std::to_string(coinCounts[ONE_CENT]));
		cv::putText(frame, str, cv::Point(20, 150), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 150), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. 2 CENTIMOS: ").append(std::to_string(coinCounts[TWO_CENTS]));
		cv::putText(frame, str, cv::Point(20, 175), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 175), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. 5 CENTIMOS: ").append(std::to_string(coinCounts[FIVE_CENTS]));
		cv::putText(frame, str, cv::Point(20, 200), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 200), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. 10 CENTIMOS: ").append(std::to_string(coinCounts[TEN_CENTS]));
		cv::putText(frame, str, cv::Point(20, 225), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 225), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. 20 CENTIMOS: ").append(std::to_string(coinCounts[TWENTY_CENTS]));
		cv::putText(frame, str, cv::Point(20, 250), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 250), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. 50 CENTIMOS: ").append(std::to_string(coinCounts[FIFTY_CENTS]));
		cv::putText(frame, str, cv::Point(20, 275), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 275), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. 1 EURO: ").append(std::to_string(coinCounts[ONE_EURO]));
		cv::putText(frame, str, cv::Point(20, 300), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 300), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. 2 EURO: ").append(std::to_string(coinCounts[TWO_EUROS]));
		cv::putText(frame, str, cv::Point(20, 325), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 325), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

		/* Exibe a frame */
		cv::imshow("VC - VIDEO", frame);

		/* Sai da aplica��o, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);
	}

	vc_image_free(image);
	vc_image_free(image_gray);
	vc_image_free(image_hsv);
	vc_image_free(image_bin);
	vc_image_free(image_closing);
	vc_image_free(image_opening);
	vc_image_free(image_labeled);



	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de v�deo */
	capture.release();

	return 0;
}