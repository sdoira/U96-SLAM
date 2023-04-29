
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <time.h>
#include <conio.h>
#include <direct.h>
#include <windows.h>
#include <thread>

#define DEVICE_ID 1
#define CR 0x0d
#define ESC 0x1b

struct CALENDER_DATE {
	int hours;
	int minutes;
	int seconds;
	int day;
	int month;
	int year;
};

void getCalenderDate(struct CALENDER_DATE *date)
{
	// current system time
	time_t now;
	time(&now);

	// calender time
	struct tm *local = localtime(&now);
	date->hours = local->tm_hour;
	date->minutes = local->tm_min;
	date->seconds = local->tm_sec;
	date->day = local->tm_mday;
	date->month = local->tm_mon + 1;
	date->year = local->tm_year + 1900;
}

void ThreadFunc(char *ch) {
	*ch = 0;
	*ch = _getch();
}

int main (void)
{
	cv::VideoCapture cap;
	std::thread th;
	char ch = 0;

	printf("Press Enter to capture image, press ESC to quit.\n");
	while (1)
	{
		//-----------------------------------------------------------
		// connecting to the device
		//-----------------------------------------------------------
		printf("connecting to the device...\n");
		while (!cap.open(DEVICE_ID)) {
			Sleep(500);
		}
		printf("connected\n");

		//-----------------------------------------------------------
		// create directory
		//-----------------------------------------------------------
		struct CALENDER_DATE date;
		getCalenderDate(&date);
		char path[200];
		sprintf(path, "%04d_%02d%02d_%02d%02d", date.year, date.month, date.day, date.hours, date.minutes);
		_mkdir(path);

		//-----------------------------------------------------------
		// capture image
		//-----------------------------------------------------------
		cv::Rect roiLeft, roiRight;
		int count = 0;
		th = std::thread(ThreadFunc, &ch);
		int esc_hit = 0;
		int cr_hit = 0;
		while (1)
		{
			// capture image
			cv::Mat frame;
			bool ret = cap.read(frame);
			if (ret == 0) {
				printf("disconnected\n");
				cap.release();
				break;
			}

			cv::imshow("frame", frame);
			cv::waitKey(33);

			// keyboard input
			if (ch != 0) {
				th.join();
				if (ch == CR) {
					cr_hit = 1;
				}
				else if (ch == ESC) {
					// ESC is pressed to quit
					esc_hit = 1;
					break;
				}
				ch = 0;
				th = std::thread(ThreadFunc, &ch);
			}

			// CR is pressed to save image files
			if (cr_hit) {
				cr_hit = 0;
				if (count == 0) {
					roiLeft.x = 0;
					roiLeft.y = 0;
					roiLeft.width = frame.cols / 2;
					roiLeft.height = frame.rows;

					roiRight.x = frame.cols / 2;
					roiRight.y = 0;
					roiRight.width = frame.cols / 2;
					roiRight.height = frame.rows;
				}

				char filename[100];
				cv::Mat matLeft(frame, roiLeft);
				sprintf(filename, "%s\\%04d_l.jpg", path, count);
				imwrite(filename, matLeft);

				cv::Mat matRight(frame, roiRight);
				sprintf(filename, "%s\\%04d_r.jpg", path, count);
				imwrite(filename, matRight);

				printf("captured %06d.jpg\n", count);

				count++;
			}
		}

		if (esc_hit) {
			break;
		}
	}

	return 0;
}
