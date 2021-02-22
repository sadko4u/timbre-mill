/*
 * Copyright (C) 2021 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2021 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of timbre-mill
 * Created on: 22 февр. 2021 г.
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

#ifndef PRIVATE_CONFIG_TOOL_H_
#define PRIVATE_CONFIG_TOOL_H_

#include <lsp-plug.in/common/types.h>
#include <lsp-plug.in/runtime/LSPString.h>
#include <lsp-plug.in/expr/Variables.h>

#include <private/config/config.h>

namespace timbremill
{
    status_t build_variables(expr::Variables *vars, config_t *cfg, fgroup_t *fg, LSPString *master, LSPString *child);

    status_t process_file_group(config_t *cfg, fgroup_t *fg);

    status_t process_file_groups(config_t *cfg);

    int main(int argc, const char **argv);
}

#endif /* PRIVATE_CONFIG_TOOL_H_ */
