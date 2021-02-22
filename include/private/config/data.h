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

    enum fproduce_t
    {
        OUT_IR      = 1 << 0,
        OUT_RAW     = 1 << 1,
        OUT_AUDIO   = 1 << 2,

        OUT_ALL     = OUT_IR | OUT_RAW | OUT_AUDIO
    };

    typedef struct cfg_flag_t
    {
        const char     *name;
        ssize_t         value;
    } cfg_flag_t;

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
     * Sample cut
     */
    struct irfile_t
    {
        private:
            irfile_t & operator = (const irfile_t &);

        public:
            float                   fHeadCut;       // Head cut (%)
            float                   fTailCut;       // Tail cut (%)
            float                   fFadeIn;        // Head fade-out (%)
            float                   fFadeOut;       // Tail fade-out (%)
            LSPString               sFile;          // Format of IR file name with modifications
            LSPString               sRaw;           // Format of IR file name without modifications

        public:
            explicit irfile_t();
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
            LSPString                               sFile;          // Format of data output file name
            ssize_t                                 nSampleRate;    // Sample rate for output files
            ssize_t                                 nFftRank;       // FFT rank
            ssize_t                                 nProduce;       // List of files to produce (flags)
            float                                   fGainRange;     // Gain range (in decibels)
            irfile_t                                sIR;            // IR file data
            lltl::pphash<LSPString, fgroup_t>       vGroups;        // List of file groups

        public:
            explicit config_t();
            ~config_t();

        public:
            void clear();
    };


    /**
     * Flags for 'produce' option of the configuration
     */
    extern const cfg_flag_t     produce_flags[];

    /**
     * Find flag by given name
     * @param s name of the flag
     * @param flags list of available flags
     * @return pointer to found flag descriptor or NULL
     */
    const cfg_flag_t           *find_config_flag(const LSPString *s, const cfg_flag_t *flags);
}

#endif /* PRIVATE_CONFIG_DATA_H_ */
