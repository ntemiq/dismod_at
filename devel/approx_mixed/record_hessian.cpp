// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rate Estimation as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# include <dismod_at/approx_mixed.hpp>

/*
$begin approx_mixed_record_hessian$$
$spell
	vec
	const
	Cpp
$$

$section approx_mixed: Record the Hessian of the Joint Density$$

$head Syntax$$
$codei%record_hessian(%fixed_vec%, %random_vec%)%$$

$head Private$$
This function is $code private$$ to the $code approx_mixed$$ class
and cannot be used by a derived
$cref/approx_object/approx_mixed_derived_ctor/approx_object/$$.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<a1_double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/approx_mixed/Fixed Effects, theta/$$
vector $latex \theta$$ at which the recording is made.

$head random_vec$$
This argument has prototype
$codei%
	const CppAD::vector<a1_double>& %random_vec%
%$$
It specifies the value of the
$cref/random effects/approx_mixed/Random Effects, u/$$
vector $latex u$$ at which the recording is made.

$head hessian_$$
The input value of the member variable
$codei%
	CppAD::ADFun<a1_double> hessian_
%$$
does not matter.
Upon return it contains the corresponding recording for the lower triangle of
$latex f^{(2)} ( \theta , u )$$.
Note that the matrix is symmetric and hence can be recovered from
its lower triangle.

$head hessian_row_, hessian_col_$$
The input value of the member variables
$codei%
	CppAD::vector<size_t> hessian_row_, hessian_col_
%$$
do not matter.
Upon return the contain the row indices and column indices
for the sparse Hessian represented by $code hessian_$$; i.e.
$codei%hessian_row_[%k%]%$$ and $codei%hessian_col_[%k%]%$$
are the row and column indices for the Hessian element
that corresponds to the $th k$$ component of the function
corresponding to $code hessian_$$.

$head joint_density$$
The member function $code joint_density$$ called
with arguments of type $code a3d_vector$$.

$end
*/

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

void approx_mixed::record_hessian(
	const a1d_vector& fixed_vec  ,
	const a1d_vector& random_vec )
{	size_t i, j;
	size_t n_both = n_fixed_ + n_random_;

	//	create an a2d_vector containing (theta, u)
	a2d_vector a2_both(n_both);
	for(j = 0; j < n_fixed_; j++)
		a2_both[j] = fixed_vec[j];
	for(j = 0; j < n_random_; j++)
		a2_both[n_fixed_ + j] = random_vec[j];

	//	create an a3d_vector containing (theta, u)
	a3d_vector a3_both(n_both);
	for(j = 0; j < n_both; j++)
		a3_both[j] = a2_both[j];

	// start recording using a3_double operations
	CppAD::Independent( a3_both );

	// extract theta and u from both
	a3d_vector a3_theta(n_fixed_), a3_u(n_random_);
	for(j = 0; j < n_fixed_; j++)
		a3_theta[j] = a3_both[j];
	for(j = 0; j < n_random_; j++)
		a3_u[j] = a3_both[n_fixed_ + j];

	// compute f(theta, u) using a3_double operations
	a3d_vector a3_vec = joint_density(a3_theta, a3_u);
	a3d_vector a3_sum(1);
	a3_sum[0]    = a3_vec[0];
	size_t n_abs = a3_vec.size() - 1;
	for(i = 0; i < n_abs; i++)
		a3_sum[0] += abs( a3_vec[1 + i] );
	CppAD::ADFun<a2_double> a2_f;
	a2_f.Dependent(a3_both, a3_sum);

	// compute sparsity pattern corresponding to f^{(1)} (theta, u)
	typedef CppAD::vector< std::set<size_t> > sparsity_pattern;
	sparsity_pattern r(n_both);
	for(i = 0; i < n_both; i++)
		r[i].insert(i);
	a2_f.ForSparseJac(n_both, r);

	// compute sparsity pattern corresponding to f^{(2)} (theta, u)
	sparsity_pattern s(1);
	assert( s[0].empty() );
	s[0].insert(0);
	sparsity_pattern pattern = a2_f.RevSparseHes(n_both, s);

	// determine row and column indices in lower triangle of Hessian
	hessian_row_.clear();
	hessian_col_.clear();
	std::set<size_t>::iterator itr;
	for(i = n_fixed_; i < n_both; i++)
	{	for(itr = pattern[i].begin(); itr != pattern[i].end(); itr++)
		{	j = *itr;
			// only compute lower triangular part 
			// and exclude the upper block f_{theta theta}^{(2)} (theta, u)
			if( i >= n_fixed_ && i >= j )
			{	hessian_row_.push_back(i);
				hessian_col_.push_back(j);
			}
		}
	}
	size_t K = hessian_row_.size();

	// start recording using a2_double operations
	CppAD::Independent( a2_both );

	// compute lower triangle of sparse Hessian f^{(2)} (theta , u)
	a2d_vector a2_w(1), a2_hes(K);
	a2_w[0] = 1.0;
	CppAD::sparse_hessian_work work;
	a2_f.SparseHessian(
		a2_both, a2_w, pattern, hessian_row_, hessian_col_, a2_hes, work
	);

	// complete recording of f^{(2)} (theta, u)
	hessian_.Dependent(a2_both, a2_hes);

	// optimize the recording
	hessian_.optimize();
}


} // END_DISMOD_AT_NAMESPACE
