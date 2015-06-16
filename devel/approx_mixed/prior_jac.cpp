// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# include <dismod_at/approx_mixed.hpp>

/*
$begin approx_mixed_prior_jac$$
$spell
	eval
	vec
	const
	Cpp
	Jacobian
	jac
$$

$section approx_mixed: Jacobian of Prior for Fixed Effects$$

$head Syntax$$
$icode%approx_object%.prior_jac(
	%fixed_vec%, %row_out%, %col_out%, %val_out%
)%$$

$head approx_object$$
We use $cref/approx_object/approx_mixed_derived_ctor/approx_object/$$
to denote an object of a class that is
derived from the $code approx_mixed$$ base class.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/approx_mixed/Fixed Effects, theta/$$
vector $latex \theta$$ at which $latex g^{(1)} ( \theta )$$ is evaluated.

$head row_out$$
This argument has prototype
$codei%
	CppAD::vector<size_t>& %row_out%
%$$
If the input size of this array is non-zero,
the entire vector must be the same
as for a previous call to $code prior_jac$$.
If it's input size is zero,
upon return it contains the row indices for the Jacobian elements
that are possibly non-zero.

$head col_out$$
This argument has prototype
$codei%
	CppAD::vector<size_t>& %col_out%
%$$
If the input size of this array is non-zero,
the entire vector must be the same as for
a previous call to $code prior_jac$$.
If it's input size is zero,
upon return it contains the column indices for the Jacobian elements
that are possibly non-zero (and will have the same size as $icode row_out$$).

$head val_out$$
This argument has prototype
$codei%
	CppAD::vector<double>& %val_out%
%$$
If the input size of this array is non-zero, it must have the same size
as for a previous call to $code prior_jac$$.
Upon return, it contains the value of the Jacobian elements
that are possibly non-zero (and will have the same size as $icode row_out$$).

$children%
	example/devel/approx_mixed/prior_jac_xam.cpp
%$$
$head Example$$
The file $cref prior_jac_xam.cpp$$ contains an example
and test of this procedure.
It returns true, if the test passes, and false otherwise.

$end
*/

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

void approx_mixed::prior_jac(
	const d_vector&        fixed_vec   ,
	CppAD::vector<size_t>& row_out     ,
	CppAD::vector<size_t>& col_out     ,
	d_vector&              val_out     )
{
	// make sure initilialize has been called
	if( prior_density_.size_var() == 0 )
	{	std::cerr << "approx_mixed::initialize was not called before"
		<< " approx_mixed::prior_jac" << std::endl;
		exit(1);
	}

	assert( row_out.size() == col_out.size() );
	assert( row_out.size() == val_out.size() );
	if( row_out.size() == 0 )
	{	row_out = prior_jac_row_;
		col_out = prior_jac_col_;
		val_out.resize( row_out.size() );
	}
# ifndef NDEBUG
	else
	{	size_t n_nonzero = prior_jac_row_.size();
		assert( row_out.size() == n_nonzero );
		for(size_t k = 0; k < n_nonzero; k++)
		{	assert( row_out[k] == prior_jac_row_[k] );
			assert( col_out[k] == prior_jac_col_[k] );
		}
	}
# endif
	// just checking to see if example/devel/model/fit_model_xam is this case
	assert( row_out.size() != 0 );

	CppAD::vector< std::set<size_t> > not_used;
	prior_density_.SparseJacobianForward(
		fixed_vec       ,
		not_used        ,
		row_out         ,
		col_out         ,
		val_out         ,
		prior_jac_work_
	);

	return;
}


} // END_DISMOD_AT_NAMESPACE
