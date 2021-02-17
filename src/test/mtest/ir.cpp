/*
 * Copyright (C) 2021 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2021 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of timbre-mill
 * Created on: 17 февр. 2021 г.
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

#include <lsp-plug.in/test-fw/mtest.h>
#include <lsp-plug.in/stdlib/stdio.h>
#include <lsp-plug.in/dsp-units/units.h>
#include <lsp-plug.in/dsp/dsp.h>
#include <private/audio.h>

#define SAMPLE_RATE         48000
#define FFT_PRECISION       16

MTEST_BEGIN("timbremill", ir)

    MTEST_MAIN
    {
        lsp::dspu::Sample s, pu, pp, ir;
        LSPString base, name;
        io::Path out;

        // Load the 'unmuted' audio file and compute spectral profile
        MTEST_ASSERT(base.set_native(resources()));
        MTEST_ASSERT(name.set_ascii("samples/trumpet/trp unmuted.wav"));
        MTEST_ASSERT(timbremill::load_audio_file(&s, SAMPLE_RATE, &base, &name) == STATUS_OK);
        MTEST_ASSERT(timbremill::spectral_profile(&pu, &s, FFT_PRECISION) == STATUS_OK);

        // Load the 'plunger' audio file and compute spectral profile
        MTEST_ASSERT(base.set_native(resources()));
        MTEST_ASSERT(name.set_ascii("samples/trumpet/trp plunger.wav"));
        MTEST_ASSERT(timbremill::load_audio_file(&s, SAMPLE_RATE, &base, &name) == STATUS_OK);
        MTEST_ASSERT(timbremill::spectral_profile(&pp, &s, FFT_PRECISION) == STATUS_OK);

        // Compute the impulse response
        MTEST_ASSERT(pu.channels() == pp.channels());
        MTEST_ASSERT(pu.length() == pp.length());
        MTEST_ASSERT(timbremill::timbre_impulse_response(&ir, &pu, &pp, FFT_PRECISION, 48.0f) == STATUS_OK);

        // Save the impulse response
        MTEST_ASSERT(out.fmt("%s/%s-ir.wav", tempdir(), full_name()) > 0);
        ir.set_sample_rate(SAMPLE_RATE);
        printf("Writing IR to file: %s\n", out.as_native());
        MTEST_ASSERT(ir.save(&out) == ssize_t(ir.length()));

        // Open output file
        MTEST_ASSERT(out.fmt("%s/%s-osc.csv", tempdir(), full_name()) > 0);
        printf("Dumping oscillogram to file: %s\n", out.as_native());
        FILE *fd        = fopen(out.as_native(), "w");
        MTEST_ASSERT(fd != NULL);

        // Output header
        fprintf(fd, "sample;");
        for (size_t j=0; j<ir.channels(); ++j)
            fprintf(fd, "ir channel %d;", int(j));
        for (size_t j=0; j<ir.channels(); ++j)
            fprintf(fd, "ir channel %d db;", int(j));
        fprintf(fd, "\n");

        // Output data
        for (size_t i=0; i <= ir.length(); ++i)
        {
            fprintf(fd, "%d;", int(i));
            for (size_t j=0; j<ir.channels(); ++j)
            {
                float *c = ir.channel(j);
                fprintf(fd, "%g;", c[i]);
            }
            for (size_t j=0; j<ir.channels(); ++j)
            {
                float *c = ir.channel(j);
                fprintf(fd, "%.2f;", dspu::gain_to_db(fabs(c[i])));
            }
            fprintf(fd, "\n");
        }

        // Close the file and exit
        fclose(fd);
    }

MTEST_END



