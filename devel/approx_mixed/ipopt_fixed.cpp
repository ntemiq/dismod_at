// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rate Estimation as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# include <dismod_at/ipopt_fixed.hpp>

namespace {

	// merge two (row, col) sparsity patterns into one
	void merge_sparse(
		const CppAD::vector<size_t>& row_one      , // first sparsity pattern
		const CppAD::vector<size_t>& col_one      ,
		const CppAD::vector<size_t>& row_two      , // second sparsity pattern
		const CppAD::vector<size_t>& col_two      ,
		CppAD::vector<size_t>&       row_out      , // merged sparsity pattern
		CppAD::vector<size_t>&       col_out      ,
		CppAD::vector<size_t>&       one_2_out    , // maps first into merged
		CppAD::vector<size_t>&       two_2_out    ) // maps second into merged
	{	assert( row_out.size() == 0 );
		assert( col_out.size() == 0 );
		//
		assert( row_one.size() == col_one.size() );
		assert( row_one.size() == one_2_out.size() );
		//
		assert( row_two.size() == col_two.size() );
		assert( row_two.size() == two_2_out.size() );
		//
		//
		size_t n_one = row_one.size();
		size_t n_two = row_two.size();
		//
		// compute maximum column index
		size_t max_col = 0;
		for(size_t k = 0; k < n_one; k++)
			max_col = std::max( col_one[k], max_col );
		for(size_t k = 0; k < n_two; k++)
			max_col = std::max( col_two[k], max_col );
		//
		// keys for sorting
		CppAD::vector<size_t> key_one(n_one), key_two(n_two);
		for(size_t k = 0; k < n_one; k++)
			key_one[k] = row_one[k] * max_col + col_one[k];
		for(size_t k = 0; k < n_two; k++)
			key_two[k] = row_two[k] * max_col + col_two[k];
		//
		// sort both
		CppAD::vector<size_t> ind_one(n_one), ind_two(n_two);
		CppAD::index_sort(key_one, ind_one);
		CppAD::index_sort(key_two, ind_two);
		//
		// now merge into row_out and col_out
		size_t k_one = 0, k_two = 0;
		while( k_one < n_one && k_two < n_two )
		{	if( key_one[k_one] == key_two[k_two] )
			{	assert( row_one[k_one] == row_two[k_two] );
				assert( col_one[k_one] == col_two[k_two] );
				//
				row_out.push_back( row_one[k_one] );
				col_out.push_back( col_one[k_one] );
				//
				one_2_out[k_one] = row_out.size();
				two_2_out[k_two] = row_out.size();
				//
				k_one++;
				k_two++;
			}
			else if( key_one[k_one] < key_two[k_two] )
			{	row_out.push_back( row_one[k_one] );
				col_out.push_back( col_one[k_one] );
				one_2_out[k_one] = row_out.size();
				k_one++;
			}
			else
			{	assert( key_two[k_two] < key_one[k_one] );
				row_out.push_back( row_two[k_two] );
				col_out.push_back( col_two[k_two] );
				two_2_out[k_two] = row_out.size();
				k_two++;
			}
		}
		while( k_one < n_one )
		{	row_out.push_back( row_one[k_one] );
			col_out.push_back( col_one[k_one] );
			k_one++;
		}
		while( k_two < n_two )
		{	row_out.push_back( row_two[k_two] );
			col_out.push_back( col_two[k_two] );
			k_two++;
		}
		//
		return;
	}
}

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE
/* $$
$begin ipopt_fixed_ctor$$
$spell
	hes
	vec
	eval
	ipopt
	const
	CppAD
	nnz_jac
	Jacobian
$$

$section Ipopt Example: Constructor and Destructor$$

$head Syntax$$
$codei%ipopt_fixed %ipopt_object%(
	%fixed_lower%, %fixed_in%, %fixed_upper%, %random_in%, %approx_object%
)%$$

$head Prototype$$
The arguments has prototype
$codei%
	const CppAD::vector<double>& %fixed_lower%
	const CppAD::vector<double>& %fixed_in%
	const CppAD::vector<double>& %fixed_upper%
	const CppAD::vector<double>& %random_in%
	approx_mixed&                %approx_object%
%$$

$head References$$
The values of the arguments are stored by reference and hence
the arguments must not be deleted while $icode ipopt_object$$
is still being used.

$head fixed_lower$$
specifies the lower limits for the
$cref/fixed_effects/model_variable/Fixed Effects, theta/$$.
Note that
$code%
	- std::numeric_limits<double>::infinity()
%$$
is used for minus infinity; i.e., no lower bound.

$head fixed_in$$
specifies the initial value (during optimization) for the fixed effects.
It must hold for each $icode j$$ that
$codei%
	%fixed_lower%[%j%] <= %fixed_in%[%j%] <= %fixed_upper%[%j%]
%$$

$head fixed_upper$$
specifies the upper limits for the fixed effects.
Note that
$code%
	std::numeric_limits<double>::infinity()
%$$
is used for plus infinity; i.e., no upper bound.

$head random_in$$
specifies the initial value (for initial optimization) of the random effects.

$head approx_object$$
The argument $icode approx_object$$ is an object of a class that is
derived from the $code approx_mixed$$ base class.

$head Non-Const Member Variables$$
The following member variables are set by the constructor
and should not be modified.

$subhead prior_n_abs_$$
number of absolute value terms in the
$cref/prior_density/approx_mixed_prior_density/$$.

$head prior_nnz_jac_$$
number of non-zeros in the Jacobian of the prior density.

$head lag_hes_row_$$
The row indices for the sparse representation of the Hessian
of the Lagrangian (for any Lagrange multiplier values).

$head lag_hes_col_$$
The column indices for the sparse representation of the Hessian
of the Lagrangian (for any Lagrange multiplier values).


$end
*/
ipopt_fixed::ipopt_fixed(
	const d_vector&     fixed_lower   ,
	const d_vector&     fixed_in      ,
	const d_vector&     fixed_upper   ,
	const d_vector&     random_in     ,
	approx_mixed&       approx_object ) :
n_fixed_       ( fixed_in.size()  )   ,
n_random_      ( random_in.size() )   ,
fixed_lower_   ( fixed_lower      )   ,
fixed_in_      ( fixed_in         )   ,
fixed_upper_   ( fixed_upper      )   ,
random_in_     ( random_in        )   ,
approx_object_ ( approx_object    )
{
	// -----------------------------------------------------------------------
	// set nlp_lower_bound_inf_, nlp_upper_bound_inf_
	// -----------------------------------------------------------------------
	nlp_lower_bound_inf_ = - 1e19;
	nlp_upper_bound_inf_ = + 1e19;
	double inf           = std::numeric_limits<double>::infinity();
	for(size_t j = 0; j < n_fixed_; j++)
	{	if( fixed_lower[j] != - inf ) nlp_lower_bound_inf_ =
				std::min(nlp_lower_bound_inf_, fixed_lower[j] );
		//
		if( fixed_upper[j] != inf ) nlp_upper_bound_inf_ =
				std::max(nlp_upper_bound_inf_, fixed_upper[j] );
	}
	// -----------------------------------------------------------------------
	// set prior_n_abs_
	// -----------------------------------------------------------------------
	// prior density at the initial fixed effects vector
	d_vector prior_vec = approx_object_.prior_eval(fixed_in);
	assert( prior_vec.size() > 0 );
	prior_n_abs_ = prior_vec.size() - 1;
	// -----------------------------------------------------------------------
	// set prior_nnz_jac_
	// -----------------------------------------------------------------------
	s_vector prior_row, prior_col;
	d_vector prior_val;
	approx_object.prior_jac(fixed_in, prior_row, prior_col, prior_val);
	prior_nnz_jac_ = prior_row.size();
	// -----------------------------------------------------------------------
	// set lag_hes_row_, lag_hes_col_, laplace_2_lag_, prior_2_lag_
	// -----------------------------------------------------------------------
	// row and column indices for contribution from joint density
	s_vector laplace_row, laplace_col;
	d_vector laplace_val;
	approx_object.laplace_hes_fix(
		fixed_in, random_in, laplace_row, laplace_col, laplace_val
	);
	// row and column indices for contribution form prior density
	prior_row.clear();
	prior_col.clear();
	prior_val.clear();
	d_vector weight( n_fixed_ );
	for(size_t i = 0; i < weight.size(); i++)
		weight[i] = 1.0;
	approx_object.prior_hes(fixed_in, weight, prior_row, prior_col, prior_val);
	//
	// merge to form sparsity for Lagrangian
	merge_sparse(
		laplace_row      ,
		laplace_col      ,
		prior_row        ,
		prior_col        ,
		lag_hes_row_     ,
		lag_hes_col_     ,
		laplace_2_lag_   ,
		prior_2_lag_
	);
	// -----------------------------------------------------------------------
	// set size of fixed_tmp_, random_tmp_, prior_vec_tmp_
	// -----------------------------------------------------------------------
	fixed_tmp_.resize( n_fixed_ );
	random_tmp_.resize( n_random_ );
	prior_vec_tmp_.resize( prior_n_abs_ + 1 );
	// -----------------------------------------------------------------------
}
ipopt_fixed::~ipopt_fixed(void)
{ }
/* $$
$end
------------------------------------------------------------------------------
$begin ipopt_fixed_get_nlp_info$$
$spell
	ipopt_fixed_get_nlp_info
	nnz_jac
	Jacobian
	bool
	Enum
	bool
	nlp
$$

$section Return Information About Problem Sizes$$

$head Syntax$$
$icode%ok% = get_nlp_info(%n%, %m%, %nnz_jac_g%, %nnz_h_lag%, %index_style%)%$$

$head n$$
is set to the number of variables in the problem (dimension of x).

$head m$$
is set to the number of constraints in the problem (dimension of g(x)).

$head nnz_jac_g$$
is set to the number of nonzero entries in the Jacobian of g(x).

$head nnz_h_lag$$
is set to the number of nonzero entries in the Hessian of the Lagrangian
$latex f(x) + \lambda^\R{T} g(x)$$.

$head index_style$$
is set to the numbering style used for row/col entries in the sparse matrix
format (C_STYLE: 0-based, FORTRAN_STYLE: 1-based).

$head ok$$
is set to true.
If set to false, the optimization would terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$end
-------------------------------------------------------------------------------
*/
bool ipopt_fixed::get_nlp_info(
	Index&          n            ,  // out
	Index&          m            ,  // out
	Index&          nnz_jac_g    ,  // out
	Index&          nnz_h_lag    ,  // out
	IndexStyleEnum& index_style  )  // out
{
	n           = n_fixed_;
	m           = 2 * prior_n_abs_;
	nnz_jac_g   = 2 * prior_nnz_jac_;
	nnz_h_lag   = lag_hes_row_.size();
	index_style = C_STYLE;
	//
	return true;
}
/*
-------------------------------------------------------------------------------
$begin ipopt_fixed_get_bounds_info$$
$spell
	ipopt
	bool
$$

$section Return Optimization Bounds$$

$head Syntax$$
$icode%ok% = get_bounds_info(%n%, %x_l%, %x_u%, %m%, %g_l%, %g_u%)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head x_l$$
set to the lower bounds for $icode x$$ (has size $icode n$$).

$head x_u$$
set to the upper bounds for $icode x$$ (has size $icode n$$).

$head m$$
is the number of constraints in the problem (dimension of g(x)).

$head g_l$$
set to the lower bounds for $icode g(x)$$ (has size $icode m$$).

$head g_u$$
set to the upper bounds for $icode g(x)$$ (has size $icode m$$).

$head ok$$
is set to true.
If set to false, the optimization would terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$end
-------------------------------------------------------------------------------
*/
bool ipopt_fixed::get_bounds_info(
		Index       n        ,   // in
		Number*     x_l      ,   // out
		Number*     x_u      ,   // out
		Index       m        ,   // in
		Number*     g_l      ,   // out
		Number*     g_u      )   // out
{
	assert( n > 0 && size_t(n) == n_fixed_ + prior_n_abs_ );
	assert( m >= 0 && size_t(m) == 2 * prior_n_abs_ );

	for(size_t j = 0; j < n_fixed_; j++)
	{	// map infinity to crazy value required by ipopt
		if( fixed_lower_[j] == - std::numeric_limits<double>::infinity() )
			x_l[j] = nlp_lower_bound_inf_;
		else
			x_l[j] = fixed_lower_[j];
		//
		if( fixed_upper_[j] == std::numeric_limits<double>::infinity() )
			x_l[j] = nlp_upper_bound_inf_;
		else
			x_u[j] = fixed_upper_[j];
	}
	// auxillary varibles for absolute value terms
	for(size_t j = 0; j < prior_n_abs_; j++)
	{	x_l[n_fixed_ + j] = nlp_lower_bound_inf_;
		x_u[n_fixed_ + j] = nlp_upper_bound_inf_;
	}
	//
	// constraints for absolute value terms
	for(size_t j = 0; j < 2 * prior_n_abs_; j++)
	{	g_l[j] = 0.0;
		g_u[j] = nlp_upper_bound_inf_;
	}
	//
	return true;
}
/*
-------------------------------------------------------------------------------
$begin ipopt_fixed_get_starting_point$$
$spell
	init
	ipopt
	bool
$$

$section Return Initial Values Where Optimization is Started$$

$head Syntax$$
$icode%ok% = get_starting_point(
	%n%, %init_x%, %x%, %init_z%, %z_L%, %z_U%, %m%, %init_lambda%, %lambda%
)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head init_x$$
if true, the ipopt options specify that the this routine
will provide an initial value for $icode x$$.

$head x$$
if $icode init_x$$ is true,
set to the initial value for the primal variables (has size $icode n$$).

$head init_z$$
assumes $icode init_z$$ is false.
If it were true, the ipopt options specify that the this routine
will provide an initial value for $icode x$$ upper and lower bound
multipliers.

$head z_L$$
if $icode init_z$$ is true,
set to the initial value for the lower bound multipliers (has size $icode n$$).

$head z_U$$
if $icode init_z$$ is true,
set to the initial value for the upper bound multipliers (has size $icode n$$).

$head init_lambda$$
assumes $icode init_lambda$$ is false.
If it were true, the ipopt options specify that the this routine
will provide an initial value for $icode g(x)$$ upper and lower bound
multipliers.

$head lambda$$
if $icode init_lambda$$ is true,
set to the initial value for the $icode g(x)$$ multipliers
(has size $icode m$$).

$head ok$$
is set to true.
If set to false, the optimization would terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$end
-------------------------------------------------------------------------------
*/
bool ipopt_fixed::get_starting_point(
	Index           n            ,  // in
	bool            init_x       ,  // in
	Number*         x            ,  // out
	bool            init_z       ,  // in
	Number*         z_L          ,  // out
	Number*         z_U          ,  // out
	Index           m            ,  // out
	bool            init_lambda  ,  // in
	Number*         lambda       )  // out
{
	assert( init_x == true );
	assert( init_z == false );
	assert( init_lambda == false );
	assert( n > 0 && size_t(n) == n_fixed_ + prior_n_abs_ );
	assert( m >= 0 && size_t(m) == 2 * prior_n_abs_ );

	// prior density at the initial fixed effects vector
	d_vector vec = approx_object_.prior_eval(fixed_in_);
	assert( vec.size() > 1 + prior_n_abs_ );

	for(size_t j = 0; j < n_fixed_; j++)
		x[j] = fixed_in_[j];
	for(size_t j = 0; j < prior_n_abs_; j++)
		x[n_fixed_ + j] = CppAD::abs( vec[1 + j] );

	return true;
}
/*
-------------------------------------------------------------------------------
$begin ipopt_fixed_eval_f$$
$spell
	ipopt
	bool
	eval
	obj
	const
$$

$section Compute Value of Objective$$

$head Syntax$$
$icode%ok% = eval_f(%n%, %x%, %new_x%, %obj_value%)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head x$$
is the value for the primal variables at which the objective
f(x) is computed (has size $icode n$$).

$head new_x$$
if true, no Ipopt evaluation method was previous called with the same
value for $icode x$$.

$head obj_val$$
set to the initial value of the objective function f(x).

$head ok$$
returns true.
If set to false, the optimization would terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$end
-------------------------------------------------------------------------------
*/
bool ipopt_fixed::eval_f(
	Index           n         ,  // in
	const Number*   x         ,  // in
	bool            new_x     ,  // in
	Number&         obj_value )  // out
{
	assert( n > 0 && size_t(n) == n_fixed_ + prior_n_abs_ );
	//
	// check if we are initializing optimal value so far
	if( fixed_opt_.size() == 0 )
	{	objective_opt_ = std::numeric_limits<double>::infinity();
		fixed_opt_.resize(n_fixed_);
		random_opt_.resize(n_random_);

		// using random_in_ for initial random effects
		random_tmp_ = random_in_;
	}
	else
	{	// using so far optimal random effects
		random_tmp_ = random_opt_;
	}
	//
	// value of fixed effects corresponding to this x
	for(size_t j = 0; j < n_fixed_; j++)
		fixed_tmp_[j] = x[j];
	//
	// compute the optimal random effects corresponding to fixed effects
	random_tmp_ = approx_object_.optimize_random(fixed_tmp_, random_tmp_);
	//
	// compute joint part of the Laplace objective
	double H = approx_object_.laplace_eval(
		fixed_tmp_, fixed_tmp_, random_tmp_
	);
	//
	// prior part of objective
	prior_vec_tmp_ = approx_object_.prior_eval(fixed_tmp_);
	//
	// only include smooth part of prior in objective
	obj_value = H + prior_vec_tmp_[0];
	//
	// use contraints to represent absolute value part
	for(size_t j = 0; j < prior_n_abs_; j++)
		obj_value += x[n_fixed_ + j];
	//
	// compute the true objective (without constraints)
	double obj_tmp = H + prior_vec_tmp_[0];
	for(size_t j = 0; j < prior_n_abs_; j++)
		obj_tmp += CppAD::abs( prior_vec_tmp_[1+j] );
	//
	// check if so far optimal
	if( obj_tmp < objective_opt_ )
	{	objective_opt_ = obj_tmp;
		fixed_opt_     = fixed_tmp_;
		random_opt_    = random_tmp_;
	}
	return true;
}
/*
-------------------------------------------------------------------------------
$begin ipopt_fixed_eval_grad_f$$
$spell
	ipopt
	bool
	eval
	const
$$

$section Compute Gradient of the Objective$$

$head Syntax$$
$icode%ok% = eval_grad_f(%n%, %x%, %new_x%, %grad_f%)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head x$$
is the value for the primal variables at which the gradient
$latex \nabla f(x)$$ is computed (has size $icode n$$).

$head new_x$$
if true, no Ipopt evaluation method was previous called with the same
value for $icode x$$.

$head grad_f$$
is set to the value for the gradient $latex \nabla f(x)$$
(has size $icode m$$).

$head ok$$
if set to false, the optimization will terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$head Source$$
$codep */
bool ipopt_fixed::eval_grad_f(
	Index           n         ,  // in
	const Number*   x         ,  // in
	bool            new_x     ,  // in
	Number*         grad_f    )  // out
{
	assert( n == 2 );
	grad_f[0] = 0.0;
	grad_f[1] = - 2.0 * (x[1] - 2.0);
	return true;
}
/* $$
$end
-------------------------------------------------------------------------------
$begin ipopt_fixed_eval_g$$
$spell
	ipopt
	bool
	const
	eval
$$

$section Compute Value of Constraint Functions$$

$head Syntax$$
$icode%ok% = eval_g(%n%, %x%, %new_x%, %m%, %g%)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head x$$
is the value for the primal variables at which the constraints
$latex g(x)$$ is computed (has size $icode n$$).

$head new_x$$
if true, no Ipopt evaluation method was previous called with the same
value for $icode x$$.

$head m$$
is the number of constraints in the problem (dimension of g(x)).

$head g$$
is set to the value for the constraint functions (has size $icode m$$).

$head ok$$
if set to false, the optimization will terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$head Source$$
$codep */
bool ipopt_fixed::eval_g(
	Index           n        ,  // in
	const Number*   x        ,  // in
	bool            new_x    ,  // in
	Index           m        ,  // in
	Number*         g        )  // out
{
	assert( n == 2 );
	//
	//
	assert( m = 1 );
	//
	g[0] = - (x[0] * x[0] + x[1] - 1.0);
	//
	return true;
}
/* $$
$end
-------------------------------------------------------------------------------
$begin ipopt_fixed_eval_jac_g$$
$spell
	ipopt
	bool
	eval
	const
	nele_jac
	Jacobian
	nnz
$$

$section Compute Jacobian of Constraint Functions$$

$head Syntax$$
$icode%ok% = eval_jac_g(
	%n%, %x%, %new_x%, %m%, %nele_jac%, %iRow%, %jCol%, %values%
)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head x$$
is the value for the primal variables at which the Jacobian
of the constraints $latex \nabla g(x)$$ is computed (has size $icode n$$).

$head new_x$$
if true, no Ipopt evaluation method was previous called with the same
value for $icode x$$.

$head m$$
is the number of constraints in the problem (dimension of g(x)).

$head nele_jac$$
is the number of non-zero elements in the Jacobian of $icode g(x)$$; i.e.,
the same as
$cref/nnz_jac_g/ipopt_fixed_get_nlp_info/nnz_jac_g/$$.

$head iRow$$
If $icode values$$ is $code NULL$$,
$icode iRow$$ has size $icode nele_jac$$ and is set to the
row indices for the non-zero entries in the Jacobian of the constraints
$latex g^{(1)} (x)$$.

$head jCol$$
If $icode values$$ is $code NULL$$,
$icode jCol$$ has size $icode nele_jac$$ and is set to the
column indices for the non-zero entries in the Jacobian of the constraints
$latex g^{(1)} (x)$$.

$head values$$
If $icode values$$ is not $code NULL$$,
it has size $icode nele_jac$$ and $icode%values%[%k%]%$$
is set to the value of element of the Jacobian $latex g^{(1)} (x)$$
with row index $icode%iRow%[%k%]%$$
and column index $icode%jCol%[%k%]%$$.

$head ok$$
if set to false, the optimization will terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$head Source$$
$codep */
bool ipopt_fixed::eval_jac_g(
	Index           n        ,  // in
	const Number*   x        ,  // in
	bool            new_x    ,  // in
	Index           m        ,  // in
	Index           nele_jac ,  // in
	Index*          iRow     ,  // out
	Index*          jCol     ,  // out
	Number*         values   )  // out
{
	assert( nele_jac == 2 );
	if( values == NULL )
	{
		iRow[0] = 0;
		jCol[0] = 0;
		//
		iRow[1] = 0;
		jCol[1] = 1;
		//
		return true;
	}
	assert( n == 2 );
	assert( m == 1 );
	//
	values[0] = - 2.0 * x[0];
	values[1] = - 1.0;
	//
	return true;
}
/* $$
$end
-------------------------------------------------------------------------------
$begin ipopt_fixed_eval_h$$
$spell
	ipopt
	bool
	eval
	const
	obj
	nele_hess
	nnz
$$

$section Compute the Hessian of the Lagrangian$$

$head Syntax$$
$icode%ok% = eval_h(
	%n%, %x%, %new_x%,%obj_factor%, %m%, %lambda%, %new_lambda%,%$$
$icode%nele_hess%, %iRow%, %jCol%, %values%
)%$$

$head Lagrangian$$
The Lagrangian is defined to be
$latex \[
	L(x) = \alpha f(x) + \sum_{i=0}^{m-1} \lambda_i g_i (x)
\] $$

$head n$$
is the number of variables in the problem (dimension of x).

$head x$$
is the value for the primal variables at which the
Hessian of the Lagrangian is computed (has size $icode n$$).

$head new_x$$
if true, no Ipopt evaluation method was previous called with the same
value for $icode x$$.

$head obj_factor$$
is the factor $latex \alpha$$ that multiplies the objective f(x)
in the definition of the Lagrangian.

$head m$$
is the number of constraints in the problem (dimension of g(x)).

$head lambda$$
is the value of the constraint multipliers $latex \lambda$$
at which the Hessian is to be evaluated (has size $icode m$$).

$head new_lambda$$
if true, no Ipopt evaluation method was previous called with the same
value for $icode lambda$$.

$head nele_hess$$
is the number of non-zero elements in the Hessian $latex L^{(2)} (x)$$; i.e.,
the same as
$cref/nnz_h_lag/ipopt_fixed_get_nlp_info/nnz_h_lag/$$.

$head iRow$$
If $icode values$$ is $code NULL$$,
$icode iRow$$ has size $icode nele_hess$$ and is set to the
row indices for the non-zero entries in the Hessian
$latex L^{(2)} (x)$$.

$head jCol$$
If $icode values$$ is $code NULL$$,
$icode jCol$$ has size $icode nele_hess$$ and is set to the
column indices for the non-zero entries in the Hessian
$latex L^{(2)} (x)$$.

$head values$$
If $icode values$$ is not $code NULL$$,
it has size $icode nele_hess$$ and $icode%values%[%k%]%$$
is set to the value of element of the Hessian $latex L^{(2)} (x)$$
with row index $icode%iRow%[%k%]%$$
and column index $icode%jCol%[%k%]%$$.

$head ok$$
if set to false, the optimization will terminate with status set to
$cref/USER_REQUESTED_STOP
	/ipopt_fixed_finalize_solution/status/USER_REQUESTED_STOP/$$.

$head Source$$
$codep */
bool ipopt_fixed::eval_h(
	Index         n              ,  // in
	const Number* x              ,  // in
	bool          new_x          ,  // in
	Number        obj_factor     ,  // in
	Index         m              ,  // in
	const Number* lambda         ,  // in
	bool          new_lambda     ,  // in
	Index         nele_hess      ,  // in
	Index*        iRow           ,  // out
	Index*        jCol           ,  // out
	Number*       values         )  // out
{
	assert( nele_hess == 2 );
	if( values == NULL )
	{
		iRow[0] = 0;
		jCol[0] = 0;
		//
		iRow[1] = 1;
		jCol[1] = 1;
		//
		return true;
	}
	assert( n == 2 );
	assert( m == 1 );
	//
	values[0] = - 2.0 * lambda[0];
	values[1] = - 2.0 * obj_factor;
	//
	return true;
}
/* $$
$end
-------------------------------------------------------------------------------
$begin ipopt_fixed_finalize_solution$$
$spell
	ipopt
	bool
	eval
	const
	obj
	ip
	cq
	namespace
	infeasibility
	doesn't
	Inf
	naninf
	std
	cout
	endl
	fabs
	tol
$$

$section Get Solution Results$$

$head Syntax$$
$codei%finalize_solution(
	%status%, %n%, %x%, %z_L%, %z_U%, %m%, %g%,%$$
$icode%lambda%, %obj_value%, %ip_data%, %ip_cq%
)%$$

$head n$$
is the number of variables in the problem (dimension of x).

$head x$$
is the final value (best value found) for the primal variables
(has size $icode n$$).

$head z_L$$
is the final value for the $icode x$$ lower bound constraint multipliers
(has size $icode n$$).

$head z_U$$
is the final value for the $icode x$$ upper bound constraint multipliers
(has size $icode n$$).

$head m$$
is the number of constraints in the problem (dimension of g(x)).

$head lambda$$
is the final value for the g(x) constraint multipliers $latex \lambda$$.

$head obj_value$$
is the value of the objective f(x) at the final $icode x$$ value.

$head ip_data$$
Unspecified; i.e., not part of the Ipopt user API.

$head ip_cq$$
Unspecified; i.e., not part of the Ipopt user API.

$head status$$
These status values are in the $code Ipopt$$ namespace; e.g.,
$code SUCCESS$$ is short for $code Ipopt::SUCCESS$$:

$subhead SUCCESS$$
Algorithm terminated successfully at a locally optimal point,
satisfying the convergence tolerances (can be specified by options).

$subhead MAXITER_EXCEEDED$$
Maximum number of iterations exceeded (can be specified by an option).

$subhead CPUTIME_EXCEEDED$$
Maximum number of CPU seconds exceeded (can be specified by an option).

$subhead STOP_AT_TINY_STEP$$
Algorithm proceeds with very little progress.

$subhead STOP_AT_ACCEPTABLE_POINT$$
Algorithm stopped at a point that was converged, not to desired
tolerances, but to acceptable tolerances (see the acceptable-... options).

$subhead LOCAL_INFEASIBILITY$$
Algorithm converged to a point of local infeasibility. Problem may be
infeasible.

$subhead USER_REQUESTED_STOP$$
A user call-back function returned false, i.e.,
the user code requested a premature termination of the optimization.

$subhead DIVERGING_ITERATES$$
It seems that the iterates diverge.

$subhead RESTORATION_FAILURE$$
Restoration phase failed, algorithm doesn't know how to proceed.

$subhead ERROR_IN_STEP_COMPUTATION$$
An unrecoverable error occurred while Ipopt tried to compute
the search direction.

$subhead INVALID_NUMBER_DETECTED$$
Algorithm received an invalid number (such as NaN or Inf) from
the NLP; see also option check_derivatives_for_naninf.

$head Source$$
$codep */
void ipopt_fixed::finalize_solution(
	Ipopt::SolverReturn               status    ,  // in
	Index                             n         ,  // in
	const Number*                     x         ,  // in
	const Number*                     z_L       ,  // in
	const Number*                     z_U       ,  // in
	Index                             m         ,  // in
	const Number*                     g         ,  // in
	const Number*                     lambda    ,  // in
	Number                            obj_value ,  // in
	const Ipopt::IpoptData*           ip_data   ,  // in
	Ipopt::IpoptCalculatedQuantities* ip_cq     )  // in
{	bool ok = true;

	// default tolerance
	double tol = 1e-08;

	// check problem dimensions
	ok &= n == 2;
	ok &= m == 1;

	// check that x is feasible
	ok &= (-1.0 <= x[0]) && (x[0] <= +1.0);

	// check that the bound multipliers are feasible
	ok &= (0.0 <= z_L[0]) && (0.0 <= z_L[1]);
	ok &= (0.0 <= z_U[0]) && (0.0 <= z_U[1]);

	// check that the constraint on g(x) is satisfied
	ok &= std::fabs( x[0] * x[0] + x[1] - 1.0 ) <= 10. * tol;

	// Check the partial of the Lagrangian w.r.t x[0]
	ok &= std::fabs(- lambda[0] * 2.0 * x[0] - z_L[0] + z_U[0] ) <= 10. * tol;

	// Check the partial of the Lagrangian w.r.t x[1]
	ok &= std::fabs( 2.0 * (x[1] - 2.0) + lambda[0]) <= 10. * tol;

	// set member variable finalize_solution_ok_
	finalize_solution_ok_ = ok;
}
/* $$
$end
-------------------------------------------------------------------------------
*/
} // END_DISMOD_AT_NAMESPACE
