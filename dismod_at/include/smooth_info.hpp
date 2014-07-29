// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rate Estimation as Functions of Age and Time
          Copyright (C) 2014-14 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the 
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# ifndef DISMOD_AT_SMOOTH_GRID_HPP
# define DISMOD_AT_SMOOTH_GRID_HPP

# include <cppad/cppad.hpp>
# include <dismod_at/include/get_smooth_table.hpp>
# include <dismod_at/include/get_smooth_grid.hpp>

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

class smooth_info {
private:
	// grid of age values for this smoothing
	CppAD::vector<size_t> age_id_;
	// grid of time values for this smoothing
	CppAD::vector<size_t> time_id_;
	// like_id for function values
	CppAD::vector<size_t> value_like_id_;
	// like_id for function difference in age direction
	CppAD::vector<size_t> dage_like_id_;
	// like_id for function difference in time direction
	CppAD::vector<size_t> dtime_like_id_;
	// like_id for multiplier of value likelihood standard deviations
	size_t mulstd_value_;
	// like_id for multiplier of dage likelihood standard deviations
	size_t mulstd_dage_;
	// like_id for multiplier of dtime likelihood standard deviations
	size_t mulstd_dtime_;
public:
	// constructor
	smooth_info(
		size_t                                   smooth_id         ,
		const CppAD::vector<smooth_struct>&      smooth_table      ,
		const CppAD::vector<smooth_grid_struct>& smooth_grid_table
	);
	// testing constructor
	smooth_info(
		const CppAD::vector<size_t>&  age_id         ,
		const CppAD::vector<size_t>&  time_id        ,
		const CppAD::vector<size_t>&  value_like_id  ,
		const CppAD::vector<size_t>&  dage_like_id   ,
		const CppAD::vector<size_t>&  dtime_like_id  ,
		size_t                        mulstd_value   ,
		size_t                        mulstd_dage    ,
		size_t                        mulstd_dtime
	);
	//
	size_t age_size(void)  const;
	size_t time_size(void) const;
	//
	size_t age_id(size_t i)  const;
	size_t time_id(size_t j) const;
	//
	size_t value_like_id(size_t i, size_t j) const;
	size_t dage_like_id(size_t i, size_t j)  const;
	size_t dtime_like_id(size_t i, size_t j) const;
	//
	size_t mulstd_value(void) const;
	size_t mulstd_dage(void)  const;
	size_t mulstd_dtime(void) const;
};

} // END_DISMOD_AT_NAMESPACE

# endif
