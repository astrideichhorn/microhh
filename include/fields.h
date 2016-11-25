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

#ifndef FIELDS
#define FIELDS
#include <map>
#include <vector>
#include "field3d.h"

class Master;
class Input;
class Model;
class Grid;
class Stats;
struct Mask;

typedef std::map<std::string, Field3d *> FieldMap;

class Fields
{
    public:
        // functions
        Fields(Model*, Input*); ///< Constructor of the fields class.
        ~Fields();              ///< Destructor of the fields class.

        void init();         ///< Initialization of the field arrays.
        void create(Input*); ///< Initialization of the fields (random perturbations, vortices).
        void create_stats(); ///< Initialization of the fields statistics.

        void exec();
        void get_mask(Field3d*, Field3d*, Mask*);
        void exec_stats(Mask*);

        void init_momentum_field  (Field3d*&, Field3d*&, std::string, std::string, std::string);
        void init_prognostic_field(std::string, std::string, std::string);
        void init_diagnostic_field(std::string, std::string, std::string);
        void init_tmp_field       (std::string, std::string, std::string);

        void save(int);
        void load(int);

        double check_momentum();
        double check_tke();
        double check_mass();

        void set_calc_mean_profs(bool);
        void set_minimum_tmp_fields(int);

        void exec_cross();
        void exec_dump();

        Field3d* u; ///< Field3d instance of x velocity component
        Field3d* v; ///< Field3d instance of y velocity component
        Field3d* w; ///< Field3d instance of vertical velocity component

        Field3d* ut; ///< Field3d instance of x velocity component tendency 
        Field3d* vt; ///< Field3d instance of y velocity component tendency
        Field3d* wt; ///< Field3d instance of vertical velocity component tendency 

        FieldMap a;  ///< Map containing all field3d instances
        FieldMap ap; ///< Map containing all prognostic field3d instances
        FieldMap at; ///< Map containing all tendency field3d instances

        FieldMap mp; ///< Map containing all momentum field3d instances
        FieldMap mt; ///< Map containing all momentum tendency field3d instances

        FieldMap sd; ///< Map containing all diagnostic scalar field3d instances
        FieldMap sp; ///< Map containing all prognostic scalar field3d instances
        FieldMap st; ///< Map containing all prognostic scalar tendency field3d instances

        FieldMap atmp; ///< Map containing all temporary field3d instances

        double* rhoref;  ///< Reference density at full levels 
        double* rhorefh; ///< Reference density at half levels

        // TODO remove these to and bring them to diffusion model
        double visc;

        /* 
         *Device (GPU) functions and variables
         */
        enum Offset_type {Offset, No_offset};

        void prepare_device();  ///< Allocation of all fields at device 
        void forward_device();  ///< Copy of all fields from host to device
        void backward_device(); ///< Copy of all fields required for statistics and output from device to host
        void clear_device();    ///< Deallocation of all fields at device

        void forward_field_device_3d (double*, double*, Offset_type); ///< Copy of a single 3d field from host to device
        void forward_field_device_2d (double*, double*, Offset_type); ///< Copy of a single 2d field from host to device
        void forward_field_device_1d (double*, double*, int);         ///< Copy of a single array from host to device
        void backward_field_device_3d(double*, double*, Offset_type); ///< Copy of a single 3d field from device to host
        void backward_field_device_2d(double*, double*, Offset_type); ///< Copy of a single 2d field from device to host
        void backward_field_device_1d(double*, double*, int);         ///< Copy of a single array from device to host

        double* rhoref_g;  ///< Reference density at full levels at device
        double* rhorefh_g; ///< Reference density at half levels at device

    private:
        // variables
        Model*  model;
        Grid*   grid;
        Master* master;
        Stats*  stats;

        bool calc_mean_profs;

        // cross sections
        std::vector<std::string> crosslist; ///< List with all crosses from the ini file.
        std::vector<std::string> dumplist;  ///< List with all 3d dumps from the ini file.

        // Cross sections split per type.
        std::vector<std::string> crosssimple;
        std::vector<std::string> crosslngrad;   
        std::vector<std::string> crossbot;
        std::vector<std::string> crosstop;
        std::vector<std::string> crossfluxbot;
        std::vector<std::string> crossfluxtop;

        void check_added_cross(std::string, std::string, std::vector<std::string>*, std::vector<std::string>*);

        // masks
        void calc_mask_wplus(double*, double*, double*, int*, int*, int*, double*);
        void calc_mask_wmin (double*, double*, double*, int*, int*, int*, double*);

        // perturbations
        double rndamp;
        double rndz;
        double rndexp;
        double vortexamp;
        int vortexnpair;
        std::string vortexaxis;

        // Kernels for the check functions.
        double calc_momentum_2nd(double*, double*, double*, double*);
        double calc_tke_2nd     (double*, double*, double*, double*);
        double calc_mass        (double*, double*);

        int add_mean_prof(Input*, std::string, double*, double);
        int randomize    (Input*, std::string, double*);
        int add_vortex_pair(Input*);

        // statistics
        double* umodel;
        double* vmodel;

        int n_tmp_fields;   // number of temporary fields

        /* 
         *Device (GPU) functions and variables
         */
        void forward_field3d_device(Field3d *);  ///< Copy of a complete Field3d instance from host to device
        void backward_field3d_device(Field3d *); ///< Copy of a complete Field3d instance from device to host
};
#endif
