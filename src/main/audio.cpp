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

namespace timbremill
{
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
                sample->channels(), sample->sample_rate(),
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
}


