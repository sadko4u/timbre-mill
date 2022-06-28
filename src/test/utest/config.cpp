/*
 * Copyright (C) 2021 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2021 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of timbre-mill
 * Created on: 13 февр. 2021 г.
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
#include <private/config/config.h>

UTEST_BEGIN("timbremill", config)

    void validate_config(timbremill::config_t *cfg)
    {
        LSPString key;
        timbremill::fgroup_t *g;

        UTEST_ASSERT(cfg->vGroups.size() == 2);

        // Validate root parameters
        UTEST_ASSERT(cfg->nSampleRate == 44100);
        UTEST_ASSERT(cfg->nFftRank == 16);
        UTEST_ASSERT(cfg->nProduce == (timbremill::OUT_RAW | timbremill::OUT_AUDIO));
        UTEST_ASSERT(float_equals_absolute(cfg->fGainRange, 72.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->fDry, -18.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->fWet, -6.0f));
        UTEST_ASSERT(cfg->sSrcPath.equals_ascii("/home/test"));
        UTEST_ASSERT(cfg->sDstPath.equals_ascii("/home/out"));
        UTEST_ASSERT(cfg->sIR.sFile.equals_ascii("%{master_name}/test-${file_name} - IR.wav"));
        UTEST_ASSERT(cfg->sIR.sRaw.equals_ascii("%{master_name}/test-${file_name} - Raw IR.wav"));
        UTEST_ASSERT(float_equals_absolute(cfg->sIR.fHeadCut, 45.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->sIR.fTailCut, 5.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->sIR.fFadeIn, 2.0f));
        UTEST_ASSERT(float_equals_absolute(cfg->sIR.fFadeOut, 50.0f));
        UTEST_ASSERT(cfg->bMastering == true);
        UTEST_ASSERT(cfg->sFile.equals_ascii("%{master_name}/test-${file_name} - processed.wav"));
        UTEST_ASSERT(float_equals_absolute(cfg->fNormGain, -10.0f));
        UTEST_ASSERT(cfg->nNormalize == timbremill::NORM_ABOVE);
        UTEST_ASSERT(cfg->bLatencyCompensation == false);
        UTEST_ASSERT(cfg->bMatchLength == true);

        // Validate "group1"
        UTEST_ASSERT(key.set_ascii("group1"));
        UTEST_ASSERT((g = cfg->vGroups.get(&key)) != NULL);
        {
            UTEST_ASSERT(g->sMaster.equals_ascii("file1.wav"));
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
            UTEST_ASSERT(g->vFiles.size() == 2);
            UTEST_ASSERT(g->vFiles.get(0)->equals_ascii("a-out.wav"));
            UTEST_ASSERT(g->vFiles.get(1)->equals_ascii("b-out.wav"));
        }
    }


    void test_load_config(const char *filename)
    {
        io::Path path;
        timbremill::config_t cfg;

        UTEST_ASSERT(path.fmt("%s/config/%s", resources(), filename) > 0);
        printf("Testing configuration file %s...\n", path.as_native());

        UTEST_ASSERT(timbremill::parse_config(&cfg, &path) == STATUS_OK);

        validate_config(&cfg);
    }


    UTEST_MAIN
    {
        test_load_config("test.json");
    }

UTEST_END


