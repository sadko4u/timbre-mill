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


namespace timbremill
{
    using namespace lsp;

    /**
     * Load audio file
     *
     * @param sample sample to store audio data
     * @param srate desired sample rate
     * @param base base directory
     * @param name name of the file
     * @return status of operation
     */
    status_t load_audio_file(dspu::Sample *sample, size_t srate, const LSPString *base, const LSPString *name);
}


#endif /* PRIVATE_AUDIO_H_ */
