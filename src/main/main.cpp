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

#include <lsp-plug.in/common/types.h>
#include <private/config/config.h>
#include <private/config/cmdline.h>

namespace timbremill
{
    int main(int argc, const char **argv)
    {
        // Parse configuration from file and cmdline
        config_t cfg;
        status_t res = parse_cmdline(&cfg, argc, argv);
        if (res != STATUS_OK)
            return (res == STATUS_SKIP) ? STATUS_OK : res;

        return 0;
    }
}

#ifndef LSP_IDE_DEBUG
    int main(int argc, const char **argv)
    {
        return timbremill::main(argc, argv);
    }
#endif /* LSP_IDE_DEBUG */
