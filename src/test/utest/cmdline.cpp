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
        UTEST_ASSERT(cfg->sSrcPath.equals_ascii("/home/user/in"));
        UTEST_ASSERT(cfg->sDstPath.equals_ascii("/home/user/out"));
        UTEST_ASSERT(cfg->sOutIR.equals_ascii("%{master_name}-${file_name} - IR.wav"));
        UTEST_ASSERT(cfg->sOutData.equals_ascii("%{master_name}-${file_name} - processed.wav"));

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

    void parce_cmdline(timbremill::config_t *cfg)
    {
        static const char *ext_argv[] =
        {
            "-d",   "/home/user/out",
            "-od",  "%{master_name}-${file_name} - processed.wav",
            "-oi",  "%{master_name}-${file_name} - IR.wav",
            "-sr",  "88200",
            "-s",   "/home/user/in",
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

    UTEST_MAIN
    {
        // Parse configuration from file and cmdline
        timbremill::config_t cfg;
        parce_cmdline(&cfg);

        // Validate the final configuration
        validate_config(&cfg);
    }

UTEST_END


