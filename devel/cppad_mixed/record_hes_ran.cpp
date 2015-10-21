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
$begin cppad_mixed_record_hes_ran$$
$spell
	cppad
	hes hes
	vec
	const
	Cpp
	logdet
$$

$section Record Hessian of Random Negative Log-Likelihood w.r.t Random Effects$$

$head Syntax$$
$codei%record_hes_ran(%fixed_vec%, %random_vec%)%$$

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

$head hes_ran_row_, hes_ran_col_$$
The input value of the member variables
$codei%
	CppAD::vector<size_t> hes_ran_row_, hes_ran_col_
%$$
do not matter.
Upon return they contain the row indices and column indices
for the sparse Hessian;
see call to $code SparseHessian$$ below.
These indices are relative to both the fixed and random effects
with the fixed effects coming first.

$subhead Random Effects Index$$
To get the indices relative to just the random effects, subtract
$code n_fixed_$$; i.e.,
$codei%
	hes_ran_row_[%k%] - n_fixed_
	hes_ran_col_[%k%] - n_fixed_
%$$
are between zero and the $code n_random_$$ and
are the row and column indices for the Hessian element
that corresponds to the $th k$$ component of
$icode a1_val_out$$ in the call to $code SparseHessian$$ below.

$subhead Lower Triangle$$
The result are only for the lower triangle of the Hessian; i.e.,
$codei%
	hes_ran_row_[%k%] >= hes_ran_col_[%k%]
%$$

$subhead Order$$
The results are in column major order; i.e.,
$codei%
	hes_ran_col_[%k%] <= hes_ran_col_[%k+1%]
	if( hes_ran_col_[%k%] == hes_ran_col_[%k+1%] )
		hes_ran_row_[%k%] <= hes_ran_row_[%k+1%]
%$$

$head hes_ran_work_$$
The input value of the member variable
$codei%
	CppAD::sparse_hessian_work hes_ran_work_
%$$
does not matter.
Upon return it contains the necessary information so that
$codei%
	a1_ran_like_.SparseHessian(
		%a1_both_vec%,
		%a1_w%,
		%not_used%,
		hes_ran_row_,
		hes_ran_col_,
		%a1_val_out%,
		hes_ran_work_
	);
%$$
can be used to calculate the lower triangle of the sparse Hessian
$latex \[
	f_{uu}^{(2)} ( \theta , u )
\]$$
see $cref/f(theta, u)/
	cppad_mixed_theory/
	Random Negative Log-Likelihood, f(theta, u)
/$$.
Note that the matrix is symmetric and hence can be recovered from
its lower triangle.

$head newton_atom_$$
The input value of the member variable
$codei%
	newton_step newton_atom_
%$$
must be the same as after its constructor; i.e.,
no member functions had been called.
Upon return, $code newton_atom_$$ can be used to compute
the log of the determinant and the Newton step using
$codei%
	newton_atom_(%a1_theta_u_v%, %a1_logdet_step%)
%$$
be more specific, $icode%a1_logdet_step%[0]%$$ is the log of the determinant of
$latex f_{uu}^{(2)} ( \theta , u ) $$ and the rest of the vector is the
Newton step
$latex \[
	s = f_{uu}^{(2)} ( \theta , u )^{-1} v
\] $$


$contents%devel/cppad_mixed/newton_step.cpp
%$$

$end
*/

# define DISMOD_AT_SET_SPARSITY 1

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

void cppad_mixed::record_hes_ran(
	const d_vector& fixed_vec  ,
	const d_vector& random_vec )
{	assert( ! record_hes_ran_done_ );
	assert( fixed_vec.size() == n_fixed_ );
	assert( random_vec.size() == n_random_ );
	size_t i, j;

	// total number of variables
	size_t n_total = n_fixed_ + n_random_;

	//	create an a1d_vector containing (theta, u)
	a1d_vector a1_both(n_total);
	pack(fixed_vec, random_vec, a1_both);

	// compute Jacobian sparsity corresponding to parital w.r.t. random effects
# if DISMOD_AT_SET_SPARSITY
	typedef CppAD::vector< std::set<size_t> > sparsity_pattern;
	sparsity_pattern r(n_total);
	for(i = n_fixed_; i < n_total; i++)
		r[i].insert(i);
# else
	typedef CppAD::vectorBool sparsity_pattern;
	sparsity_pattern r(n_total * n_total);
	for(i = 0; i < n_total; i++)
	{	for(j = 0; j < n_total; j++)
			r[i * n_total + j] = (i >= n_fixed_) && (i == j);
	}
# endif
	a1_ran_like_.ForSparseJac(n_total, r);

	// compute sparsity pattern corresponding to paritls w.r.t. (theta, u)
	// of partial w.r.t. u of f(theta, u)
	bool transpose = true;
	sparsity_pattern s(1), pattern;
# if DISMOD_AT_SET_SPARSITY
	assert( s[0].empty() );
	s[0].insert(0);
# else
	s[0] = true;
# endif
	pattern = a1_ran_like_.RevSparseHes(n_total, s, transpose);


	// determine row and column indices in lower triangle of Hessian
	// and set key for column major sorting
	CppAD::vector<size_t> row, col, key;
# if DISMOD_AT_SET_SPARSITY
	std::set<size_t>::iterator itr;
	for(i = n_fixed_; i < n_total; i++)
	{	for(itr = pattern[i].begin(); itr != pattern[i].end(); itr++)
		{	j = *itr;
			assert( j >= n_fixed_ );
			// only compute lower triangular part of Hessian w.r.t u only
			if( i >= j )
			{	row.push_back(i);
				col.push_back(j);
				key.push_back( i + j * n_total );
			}
		}
	}
# else
	for(i = n_fixed_; i < n_total; i++)
	{	for(j = n_fixed_; j < n_total; j++)
		{	if( pattern[i * n_total + j] && i >= j )
			{	// only compute lower triangular of Hessian w.r.t u only
				if( i >= j )
				{	row.push_back(i);
					col.push_back(j);
					key.push_back( i + j * n_total );
				}
			}
		}
	}
# endif
	// set hes_ran_row_ and hes_ran_col_ in colum major order
	size_t K = row.size();
	CppAD::vector<size_t> ind(K);
	CppAD::index_sort(key, ind);
	hes_ran_row_.resize(K);
	hes_ran_col_.resize(K);
	for(size_t k = 0; k < row.size(); k++)
	{	hes_ran_row_[k] = row[ ind[k] ];
		hes_ran_col_[k] = col[ ind[k] ];
	}

	// create a weighting vector
	a1d_vector a1_w(1);
	a1_w[0] = 1.0;

	// place where results go (not usd here)
	a1d_vector a1_val_out( hes_ran_row_.size() );

	// compute the work vector
	a1_ran_like_.SparseHessian(
		a1_both,
		a1_w,
		pattern,
		hes_ran_row_,
		hes_ran_col_,
		a1_val_out,
		hes_ran_work_
	);
	//
	newton_atom_.initialize(a1_ran_like_, fixed_vec, random_vec);
	//
	record_hes_ran_done_ = true;
}


} // END_DISMOD_AT_NAMESPACE
