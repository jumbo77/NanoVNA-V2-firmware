/* 
 * fft.cpp is Based on
 * Free FFT and convolution (C)
 * 
 * Copyright (c) 2019 Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/free-small-fft-in-multiple-languages
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */


#include <math.h>
#include <stdint.h>

static uint16_t reverse_bits(uint16_t x, int n) {
	uint16_t result = 0;
	for (int i = 0; i < n; i++, x >>= 1)
		result = (result << 1) | (x & 1U);
	return result;
}

static const float sin_table[] = {
	/*
	 * float has about 7.2 digits of precision
		#include <stdio.h>
		#include <math.h>
		#include <stdint.h>

		int FFT_SIZE = 512;

		int main() {
			for (int i = 0; i < FFT_SIZE - (FFT_SIZE / 4); i++) {
				printf("% .8f,%c", sin(2 * M_PI * i / FFT_SIZE), i % 8 == 7 ? '\n' : ' ');
			}
		}
	*/
	 0.00000000,  0.01227154,  0.02454123,  0.03680722,  0.04906767,  0.06132074,  0.07356456,  0.08579731,
	 0.09801714,  0.11022221,  0.12241068,  0.13458071,  0.14673047,  0.15885814,  0.17096189,  0.18303989,
	 0.19509032,  0.20711138,  0.21910124,  0.23105811,  0.24298018,  0.25486566,  0.26671276,  0.27851969,
	 0.29028468,  0.30200595,  0.31368174,  0.32531029,  0.33688985,  0.34841868,  0.35989504,  0.37131719,
	 0.38268343,  0.39399204,  0.40524131,  0.41642956,  0.42755509,  0.43861624,  0.44961133,  0.46053871,
	 0.47139674,  0.48218377,  0.49289819,  0.50353838,  0.51410274,  0.52458968,  0.53499762,  0.54532499,
	 0.55557023,  0.56573181,  0.57580819,  0.58579786,  0.59569930,  0.60551104,  0.61523159,  0.62485949,
	 0.63439328,  0.64383154,  0.65317284,  0.66241578,  0.67155895,  0.68060100,  0.68954054,  0.69837625,
	 0.70710678,  0.71573083,  0.72424708,  0.73265427,  0.74095113,  0.74913639,  0.75720885,  0.76516727,
	 0.77301045,  0.78073723,  0.78834643,  0.79583690,  0.80320753,  0.81045720,  0.81758481,  0.82458930,
	 0.83146961,  0.83822471,  0.84485357,  0.85135519,  0.85772861,  0.86397286,  0.87008699,  0.87607009,
	 0.88192126,  0.88763962,  0.89322430,  0.89867447,  0.90398929,  0.90916798,  0.91420976,  0.91911385,
	 0.92387953,  0.92850608,  0.93299280,  0.93733901,  0.94154407,  0.94560733,  0.94952818,  0.95330604,
	 0.95694034,  0.96043052,  0.96377607,  0.96697647,  0.97003125,  0.97293995,  0.97570213,  0.97831737,
	 0.98078528,  0.98310549,  0.98527764,  0.98730142,  0.98917651,  0.99090264,  0.99247953,  0.99390697,
	 0.99518473,  0.99631261,  0.99729046,  0.99811811,  0.99879546,  0.99932238,  0.99969882,  0.99992470,
	 1.00000000,  0.99992470,  0.99969882,  0.99932238,  0.99879546,  0.99811811,  0.99729046,  0.99631261,
	 0.99518473,  0.99390697,  0.99247953,  0.99090264,  0.98917651,  0.98730142,  0.98527764,  0.98310549,
	 0.98078528,  0.97831737,  0.97570213,  0.97293995,  0.97003125,  0.96697647,  0.96377607,  0.96043052,
	 0.95694034,  0.95330604,  0.94952818,  0.94560733,  0.94154407,  0.93733901,  0.93299280,  0.92850608,
	 0.92387953,  0.91911385,  0.91420976,  0.90916798,  0.90398929,  0.89867447,  0.89322430,  0.88763962,
	 0.88192126,  0.87607009,  0.87008699,  0.86397286,  0.85772861,  0.85135519,  0.84485357,  0.83822471,
	 0.83146961,  0.82458930,  0.81758481,  0.81045720,  0.80320753,  0.79583690,  0.78834643,  0.78073723,
	 0.77301045,  0.76516727,  0.75720885,  0.74913639,  0.74095113,  0.73265427,  0.72424708,  0.71573083,
	 0.70710678,  0.69837625,  0.68954054,  0.68060100,  0.67155895,  0.66241578,  0.65317284,  0.64383154,
	 0.63439328,  0.62485949,  0.61523159,  0.60551104,  0.59569930,  0.58579786,  0.57580819,  0.56573181,
	 0.55557023,  0.54532499,  0.53499762,  0.52458968,  0.51410274,  0.50353838,  0.49289819,  0.48218377,
	 0.47139674,  0.46053871,  0.44961133,  0.43861624,  0.42755509,  0.41642956,  0.40524131,  0.39399204,
	 0.38268343,  0.37131719,  0.35989504,  0.34841868,  0.33688985,  0.32531029,  0.31368174,  0.30200595,
	 0.29028468,  0.27851969,  0.26671276,  0.25486566,  0.24298018,  0.23105811,  0.21910124,  0.20711138,
	 0.19509032,  0.18303989,  0.17096189,  0.15885814,  0.14673047,  0.13458071,  0.12241068,  0.11022221,
	 0.09801714,  0.08579731,  0.07356456,  0.06132074,  0.04906767,  0.03680722,  0.02454123,  0.01227154,
	 0.00000000, -0.01227154, -0.02454123, -0.03680722, -0.04906767, -0.06132074, -0.07356456, -0.08579731,
	-0.09801714, -0.11022221, -0.12241068, -0.13458071, -0.14673047, -0.15885814, -0.17096189, -0.18303989,
	-0.19509032, -0.20711138, -0.21910124, -0.23105811, -0.24298018, -0.25486566, -0.26671276, -0.27851969,
	-0.29028468, -0.30200595, -0.31368174, -0.32531029, -0.33688985, -0.34841868, -0.35989504, -0.37131719,
	-0.38268343, -0.39399204, -0.40524131, -0.41642956, -0.42755509, -0.43861624, -0.44961133, -0.46053871,
	-0.47139674, -0.48218377, -0.49289819, -0.50353838, -0.51410274, -0.52458968, -0.53499762, -0.54532499,
	-0.55557023, -0.56573181, -0.57580819, -0.58579786, -0.59569930, -0.60551104, -0.61523159, -0.62485949,
	-0.63439328, -0.64383154, -0.65317284, -0.66241578, -0.67155895, -0.68060100, -0.68954054, -0.69837625,
	-0.70710678, -0.71573083, -0.72424708, -0.73265427, -0.74095113, -0.74913639, -0.75720885, -0.76516727,
	-0.77301045, -0.78073723, -0.78834643, -0.79583690, -0.80320753, -0.81045720, -0.81758481, -0.82458930,
	-0.83146961, -0.83822471, -0.84485357, -0.85135519, -0.85772861, -0.86397286, -0.87008699, -0.87607009,
	-0.88192126, -0.88763962, -0.89322430, -0.89867447, -0.90398929, -0.90916798, -0.91420976, -0.91911385,
	-0.92387953, -0.92850608, -0.93299280, -0.93733901, -0.94154407, -0.94560733, -0.94952818, -0.95330604,
	-0.95694034, -0.96043052, -0.96377607, -0.96697647, -0.97003125, -0.97293995, -0.97570213, -0.97831737,
	-0.98078528, -0.98310549, -0.98527764, -0.98730142, -0.98917651, -0.99090264, -0.99247953, -0.99390697,
	-0.99518473, -0.99631261, -0.99729046, -0.99811811, -0.99879546, -0.99932238, -0.99969882, -0.99992470
};

/***
 * dir = forward: 0, inverse: 1
 * https://www.nayuki.io/res/free-small-fft-in-multiple-languages/fft.c
 */
void fft512(float array[][2], const uint8_t dir) {
	constexpr uint16_t n = 512;
	constexpr uint8_t levels = 9; // log2(n)
	const float* const cos_table = &sin_table[n/4];

	const uint8_t real =   dir & 1;
	const uint8_t imag = ~real & 1;

	for (uint16_t i = 0; i < n; i++) {
		uint16_t j = reverse_bits(i, levels);
		if (j > i) {
			float temp = array[i][real];
			array[i][real] = array[j][real];
			array[j][real] = temp;
			temp = array[i][imag];
			array[i][imag] = array[j][imag];
			array[j][imag] = temp;
		}
	}

	// Cooley-Tukey decimation-in-time radix-2 FFT
	for (uint16_t size = 2; size <= n; size *= 2) {
		uint16_t halfsize = size / 2;
		uint16_t tablestep = n / size;
		for (uint16_t i = 0; i < n; i += size) {
			for (uint16_t j = i, k = 0; j < i + halfsize; j++, k += tablestep) {
				uint16_t l = j + halfsize;
				float tpre =  array[l][real] * cos_table[k] + array[l][imag] * sin_table[k];
				float tpim = -array[l][real] * sin_table[k] + array[l][imag] * cos_table[k] ;
				array[l][real] = array[j][real] - tpre;
				array[l][imag] = array[j][imag] - tpim;
				array[j][real] += tpre;
				array[j][imag] += tpim;
			}
		}
		if (size == n)  // Prevent overflow in 'size *= 2'
			break;
	}
}
