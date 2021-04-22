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

#include <lsp-plug.in/lltl/pphash.h>
#include <lsp-plug.in/stdlib/string.h>
#include <lsp-plug.in/stdlib/stdio.h>
#include <lsp-plug.in/io/InStringSequence.h>
#include <lsp-plug.in/expr/Tokenizer.h>

#include <private/config/data.h>
#include <private/config/cmdline.h>
#include <private/config/config.h>

namespace timbremill
{
    static const char *options[] =
    {
        "-c",   "--config",         "Configuration file name (required)",
        "-d",   "--dst-path",       "Destination path to store audio files",
        "-dg",  "--dry",            "The amount (in dB) of unprocessed signal in output file",
        "-f",   "--file",           "Format of the output file name",
        "-fr",  "--fft-rank",       "The FFT rank (resolution) used for profiling",
        "-h",   "--help",           "Output this help message",
        "-ir",  "--ir-file",        "Format of the processed impulse response file name",
        "-iw",  "--ir-raw",         "Format of the raw impulse response file name",
        "-ifi", "--ir-fade-in",     "The amount (in %) of fade-in for the IR file",
        "-ifo", "--ir-fade-out",    "The amount (in %) of fade-out for the IR file",
        "-ihc", "--ir-head-cut",    "The amount (in %) of head cut for the IR file",
        "-itc", "--ir-tail-cut",    "The amount (in %) of tail cut for the IR file",
        "-m",   "--mastering",      "Work as auto-mastering tool instead of timbral correction",
        "-n",   "--normalize",      "Set normalization mode",
        "-ng",  "--norm-gain",      "Set normalization peak gain (in dB)",
        "-p",   "--produce",        "Comma-separated list of produced output files (ir,raw,audio,all)",
        "-s",   "--src-path",       "Source path to take files from",
        "-sr",  "--srate",          "Sample rate of output files",
        "-wg",  "--wet",            "The amount (in dB) of processed signal in output file",

        NULL
    };

    status_t parse_cmdline_flags(ssize_t *dst, const char *name, const char *value, const cfg_flag_t *flags)
    {
        // Parse values
        size_t n = 0;
        io::InStringSequence is;
        expr::Tokenizer tok(&is);
        status_t res = STATUS_OK;
        size_t xdst  = 0;
        const cfg_flag_t *xf;

        if ((res = is.wrap(value)) != STATUS_OK)
        {
            fprintf(stderr, "Error parsing argument '%s', error code: %d\n", name, int(res));
            return res;
        }

        while ((res = tok.get_token(expr::TF_GET | expr::TF_XKEYWORDS)) != expr::TT_EOF)
        {
            if (n > 0)
            {
                if (tok.current() != expr::TT_COMMA)
                {
                    fprintf(stderr, "Argument '%s': invalid syntax\n", name);
                    return STATUS_BAD_FORMAT;
                }
                if ((res = tok.get_token(expr::TF_GET | expr::TF_XKEYWORDS)) == expr::TT_EOF)
                {
                    fprintf(stderr, "Argument '%s': invalid syntax\n", name);
                    return STATUS_BAD_FORMAT;
                }
            }

            if (tok.current() != expr::TT_BAREWORD)
            {
                fprintf(stderr, "Argument '%s': invalid syntax\n", name);
                return STATUS_BAD_FORMAT;
            }

            if ((xf = find_config_flag(tok.text_value(), flags)) == NULL)
            {
                fprintf(stderr, "Argument '%s': unknown value '%s'\n", name, tok.text_value()->get_native());
                return STATUS_BAD_FORMAT;
            }

            xdst   |= xf->value;
            ++n;
        }

        *dst    = xdst;

        return STATUS_OK;
    }

    status_t print_usage(const char *name, bool fail)
    {
        LSPString buf, fmt;
        size_t maxlen = 0;

        // Estimate maximum parameter size
        for (const char **p = timbremill::options; *p != NULL; p += 3)
        {
            buf.fmt_ascii("%s, %s", p[0], p[1]);
            maxlen  = lsp_max(buf.length(), maxlen);
        }
        fmt.fmt_ascii("  %%-%ds    %%s\n", int(maxlen));

        // Output usage
        printf("usage: %s [arguments]\n", name);
        printf("available arguments:\n");
        for (const char **p = timbremill::options; *p != NULL; p += 3)
        {
            buf.fmt_ascii("%s, %s", p[0], p[1]);
            printf(fmt.get_native(), buf.get_native(), p[2]);
        }

        return (fail) ? STATUS_BAD_ARGUMENTS : STATUS_SKIP;
    }

    status_t parse_cmdline_int(ssize_t *dst, const char *val, const char *parameter)
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
            default:
                fprintf(stderr, "Bad '%s' value\n", parameter);
                return STATUS_INVALID_VALUE;
        }

        if (t.get_token(expr::TF_GET) != expr::TT_EOF)
        {
            fprintf(stderr, "Bad '%s' value\n", parameter);
            return STATUS_INVALID_VALUE;
        }

        *dst = ivalue;

        return STATUS_OK;
    }

    status_t parse_cmdline_float(float *dst, const char *val, const char *parameter)
    {
        LSPString in;
        if (!in.set_native(val))
        {
            fprintf(stderr, "Out of memory\n");
            return STATUS_NO_MEM;
        }

        io::InStringSequence is(&in);
        expr::Tokenizer t(&is);
        float fvalue;

        switch (t.get_token(expr::TF_GET))
        {
            case expr::TT_IVALUE: fvalue = t.int_value(); break;
            case expr::TT_FVALUE: fvalue = t.float_value(); break;
            default:
                fprintf(stderr, "Bad '%s' value\n", parameter);
                return STATUS_INVALID_VALUE;
        }

        if (t.get_token(expr::TF_GET) != expr::TT_EOF)
        {
            fprintf(stderr, "Bad '%s' value\n", parameter);
            return STATUS_INVALID_VALUE;
        }

        *dst = fvalue;

        return STATUS_OK;
    }

    status_t parse_cmdline_enum(ssize_t *dst, const char *parameter, const char *val, const cfg_flag_t *flags)
    {
        LSPString in;
        if (!in.set_native(val))
        {
            fprintf(stderr, "Out of memory\n");
            return STATUS_NO_MEM;
        }

        io::InStringSequence is(&in);
        expr::Tokenizer t(&is);
        const cfg_flag_t *flag = NULL;

        switch (t.get_token(expr::TF_GET | expr::TF_XKEYWORDS))
        {
            case expr::TT_BAREWORD:
                if ((flag = find_config_flag(t.text_value(), flags)) == NULL)
                {
                    fprintf(stderr, "Bad '%s' value\n", parameter);
                    return STATUS_BAD_FORMAT;
                }
                break;

            default:
                fprintf(stderr, "Bad '%s' value\n", parameter);
                return STATUS_BAD_FORMAT;
        }

        if (t.get_token(expr::TF_GET) != expr::TT_EOF)
        {
            fprintf(stderr, "Bad '%s' value\n", parameter);
            return STATUS_INVALID_VALUE;
        }

        *dst = flag->value;

        return STATUS_OK;
    }

    status_t parse_cmdline_bool(bool *dst, const char *val, const char *parameter)
    {
        LSPString in;
        if (!in.set_native(val))
        {
            fprintf(stderr, "Out of memory\n");
            return STATUS_NO_MEM;
        }

        io::InStringSequence is(&in);
        expr::Tokenizer t(&is);
        bool bvalue;

        switch (t.get_token(expr::TF_GET))
        {
            case expr::TT_IVALUE: bvalue = t.int_value(); break;
            case expr::TT_FVALUE: bvalue = t.float_value() >= 0.5f; break;
            case expr::TT_TRUE: bvalue = true; break;
            case expr::TT_FALSE: bvalue = false; break;
            default:
                fprintf(stderr, "Bad '%s' value\n", parameter);
                return STATUS_INVALID_VALUE;
        }

        if (t.get_token(expr::TF_GET) != expr::TT_EOF)
        {
            fprintf(stderr, "Bad '%s' value\n", parameter);
            return STATUS_INVALID_VALUE;
        }

        *dst = bvalue;

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
            for (const char **p = timbremill::options; *p != NULL; p += 3)
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
                xopt = opt;

            // Parse options
            bool found = false;
            for (const char **p = timbremill::options; *p != NULL; p += 3)
                if (!strcmp(xopt, p[1]))
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
        const char *cfg_name = options.get("--config");
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
        if ((val = options.get("--file")) != NULL)
            cfg->sFile.set_native(val);
        if ((val = options.get("--ir-file")) != NULL)
            cfg->sIR.sFile.set_native(val);
        if ((val = options.get("--ir-raw")) != NULL)
            cfg->sIR.sRaw.set_native(val);
        if ((val = options.get("--ir-fade-in")) != NULL)
        {
            if ((res = parse_cmdline_float(&cfg->sIR.fFadeIn, val, "IR fade in")) != STATUS_OK)
                return res;
        }
        if ((val = options.get("--ir-fade-out")) != NULL)
        {
            if ((res = parse_cmdline_float(&cfg->sIR.fFadeOut, val, "IR fade out")) != STATUS_OK)
                return res;
        }
        if ((val = options.get("--ir-head-cut")) != NULL)
        {
            if ((res = parse_cmdline_float(&cfg->sIR.fHeadCut, val, "IR head cut")) != STATUS_OK)
                return res;
        }
        if ((val = options.get("--ir-tail-cut")) != NULL)
        {
            if ((res = parse_cmdline_float(&cfg->sIR.fTailCut, val, "IR tail cut")) != STATUS_OK)
                return res;
        }
        if ((val = options.get("--dst-path")) != NULL)
            cfg->sDstPath.set_native(val);
        if ((val = options.get("--src-path")) != NULL)
            cfg->sSrcPath.set_native(val);
        if ((val = options.get("--srate")) != NULL)
        {
            if ((res = parse_cmdline_int(&cfg->nSampleRate, val, "sample rate")) != STATUS_OK)
                return res;
        }
        if ((val = options.get("--fft-rank")) != NULL)
        {
            if ((res = parse_cmdline_int(&cfg->nFftRank, val, "FFT rank")) != STATUS_OK)
                return res;
        }
        if ((val = options.get("--gain-range")) != NULL)
        {
            if ((res = parse_cmdline_float(&cfg->fGainRange, val, "gain range")) != STATUS_OK)
                return res;
        }
        if ((val = options.get("--produce")) != NULL)
        {
            if ((res = parse_cmdline_flags(&cfg->nProduce, "produce", val, produce_flags)) != STATUS_OK)
                return res;
        }
        if ((val = options.get("--dry")) != NULL)
        {
            if ((res = parse_cmdline_float(&cfg->fDry, val, "dry")) != STATUS_OK)
                return res;
        }
        if ((val = options.get("--wet")) != NULL)
        {
            if ((res = parse_cmdline_float(&cfg->fWet, val, "wet")) != STATUS_OK)
                return res;
        }
        if ((val = options.get("--mastering")) != NULL)
        {
            if ((res = parse_cmdline_bool(&cfg->bMastering, val, "mastering")) != STATUS_OK)
                return res;
        }
        if ((val = options.get("--norm-gain")) != NULL)
        {
            if ((res = parse_cmdline_float(&cfg->fNormGain, val, "norm-gain")) != STATUS_OK)
                return res;
        }
        if ((val = options.get("--normalize")) != NULL)
        {
            if ((res = parse_cmdline_enum(&cfg->nNormalize, "normalize", val, normalize_flags)) != STATUS_OK)
                return res;
        }

        return STATUS_OK;
    }
}



