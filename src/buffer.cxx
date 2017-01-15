/*
 * MicroHH
 * Copyright (c) 2011-2015 Chiel van Heerwaarden
 * Copyright (c) 2011-2015 Thijs Heus
 * Copyright (c) 2014-2015 Bart van Stratum
 *
 * This file is part of MicroHH
 *
 * MicroHH is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * MicroHH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with MicroHH.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdio>
#include <cmath>
#include <stdlib.h>
#include "master.h"
#include "input.h"
#include "grid.h"
#include "fields.h"
#include "buffer.h"
#include "defines.h"
#include "model.h"

Buffer::Buffer(Model* modelin, Input* inputin)
{
    model  = modelin;
    grid   = model->grid;
    fields = model->fields;
    master = model->master;

    int nerror = 0;
    nerror += inputin->get_item(&swbuffer, "buffer", "swbuffer", "", "0");

    if (swbuffer == "1")
    {
        nerror += inputin->get_item(&swupdate, "buffer", "swupdate", "", "0");
        nerror += inputin->get_item(&zstart  , "buffer", "zstart"  , "");
        nerror += inputin->get_item(&sigma   , "buffer", "sigma"   , "", 2.);
        nerror += inputin->get_item(&beta    , "buffer", "beta"    , "", 2.);
    }

    if (nerror)
        throw 1;

    if (swbuffer == "1" && swupdate == "1")
        fields->set_calc_mean_profs(true);
}

Buffer::~Buffer()
{
    for (std::map<std::string, double*>::const_iterator it=bufferprofs.begin(); it!=bufferprofs.end(); ++it)
        delete[] it->second;

    #ifdef USECUDA
    clear_device();
    #endif
}

void Buffer::init()
{
    if (swbuffer == "1")
    {
        if (swupdate == "1")
        {
            bufferprofs["w"] = new double[grid->kcells];
        }
        else
        {
            // Allocate the buffer arrays.
            for (FieldMap::const_iterator it=fields->ap.begin(); it!=fields->ap.end(); ++it)
                bufferprofs[it->first] = new double[grid->kcells];
        }
    }
}

void Buffer::create(Input* inputin)
{
    int nerror = 0;

    if (swbuffer == "1")
    {
        // Find the starting points.
        bufferkstart  = grid->kstart;
        bufferkstarth = grid->kstart;

        for (int k=grid->kstart; k<grid->kend; ++k)
        {
            // Check if the cell center is in the buffer zone.
            if (grid->z[k] < this->zstart)
                ++bufferkstart;
            // Check if the cell face is in the buffer zone.
            if (grid->zh[k] < this->zstart)
                ++bufferkstarth;
        }

        // Check whether the lowest of the two levels is contained in the buffer layer.
        if (bufferkstarth == grid->kend)
        {
            ++nerror;
            master->print_error("buffer is too close to the model top\n");
        }

        // Allocate the buffer for w on 0.
        for (int k=0; k<grid->kcells; ++k)
             bufferprofs["w"][k] = 0.;

        if (swupdate == "0")
        {
            // Set the buffers according to the initial profiles of the variables.
            nerror += inputin->get_prof(&bufferprofs["u"][grid->kstart], "u", grid->kmax);
            nerror += inputin->get_prof(&bufferprofs["v"][grid->kstart], "v", grid->kmax);

            // In case of u and v, subtract the grid velocity.
            for (int k=grid->kstart; k<grid->kend; ++k)
            {
                bufferprofs["u"][k] -= grid->utrans;
                bufferprofs["v"][k] -= grid->vtrans;
            }


            for (FieldMap::const_iterator it=fields->sp.begin(); it!=fields->sp.end(); ++it)
                nerror += inputin->get_prof(&bufferprofs[it->first][grid->kstart], it->first, grid->kmax);
        }
    }

    if (nerror)
        throw 1;
}

#ifndef USECUDA
void Buffer::exec()
{
    if (swbuffer == "1")
    {
        if (swupdate == "1")
        {
            // Calculate the buffer tendencies.
            buffer(fields->mt["u"]->data, fields->mp["u"]->data, fields->mp["u"]->datamean, grid->z );
            buffer(fields->mt["v"]->data, fields->mp["v"]->data, fields->mp["v"]->datamean, grid->z );
            buffer(fields->mt["w"]->data, fields->mp["w"]->data, bufferprofs["w"]         , grid->zh);

            for (FieldMap::const_iterator it=fields->sp.begin(); it!=fields->sp.end(); ++it)
                buffer(fields->st[it->first]->data, it->second->data, it->second->datamean, grid->z);
        }
        else
        {
            // Calculate the buffer tendencies.
            buffer(fields->mt["u"]->data, fields->mp["u"]->data, bufferprofs["u"], grid->z );
            buffer(fields->mt["v"]->data, fields->mp["v"]->data, bufferprofs["v"], grid->z );
            buffer(fields->mt["w"]->data, fields->mp["w"]->data, bufferprofs["w"], grid->zh);

            for (FieldMap::const_iterator it=fields->sp.begin(); it!=fields->sp.end(); ++it)
                buffer(fields->st[it->first]->data, it->second->data, bufferprofs[it->first], grid->z);
        }
    }
}
#endif

void Buffer::buffer(double* const restrict at, const double* const restrict a, 
                    const double* const restrict abuf, const double* const restrict z)
{ 
    const int jj = grid->icells;
    const int kk = grid->ijcells;

    const double zsizebuf = grid->zsize - this->zstart;

    double sigmaz;

    for (int k=bufferkstart; k<grid->kend; ++k)
    {
        sigmaz = this->sigma*std::pow((z[k]-this->zstart)/zsizebuf, this->beta);
        for (int j=grid->jstart; j<grid->jend; ++j)
#pragma ivdep
            for (int i=grid->istart; i<grid->iend; ++i)
            {
                const int ijk = i + j*jj + k*kk;
                at[ijk] -= sigmaz*(a[ijk]-abuf[k]);
            }
    }
}
