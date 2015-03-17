// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rate Estimation as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# ifndef DISMOD_AT_PRIOR_DENSITY_HPP
# define DISMOD_AT_PRIOR_DENSITY_HPP

# include <cppad/vector.hpp>
# include "pack_info.hpp"
# include "get_prior_table.hpp"
# include "get_smooth_table.hpp"
# include "smooth_info.hpp"
# include "residual_density.hpp"

namespace dismod_at {
	class prior_density {
	private:
		// data
		const pack_info                    pack_object_;
		const CppAD::vector<double>        age_table_;
		const CppAD::vector<double>        time_table_;
		const CppAD::vector<prior_struct>  prior_table_;
		const CppAD::vector<smooth_info>   s_info_vec_;

		// functions
		template <class Float>
		residual_struct<Float> log_prior_density(
			residual_type_enum  type    ,
			const prior_struct& prior   ,
			const Float&        variable
		) const;
		template <class Float>
		void log_prior_density_on_grid(
			CppAD::vector< residual_struct<Float> >& residual_vec ,
			residual_type_enum  type                              ,
			size_t                                   offset       ,
			const CppAD::vector<Float>&              pack_vec     ,
			const smooth_info&                       s_info
		) const;
	public:
		prior_density(
			const pack_info&                       pack_object     ,
			const CppAD::vector<double>&           age_table       ,
			const CppAD::vector<double>&           time_table      ,
			const CppAD::vector<prior_struct>&     prior_table     ,
			const CppAD::vector<smooth_info>&      s_info_vec
		);
		template <class Float>
		CppAD::vector< residual_struct<Float> > eval(
			const CppAD::vector<Float>& pack_vec
		) const;

	};
}

# endif
