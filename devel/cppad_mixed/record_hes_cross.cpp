// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# include <dismod_at/cppad_mixed.hpp>

/*
$begin cppad_mixed_record_hes_cross$$
$spell
	cppad
	hes hes
	vec
	const
	Cpp
	logdet
$$

$section Set Up Hessian of Cross Terms; i.e., w.r.t Fixed and Random Effects$$

$head Syntax$$
$codei%record_hes_cross(%fixed_vec%, %random_vec%)%$$

$head Private$$
This function is $code private$$ to the $code cppad_mixed$$ class
and cannot be used by a derived
$cref/mixed_object/cppad_mixed_derived_ctor/mixed_object/$$.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/cppad_mixed/Fixed Effects, theta/$$
vector $latex \theta$$ at which the recording is made.

$head random_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %random_vec%
%$$
It specifies the value of the
$cref/random effects/cppad_mixed/Random Effects, u/$$
vector $latex u$$ at which the recording is made.

$head hes_cross_row_$$
The input value of this member variable does not matter.
Upon return
$codei%
	hes_cross_row_[%k%] - n_fixed_
%$$
is the random effects index for this cross partial in
$latex f_{u \theta}^{(2)}$$.

$head hes_cross_col_$$
The input value of this member variable does not matter.
Upon return
$codei%
	hes_cross_col_[%k%]
%$$
is the fixed effects index for this cross partial in
$latex f_{u \theta}^{(2)}$$.

$head hes_cross_work_$$
The input value of the member variable
$codei%
	CppAD::sparse_hessian_work hes_cross_work_
%$$
does not matter.
Upon return it contains the necessary information so that
$codei%
	a0_ran_like_.SparseHessian(
		%both_vec%,
		%w%,
		%not_used%,
		hes_cross_row_,
		hes_cross_col_,
		%val_out%,
		hes_cross_work_
	);
%$$
can be used to calculate the non-zero cross terms in
$latex f_{u \theta}^{(2)}$$.

$children%
	example/devel/cppad_mixed/private/hes_cross_xam.cpp
%$$
$head Example$$
The file $cref hes_cross_xam.cpp$$ contains an example
and test of this procedure.
It returns true, if the test passes, and false otherwise.

$end
*/

# define DISMOD_AT_SET_SPARSITY 1

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

void cppad_mixed::record_hes_cross(
	const d_vector& fixed_vec  ,
	const d_vector& random_vec )
{
	assert( ! record_hes_cross_done_ );
	assert( fixed_vec.size() == n_fixed_ );
	assert( random_vec.size() == n_random_ );
	size_t i, j;

	// total number of variables
	size_t n_total = n_fixed_ + n_random_;

	// create a d_vector containing (theta, u)
	d_vector both(n_total);
	pack(fixed_vec, random_vec, both);

	// compute Jacobian sparsity corresponding to parital w.r.t. fixed effects
# if DISMOD_AT_SET_SPARSITY
	typedef CppAD::vector< std::set<size_t> > sparsity_pattern;
	sparsity_pattern r(n_total);
	for(i = 0; i < n_fixed_; i++)
		r[i].insert(i);
# else
	typedef CppAD::vectorBool sparsity_pattern;
	sparsity_pattern r(n_total * n_total);
	for(i = 0; i < n_total; i++)
	{	for(j = 0; j < n_total; j++)
			r[i * n_total + j] = (i < n_fixed_) && (i == j);
	}
# endif
	a0_ran_like_.ForSparseJac(n_total, r);

	// compute sparsity pattern corresponding to paritls w.r.t. (theta, u)
	// of partial w.r.t. theta of f(theta, u)
	bool transpose = true;
	sparsity_pattern s(1), pattern;
# if DISMOD_AT_SET_SPARSITY
	assert( s[0].empty() );
	s[0].insert(0);
# else
	s[0] = true;
# endif
	pattern = a0_ran_like_.RevSparseHes(n_total, s, transpose);


	// User row index for random effect and column index for fixed effect
	hes_cross_row_.clear();
	hes_cross_col_.clear();
# if DISMOD_AT_SET_SPARSITY
	std::set<size_t>::iterator itr;
	for(i = n_fixed_; i < n_total; i++)
	{	for(itr = pattern[i].begin(); itr != pattern[i].end(); itr++)
		{	j = *itr;
			assert( j < n_fixed_ );
			hes_cross_row_.push_back(i);
			hes_cross_col_.push_back(j);
		}
	}
# else
	for(i = n_fixed_; i < n_total; i++)
	{	for(j = 0; j < n_fixed_; j++)
		{	if( pattern[i * n_total + j] )
			{	hes_cross_row_.push_back(i);
				hes_cross_col_.push_back(j);
			}
		}
	}
# endif

	// create a weighting vector
	d_vector w(1);
	w[0] = 1.0;

	// place where results go (not usd here)
	size_t K = hes_cross_row_.size();
	d_vector val_out(K);

	// compute the work vector
	a0_ran_like_.SparseHessian(
		both,
		w,
		pattern,
		hes_cross_row_,
		hes_cross_col_,
		val_out,
		hes_cross_work_
	);
	//
	record_hes_cross_done_ = true;
}


} // END_DISMOD_AT_NAMESPACE
