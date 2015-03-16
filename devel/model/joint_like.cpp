// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rate Estimation as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
/*
$begin joint_like_ctor$$
$spell
	vec
	var
	const
	CppAD
	struct
	std
$$

$section Joint Likelihood$$

$head Syntax$$
$codei%joint_like %joint_object%(%data_object%, %prior_object%)%$$

$head Purpose$$
This object can be used to evaluate the joint likelihood of the
$cref/fixed effects/model_variable/Fixed Effects, theta/$$, $latex \theta$$,
$cref/random effects/model_variable/Random Effects, u/$$, $latex u$$,
and the
$cref/measurement vector/data_like/Data Table Notation/y_i/$$, $latex y$$
as a function of the fixed and random effects; i.e., to evaluate
$latex \[
f( u , \theta ) = \B{p}( y | u , \theta ) \B{p}( u | \theta ) \B{p}( \theta )
\] $$
up to a constant multiple that does not depend on
$latex u$$ or $latex \theta$$.

$head References$$
The $icode joint_object$$ will hold a reference to its arguments
$icode data_object$$, $icode pack_object$$, and $icode prior_table$$.
For this reason, $icode joint_object$$ cannot be used once after
$icode data_object$$, $icode pack_object$$, or $icode prior_table$$
has been deleted.

$head data_object$$
This argument has prototype
$codei%
	const data_model& %data_object%
%$$
see $cref/data_model/devel_data_model/$$,
and contains the information necessary for evaluation of
$latex \B{p}( y | u , \theta )$$.

$head pack_object$$
This argument has prototype
$codei%
	const pack_info& %pack_object%
%$$
and is the $cref pack_info$$ information corresponding to corresponding
to this problem.

$head prior_table$$
This argument has prototype
$codei%
	const CppAD::vector<prior_struct>&  %prior_table%
%$$
and is the $cref/prior_table/get_prior_table/prior_table/$$.
Only to following fields are used
$cref/density_id/prior_table/density_id/$$,
$cref/mean/prior_table/mean/$$,
$cref/std/prior_table/std/$$,
$cref/eta/prior_table/eta/$$.

$end
-----------------------------------------------------------------------------
*/
# include <dismod_at/joint_like.hpp>

namespace dismod_at { // BEGIN DISMOD_AT_NAMESPACE

// consctructor
joint_like::joint_like(
	const pack_info&      pack_object  ,
	const data_model&     data_object  ,
	const prior_density&  prior_object )
:
pack_object_(pack_object)  ,
data_object_(data_object)  ,
prior_object_(prior_object)
{ }

/*
-----------------------------------------------------------------------------
$begin joint_like_eval$$
$spell
	CppAD
	const
	vec
	eval
	struct
$$

$section Evaluating the Joint Likelihood$$

$head Syntax$$
$icode%residual_vec% = %joint_object%.eval(%fixed_vec%, %random_vec%)%$$

$head Float$$
The type $icode Float$$ must be one of the following:
$code double$$, $code AD<double>$$, $code AD< AD<double> >$$,
where $code AD$$ is $code CppAD::AD$$.

$head fixed_vec$$
This argument has prototype
$codei%
	const CppAD::vector<%Float%>& %fixed_vec%
%$$
Its order is the same as for
$cref/put_fixed_effect/fixed_effect/put_fixed_effect/$$.

$head random_vec$$
This argument has prototype
$codei%
	const CppAD::vector<%Float%>& %random_vec%
%$$
Its order is the same as for
$cref/put_random_effect/random_effect/put_random_effect/$$.

$head residual_vec$$
The return value has prototype
$codei%
	CppAD::vector< residual_struct<%Float%> > %residual_vec%
%$$
The order of the residuals is unspecified (at this time).
The log of the prior density for the
$cref/fixed/model_variable/Fixed Effects, theta/$$
and $cref/random/model_variable/Random Effects, u/$$ effects,
and the data
$cref/measurement vector/data_like/Data Table Notation/y_i/$$, $latex y$$,
$latex \[
	\B{p}( y | u , \theta ) \B{p}( u | \theta ) \B{p}( \theta )
\] $$
is the sum of the log of the probabilities corresponding to all the
$cref/residuals/residual_density/$$ in $icode residual_vec$$.

$end
-----------------------------------------------------------------------------
*/

template <class Float>
CppAD::vector< residual_struct<Float> > joint_like::eval(
	const CppAD::vector<Float>&  fixed_vec  ,
	const CppAD::vector<Float>&  random_vec )
{	CppAD::vector<Float> pack_vec( pack_object_.size() );

	// fixed_vec -> pack_vec
	get_fixed_effect(pack_object_, pack_vec, fixed_vec);

	// random_vec -> pack_vec
	get_random_effect(pack_object_, pack_vec, random_vec);

	// prior density
	CppAD::vector< residual_struct<Float> > prior_residual_vec;
	prior_residual_vec = prior_object_.eval(pack_vec);
	size_t n_prior = prior_residual_vec.size();

	// data likelihood
	CppAD::vector< residual_struct<Float> > data_residual_vec;
	data_residual_vec = data_object_.like_all(pack_vec);
	size_t n_data = data_residual_vec.size();

	// combine as one vector
	CppAD::vector< residual_struct<Float> >& residual_vec(n_prior + n_data);
	for(size_t i = 0; i < n_prior; i++)
		residual_vec[i] = prior_residual_vec[i];
	for(size_t i = 0; i < n_data; i++)
		residual_vec[n_prior + i] = data_residual_vec[i];

	// result
	return residual_vec;
}



} // END DISMOD_AT_NAMESPACE
