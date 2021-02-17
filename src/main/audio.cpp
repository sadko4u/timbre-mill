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

#include <private/audio.h>
#include <lsp-plug.in/stdlib/stdio.h>
#include <lsp-plug.in/common/alloc.h>
#include <lsp-plug.in/dsp/dsp.h>
#include <lsp-plug.in/dsp-units/misc/windows.h>

namespace timbremill
{
    typedef struct spc_calc_t
    {
        float  *buf;        // Buffer
        float  *tmp;        // Temporary data
        float  *wnd;        // Window
        float  *fft;        // FFT buffer

        float  *spc;        // Output spectral data

        size_t  bins;       // Number of bins
        size_t  radix;      // FFT radix
    } spc_calc_t;

    status_t load_audio_file(dspu::Sample *sample, size_t srate, const LSPString *base, const LSPString *name)
    {
        status_t res;
        io::Path path;

        // Generate file name
        if ((res = path.set(name)) != STATUS_OK)
        {
            fprintf(stderr, "  could not read file '%s', error code: %d\n", name->get_native(), int(res));
            return res;
        }
        if (!path.is_absolute())
        {
            if ((res = path.set(base, name)) != STATUS_OK)
            {
                fprintf(stderr, "  could not read file '%s', error code: %d\n", name->get_native(), int(res));
                return res;
            }
        }

        // Load sample from file
        if ((res = sample->load(&path)) != STATUS_OK)
        {
            fprintf(stderr, "  could not read file '%s', error code: %d\n", path.as_native(), int(res));
            return res;
        }

        uint64_t duration = (uint64_t(sample->samples()) * 1000) / sample->sample_rate();
        size_t ms = duration % 1000;
        duration /= 1000;
        size_t s = duration % 60;
        duration /= 60;
        size_t m = duration % 60;
        size_t h = duration / 60;

        fprintf(stdout, "  loaded file: '%s', channels: %d, sample rate: %d, duration: %02d:%02d:%02d.%03d\n",
                path.as_native(),
                int(sample->channels()), int(sample->sample_rate()),
                int(h), int(m), int(s), int(ms)
        );

        // Resample audio data
        if ((res = sample->resample(srate)) != STATUS_OK)
        {
            fprintf(stderr, "  could not resample file '%s' to sample rate %d, error code: %d\n",
                    path.as_native(), int(srate), int(res)
            );
            return res;
        }

        return STATUS_OK;
    }

    void compute_spectrum_step(spc_calc_t *calc)
    {
        dsp::mul3(calc->tmp, calc->buf, calc->wnd, calc->bins);
        dsp::pcomplex_r2c(calc->fft, calc->tmp, calc->bins);
        dsp::packed_direct_fft(calc->fft, calc->fft, calc->radix);
        dsp::pcomplex_mod(calc->tmp, calc->fft, calc->bins);
        dsp::add2(calc->spc, calc->tmp, calc->bins);
    }

    status_t compute_spectrum(spc_calc_t *calc, dspu::Sample *out, const float *src, size_t length)
    {
        // Initialize data
        dsp::fill_zero(calc->buf, calc->bins);
        dsp::fill_zero(calc->tmp, calc->bins);
        dsp::fill_zero(calc->spc, calc->bins);
        dsp::fill_zero(calc->fft, calc->bins);
        dspu::windows::blackman_nuttall(calc->wnd, calc->bins);

        // Process the data with half-sized chunks
        size_t offset = 0, half = calc->bins >> 1, steps = 0;
        while (offset < length)
        {
            // Fill the buffer with data
            size_t to_process   = lsp_min(length - offset, half);
            dsp::move(calc->buf, &calc->buf[half], half);
            dsp::copy(&calc->buf[half], &src[offset], to_process);
            if (to_process < half)
                dsp::fill_zero(&calc->buf[calc->bins - to_process], calc->bins - to_process);

            // Apply the window function and convert into complex numbers
            compute_spectrum_step(calc);

            // Update position
            offset     += to_process;
            steps      += 1;
        }

        // Do the last step
        dsp::move(calc->buf, &calc->buf[half], half);
        dsp::fill_zero(&calc->buf[half], half);
        compute_spectrum_step(calc);
        steps      += 1;

        // Compute the average spectrum at the output
        dsp::mul_k2(calc->spc, 1.0f / steps, calc->bins);

        return STATUS_OK;
    }

    status_t spectral_profile(dspu::Sample *profile, const dspu::Sample *src, size_t precision)
    {
        dspu::Sample out;
        spc_calc_t calc;
        status_t res;

        uint8_t *ptr    = NULL;
        size_t bins     = 1 << precision;

        // Allocate the buffers for processing
        size_t to_alloc = bins * 3 + bins * 2; // buf + tmp + spc + wnd + fft
        calc.buf        = alloc_aligned<float>(ptr, to_alloc, 64);
        if (calc.buf == NULL)
            return STATUS_NO_MEM;

        calc.tmp        = &calc.buf[bins];
        calc.wnd        = &calc.tmp[bins];
        calc.fft        = &calc.wnd[bins];
        calc.spc        = NULL;

        // Allocate the sample data
        if (!out.init(src->channels(), bins, bins))
        {
            free_aligned(ptr);
            return STATUS_NO_MEM;
        }

        // Now we can estimate the spectrum data for each channel
        for (size_t i=0, n=src->channels(); i<n; ++i)
        {
            calc.spc        = out.channel(i);
            calc.bins       = bins;
            calc.radix      = precision;

            res = compute_spectrum(&calc, &out, src->channel(i), src->length());
            if (res != STATUS_OK)
            {
                free_aligned(ptr);
                return res;
            }
        }

        // Release allocated data and return result
        profile->swap(&out);
        free_aligned(ptr);

        return STATUS_OK;
    }

    status_t timbre_impulse_response(
            dspu::Sample *dst,
            const dspu::Sample *master, const dspu::Sample *child,
            size_t precision, float db_range
    )
    {
        dspu::Sample out;
        status_t res;

        // Check sizes
        if (master->samples() != child->samples())
        {
            fprintf(stderr, "  The lenghts of audio profiles differ\n");
            return STATUS_BAD_ARGUMENTS;
        }
        if (master->channels() != child->channels())
        {
            fprintf(stderr, "  The number of channels of audio profiles differ\n");
            return STATUS_BAD_ARGUMENTS;
        }

        // Copy the data from child sample to the output sample
        if ((res = out.copy(child)) != STATUS_OK)
        {
            fprintf(stderr, "  Error initializing the sample data\n");
            return STATUS_BAD_ARGUMENTS;
        }

        // Process each channel of the samples
        uint8_t *ptr    = NULL;
        size_t bins     = 1 << precision;
        size_t half     = bins >> 1;

        // Allocate the buffers for processing
        size_t to_alloc = bins * 2 + bins * 2; // fft + tmp + wnd
        float *fft      = alloc_aligned<float>(ptr, to_alloc, 64);
        float *tmp      = &fft[bins * 2];
        float *wnd      = &tmp[bins];
        if (fft == NULL)
            return STATUS_NO_MEM;

        dspu::windows::blackman_nuttall(wnd, bins);

        // Make impulse response for each channel
        for (size_t i=0, n=out.channels(); i<n; ++i)
        {
            float *chan = out.channel(i);
            dsp::div2(chan, master->channel(i), out.samples());     // Compute reverse specrum characterisic

            dsp::pcomplex_r2c(fft, chan, bins);                     // Prepare the FFT buffer with zero phase
            dsp::packed_reverse_fft(fft, fft, precision);           // Perform reverse FFT
            dsp::pcomplex_c2r(tmp, fft, bins);                      // Convert back to real data, drop complex data which is 0
            dsp::copy(chan, &tmp[half], half);                      // Make the IR linear-phase
            dsp::copy(&chan[half], tmp, half);
            dsp::mul2(chan, wnd, bins);                             // Apply window
        }

        // Release allocated data and return result
        dst->swap(&out);
        free_aligned(ptr);

        return STATUS_OK;
    }

}


