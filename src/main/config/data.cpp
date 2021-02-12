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

#include <private/config/data.h>

namespace timbremill
{
    using namespace lsp;


    fgroup_t::fgroup_t()
    {
    }

    fgroup_t::~fgroup_t()
    {
        clear();
    }

    void fgroup_t::clear()
    {
        for (size_t i=0, n=vFiles.size(); i<n; ++i)
        {
            LSPString *slave = vFiles.uget(i);
            if (slave != NULL)
                delete slave;
        }

        vFiles.flush();
    }

    config_t::config_t()
    {
    }

    config_t::~config_t()
    {
        clear();
    }

    void config_t::clear()
    {
        lltl::parray<fgroup_t> groups;
        vGroups.values(&groups);
        vGroups.flush();

        for (size_t i=0, n=groups.size(); i<n; ++i)
        {
            fgroup_t *g = groups.uget(i);
            if (g != NULL)
                delete g;
        }
        groups.flush();
    }

}

