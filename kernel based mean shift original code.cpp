//测试GITHUB功能，程序不完善。
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <math.h>
#include <conio.h>
#include <ctype.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

bool selectObject = false;
bool trackObject = false;
bool paused = false;
bool start = false;

Point origin;
Rect selection;

Mat image;
Mat kernel;
Mat histogram_image;
Mat histogram_image_canditate;
Mat frame;
Mat origImage;
Mat weightImage;
Mat location_canditate;

VideoCapture cap;

/*****************************      �����ص�����    *************************************/
void onMouse(int event, int x, int y, int flags, void*)
{
	if (selectObject)//ֻ�е�������������ȥʱ����Ч��Ȼ��ͨ��if���������Ϳ���ȷ����ѡ���ľ�������selection��
	{
		selection.x = MIN(x, origin.x);//�������ϽǶ�������
		selection.y = MIN(y, origin.y);
		selection.width = std::abs(x - origin.x);//���ο�
		selection.height = std::abs(y - origin.y);//���θ�

		selection &= Rect(0, 0, image.cols, image.rows);//����ȷ����ѡ�ľ���������ͼƬ��Χ�ڣ�a &= b =�� a = a&b����λ���롯��
	}

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		origin = Point(x, y);
		selection = Rect(x, y, 0, 0);//�����հ���ȥʱ��ʼ����һ����������
		selectObject = true;
		break;
	case CV_EVENT_LBUTTONUP:
		selectObject = false;
		if (selection.width > 0 && selection.height > 0)
			trackObject = true;
		break;
	}
}

/**********************************  ������  *****************************************/
int main()
{
	cap = cv::VideoCapture("F://AVSS_AB_Easy_Divx.AVI");

	cv::Rect trackWindow;
	cv::namedWindow("WINDOW");

	cv::setMouseCallback("WINDOW", onMouse, 0);

	double kernel_sum = 0.0;
	double kernel_norm = 0.0;
	double Y[2];
	double center[2];

	center[0] = 0.0;
	center[1] = 0.0;
	Y[0] = 0.0;
	Y[1] = 0.0;

	for (;;)
	{
		if (!paused)
		{
			cap.read(frame);

			if (frame.empty())
				break;
		}

		frame.copyTo(image);

		//cv::imshow("WINDOW", image);//���Ե�1
		//if (cv::waitKey(33) == 27)
		//	break;

		if (!paused)
		{
			image.copyTo(origImage);


			if (trackObject)//һ�������������¼�����BUTTONDOWN��BUTTONUP�����Ժ�ִ��
			{

				if (trackObject == true)
				{
					//selection.x = 270;
					//selection.y = 360;
					//selection.width = 4;
					//selection.height = 4;//���ڲ��ԵĹ̶�������

					//rectangle(origImage, selection, cv::Scalar(0), 2);
					//imshow("WINDOW", origImage);
					//if (cv::waitKey(33) == 27)
					//	break;

					/*************************************�����˺���*************************************/
					center[0] = double(selection.width / 2.0);//��Ӧy(2)
					center[1] = double(selection.height / 2.0);//��Ӧy(1)

					double windowSize = (center[0] * center[0]) + (center[1] * center[1]);

					kernel = Mat::zeros(selection.height + 1, selection.width + 1, CV_64F);

					kernel_sum = 0.0;

					for (int i = 1; i <= selection.width; i++)
					{
						for (int j = 1; j <= selection.height; j++)
						{
							double dist = ((i - center[0])*(i - center[0])) + ((j - center[1])*(j - center[1]));
							double m = double(1 - dist / windowSize);
							kernel.at<double>(j, i) = double(1 - dist / windowSize);

							kernel_sum += kernel.at<double>(j, i);
						}
					}
					std::cout << "\n" << kernel_sum << "\n";//���Ե�2

					kernel_norm = 1.0 / kernel_sum;

					/**********************************����ѡ��Ŀ��ֱ��ͼ***********************************/
					histogram_image = Mat::zeros(1, 4097, CV_64F);

					cv::Vec3d pixel_value;
					cv::Vec3d histogram_value;

					int location = 0;

					int x_point = selection.x;
					int y_point = selection.y;

					x_point = selection.x;

					for (int p = 1; p <= selection.width; p++)
					{
						y_point = selection.y;

						for (int q = 1; q <= selection.height; q++)
						{
							pixel_value = origImage.at<cv::Vec3b>(y_point, x_point);

							histogram_value[0] = int(pixel_value[0] / 16.0);//B
							histogram_value[1] = int(pixel_value[1] / 16.0);//G
							histogram_value[2] = int(pixel_value[2] / 16.0);//R

							location = (histogram_value[0] * 256 + histogram_value[1] * 16 + histogram_value[2])+1;

							double m = 0.0;
							m = kernel.at<double>(q, p)*(kernel_norm);
							std::cout << '\n' << m << '\n';

							histogram_image.at<double>(0, location) += kernel.at<double>(q, p)*(kernel_norm);						

							y_point++;
						}
						x_point++;
					}
					trackObject = false;//��Ŀ��ģ�ͼ������ɺ������Ƴ�ѭ���������ظ����㣬�ǳ���Ҫ�Ĳ������á�
					start = true;
				}
			}

			if (start)
			{
				Y[0] = 2.0;
				Y[1] = 2.0;
			}


			int iteration = 0;
			
			while ((Y[0] * Y[0] + Y[1] * Y[1]>0.5) && iteration<20)/**********��������************************/
			{
				iteration = iteration + 1;
				/*std::cout<<"\n"<<iteration<<"\n";*/
				/***********************************������ѡ��ֱ��ͼ******************************/
				histogram_image_canditate = Mat::zeros(1, 4097, CV_64F);
				location_canditate = Mat::zeros(selection.height + 1, selection.width + 1, CV_64F);

				cv::Vec3d pixel_value_canditate;
				cv::Vec3d histogram_value_canditate;

				int x_point_canditate = selection.x;
				int y_point_canditate = selection.y;

				x_point_canditate = selection.x;

				for (int p = 1; p <= selection.width; p++)
				{
					y_point_canditate = selection.y;

					for (int q = 1; q <= selection.height; q++)
					{
						pixel_value_canditate = origImage.at<cv::Vec3b>(y_point_canditate, x_point_canditate);

						histogram_value_canditate[0] = int(pixel_value_canditate[0] / 16.0);
						histogram_value_canditate[1] = int(pixel_value_canditate[1] / 16.0);
						histogram_value_canditate[2] = int(pixel_value_canditate[2] / 16.0);

						location_canditate.at<double>(q, p) = (256 * histogram_value_canditate[0] + 16 * histogram_value_canditate[1] + histogram_value_canditate[2])+1;

						double n = 0.0;
						n = kernel.at<double>(q, p)*kernel_norm;
						std::cout <<'\n'<< n << '\n';

						histogram_image_canditate.at<double>(0, (location_canditate.at<double>(q, p))) += kernel.at<double>(q, p)*kernel_norm;

						y_point_canditate++;
					}
					x_point_canditate++;
				}
				/**************************************Ȩֵ����******************************************/
				weightImage = cv::Mat(1, 4097, CV_64F, cv::Scalar(0));

				for (int i = 1; i<=4096; i++)
				{
					if ((histogram_image_canditate.at<double>(0, i) != 0))
					{
						weightImage.at<double>(0, i) = sqrt((histogram_image.at<double>(0, i)) / (histogram_image_canditate.at<double>(0, i)));
					}
					else
					{
						weightImage.at<double>(0, i) = 0.0;
					}

					double k = 0.0;
					k = weightImage.at<double>(0, i);
					std::cout << '\n' << i <<"  "<<k << '\n';
				}

				double norm_i = 0.0;
				double norm_j = 0.0;

				double norm_x = 0.0;
				double norm_y = 0.0;

				double weight_sum = 0.0;

				for (int i = 1; i <= selection.width; i++)
				{
					for (int j = 1; j <= selection.height; j++)
					{
						norm_i = i - center[0] - 0.5;
						norm_j = j - center[1] - 0.5;

						norm_x += weightImage.at<double>(0, location_canditate.at<double>(j, i))*(norm_i);
						norm_y += weightImage.at<double>(0, location_canditate.at<double>(j, i))*(norm_j);

						weight_sum += weightImage.at<double>(0, location_canditate.at<double>(j, i));
					}
				}

				Y[0] = norm_x / weight_sum;
				std::cout << "\n" << Y[0] << "\n";
				Y[1] = norm_y / weight_sum;
				std::cout << "\n" << Y[1] << "\n";

				//�ı�ѡ����λ�ã�Ϊ��һ�ε�����ʼ��λ��
				trackWindow.x = selection.x + Y[1];
				trackWindow.y = selection.y + Y[0];
				trackWindow.width = selection.width;
				trackWindow.height = selection.height;

				if ((trackWindow.x+trackWindow.width)>origImage.cols || (trackWindow.y+trackWindow.height)>origImage.rows )
				{
					paused = true;
					cout << "\n" << "Target is disappeared!" << "\n";
				}
				selection = trackWindow;

			}//while����

			/***********************�������н���*********************/
			rectangle(origImage, trackWindow, cv::Scalar(0), 2);

			imshow("WINDOW", origImage);
			
			char c = (char)waitKey(33);
			if (c == 27)
				break;
			switch (c)
			{
			case 'p':
				paused = !paused;
				break;
			default:
				;
			}


		}//if��ͣ����


	}
}
