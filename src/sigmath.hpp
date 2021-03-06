/**
 * @file
 * @author Samuel Andrew Wisner, awisner94@gmail.com
 * @author Nicholas K. Nolan
 * @brief Contains the functions necessary to perform the mathematical
 * operations required by this program.
 * */

#ifndef sigmath_H
#define sigmath_H

#include <complex>
#include "definitions.hpp"

namespace avda {
	// PROTOTYPES

	/**
	 * Ensures all elements in an array are positive. Note that this function
	 * replaces array elements if necessary. It does not populate a new array.
	 *
	 * @param data array whose elements must all be positive
	 *
	 * @param size number of elements in the data array
	 */
	void absolute(float32* data, uint32 size);

	/**
	 * Takes the average of all elements in an array
	 *
	 * @param data array from which to compute the average
	 *
	 * @param size number of elements in the data array
	 *
	 * @return computed average
	 */
	float32 average(float32* data, uint32 size);

	/**
	 * Finds the averages of the elements of an array of DataParams.
	 *
	 * @param params DataParams array
	 *
	 * @param size number of elements in the DataParams array
	 *
	 * @return DataParams structure containing the average values of the
	 * structure's elements in the params array
	 */
	DataParams average(DataParams* params, uint8 size);

	/**
	 * Converts an array of floats to "power decibels", i.e., x[n] =
	 * 20*log10(x[n]). The decibel values are written to the same array that
	 * contained the values to be converted. In other words, this function
	 * should perform an in-place, element-wise conversion.
	 *
	 * @param data array of values to be converted as well as the location
	 * where the converted values will be written
	 *
	 * @param size number of elements in the data array
	 */
	void decibels(float32* data, uint32 size);

	/**
	 * Computes the left-handed first derivative of a discrete signal. The first
	 * element will be 0.
	 *
	 * @param data array containing the discrete signal data array
	 *
	 * @param size number of elements in data
	 */
	void diff(float32* data, uint32 size);

	/**
	 * Replaces the values of an array of cfloat32's with the array's DFT using
	 * a decimation-in-frequency algorithm.
	 *
	 * This code is based on code from
	 * http://rosettacode.org/wiki/Fast_Fourier_transform#C.2B.2B.
	 *
	 * @param data array whose values should be replaced with its DFT
	 *
	 * @param size number of elements in the data array
	 */
	void fft(cfloat32* data, uint32 size);

	/**
	 * Computes the magitude of an array of complex numbers.
	 *
	 * @param orig array of complex numbers
	 *
	 * @param newmags array to which the (real) magitudes are to be written
	 *
	 * @param size number of elements in orig and newmags
	 */
	void mag(cfloat32* orig, float32* newmags, uint32 size);

	/**
	 * Finds the maximum value in an array.
	 *
	 * @param data array whose maximum value is to be found
	 *
	 * @param size number of elements in the data array
	 *
	 * @return maximum value and its index
	 */
	Maximum max(float32* data, uint32 size);

	/**
	 * Applies an nth-order moving-average filter to a discrete signal.
	 *
	 * @param data array containing the signal to which the filter should be
	 * applied
	 *
	 * @param size number of elements in the data array
	 *
	 * @param order order of the filter
	 */
	void smooth(float32* data, uint32 size, uint16 order);

	// DEFINITIONS

	void absolute(float32* data, uint32 size) {
		for(uint32 i = 0; i < size; i++) {
			data[i] = fabsf(data[i]);
		}
	}

	float32 average(float32* data, uint32 size) {
		float32 ave;

		for(uint32 i = 0; i < size; i++) {
			ave += data[i];
		}

		ave = ave / size;
		return ave;
	}

	DataParams average(DataParams* params, uint8 size) {
		DataParams ave;

		for(uint8 i = 0; i < size; i++) {
			//freq is an attribute. this is how to add structure attributes
			ave.freq += params[i].freq;
			ave.noise += params[i].noise;
		}

		ave.freq /= size;
		ave.noise /= size;
		return ave;
	}

	void decibels(float32* data, uint32 size) {
		for(uint32 i = 0; i < size; i++) {
			data[i] = 20 * log10(data[i]);
		}
	}

	void diff(float32* data, uint32 size) {
		float32 temp[size];
		temp[0] = 0;

		for(uint32 i = 1; i < size; i++) {
			temp[i] = data[i] - data[i-1];
		}

		for(uint32 i = 0; i < size; i++) {
			data[i] = temp[i];
		}
	}

	void fft(cfloat32* data, uint32 size) {
		// DFT
		uint32 k = size;
		uint32 n;
		float32 thetaT = M_PI / size;
		cfloat32 phiT(cos(thetaT), sin(thetaT));
		cfloat32 T;

		while(k > 1) {
			n = k;
			k >>= 1;
			phiT = phiT * phiT;
			T = 1.0L;

			for(uint32 l = 0; l < k; l++) {
				for(uint32 a = l; a < size; a += n) {
					uint32 b = a + k;
					cfloat32 t = data[a] - data[b];
					data[a] += data[b];
					data[b] = t * T;
				}

				T *= phiT;
			}
		}

		// Decimate
		uint32 m = (uint32)log2(size);

		for(uint32 a = 0; a < size; a++) {
			uint32 b = a;

			// Reverse bits
			b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
			b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
			b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
			b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
			b = ((b >> 16) | (b << 16)) >> (32 - m);

			if (b > a)
			{
				cfloat32 t = data[a];
				data[a] = data[b];
				data[b] = t;
			}
		}
	}

	void mag(cfloat32* orig, float32* newmags, uint32 size) {
		//loop to run throught the length of array orig
		for(uint32 n = 0; n < size; n++) {
			/* 
			 * abs should calculate the magnitude of complex array elements.
			 * saves to new array
			 */
			newmags[n] = std::abs(orig[n]);		
		}
	}

	Maximum max(float32* data, uint32 size) {
		Maximum m;

		//loop to run through the length of array data
		for (uint32 i = 0; i < size; i++) {
			/* 
			 * when value at data[i] is above max.value,
			 * sets max.value equal to data[i] and max.index equal to i
			 */
			if (data[i] > m.value) {
				m.value = data[i];
				m.index = i;
			}
		}

		return m;
	}

	void smooth(float32* data, uint32 size, uint16 order) {
		float32 coeff = 1 / (float32)order;
		float32 temp[size];

		for(uint32 i = 0; i < size; i++) {
			temp[i] = 0;

			for(uint16 j = 0; j < order && j <= i; j++) {
				temp[i] += data[i - j];
			}

			temp[i] *= coeff;
		}

		for(uint32 i = 0; i < size; i++) {
			data[i] = temp[i];
		}
	}
}

#endif
