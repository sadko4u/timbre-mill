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

#include <private/config/cmdline.h>
#include <private/config/config.h>
#include <lsp-plug.in/lltl/pphash.h>
#include <lsp-plug.in/stdlib/string.h>
#include <lsp-plug.in/stdlib/stdio.h>
#include <lsp-plug.in/io/InStringSequence.h>
#include <lsp-plug.in/expr/Tokenizer.h>

namespace timbremill
{
    status_t print_usage(const char *name, bool fail)
    {
        return (fail) ? STATUS_BAD_ARGUMENTS : STATUS_SKIP;
    }

    static const char *aliases[] =
    {
        "-c",   "--config",
        "-d",   "--dst-path",
        "-h",   "--help",
        "-od",  "--out-data",
        "-oi",  "--out-ir",
        "-sr",  "--srate",
        "-s",   "--src-path",

        NULL
    };

    static const char *options[] =
    {
        "config",
        "dst-path",
        "out-data",
        "out-ir",
        "srate",
        "src-path",

        NULL
    };

    status_t parse_cmdline_int(ssize_t *dst, const char *val)
    {
        LSPString in;
        if (!in.set_native(val))
            return STATUS_NO_MEM;

        io::InStringSequence is(&in);
        expr::Tokenizer t(&is);
        ssize_t ivalue;

        switch (t.get_token(expr::TF_GET))
        {
            case expr::TT_IVALUE: ivalue = t.int_value(); break;
            default: return STATUS_INVALID_VALUE;
        }

        if (t.get_token(expr::TF_GET) != expr::TT_EOF)
            return STATUS_INVALID_VALUE;

        *dst = ivalue;

        return STATUS_OK;
    }

    status_t parse_cmdline(config_t *cfg, int argc, const char **argv)
    {
        const char *cmd = argv[0], *val;
        lltl::pphash<char, char> options;

        // Read options to hash
        for (int i=1; i < argc; )
        {
            const char *opt = argv[i++];

            // Aliases
            for (const char **p = timbremill::aliases; *p != NULL; p += 2)
                if (!strcmp(opt, p[0]))
                {
                    opt = p[1];
                    break;
                }

            // Check arguments
            const char *xopt = opt;
            if (!strcmp(opt, "--help"))
                return print_usage(cmd, false);
            else if ((opt[0] != '-') || (opt[1] != '-'))
            {
                fprintf(stderr, "Invalid argument: %s\n", opt);
                return STATUS_BAD_ARGUMENTS;
            }
            else
                xopt = opt + 2; // Remove trailing '--'

            // Parse options
            bool found = false;
            for (const char **p = timbremill::options; *p != NULL; ++p)
                if (!strcmp(xopt, *p))
                {
                    if (i >= argc)
                    {
                        fprintf(stderr, "Not defined value for option: %s\n", opt);
                        return STATUS_BAD_ARGUMENTS;
                    }

                    // Add option to settings map
                    val = argv[i++];
                    if (options.exists(xopt))
                    {
                        fprintf(stderr, "Duplicate option: %s\n", opt);
                        return STATUS_BAD_ARGUMENTS;
                    }

                    // Try to create option
                    if (!options.create(xopt, const_cast<char *>(val)))
                    {
                        fprintf(stderr, "Not enough memory\n");
                        return STATUS_NO_MEM;
                    }

                    found       = true;
                    break;
                }

            if (!found)
            {
                fprintf(stderr, "Invalid option: %s\n", opt);
                return STATUS_BAD_ARGUMENTS;
            }
        }

        // Now we are ready to read config file
        const char *cfg_name = options.get("config");
        if (cfg_name == NULL)
        {
            fprintf(stderr, "Not defined configuration file name\n");
            return STATUS_BAD_ARGUMENTS;
        }

        // Try to parse configuration file
        status_t res = parse_config(cfg, cfg_name);
        if (res != STATUS_OK)
        {
            fprintf(stderr, "Error parsing configuration file: code=%d\n", int(res));
            return res;
        }

        // Override configuration file parameters
        if ((val = options.get("out-data")) != NULL)
            cfg->sOutData.set_native(val);
        if ((val = options.get("out-ir")) != NULL)
            cfg->sOutIR.set_native(val);
        if ((val = options.get("dst-path")) != NULL)
            cfg->sDstPath.set_native(val);
        if ((val = options.get("src-path")) != NULL)
            cfg->sSrcPath.set_native(val);
        if ((val = options.get("srate")) != NULL)
        {
            if ((res = parse_cmdline_int(&cfg->nSampleRate, val)) != STATUS_OK)
            {
                fprintf(stderr, "Bad sample rate value\n");
                return res;
            }
        }

        return STATUS_OK;
    }
}



