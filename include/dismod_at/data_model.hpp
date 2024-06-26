// SPDX-License-Identifier: AGPL-3.0-or-later
// SPDX-FileCopyrightText: University of Washington <https://www.washington.edu>
// SPDX-FileContributor: 2014-24 Bradley M. Bell
// ----------------------------------------------------------------------------
# ifndef DISMOD_AT_DATA_MODEL_HPP
# define DISMOD_AT_DATA_MODEL_HPP

# include <limits>
# include <cppad/utility/vector.hpp>
# include "subset_data.hpp"
# include "get_integrand_table.hpp"
# include "get_covariate_table.hpp"
# include "get_subgroup_table.hpp"
# include "get_density_table.hpp"
# include "get_prior_table.hpp"
# include "weight_info.hpp"
# include "pack_info.hpp"
# include "residual_density.hpp"
# include "get_data_sim_table.hpp"
# include "avg_integrand.hpp"
# include "avg_noise_effect.hpp"
# include "meas_noise_effect.hpp"
# include "cov2weight_map.hpp"

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

class data_model {
   // infromation for each data point
   typedef struct {
      density_enum          density;
      size_t                child;
      bool                  depend_on_ran_var;
   } data_ode_info;
private:
   // constant values
   const bool                   fit_simulated_data_;
   const size_t                 n_covariate_;
   const size_t                 n_child_;
   const CppAD::vector<double>& subset_cov_value_;
# ifndef NDEBUG
   const size_t                 pack_object_size_;
# endif
   //
   // set by constructor and not changed
   meas_noise_effect_enum         meas_noise_effect_;
   CppAD::vector<data_ode_info>   data_info_;
   CppAD::vector<double>          minimum_meas_cv_;
   //
   // Has replace_like been called.
   // Set false by constructor and true by replace_like.
   bool                         replace_like_called_;

   // set by consructor, except that following fields set by replace_like
   // subset_data_obj_[subset_id].density_id
   // subset_data_obj_[subset_id].hold_out
   // subset_data_obj_[subset_id].meas_value
   // subset_data_obj_[subset_id].meas_std
   CppAD::vector<subset_data_struct>         subset_data_obj_;

   // Used to compute average of integrands
   // (effectively const)
   avg_integrand                avgint_obj_;

   // Used to compute average of noise effects
   // (effectively const)
   avg_noise_effect             avg_noise_obj_;

public:
   template <class SubsetStruct>
   data_model(
      const cov2weight_map&                    cov2weight_obj     ,
      size_t                                   n_covariate        ,
      bool                                     fit_simulated_data ,
      const std::string&                       meas_noise_effect  ,
      const std::string&                       rate_case          ,
      double                                   bound_random       ,
      double                                   ode_step_size      ,
      const CppAD::vector<double>&             age_avg_grid       ,
      const CppAD::vector<double>&             age_table          ,
      const CppAD::vector<double>&             time_table         ,
      const CppAD::vector<covariate_struct>&   covariate_table    ,
      const CppAD::vector<subgroup_struct>&    subgroup_table     ,
      const CppAD::vector<integrand_struct>&   integrand_table    ,
      const CppAD::vector<mulcov_struct>&      mulcov_table       ,
      const CppAD::vector<prior_struct>&       prior_table        ,
      const CppAD::vector<SubsetStruct>&       subset_object      ,
      const CppAD::vector<double>&             subset_cov_value   ,
      const CppAD::vector<weight_info>&        w_info_vec         ,
      const CppAD::vector<smooth_info>&        s_info_vec         ,
      const pack_info&                         pack_object        ,
      const child_info&                        child_info4data
   );
   ~data_model(void);
   //
   void replace_like(
      const CppAD::vector<subset_data_struct>& subset_data_obj
   );
   //
   // compute an average integrand: data_model is effectively const
   template <class Float>
   Float average(
      size_t                        data_id  ,
      const  CppAD::vector<Float>&  pack_vec
   );
   // compute weighted residual and log-likelihood for one data points
   // (effectively const)
   template <class Float>
   residual_struct<Float> like_one(
      size_t                        data_id  ,
      const  CppAD::vector<Float>&  pack_vec ,
      const  Float&                 avg      ,
      Float&                        delta
   );
   // compute weighted residual and log-likelihood for all data points
   // (effectively const)
   template <class Float>
   CppAD::vector< residual_struct<Float> > like_all(
      bool                          hold_out ,
      bool                          parent   ,
      const  CppAD::vector<Float>&  pack_vec
   );
};

} // END_DISMOD_AT_NAMESPACE

# endif
