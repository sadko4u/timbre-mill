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

#include <lsp-plug.in/test-fw/utest.h>
#include <lsp-plug.in/test-fw/helpers.h>
#include <lsp-plug.in/common/status.h>
#include <lsp-plug.in/lltl/parray.h>
#include <lsp-plug.in/io/Path.h>
#include <private/config/cmdline.h>

UTEST_BEGIN("timbremill", cmdline)

    void validate_config(timbremill::config_t *cfg)
    {
        LSPString key;
        timbremill::fgroup_t *g;

        UTEST_ASSERT(cfg->vGroups.size() == 2);

        // Validate root parameters
        UTEST_ASSERT(cfg->nSampleRate == 88200);
        UTEST_ASSERT(cfg->nFftRank == 8);
        UTEST_ASSERT(cfg->nProduce == (timbremill::OUT_IR | timbremill::OUT_AUDIO));
        UTEST_ASSERT(float_equals_absolute(cfg->fDry, -19.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->fWet, -7.0f));
        UTEST_ASSERT(cfg->sSrcPath.equals_ascii("/home/user/in"));
        UTEST_ASSERT(cfg->sDstPath.equals_ascii("/home/user/out"));
        UTEST_ASSERT(cfg->sIR.sFile.equals_ascii("%{master_name}-${file_name} - IR.wav"));
        UTEST_ASSERT(cfg->sIR.sRaw.equals_ascii("%{master_name}-${file_name} - Raw IR.wav"));
        UTEST_ASSERT(float_equals_absolute(cfg->sIR.fHeadCut, 46.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->sIR.fTailCut, 6.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->sIR.fFadeIn, 3.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->sIR.fFadeOut, 51.0f));
        UTEST_ASSERT(cfg->bMastering == true);
        UTEST_ASSERT(cfg->sFile.equals_ascii("%{master_name}-${file_name} - processed.wav"));
        UTEST_ASSERT(float_equals_absolute(cfg->fNormGain, -12.0f));
        UTEST_ASSERT(cfg->nNormalize == timbremill::NORM_ALWAYS);
        UTEST_ASSERT(cfg->bLatencyCompensation == false);
        UTEST_ASSERT(cfg->bMatchLength == true);

        // Validate "group1"
        UTEST_ASSERT(key.set_ascii("group1"));
        UTEST_ASSERT((g = cfg->vGroups.get(&key)) != NULL);
        {
            UTEST_ASSERT(g->sMaster.equals_ascii("file1.wav"));
            UTEST_ASSERT(g->sName.equals_ascii("group1"));
            UTEST_ASSERT(g->vFiles.size() == 3);
            UTEST_ASSERT(g->vFiles.get(0)->equals_ascii("out-file1.wav"));
            UTEST_ASSERT(g->vFiles.get(1)->equals_ascii("out-file2.wav"));
            UTEST_ASSERT(g->vFiles.get(2)->equals_ascii("out-file3.wav"));
        }

        // Validate "group1"
        UTEST_ASSERT(key.set_ascii("group2"));
        UTEST_ASSERT((g = cfg->vGroups.get(&key)) != NULL);
        {
            UTEST_ASSERT(g->sMaster.equals_ascii("a.wav"));
            UTEST_ASSERT(g->sName.equals_ascii("group2"));
            UTEST_ASSERT(g->vFiles.size() == 2);
            UTEST_ASSERT(g->vFiles.get(0)->equals_ascii("a-out.wav"));
            UTEST_ASSERT(g->vFiles.get(1)->equals_ascii("b-out.wav"));
        }
    }

    void parse_cmdline(timbremill::config_t *cfg)
    {
        static const char *ext_argv[] =
        {
            "-d",   "/home/user/out",
            "-f",   "%{master_name}-${file_name} - processed.wav",
            "-p",   "ir, audio",
            "-fr",  "8",
            "-ir",  "%{master_name}-${file_name} - IR.wav",
            "-iw",  "%{master_name}-${file_name} - Raw IR.wav",
            "-ihc", "46",
            "-itc", "6",
            "-ifi", "3",
            "-ifo", "51",
            "-sr",  "88200",
            "-s",   "/home/user/in",
            "-dg",  "-19",
            "-wg",  "-7",
            "-m",   "true",
            "-ng",  "-12",
            "-n",   "ALWAYS",
            "-ml",  "true",
            "-c",
            NULL
        };

        io::Path path;
        UTEST_ASSERT(path.fmt("%s/config/test.json", resources()) > 0);

        lltl::parray<char> argv;
        UTEST_ASSERT(argv.add(const_cast<char *>(full_name())));
        for (const char **pv = ext_argv; *pv != NULL; ++pv)
        {
            UTEST_ASSERT(argv.add(const_cast<char *>(*pv)));
        }

        UTEST_ASSERT(argv.add(const_cast<char *>(path.as_native())));
        status_t res = timbremill::parse_cmdline(cfg, argv.size(), const_cast<const char **>(argv.array()));
        UTEST_ASSERT(res == STATUS_OK);
    }

    void parse_alt_cmdline(timbremill::config_t *cfg)
    {
        static const char *ext_argv[] =
        {
            "-g",   "test-group",
            "-mf",  "master-file.wav",
            "-cf",  "child-file1.wav",
            "-cf",  "child-file2.wav",
            "-f",   "out-file.wav",
            "-lc",  "true",
            NULL
        };

        lltl::parray<char> argv;
        UTEST_ASSERT(argv.add(const_cast<char *>(full_name())));
        for (const char **pv = ext_argv; *pv != NULL; ++pv)
        {
            UTEST_ASSERT(argv.add(const_cast<char *>(*pv)));
        }

        status_t res = timbremill::parse_cmdline(cfg, argv.size(), const_cast<const char **>(argv.array()));
        UTEST_ASSERT(res == STATUS_OK);
    }

    void validate_alt_config(timbremill::config_t *cfg)
    {
        LSPString key;
        timbremill::fgroup_t *g;

        UTEST_ASSERT(cfg->vGroups.size() == 1);

        // Validate root parameters
        UTEST_ASSERT(cfg->nSampleRate == 48000);
        UTEST_ASSERT(cfg->nFftRank == 12);
        UTEST_ASSERT(cfg->nProduce == timbremill::OUT_AUDIO);
        UTEST_ASSERT(float_equals_absolute(cfg->fDry, -1000.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->fWet, 0.0f));
        UTEST_ASSERT(cfg->sSrcPath.equals_ascii(""));
        UTEST_ASSERT(cfg->sDstPath.equals_ascii(""));
        UTEST_ASSERT(cfg->sIR.sFile.equals_ascii("${master_name}/${file_name} - IR.wav"));
        UTEST_ASSERT(cfg->sIR.sRaw.equals_ascii("${master_name}/${file_name} - Raw IR.wav"));
        UTEST_ASSERT(float_equals_absolute(cfg->sIR.fHeadCut, 0.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->sIR.fTailCut, 0.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->sIR.fFadeIn, 0.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->sIR.fFadeOut, 0.0f));
        UTEST_ASSERT(cfg->bMastering == false);
        UTEST_ASSERT(cfg->sFile.equals_ascii("out-file.wav"));
        UTEST_ASSERT(float_equals_absolute(cfg->fNormGain, 0.0f));
        UTEST_ASSERT(cfg->nNormalize == timbremill::NORM_NONE);
        UTEST_ASSERT(cfg->bLatencyCompensation == true);
        UTEST_ASSERT(cfg->bMatchLength == false);

        // Validate "test-group"
        UTEST_ASSERT(key.set_ascii("test-group"));
        UTEST_ASSERT((g = cfg->vGroups.get(&key)) != NULL);
        {
            UTEST_ASSERT(g->sMaster.equals_ascii("master-file.wav"));
            UTEST_ASSERT(g->sName.equals_ascii("test-group"));
            UTEST_ASSERT(g->vFiles.size() == 2);
            UTEST_ASSERT(g->vFiles.get(0)->equals_ascii("child-file1.wav"));
            UTEST_ASSERT(g->vFiles.get(1)->equals_ascii("child-file2.wav"));
        }
    }

    UTEST_MAIN
    {
        // Parse configuration from file and cmdline
        {
            timbremill::config_t cfg;
            parse_cmdline(&cfg);
            validate_config(&cfg);
        }

        // Parse alternative configuration cmdline
        {
            timbremill::config_t cfg;
            parse_alt_cmdline(&cfg);
            validate_alt_config(&cfg);
        }
    }

UTEST_END


