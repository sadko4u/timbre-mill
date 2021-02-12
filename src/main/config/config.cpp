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

#include <private/config/config.h>
#include <private/config/json.h>

#include <lsp-plug.in/io/InSequence.h>
#include <lsp-plug.in/io/InMarkSequence.h>

namespace timbremill
{
    using namespace lsp;

    status_t parse_config(config_t *cfg, const char *path)
    {
        io::InSequence sq;
        status_t res = sq.open(path);
        if (res != STATUS_OK)
            return res;

        res = parse_config(cfg, &sq);
        status_t rc = sq.close();

        return (res != STATUS_OK) ? res : rc;
    }

    status_t parse_config(config_t *cfg, const LSPString *path)
    {
        io::InSequence sq;
        status_t res = sq.open(path);
        if (res != STATUS_OK)
            return res;

        res = parse_config(cfg, &sq);
        status_t rc = sq.close();

        return (res != STATUS_OK) ? res : rc;
    }

    status_t parse_config(config_t *cfg, const io::Path *path)
    {
        io::InSequence sq;
        status_t res = sq.open(path);
        if (res != STATUS_OK)
            return res;

        res = parse_config(cfg, &sq);
        status_t rc = sq.close();

        return (res != STATUS_OK) ? res : rc;
    }

    status_t parse_config(config_t *cfg, FILE *in)
    {
        io::InSequence sq;
        status_t res = sq.wrap(in, false);
        if (res != STATUS_OK)
            return res;

        res = parse_config(cfg, &sq);
        status_t rc = sq.close();

        return (res != STATUS_OK) ? res : rc;
    }

    status_t parse_config(config_t *cfg, io::IInStream *is)
    {
        io::InSequence sq;
        status_t res = sq.wrap(is, false);
        if (res != STATUS_OK)
            return res;

        res = parse_config(cfg, &sq);
        status_t rc = sq.close();

        return (res != STATUS_OK) ? res : rc;
    }

    status_t parse_config(config_t *cfg, io::IInSequence *is)
    {
        io::InMarkSequence ims;
        status_t res;

        if ((res = ims.wrap(is, false)) == STATUS_OK)
        {
            res = ims.mark(0x1000);
            if (res == STATUS_OK)
            {
                // Parse json configuration
                res = parse_json_config(cfg, is);

                if (res != STATUS_OK)
                    res = STATUS_BAD_FORMAT;
            }
        }

        // Close the sequence
        if (res == STATUS_OK)
            res = ims.close();
        else
            ims.close();

        return res;
        if ((res = ims.wrap(is, false)) != STATUS_OK)
            return res;

        return res;
    }
}


