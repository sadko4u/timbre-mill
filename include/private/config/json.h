/*
 * Copyright (C) 2021 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2021 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of timbre-mill
 * Created on: 10 февр. 2021 г.
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

#ifndef PRIVATE_CONFIG_JSON_H_
#define PRIVATE_CONFIG_JSON_H_

#include <private/config/config.h>
#include <lsp-plug.in/io/IInSequence.h>

namespace timbremill
{
    using namespace lsp;

    /**
     * Parse configuration file in JSON format
     *
     * @param cfg configuration to update
     * @param is input stream
     * @return status of operation
     */
    status_t parse_json_config(config_t *cfg, io::IInSequence *is);

}

#endif /* PRIVATE_CONFIG_JSON_H_ */
