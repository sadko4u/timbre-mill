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
#include <lsp-plug.in/expr/Expression.h>
#include <lsp-plug.in/dsp-units/misc/windows.h>
#include <lsp-plug.in/dsp-units/misc/fade.h>
#include <lsp-plug.in/dsp-units/util/Convolver.h>

namespace timbremill
{
    using namespace lsp;

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

    typedef struct duration_t
    {
        size_t h;
        size_t m;
        size_t s;
        size_t ms;
    } duration_t;

    void calc_duration(duration_t *d, const dspu::Sample *sample)
    {
        uint64_t duration = (uint64_t(sample->samples()) * 1000) / sample->sample_rate();
        d->ms = duration % 1000;
        duration /= 1000;
        d->s = duration % 60;
        duration /= 60;
        d->m = duration % 60;
        d->h = duration / 60;
    }

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

        duration_t d;
        calc_duration(&d, sample);
        fprintf(stdout, "  loaded file: '%s', channels: %d, samples: %d, sample rate: %d, duration: %02d:%02d:%02d.%03d\n",
                path.as_native(),
                int(sample->channels()), int(sample->length()), int(sample->sample_rate()),
                int(d.h), int(d.m), int(d.s), int(d.ms)
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

    status_t save_audio_file(dspu::Sample *sample, const LSPString *base, const LSPString *fmt, expr::Resolver *vars)
    {
        status_t res;
        expr::Expression x;
        expr::value_t val;
        LSPString fname;
        io::Path path, dir;

        // Parse the expression
        if ((res = x.parse(fmt, expr::Expression::FLAG_STRING)) != STATUS_OK)
        {
            fprintf(stderr, "  invalid expression: '%s'\n", fmt->get_native());
            return STATUS_BAD_FORMAT;
        }

        // Evaluate the expression and cast to string
        expr::init_value(&val);
        x.set_resolver(vars);
        if ((res = x.evaluate(&val)) == STATUS_OK)
            res = expr::cast_string(&val);
        if (res != STATUS_OK)
        {
            expr::destroy_value(&val);
            fprintf(stderr, "  could not evaluate expression: '%s'\n", fmt->get_native());
            return STATUS_BAD_FORMAT;
        }
        fname.swap(val.v_str);
        expr::destroy_value(&val);

        // Generate file name
        if ((res = path.set(&fname)) != STATUS_OK)
        {
            fprintf(stderr, "  could not write file '%s', error code: %d\n", fname.get_native(), int(res));
            return res;
        }
        if (!path.is_absolute())
        {
            if ((res = path.set(base, &fname)) != STATUS_OK)
            {
                fprintf(stderr, "  could not write file '%s', error code: %d\n", fname.get_native(), int(res));
                return res;
            }
        }

        // Create parent directory recursively
        res = path.get_parent(&dir);
        if (res == STATUS_OK)
        {
            if ((res = dir.mkdir(true)) != STATUS_OK)
            {
                fprintf(stderr, "  could not create directory '%s', error code: %d\n", dir.as_native(), int(res));
                return res;
            }
        }
        else if (res != STATUS_NOT_FOUND)
        {
            fprintf(stderr, "  could not obtain parent directory for file '%s', error code: %d\n", fname.get_native(), int(res));
            return res;
        }

        // Load sample from file
        if ((res = sample->save(&path)) < 0)
        {
            fprintf(stderr, "  could not write file '%s', error code: %d\n", path.as_native(), int(-res));
            return -res;
        }

        duration_t d;
        calc_duration(&d, sample);
        fprintf(stdout, "  saved file: '%s', channels: %d, samples: %d, sample rate: %d, duration: %02d:%02d:%02d.%03d\n",
                path.as_native(),
                int(sample->channels()), int(sample->length()), int(sample->sample_rate()),
                int(d.h), int(d.m), int(d.s), int(d.ms)
        );

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
        size_t precision, float db_range)
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

    status_t profile_to_impulse_response(dspu::Sample *dst, const dspu::Sample *profile, size_t precision)
    {
        dspu::Sample out;

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
        out.resize(profile->channels(), bins, bins);

        // Make impulse response for each channel
        for (size_t i=0, n=out.channels(); i<n; ++i)
        {
            float *dst_chan = out.channel(i);
            const float *src_chan = profile->channel(i);

            dsp::pcomplex_r2c(fft, src_chan, bins);                 // Prepare the FFT buffer with zero phase
            dsp::packed_reverse_fft(fft, fft, precision);           // Perform reverse FFT
            dsp::pcomplex_c2r(tmp, fft, bins);                      // Convert back to real data, drop complex data which is 0
            dsp::copy(dst_chan, &tmp[half], half);                  // Make the IR linear-phase
            dsp::copy(&dst_chan[half], tmp, half);
            dsp::mul2(dst_chan, wnd, bins);                         // Apply window
        }

        // Release allocated data and return result
        dst->swap(&out);
        free_aligned(ptr);

        return STATUS_OK;
    }

    status_t trim_impulse_response(
        dspu::Sample *dst,
        ssize_t *latency,
        const dspu::Sample *src,
        const irfile_t *params)
    {
        dspu::Sample out;

        // Compute sample parameters
        ssize_t length  = src->length();
        ssize_t head    = (lsp_limit(params->fHeadCut, 0.0f, 100.0f) * 0.01f) * length;
        ssize_t tail    = (lsp_limit(params->fTailCut, 0.0f, 100.0f) * 0.01f) * length;
        ssize_t fadein  = (lsp_max(params->fFadeIn, 0.0f) * 0.01f) * length;
        ssize_t fadeout = (lsp_max(params->fFadeOut, 0.0f) * 0.01f) * length;
        ssize_t count   = lsp_max(length - head - tail, 0);

        // Initialize sample
        if (!out.init(src->channels(), count, count))
            return STATUS_NO_MEM;

        if (count > 0)
        {
            for (size_t i=0, n=src->channels(); i<n; ++i)
            {
                const float *s  = src->channel(i);
                float *d        = out.channel(i);

                // Copy data to channel and apply fades
                dsp::copy(d, &s[head], count);
                dspu::fade_in(d, d, fadein, count);
                dspu::fade_out(d, d, fadeout, count);
            }
        }

        // Save sample
        out.set_sample_rate(src->sample_rate());
        dst->swap(&out);
        *latency        = (length >> 1) - head; // Output latency of the sample

        return STATUS_OK;
    }

    status_t convolve(dspu::Sample *dst, const dspu::Sample *src, const dspu::Sample *ir, ssize_t latency, float dry, float wet)
    {
        dspu::Sample out;
        dspu::Convolver cv;

        // Allocate necessary buffers
        ssize_t dry_length  = src->length();
        ssize_t wet_length  = dry_length + ir->length(); // The length of wet (processed) signal
        ssize_t length      = (latency > 0) ?
                              lsp_max(wet_length, ssize_t(dry_length + latency)) :
                              lsp_max(ssize_t(wet_length - latency), dry_length);
        if (!out.init(src->channels(), length, length))
            return STATUS_NO_MEM;

        // Allocate buffer for convolution tail
        uint8_t *ptr;
        float *buf          = alloc_aligned<float>(ptr, wet_length);
        if (buf == NULL)
            return STATUS_NO_MEM;

        // Perform signal processing
        for (size_t i=0, n=src->channels(); i<n; ++i)
        {
            // Initialize convolver
            if (!cv.init(ir->channel(i), ir->length(), 16, 0))
            {
                free_aligned(buf);
                return STATUS_NO_MEM;
            }

            // Perform convolution
            dsp::fill_zero(buf, wet_length);
            cv.process(buf, src->channel(i), dry_length);                   // The main convolution
            cv.process(&buf[dry_length], &buf[dry_length], ir->length());   // The tail of convolution

            // Apply dry (unprocessed signal)
            float *dp       = out.channel(i);
            if (latency > 0)
                dsp::mul_k3(&dp[latency], src->channel(i), dry, dry_length);
            else
                dsp::mul_k3(dp, src->channel(i), dry, dry_length);

            // Apply wet (processed) signal
            if (latency > 0)
                dsp::fmadd_k3(dp, buf, wet, wet_length);
            else
                dsp::fmadd_k3(&dp[-latency], buf, wet, wet_length);
        }

        // Save sample
        dst->swap(&out);
        free_aligned(buf);

        return STATUS_OK;
    }

    status_t normalize(dspu::Sample *dst, float gain, size_t mode)
    {
        if (mode == NORM_NONE)
            return STATUS_OK;

        float peak  = 0.0f;
        for (size_t i=0, n=dst->channels(); i<n; ++i)
        {
            float cpeak = dsp::abs_max(dst->channel(i), dst->length());
            peak        = lsp_max(peak, cpeak);
        }

        // No peak detected?
        if (peak < 1e-6)
            return STATUS_OK;

        switch (mode)
        {
            case NORM_BELOW:
                if (peak >= gain)
                    return STATUS_OK;
                break;
            case NORM_ABOVE:
                if (peak <= gain)
                    return STATUS_OK;
                break;
            default:
                break;
        }

        // Adjust gain
        float k = gain / peak;
        for (size_t i=0, n=dst->channels(); i<n; ++i)
            dsp::mul_k2(dst->channel(i), k, dst->length());

        return STATUS_OK;
    }

    void compensate_latency(dspu::Sample *dst, size_t samples)
    {
        size_t remove = lsp_min(samples, dst->length());
        size_t length = dst->length() - remove;

        for (size_t i=0, n=dst->channels(); i<n; ++i)
        {
            float *data = dst->channel(i);
            dsp::move(data, &data[remove], length);
        }

        dst->set_length(length);
    }
}


