/*
 * Copyright (C) 2021 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2021 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of timbre-mill
 * Created on: 11 февр. 2021 г.
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

#ifndef PRIVATE_CONFIG_CONFIG_H_
#define PRIVATE_CONFIG_CONFIG_H_

#include <lsp-plug.in/common/types.h>
#include <lsp-plug.in/stdlib/stdio.h>
#include <lsp-plug.in/runtime/LSPString.h>
#include <lsp-plug.in/io/Path.h>
#include <lsp-plug.in/io/IInSequence.h>
#include <lsp-plug.in/io/IInStream.h>

#include <private/config/data.h>

namespace timbremill
{
    using namespace lsp;

    /**
     * Parse configuration file
     * @param cfg destination configuration to commit file
     * @param path path to location of the configuration file
     * @return status of operation
     */
    status_t parse_config(config_t *cfg, const char *path);

    /**
     * Parse configuration file
     * @param cfg destination configuration to commit file
     * @param path path to location of the configuration file
     * @return status of operation
     */
    status_t parse_config(config_t *cfg, const LSPString *path);

    /**
     * Parse configuration file
     * @param cfg destination configuration to commit file
     * @param path path to location of the configuration file
     * @return status of operation
     */
    status_t parse_config(config_t *cfg, const io::Path *path);

    /**
     * Parse configuration file from the stdio file handle
     * @param cfg destination configuration to commit file
     * @param in stdio file handle
     * @return status of operation
     */
    status_t parse_config(config_t *cfg, FILE *in);

    /**
     * Parse configuratio file from the input stream
     * @param cfg destination configuration to commit file
     * @param is input stream
     * @return
     */
    status_t parse_config(config_t *cfg, io::IInStream *is);

    /**
     * Parse configuration file from the character stream file handle
     * @param cfg destination configuration to commit file
     * @param is input character sequence
     * @return status of operation
     */
    status_t parse_config(config_t *cfg, io::IInSequence *is);

}

#endif /* PRIVATE_CONFIG_CONFIG_H_ */
