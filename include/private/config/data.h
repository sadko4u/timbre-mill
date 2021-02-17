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

#ifndef PRIVATE_CONFIG_DATA_H_
#define PRIVATE_CONFIG_DATA_H_

#include <lsp-plug.in/common/types.h>
#include <lsp-plug.in/lltl/parray.h>
#include <lsp-plug.in/lltl/pphash.h>
#include <lsp-plug.in/runtime/LSPString.h>

namespace timbremill
{
    using namespace lsp;

    /**
     * File group
     */
    struct fgroup_t
    {
        private:
            fgroup_t & operator = (const fgroup_t &);

        public:
            LSPString               sName;
            LSPString               sMaster;
            lltl::parray<LSPString> vFiles;

        public:
            explicit fgroup_t();
            ~fgroup_t();

        public:
            void clear();
    };

    /**
     * Overall configuration
     */
    struct config_t
    {
        private:
            config_t & operator = (const config_t &);

        public:
            LSPString                               sSrcPath;       // Source path (for source files)
            LSPString                               sDstPath;       // Destination path (for destination files)
            LSPString                               sOutIR;         // Format of IR output file name
            LSPString                               sOutData;       // Format of data output file name
            ssize_t                                 nSampleRate;    // Sample rate for output files
            float                                   fGainRange;     // Gain range (in decibels)
            lltl::pphash<LSPString, fgroup_t>       vGroups;        // List of file groups

        public:
            explicit config_t();
            ~config_t();

        public:
            void clear();
    };

}

#endif /* PRIVATE_CONFIG_DATA_H_ */
