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

#ifndef THERMO_DRY
#define THERMO_DRY

#include "thermo.h"

class Master;
class Grid;
class Fields;
class Stats;

/**
 * Class for the dry thermodynamics.
 * This class is responsible for the computation of the right hand side term related to
 * the acceleration by buoyancy. In the dry thermodynamics temperature and buoyancy are
 * equivalent and no complex buoyancy function is required.
 */
class Thermo_dry : public Thermo
{
    public:
        Thermo_dry(Model*, Input*); ///< Constructor of the dry thermodynamics class.
        virtual ~Thermo_dry();      ///< Destructor of the dry thermodynamics class.

        void init();
        void create(Input*);
        void exec();                ///< Add the tendencies belonging to the buoyancy.
        unsigned long get_time_limit(unsigned long, double); ///< Compute the time limit (n/a for thermo_dry)


        void exec_stats(Mask*);
        void exec_cross();
        void exec_dump();

        bool check_field_exists(std::string name);
        void get_thermo_field(Field3d*, Field3d*, std::string name, bool cyclic);
        void get_buoyancy_surf(Field3d *);             ///< Compute the near-surface and bottom buoyancy for usage in another routine.
        void get_buoyancy_fluxbot(Field3d*);           ///< Compute the bottom buoyancy flux for usage in another routine.
        void get_prog_vars(std::vector<std::string>*); ///< Retrieve a list of prognostic variables.
        double get_buoyancy_diffusivity();

#ifdef USECUDA
        // GPU functions and variables
        void prepare_device();
        void clear_device();
#endif

        // Empty functions that are allowed to pass.
        void get_mask(Field3d*, Field3d*, Mask*) {}

    private:
        void init_stat();  ///< Initialize the thermo statistics
        void init_cross(); ///< Initialize the thermo cross-sections
        void init_dump();  ///< Initialize the thermo field dumps

        void calc_buoyancy(double *, double *, double *);     ///< Calculation of the buoyancy.
        void calc_N2(double *, double *, double *, double *); ///< Calculation of the Brunt-Vaissala frequency.

        // cross sections
        std::vector<std::string> crosslist;        ///< List with all crosses from ini file
        std::vector<std::string> allowedcrossvars; ///< List with allowed cross variables
        std::vector<std::string> dumplist;         ///< List with all 3d dumps from the ini file.

        void calc_buoyancy_bot(double *, double *,
                               double *, double *,
                               double *, double *); ///< Calculation of the near-surface and surface buoyancy.
        void calc_buoyancy_fluxbot(double *, double *, double *);  ///< Calculation of the buoyancy flux at the bottom.
        void calc_buoyancy_tend_2nd(double *, double *, double *); ///< Calculation of the buoyancy tendency with 2nd order accuracy.
        void calc_buoyancy_tend_4th(double *, double *, double *); ///< Calculation of the buoyancy tendency with 4th order accuracy.

        void calc_base_state(double *, double *, double *, double *, double *, double *, double *, double *, double); ///< For anelastic setup, calculate base state from initial input profiles

        Stats* stats;

        std::string swbasestate;

        double pbot;   ///< Surface pressure.
        double thref0; ///< Reference potential temperature in case of Boussinesq

        double* thref;
        double* threfh;
        double* pref;
        double* prefh;
        double* exnref;
        double* exnrefh;

        // GPU functions and variables
        double* thref_g;
        double* threfh_g;
        double* pref_g;
        double* prefh_g;
        double* exnref_g;
        double* exnrefh_g;
};
#endif
