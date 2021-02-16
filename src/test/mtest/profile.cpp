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
#include <private/audio.h>

#define SAMPLE_RATE         48000
#define FFT_PRECISION       16

MTEST_BEGIN("timbremill", profile)

    MTEST_MAIN
    {
        lsp::dspu::Sample s, prof;
        LSPString base, name;
        io::Path out;

        // Load the audio file and compute spectral profile
        MTEST_ASSERT(base.set_native(resources()));
        MTEST_ASSERT(name.set_ascii("samples/trumpet/trp unmuted.wav"));
        MTEST_ASSERT(timbremill::load_audio_file(&s, SAMPLE_RATE, &base, &name) == STATUS_OK);
        MTEST_ASSERT(timbremill::spectral_profile(&prof, &s, FFT_PRECISION) == STATUS_OK);
        MTEST_ASSERT(out.fmt("%s/%s-spectrum.csv", tempdir(), full_name()) > 0);

        printf("Writing result to file: %s\n", out.as_native());
        FILE *fd        = fopen(out.as_native(), "w");
        MTEST_ASSERT(fd != NULL);

        // Emit the output file
        ssize_t half    = prof.length() >> 1;
        float kf        = (SAMPLE_RATE * 0.5f) / half;
        float kn        = 1.0f / half;

        // Output header
        fprintf(fd, "frequency;");
        for (size_t j=0; j<prof.channels(); ++j)
            fprintf(fd, "channel %d;", int(j));
        fprintf(fd, "\n");

        // Output data
        for (ssize_t i=0; i<=half; ++i)
        {
            fprintf(fd, "%.3f;", i * kf);
            for (size_t j=0; j<prof.channels(); ++j)
            {
                float *c = prof.channel(j);
                fprintf(fd, "%.2f;", dspu::gain_to_db(c[i] * kn));
            }
            fprintf(fd, "\n");
        }

        // Close the file and exit
        fclose(fd);
    }

MTEST_END



