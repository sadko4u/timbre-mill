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
#include <lsp-plug.in/dsp/dsp.h>
#include <lsp-plug.in/dsp-units/sampling/Sample.h>

#include <private/config/config.h>
#include <private/config/cmdline.h>
#include <private/audio.h>

namespace timbremill
{
    status_t process_file_group(config_t *cfg, fgroup_t *fg)
    {
        dspu::Sample master;
        status_t res;

        // Analyze group settings
        if (fg->sMaster.is_empty())
        {
            fprintf(stdout, "  group '%s' does not have master file, skipping\n");
            return STATUS_OK;
        }
        else if (fg->vFiles.is_empty())
        {
            fprintf(stdout, "  group '%s' does not have any child files, skipping\n");
            return STATUS_OK;
        }

        // Read the master file
        if ((res = load_audio_file(&master, cfg->nSampleRate, &cfg->sSrcPath, &fg->sMaster)) != STATUS_OK)
            return res;

        for (size_t i=0, n=fg->vFiles.size(); i<n; ++i)
        {
            dspu::Sample child;
            LSPString *fname = fg->vFiles.uget(i);
            if (fname == NULL)
                return STATUS_UNKNOWN_ERR;

            // Load the child file
            if ((res = load_audio_file(&child, cfg->nSampleRate, &cfg->sSrcPath, fname)) != STATUS_OK)
                return res;
        }

        return STATUS_OK;
    }

    status_t process_file_groups(config_t *cfg)
    {
        lltl::parray<LSPString> gnames;
        if (!cfg->vGroups.keys(&gnames))
            return STATUS_NO_MEM;

        for (size_t i=0, n=gnames.size(); i<n; ++i)
        {
            LSPString *gname = gnames.uget(i);
            if (gname == NULL)
                return STATUS_NO_MEM;

            fgroup_t *fg = cfg->vGroups.get(gname);
            if (fg == NULL)
                return STATUS_UNKNOWN_ERR;

            printf("processing group '%s'...\n", gname->get_native());

            status_t res = process_file_group(cfg, fg);
            if (res != STATUS_OK)
                return res;
        }

        return STATUS_OK;
    }

    int main(int argc, const char **argv)
    {
        // Parse configuration from file and cmdline
        config_t cfg;
        status_t res = parse_cmdline(&cfg, argc, argv);
        if (res != STATUS_OK)
            return (res == STATUS_SKIP) ? STATUS_OK : res;

        // Perform data processing
        dsp::context_t ctx;
        dsp::init();
        dsp::start(&ctx);
        res = process_file_groups(&cfg);
        dsp::finish(&ctx);

        // Analyze result
        if (res != STATUS_OK)
            return res;

        return 0;
    }
}

#ifndef LSP_IDE_DEBUG
    int main(int argc, const char **argv)
    {
        return timbremill::main(argc, argv);
    }
#endif /* LSP_IDE_DEBUG */
