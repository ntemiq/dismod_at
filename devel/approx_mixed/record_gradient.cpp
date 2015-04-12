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
$begin approx_mixed_record_gradient$$
$spell
	vec
	const
	Cpp
$$

$section approx_mixed: Record Gradient of Joint Density w.r.t Random Effects$$

$head Syntax$$
$codei%record_gradient(%fixed_vec%, %random_vec%)%$$

$head Private$$
This function is $code private$$ to the $code approx_mixed$$ class
and cannot be used by a derived
$cref/approx_object/approx_mixed_derived_ctor/approx_object/$$.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %fixed_vec%
%$$
It specifies the value of the
$cref/fixed effects/approx_mixed/Fixed Effects, theta/$$
vector $latex \theta$$ at which the recording is made.

$head random_vec$$
This argument has prototype
$codei%
	const CppAD::vector<double>& %random_vec%
%$$
It specifies the value of the
$cref/random effects/approx_mixed/Random Effects, u/$$
vector $latex u$$ at which the recording is made.

$head gradient_$$
The input value of the member variable
$codei%
	CppAD::ADFun<a2_double> gradient_
%$$
does not matter.
Upon return it contains the corresponding recording for the gradient
$latex f_u^{(1)} ( \theta , u )$$.


$end
*/

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

void approx_mixed::record_gradient(
	const d_vector& fixed_vec  ,
	const d_vector& random_vec )
{
	//	create an a3d_vector containing (theta, u)
	a3d_vector a3_both( n_fixed_ + n_random_ );
	pack(fixed_vec, random_vec, a3_both);

	// start recording f_uu (theta, u) using a3_double operations
	CppAD::Independent( a3_both );

	// create an a4d_vector containing theta and u
	a4d_vector a4_theta(n_fixed_), a4_u(n_random_);
	unpack(a4_theta, a4_u, a3_both);

	// compute f(u) using a4_double operations
	CppAD::Independent(a4_u);
	//
	a4d_vector a4_both(n_fixed_ + n_random_);
	pack(a4_theta, a4_u, a4_both);
	a4d_vector a4_vec = a4_joint_density_.Forward(0, a4_both);
	a4d_vector a4_sum(1);
	a4_sum[0]    = a4_vec[0];
	size_t n_abs = a4_vec.size() - 1;
	for(size_t i = 0; i < n_abs; i++)
		a4_sum[0] += abs( a4_vec[1 + i] );
	CppAD::ADFun<a3_double> a3_f;
	a3_f.Dependent(a4_u, a4_sum);

	// zero order forward mode
	a3d_vector a3_theta(n_fixed_), a3_u(n_random_);
	unpack(a3_theta, a3_u, a3_both);
	a3_f.Forward(0, a3_u);

	// first order reverse
	a3d_vector a3_grad(n_random_), a3_w(1);
	a3_w[0] = a3_double( 1.0 );
	a3_grad = a3_f.Reverse(1, a3_w);

	// complete recording of f_u^{(1)} (u, theta)
	gradient_.Dependent(a3_both, a3_grad);

	// optimize the recording
	gradient_.optimize();
}


} // END_DISMOD_AT_NAMESPACE
