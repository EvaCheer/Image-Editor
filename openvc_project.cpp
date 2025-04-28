#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <cmath>
#include <iostream>
using namespace cv;
using namespace std;

struct ImageEditorState {
	Mat original_image;
	int brightness = 50;
	int contrast = 10;
	int gaussian_blur = 0;
	int sharpness = 0;	
};

template<typename T>
T clamp(T v, T lo, T hi) {
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

string readPath() {
	string path;
	while (true) {
		cout << "Enter path to image: ";
		cin >> path;
		if (!imread(path).empty()) {
			return path;
		}
		cout << "Invalid path. Please try again.\n";
	}
}

Mat readImage(const string& path) {
	Mat img = imread(path);
	if (img.empty()) {
		cout << "Could not open or find the image\n";
	}
	return img;
}

void displayImage(Mat& img) {
	namedWindow("First OpenCV Application", WINDOW_AUTOSIZE);
	cv::imshow("First OpenCV Application", img);
	cv::moveWindow("First OpenCV Application", 0, 45);
}


Mat generateGaussianKernel(int kernel_size, double sigma) {
	Mat kernel(kernel_size, kernel_size, CV_64F);
	int half_size = kernel_size / 2;
	double sum = 0.0;

	for (int y = -half_size; y <= half_size; y++) {
		for (int x = -half_size; x <= half_size; x++) {
			double value = exp(-(x * x + y * y) / (2 * sigma * sigma));
			kernel.at<double>(y + half_size, x + half_size) = value;
			sum += value;
		}
	}
	// Normalize the kernel so that the sum of all elements equals 1
	kernel /= sum;
	return kernel;
}

Mat editBrightnessAndContrast(const Mat& image, const size_t& brightness_modifier, const double& contrast_modifier) {
	if (brightness_modifier < 0 || brightness_modifier > 100 || 
		contrast_modifier < 0.0 || contrast_modifier > 3.0) {
		cout << "Invalid modifier values." << endl;
		return image.clone();
	}
	Mat new_image = Mat::zeros(image.size(), image.type());
	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			for (int c = 0; c < image.channels(); c++) {
				new_image.at<Vec3b>(y,x)[c] =
				saturate_cast<uchar>(contrast_modifier * image.at<Vec3b>(y, x)[c] + brightness_modifier);
			}
		}
	}
	return new_image;
}

Mat editGaussianBlur(const Mat& image, int blur_strength) {
	if (blur_strength < 0 || blur_strength > 100) {
		cout << "Invalid blur strength." << endl;
		return image.clone();
	}

	// Kernel size must be odd
	int kernel_size = blur_strength;
	if (kernel_size % 2 == 0) kernel_size++;

	double sigma = kernel_size / 5.0;

	Mat kernel = generateGaussianKernel(kernel_size, sigma);

	Mat new_image = Mat::zeros(image.size(), image.type());
	int half_k = kernel_size / 2;

	for (int y = 0; y < image.rows; ++y) {
		for (int x = 0; x < image.cols; ++x) {
			Vec3d sum = { 0, 0, 0 };

			// Convolution
			for (int ky = -half_k; ky <= half_k; ++ky) {
				for (int kx = -half_k; kx <= half_k; ++kx) {
					int yy = clamp(y + ky, 0, image.rows - 1);
					int xx = clamp(x + kx, 0, image.cols - 1);

					Vec3b pixel = image.at<Vec3b>(yy, xx);
					double weight = kernel.at<double>(ky + half_k, kx + half_k);

					sum[0] += pixel[0] * weight;
					sum[1] += pixel[1] * weight;
					sum[2] += pixel[2] * weight;
				}
			}

			new_image.at<Vec3b>(y, x) = Vec3b(
				saturate_cast<uchar>(sum[0]),
				saturate_cast<uchar>(sum[1]),
				saturate_cast<uchar>(sum[2])
			);
		}
	}

	return new_image;
}

Mat editSharpness(const Mat& image, int sharpness_modifier) {
	if (sharpness_modifier <= 0) {
		return image.clone();
	}

	Mat blurred = editGaussianBlur(image, 4);
	Mat sharpened = Mat::zeros(image.size(), image.type());

	double alpha = 1.0 + sharpness_modifier / 10.0; // Scale original
	double beta = -sharpness_modifier / 10.0;       // Subtract blurred

	for (int y = 0; y < image.rows; ++y) {
		for (int x = 0; x < image.cols; ++x) {
			Vec3b original_pixel = image.at<Vec3b>(y, x);
			Vec3b blurred_pixel = blurred.at<Vec3b>(y, x);
			Vec3b result_pixel;

			for (int c = 0; c < image.channels(); ++c) {
				double value = alpha * original_pixel[c] + beta * blurred_pixel[c];
				result_pixel[c] = saturate_cast<uchar>(value);
			}

			sharpened.at<Vec3b>(y, x) = result_pixel;
		}
	}

	return sharpened;
}

Mat applyAllEffects(const ImageEditorState& state) {
	double contrast_modifier = state.contrast / 10.0;

	Mat edited = editBrightnessAndContrast(state.original_image, state.brightness, contrast_modifier);
	edited = editGaussianBlur(edited, state.gaussian_blur);
	edited = editSharpness(edited, state.sharpness);

	return edited;
}

void onTrackbarChange(int, void* userdata) {
	auto* state = static_cast<ImageEditorState*>(userdata);

	Mat edited = applyAllEffects(*state);
	imshow("First OpenCV Application", edited);
}

int main()
{
	string path = readPath();
	Mat image = readImage(path);
	if (image.empty()) return -1;
	displayImage(image);

	ImageEditorState state;
	state.original_image = image.clone();


	createTrackbar("Brightness", "First OpenCV Application", &state.brightness, 100, onTrackbarChange, &state);
	createTrackbar("Contrast", "First OpenCV Application", &state.contrast, 30, onTrackbarChange, &state);
	createTrackbar("Gaussian Blur", "First OpenCV Application", &state.gaussian_blur, 50, onTrackbarChange, &state);
	createTrackbar("Sharpness", "First OpenCV Application", &state.sharpness, 30, onTrackbarChange, &state);

	onTrackbarChange(0, &state);
	waitKey(0);

	return 0;
}