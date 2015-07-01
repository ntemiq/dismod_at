// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# ifndef DISMOD_AT_MANAGE_GSL_RNG_HPP
# define DISMOD_AT_MANAGE_GSL_RNG_HPP

# include <gsl/gsl_rng.h>

namespace dismod_at {
	size_t   new_gsl_rng(size_t seed);
	gsl_rng* get_gsl_rng(void);
	void     free_gsl_rng(void);
}

# endif
