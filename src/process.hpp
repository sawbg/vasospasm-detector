/**
 * @author Samuel Andrew Wisner, awisner94@gmail.com
 * @brief contains function()s related to the program's threaded processing of
 * audio data
 */

#ifndef process_H
#define process_H

#include <map>
#include <stdexcept>

#include "definitions.hpp"
#include "sigmath.hpp"

namespace vaso {	
	/**
	 * Processes the recorded audio. Meant to be run in a separate thread as the
	 * recordings are being made. This function assumes that the left-side
	 * recordings will be made first.
	 *
	 * @params data two-dimensional array (first dimension whole recordings,
	 * second dimension samples in a recording) that will contain all recorded
	 * audio
	 *
	 * @params recCount the number of recordings (left and right together) to be
	 *  made
	 *
	 * @param sampleCount the number of samples in each recording. MUST be a
	 * power of two.
	 *
	 * @param sampleFreq the sampling frequency in Hz or Samples/second
	 *
	 * @param counter pointer to an index denoting which recording is currently
	 * in progress
	 *
	 * @return a map of the averaged left- and right-side parameters in
	 * DataParams structures
	 */
	std::map<DataParams> Process(float32* data, uint8 recCount,
			uint32 sampleCount, uint32 sampleFreq, uint8* counter) {

		// just in case sampleCount isn't a power of two
		if((sampleCount & (sampleCount - 1) != 0) || sampleCount < 2) {
			throw invalid_argument(
					"The number of samples is not a power of two!");
		}

		// declare function-scoped variables
		uint32 freqSize = sampleCount / 2;
		cfloat32 cdata[recCount][sampleCount];
		float32 fdata[recCount][freqSize];
		DataParams tempParams[recCount];
		std::map<DataParams> sideParams;

		for(uint8 rCount = 0; rCount < recCount; rCount++) {
			while(*counter <= rCount);  // wait until recording is done

			// convert data to complex numbers for fft()
			for(uint32 i = 0; i < sampleCount; i++) {
				cdata[rCount][i] = data[rCount][i];
			}

			// find frequency spectrum in relative decibels
			fft(&cdata[rCount], sampleCount);
			mag(&cdata[rCount], &fdata[rCount], freqSize);
			Maximum maximum = max(&fdata[rCount], freqSize);

			for(uint32 i = 0; i < freqSize; i++) {
				fdata[rCount][i] /= maximum.value;
			}

			decibels(&fdata[rCount], freqSize);
			
			/*
			 * Run spectrum values through moving-average filter to smooth the
			 * curve and make it easier to determine the derivative.
			 */
			smooth(&fdata[rCount], freqSize, 20);

			/*
			 * Find the derivative of the smoothed spectrum. Bote that both this
			 * filter and the previous are necessary to the algorithm.
			 */
			diff(&fdata[rCount], freqSize);
			smooth(&fdata[rCount], freqSize, 100);
			absolute(&fdata[rCount], freqSize);

			// find the parameters of this specific recording
			uint16 offset = 1000;
			uint32 index = max(absolute(&fdata[rCount][offset],
						freqSize - offset)).index;
			tempParams[rCount].freq = index * (float)sampleFreq / freqSize;
			tempParams[rCount].noise =
				average(&fdata[rCount][index + 2 * offset],
						freqSize - 2 * offset);
		}

		// calculate the parameters for each side to be returned
		sideParams[Side::Left] = average(&tempParams[0], rCount / 2);
		sideParams[Side::Right] = average(&tempParams[rCount / 2], rCount / 2);
		return sideParams;
	}
}

#endif