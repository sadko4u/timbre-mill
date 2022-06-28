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

#include <private/config/json.h>
#include <lsp-plug.in/common/debug.h>
#include <lsp-plug.in/fmt/json/Parser.h>

namespace timbremill
{
    using namespace lsp;

    static status_t parse_json_config_string(LSPString *str, json::Parser *p)
    {
        json::event_t ev;

        // Should be JSON object
        status_t res = p->read_next(&ev);
        if (res != STATUS_OK)
            return res;
        else if (ev.type != json::JE_STRING)
            return STATUS_BAD_TYPE;

        // Parse string value
        if (!str->set(&ev.sValue))
            return STATUS_NO_MEM;
        return STATUS_OK;
    }

    static status_t parse_json_config_int(ssize_t *dst, json::Parser *p)
    {
        json::event_t ev;

        // Should be JSON object
        status_t res = p->read_next(&ev);
        if (res != STATUS_OK)
            return res;
        else if (ev.type == json::JE_INTEGER)
            *dst = ev.iValue;
        else
            return STATUS_BAD_TYPE;

        // Return OK status
        return STATUS_OK;
    }

    static status_t parse_json_config_float(float *dst, json::Parser *p)
    {
        json::event_t ev;

        // Should be JSON object
        status_t res = p->read_next(&ev);
        if (res != STATUS_OK)
            return res;
        else if (ev.type == json::JE_INTEGER)
            *dst    = ev.iValue;
        else if (ev.type == json::JE_DOUBLE)
            *dst    = ev.fValue;
        else
            return STATUS_BAD_TYPE;

        // Return OK status
        return STATUS_OK;
    }

    static status_t parse_json_config_bool(bool *dst, json::Parser *p)
    {
        json::event_t ev;

        // Should be JSON object
        status_t res = p->read_next(&ev);
        if (res != STATUS_OK)
            return res;
        else if (ev.type == json::JE_INTEGER)
            *dst    = ev.iValue;
        else if (ev.type == json::JE_DOUBLE)
            *dst    = ev.fValue > 0.5f;
        else if (ev.type == json::JE_BOOL)
            *dst    = ev.bValue;
        else
            return STATUS_BAD_TYPE;

        // Return OK status
        return STATUS_OK;
    }

    static status_t parse_json_config_flags(ssize_t *dst, const cfg_flag_t *flags, json::Parser *p)
    {
        json::event_t ev;
        ssize_t xdst = 0;

        // Should be JSON object
        status_t res = p->read_next(&ev);
        if (res != STATUS_OK)
            return res;
        else if (ev.type != json::JE_ARRAY_START)
            return STATUS_BAD_TYPE;

        // Read group object
        while (true)
        {
            // Read property name
            res = p->read_next(&ev);
            if (res != STATUS_OK)
                return res;
            else if (ev.type == json::JE_ARRAY_END)
                break;
            else if (ev.type != json::JE_STRING)
                return STATUS_BAD_FORMAT;

            // Parse flag
            const cfg_flag_t *xf = find_config_flag(&ev.sValue, flags);
            if (xf != NULL)
                xdst |= xf->value;
            else
                fprintf(stderr, "Warning: unknown flag '%s'\n", ev.sValue.get_native());

            // Analyze result
            if (res != STATUS_OK)
                break;
        }

        // Return success result
        *dst = xdst;

        return res;
    }

    static status_t parse_json_config_enum(ssize_t *dst, const cfg_flag_t *flags, json::Parser *p)
    {
        json::event_t ev;

        // Should be JSON object
        status_t res = p->read_next(&ev);
        if (res != STATUS_OK)
            return res;
        else if (ev.type != json::JE_STRING)
            return STATUS_BAD_TYPE;

        // Find configuration flag
        const cfg_flag_t *xf = find_config_flag(&ev.sValue, flags);
        if (xf == NULL)
        {
            fprintf(stderr, "Error: unknown flag '%s'\n", ev.sValue.get_native());
            return STATUS_BAD_FORMAT;
        }

        // Return success result
        *dst = xf->value;

        return res;
    }

    static status_t parse_json_config_group_files(fgroup_t *grp, json::Parser *p)
    {
        json::event_t ev;

        // Should be JSON object
        status_t res = p->read_next(&ev);
        if (res != STATUS_OK)
            return res;
        else if (ev.type != json::JE_ARRAY_START)
            return STATUS_BAD_TYPE;

        // Read group object
        while (true)
        {
            // Read property name
            res = p->read_next(&ev);
            if (res != STATUS_OK)
                return res;
            else if (ev.type == json::JE_ARRAY_END)
                break;
            else if (ev.type != json::JE_STRING)
                return STATUS_BAD_FORMAT;

            // Add item to the file group
            LSPString *fname = ev.sValue.clone();
            if (fname == NULL)
                return STATUS_NO_MEM;
            if (!grp->vFiles.add(fname))
            {
                delete fname;
                return STATUS_NO_MEM;
            }

            // Analyze result
            if (res != STATUS_OK)
                break;
        }

        return res;
    }

    static status_t parse_json_config_group(fgroup_t *grp, json::Parser *p)
    {
        json::event_t ev;
        bool master_set = false;
        bool files_set = false;

        // Should be JSON object
        status_t res = p->read_next(&ev);
        if (res != STATUS_OK)
            return res;
        else if (ev.type != json::JE_OBJECT_START)
            return STATUS_BAD_TYPE;

        // Read group object
        while (true)
        {
            // Read property name
            res = p->read_next(&ev);
            if (res != STATUS_OK)
                return res;
            else if (ev.type == json::JE_OBJECT_END)
                break;
            else if (ev.type != json::JE_PROPERTY)
                return STATUS_BAD_FORMAT;

            // Create group and set it's name
            if (ev.sValue.equals_ascii("master"))
            {
                if (master_set)
                {
                    lsp_error("Duplicate 'master' property");
                    res = STATUS_BAD_FORMAT;
                    break;
                }

                // Read string
                res = p->read_next(&ev);
                if (res != STATUS_OK)
                    return res;
                else if (ev.type != json::JE_STRING)
                    return STATUS_BAD_FORMAT;

                // Set string value
                if (grp->sMaster.set(&ev.sValue))
                    master_set  = true;
                else
                    res         = STATUS_NO_MEM;
            }
            else if (ev.sValue.equals_ascii("files"))
            {
                if (files_set)
                {
                    lsp_error("Duplicate 'files' property");
                    res = STATUS_BAD_FORMAT;
                    break;
                }

                files_set   = true;
                res         = parse_json_config_group_files(grp, p);
            }
            else
                res         = p->skip_current();

            // Analyze result
            if (res != STATUS_OK)
                break;
        }

        return res;
    }

    static status_t parse_json_config_groups(config_t *cfg, json::Parser *p)
    {
        json::event_t ev;

        // Should be JSON object
        status_t res = p->read_next(&ev);
        if (res != STATUS_OK)
            return res;
        else if (ev.type != json::JE_OBJECT_START)
            return STATUS_BAD_TYPE;

        // Read group object
        while (true)
        {
            // Read property name
            res = p->read_next(&ev);
            if (res != STATUS_OK)
                return res;
            else if (ev.type == json::JE_OBJECT_END)
                break;
            else if (ev.type != json::JE_PROPERTY)
                return STATUS_BAD_FORMAT;

            // Create group and set it's name
            fgroup_t *grp = new fgroup_t();
            if (!grp->sName.set(&ev.sValue))
            {
                delete grp;
                return STATUS_NO_MEM;
            }

            // Add group to configuration
            if (!cfg->vGroups.create(&grp->sName, grp))
            {
                if (cfg->vGroups.contains(&grp->sName))
                {
                    lsp_error("Duplicate group name: %s", grp->sName.get_native());
                    delete grp;
                    return STATUS_BAD_FORMAT;
                }

                delete grp;
                return STATUS_NO_MEM;
            }

            // Read group object
            res = parse_json_config_group(grp, p);

            // Analyze result
            if (res != STATUS_OK)
                break;
        }

        return res;
    }

    static status_t parse_json_config_ir(irfile_t *ir, json::Parser *p)
    {
        json::event_t ev;

        // Should be JSON object
        status_t res = p->read_next(&ev);
        if (res != STATUS_OK)
            return res;
        else if (ev.type != json::JE_OBJECT_START)
            return STATUS_BAD_TYPE;

        // Read group object
        while (true)
        {
            // Read property name
            res = p->read_next(&ev);
            if (res != STATUS_OK)
                return res;
            else if (ev.type == json::JE_OBJECT_END)
                break;
            else if (ev.type != json::JE_PROPERTY)
                return STATUS_BAD_FORMAT;

            if (ev.sValue.equals_ascii("head_cut"))
                res = parse_json_config_float(&ir->fHeadCut, p);
            else if (ev.sValue.equals_ascii("tail_cut"))
                res = parse_json_config_float(&ir->fTailCut, p);
            else if (ev.sValue.equals_ascii("fade_in"))
                res = parse_json_config_float(&ir->fFadeIn, p);
            else if (ev.sValue.equals_ascii("fade_out"))
                res = parse_json_config_float(&ir->fFadeOut, p);
            else if (ev.sValue.equals_ascii("file"))
                res = parse_json_config_string(&ir->sFile, p);
            else if (ev.sValue.equals_ascii("raw"))
                res = parse_json_config_string(&ir->sRaw, p);
            else if (ev.sValue.equals_ascii("fr_master"))
                res = parse_json_config_string(&ir->sFRMaster, p);
            else if (ev.sValue.equals_ascii("fr_child"))
                res = parse_json_config_string(&ir->sFRChild, p);
            else
                res = p->skip_current();

            // Analyze result
            if (res != STATUS_OK)
                break;
        }

        return res;
    }

    static status_t parse_json_config_root(config_t *cfg, json::Parser *p)
    {
        json::event_t ev;

        // Should be JSON object
        status_t res = p->read_next(&ev);
        if (res != STATUS_OK)
            return res;
        else if (ev.type != json::JE_OBJECT_START)
            return STATUS_BAD_TYPE;

        // Read object
        while (true)
        {
            // Read property name
            res = p->read_next(&ev);
            if (res != STATUS_OK)
                return res;
            else if (ev.type == json::JE_OBJECT_END)
                break;
            else if (ev.type != json::JE_PROPERTY)
                return STATUS_BAD_FORMAT;

            // Analyze event
            if (ev.sValue.equals_ascii("groups"))
                res = parse_json_config_groups(cfg, p);
            else if (ev.sValue.equals_ascii("ir"))
                res = parse_json_config_ir(&cfg->sIR, p);
            else if (ev.sValue.equals_ascii("src_path"))
                res = parse_json_config_string(&cfg->sSrcPath, p);
            else if (ev.sValue.equals_ascii("dst_path"))
                res = parse_json_config_string(&cfg->sDstPath, p);
            else if (ev.sValue.equals_ascii("file"))
                res = parse_json_config_string(&cfg->sFile, p);
            else if (ev.sValue.equals_ascii("srate"))
                res = parse_json_config_int(&cfg->nSampleRate, p);
            else if (ev.sValue.equals_ascii("gain_range"))
                res = parse_json_config_float(&cfg->fGainRange, p);
            else if (ev.sValue.equals_ascii("fft_rank"))
                res = parse_json_config_int(&cfg->nFftRank, p);
            else if (ev.sValue.equals_ascii("produce"))
                res = parse_json_config_flags(&cfg->nProduce, produce_flags, p);
            else if (ev.sValue.equals_ascii("dry"))
                res = parse_json_config_float(&cfg->fDry, p);
            else if (ev.sValue.equals_ascii("wet"))
                res = parse_json_config_float(&cfg->fWet, p);
            else if (ev.sValue.equals_ascii("mastering"))
                res = parse_json_config_bool(&cfg->bMastering, p);
            else if (ev.sValue.equals_ascii("norm_gain"))
                res = parse_json_config_float(&cfg->fNormGain, p);
            else if (ev.sValue.equals_ascii("latency_compensation"))
                res = parse_json_config_bool(&cfg->bLatencyCompensation, p);
            else if (ev.sValue.equals_ascii("match_length"))
                res = parse_json_config_bool(&cfg->bMatchLength, p);
            else if (ev.sValue.equals_ascii("normalize"))
                res = parse_json_config_enum(&cfg->nNormalize, normalize_flags, p);
            else
                res = p->skip_current();

            // Analyze result
            if (res != STATUS_OK)
                break;
        }

        return res;
    }

    status_t parse_json_config(config_t *cfg, io::IInSequence *is)
    {
        json::event_t ev;
        json::Parser p;

        // Wrap the sequence with wrapper
        status_t res = p.wrap(is, json::JSON_VERSION5, 0);
        if (res < 0)
            return res;

        // Read root config
        res = parse_json_config_root(cfg, &p);
        if (res == STATUS_OK)
        {
            // Ensure for EOF event
            res = p.read_next(&ev);
            if (res == STATUS_EOF)
                res     = STATUS_OK;
        }

        // Close the parser
        if (res == STATUS_OK)
            res = p.close();
        else
            p.close();

        return res;
    }
}


