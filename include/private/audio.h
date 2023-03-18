/*
 * Copyright (C) 2021 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2021 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of timbre-mill
 * Created on: 16 февр. 2021 г.
 *
 * timbre-mill is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * timbre-mill is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with timbre-mill. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PRIVATE_AUDIO_H_
#define PRIVATE_AUDIO_H_

#include <lsp-plug.in/common/status.h>
#include <lsp-plug.in/runtime/LSPString.h>
#include <lsp-plug.in/dsp-units/sampling/Sample.h>
#include <lsp-plug.in/expr/Resolver.h>

#include <private/config/data.h>
#include <private/config/config.h>

namespace timbremill
{
    using namespace lsp;

    /**
     * Load audio file and perform resampling
     *
     * @param sample sample to store audio data
     * @param file_srate pointer to save original file's sample rate
     * @param srate desired sample rate
     * @param base base directory
     * @param name name of the file
     * @return status of operation
     */
    status_t load_audio_file(dspu::Sample *sample, size_t *file_srate, size_t srate, const LSPString *base, const LSPString *name);

    /**
     * Save audio file
     *
     * @param sample sample to save
     * @param base base directory
     * @param fmt output file name format
     * @param vars variable to parametrize the output file name format
     * @return status of operation
     */
    status_t save_audio_file(dspu::Sample *sample, const LSPString *base, const LSPString *fmt, expr::Resolver *vars);

    /**
     * Compute the spectral profile for the input signal
     *
     * @param profile spectral profile containing 2^precision averaged spectrum magnitude values.
     * @param src source sample
     * @param precision the precision of the spectral profile.
     * @return status of operation
     */
    status_t spectral_profile(dspu::Sample *profile, const dspu::Sample *src, size_t precision);

    /**
     * Compute the impulse response for timbral correction. The spectral correction is computed
     * as a result of division of the child spectral characteristics by master spectral
     * characteristics. Frequencies above the niquist frequency with subtracted transition octaves
     * are considered as transitional and are not affected by the timbral correction
     *
     * @param dst destination sample to store the impulse response
     * @param master the master profile
     * @param child the child file profile
     * @param precision the FFT precision
     * @param db_range the dynamic range
     * @param sample rate the actual signal limiting sample rate
     * @param transition transition zone in octaves (number of transition octaves)
     * @return status of operation
     */
    status_t timbre_impulse_response(
        dspu::Sample *dst,
        const dspu::Sample *master, const dspu::Sample *child,
        size_t precision, float db_range, size_t sample_rate,
        float transition);

    /**
     * Produce the linear impulse response from the spectral profile
     *
     * @param dst destination sample to store the impulse response
     * @param profile the original profile
     * @param precision the FFT precision
     * @param top_sr
     * @return status of operation
     */
    status_t profile_to_impulse_response(dspu::Sample *dst, const dspu::Sample *profile, size_t precision);


    /**
     * Perform trimming of impulse response file
     *
     * @param dst destination sample to store trimmed data
     * @param src non-trimmed IR file
     * @param params trimming parameters
     * @param latency the output latency of the impulse response
     * @return status of operation
     */
    status_t trim_impulse_response(
        dspu::Sample *dst,
        ssize_t *latency,
        const dspu::Sample *src,
        const irfile_t *params);

    /**
     * Convolve impulse response with the audio sample and store in another audio sample
     *
     * @param dst destination sample to store data
     * @param src source sample to convolve
     * @param ir impulse response to convolve
     * @param latency
     * @param dry the amount of dry (unprocessed signal) in gain units (1.0f = 0 dB)
     * @param wet the amount of wet (processed signal) in gain units (1.0f = 0 dB)
     * @return status of operation
     */
    status_t convolve(dspu::Sample *dst, const dspu::Sample *src, const dspu::Sample *ir, ssize_t latency, float dry, float wet);

    /**
     * Normalize sample to the specified gain
     * @param dst sample to normalize
     * @param gain the maximum peak gain
     * @param mode the normalization mode
     * @return status of operation
     */
    status_t normalize(dspu::Sample *dst, float gain, size_t mode);

    /**
     * Cut the first amount of samples from the sample file to compensate the latency
     * @param dst destination sample to process
     * @param samples number of samples to remove from beginning
     */
    void compensate_latency(dspu::Sample *dst, size_t samples);
}


#endif /* PRIVATE_AUDIO_H_ */
