/*
 * Copyright (C) 2021 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2021 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of timbre-mill
 * Created on: 15 февр. 2021 г.
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
#include <lsp-plug.in/lltl/parray.h>

namespace timbremill
{
    int main(int argc, const char **argv);
}

MTEST_BEGIN("timbremill", main)

    MTEST_MAIN
    {
        lltl::parray<char> args;
        args.add(const_cast<char *>(full_name()));

        for (ssize_t i=0; i < argc; ++i)
            args.add(const_cast<char *>(argv[i]));

        timbremill::main(args.size(), const_cast<const char **>(args.array()));
    }

MTEST_END



