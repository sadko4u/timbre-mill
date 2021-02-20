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
#include <lsp-plug.in/expr/Variables.h>

#include <private/config/config.h>
#include <private/config/cmdline.h>
#include <private/audio.h>

#define FFT_MIN     8
#define FFT_MAX     16

namespace timbremill
{
    status_t build_variables(expr::Variables *vars, config_t *cfg, fgroup_t *fg, LSPString *master, LSPString *child)
    {
        io::Path path;
        LSPString value;
        status_t res;

        // Clear variables
        vars->clear();

        // Common variables
        if ((res = vars->set_int("srate", cfg->nSampleRate)) != STATUS_OK)
            return res;
        if ((res = vars->set_string("group", &fg->sName)) != STATUS_OK)
            return res;

        // Parse master file name
        if ((res = path.set(master)) != STATUS_OK)
            return res;

        if ((res = path.get_last(&value)) != STATUS_OK)
            return res;
        if ((res = vars->set_string("master", &value)) != STATUS_OK)
            return res;
        if ((res = path.get_ext(&value)) != STATUS_OK)
            return res;
        if ((res = vars->set_string("master_ext", &value)) != STATUS_OK)
            return res;
        if ((res = path.get_noext(&value)) != STATUS_OK)
            return res;
        if ((res = vars->set_string("master_name", &value)) != STATUS_OK)
            return res;

        // Parse child file name
        if ((res = path.set(child)) != STATUS_OK)
            return res;

        if ((res = path.get_last(&value)) != STATUS_OK)
            return res;
        if ((res = vars->set_string("file", &value)) != STATUS_OK)
            return res;
        if ((res = path.get_ext(&value)) != STATUS_OK)
            return res;
        if ((res = vars->set_string("file_ext", &value)) != STATUS_OK)
            return res;
        if ((res = path.get_noext(&value)) != STATUS_OK)
            return res;
        if ((res = vars->set_string("file_name", &value)) != STATUS_OK)
            return res;

        return STATUS_OK;
    }

    status_t process_file_group(config_t *cfg, fgroup_t *fg)
    {
        dspu::Sample master, mp;
        expr::Variables vars;
        status_t res;
        ssize_t fft_rank    = lsp_limit(cfg->nFftRank, FFT_MIN, FFT_MAX);

        // Analyze group settings
        if (fg->sMaster.is_empty())
        {
            fprintf(stdout, "  group '%s' does not have master file, skipping\n", fg->sName.get_native());
            return STATUS_OK;
        }
        else if (fg->vFiles.is_empty())
        {
            fprintf(stdout, "  group '%s' does not have any child files, skipping\n", fg->sName.get_native());
            return STATUS_OK;
        }

        // Read the master file
        if ((res = load_audio_file(&master, cfg->nSampleRate, &cfg->sSrcPath, &fg->sMaster)) != STATUS_OK)
            return res;

        // Compute the audio profile for master
        if ((res = spectral_profile(&mp, &master, fft_rank)) != STATUS_OK)
        {
            fprintf(stderr, "  error computing spectral profile for the master file '%s'\n", fg->sName.get_native());
            return res;
        }

        for (size_t i=0, n=fg->vFiles.size(); i<n; ++i)
        {
            dspu::Sample child, cp, ir, raw_ir;
            LSPString *fname = fg->vFiles.uget(i);
            if (fname == NULL)
            {
                fprintf(stderr, "  internal error\n");
                return STATUS_UNKNOWN_ERR;
            }

            // Build variables
            if ((res = build_variables(&vars, cfg, fg, &fg->sMaster, fname)) != STATUS_OK)
            {
                fprintf(stderr, "  error building pattern variables\n");
                return res;
            }

            // Load the child file
            if ((res = load_audio_file(&child, cfg->nSampleRate, &cfg->sSrcPath, fname)) != STATUS_OK)
                return res;
            if (child.channels() != master.channels())
            {
                fprintf(stderr, "  number of channels mimatch: %d (master) vs %d (child), leaving\n",
                        int(master.channels()), int(child.channels()));
                return res;
            }

            // Compute the spectral profile for the child file
            if ((res = spectral_profile(&cp, &child, fft_rank)) != STATUS_OK)
            {
                fprintf(stderr, "  error computing spectral profile for the child file '%s'\n", fname->get_native());
                return res;
            }

            // Compute the impulse response of the file
            if ((res = timbre_impulse_response(&raw_ir, &mp, &cp, fft_rank, cfg->fGainRange)) != STATUS_OK)
            {
                fprintf(stderr, "  error computing raw impulse response for the child file '%s'\n", fname->get_native());
                return res;
            }

            // Produce the trimmed IR file
            if ((res = trim_impulse_response(&ir, &raw_ir, &cfg->sIR)) != STATUS_OK)
            {
                fprintf(stderr, "  error trimming impulse response\n");
                return res;
            }

            // Save the IR file
            raw_ir.set_sample_rate(cfg->nSampleRate);
            if ((res = save_audio_file(&raw_ir, &cfg->sDstPath, &cfg->sIR.sRaw, &vars)) != STATUS_OK)
                return res;

            // Save the trimmed IR file
            ir.set_sample_rate(cfg->nSampleRate);
            if ((res = save_audio_file(&ir, &cfg->sDstPath, &cfg->sIR.sFile, &vars)) != STATUS_OK)
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
