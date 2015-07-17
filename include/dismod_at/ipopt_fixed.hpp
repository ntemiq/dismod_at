// $Id:$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
# ifndef DISMOD_AT_IPOPT_FIXED_HPP
# define DISMOD_AT_IPOPT_FIXED_HPP

/*
$begin ipopt_fixed$$
$spell
	Ipopt
	nlp
	inf
$$

$section Ipopt NLP Class Used to Optimize Fixed Effects$$

$head Private$$
This class is not part of the $cref approx_mixed$$ API.

$head nlp_lower_bound_inf()$$
This member function returns the $code double$$ value used
for minus infinity as a lower bound.

$head nlp_upper_bound_inf()$$
This member function returns the $code double$$ value used
for plus infinity as an upper bound.

$head fixed_opt()$$
This member function returns the optimal solution (so far)
for the fixed effects.

$childtable%devel/approx_mixed/ipopt_fixed.cpp
	%example/devel/approx_mixed/ipopt_xam.omh
%$$

$end
-----------------------------------------------------------------------------
*/
# include <coin/IpTNLP.hpp>
# include <dismod_at/approx_mixed.hpp>

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE
	//
	// ipopt_fixed
	class ipopt_fixed : public Ipopt::TNLP
	{
	private:
		// approx_mixed types used by this class
		typedef approx_mixed::d_vector     d_vector;
		typedef CppAD::vector<size_t>      s_vector;
		//
		// Ipopt types used by this class
		typedef Ipopt::Number               Number;
		typedef Ipopt::Index                Index;
		typedef Ipopt::TNLP::IndexStyleEnum IndexStyleEnum;
		// ---------------------------------------------------------------
		// member variables set during constructor
		const size_t n_fixed_;            // number of fixed effects
		const size_t n_random_;           // number of random effects
		const size_t n_constraint_;       // number of constraints
		//
		const d_vector& fixed_lower_;     // fixed effects lower limits
		const d_vector& fixed_upper_;     // fixed effects upper limit
		const d_vector& constraint_lower_;// constraint lower limits
		const d_vector& constraint_upper_;// constraint upper limit
		const d_vector& fixed_in_;        // fixed effects initial value
		const d_vector& random_in_;       // random effects initial value
		//
		approx_mixed&   approx_object_;   // approx_mixed for this problem
		// ---------------------------------------------------------------
		// set during constructor, otherwise const
		double nlp_lower_bound_inf_; // Ipopt's code for - infinity
		double nlp_upper_bound_inf_; // Ipopt's code for + infinity
		//
		size_t prior_n_abs_; // number of absolute values in prior
		size_t nnz_jac_g_;   // number non-zeros in Jacobian of constraints
		size_t nnz_h_lag_;   // number non-zeros in Hessian of Lagragian
		//
		s_vector prior_jac_row_; // row indices for Jacobian of prior
		s_vector prior_jac_col_; // column indices for Jacobian of prior
		d_vector prior_jac_val_; // values for Jacobian of prior
		//
		s_vector prior_hes_row_; // row indices for Hessian of prior
		s_vector prior_hes_col_; // column indices for Hessian of prior
		d_vector prior_hes_val_; // values for Hessian of prior
		//
		s_vector constraint_jac_row_; // row for Jacobian of constraint
		s_vector constraint_jac_col_; // column for Jacobian of constraint
		d_vector constraint_jac_val_; // values for Jacobian of constraint
		//
		s_vector constraint_hes_row_; // row for Hessian of constraint
		s_vector constraint_hes_col_; // column for Hessian of constraint
		d_vector constraint_hes_val_; // values for Hessian of constraint
		//
		s_vector laplace_hes_row_; // row indices for Hessian of Laplace
		s_vector laplace_hes_col_; // column indices for Hessian of Laplace
		d_vector laplace_hes_val_; // values of Hessian of Laplace approx
		//
		s_vector lag_hes_row_;   // row indices for Hessian of Lagrangian
		s_vector lag_hes_col_;   // column indices for Hessian of Lagrangian
		s_vector laplace_2_lag_;    // maps laplace_hes_row_ to lag_hes_row_
		s_vector prior_2_lag_;      // maps prior_hes_row_ to lag_hes_row_
		s_vector constraint_2_lag_; // maps constraint_hes_row to lag_hes_row
		// ---------------------------------------------------------------
		// temporaries
		d_vector        fixed_tmp_;      // size n_fixed_
		d_vector        random_tmp_;     // size n_random_
		d_vector        prior_vec_tmp_;  // size prior_n_abs_ + 1
		d_vector        c_vec_tmp_;      // size n_constraint_
		d_vector        H_beta_tmp_;     // size n_fixed_
		d_vector        w_prior_tmp_;     // size 2 * prior_n_abs
		d_vector        w_constraint_tmp_;// size n_constraint
		// ---------------------------------------------------------------
		// set by eval_f only (constructor does not modify)
		d_vector fixed_opt_; // empty until finalize_solution called
		d_vector random_h_;  // random effect at previous eval_h
		// ---------------------------------------------------------------
		// set by constructor
		// ---------------------------------------------------------------
		// set by any eval routine when new_x is true
		d_vector random_cur_; // random effects corresponding to current x
		// ---------------------------------------------------------------
	public:
		// get minus infinity
		double nlp_lower_bound_inf(void) const
		{	return nlp_lower_bound_inf_; }
		// get plus infinity
		double nlp_upper_bound_inf(void) const
		{	return nlp_upper_bound_inf_; }
		// optimal solution so far
		d_vector fixed_opt(void) const
		{	return fixed_opt_; }
		//
		// did finalize_solution agree that the solution had converged
		bool finalize_solution_ok_;
		//
		// default constructor
		ipopt_fixed(
			const d_vector& fixed_lower       ,
			const d_vector& fixed_upper       ,
			const d_vector& constraint_lower  ,
			const d_vector& constraint_upper  ,
			const d_vector& fixed_in          ,
			const d_vector& random_in         ,
			approx_mixed&   approx_object
		);
		//
		// default destructor
		virtual ~ipopt_fixed(void);
		//
		virtual bool get_nlp_info(
			Index&          n            ,
			Index&          m            ,
			Index&          nnz_jac_g    ,
			Index&          nnz_h_lag    ,
			IndexStyleEnum& index_style
		);
		virtual bool get_bounds_info(
				Index       n        ,
				Number*     x_l      ,
				Number*     x_u      ,
				Index       m        ,
				Number*     g_l      ,
				Number*     g_u
		);
		virtual bool get_starting_point(
			Index           n            ,
			bool            init_x       ,
			Number*         x            ,
			bool            init_z       ,
			Number*         z_L          ,
			Number*         z_U          ,
			Index           m            ,
			bool            init_lambda  ,
			Number*         lambda
		);
		virtual bool eval_f(
			Index           n        ,
			const Number*   x        ,
			bool            new_x    ,
			Number&         obj_value
		);
		virtual bool eval_grad_f(
			Index           n        ,
			const Number*   x        ,
			bool            new_x    ,
			Number*         grad_f
		);
		virtual bool eval_g(
			Index           n        ,
			const Number*   x        ,
			bool            new_x    ,
			Index           m        ,
			Number*         g
		);
		virtual bool eval_jac_g(
			Index           n        ,
			const Number*   x        ,
			bool            new_x    ,
			Index           m        ,
			Index           nele_jac ,
			Index*          iRow     ,
			Index*          jCol     ,
			Number*         values
		);
		virtual bool eval_h(
			Index         n              ,
			const Number* x              ,
			bool          new_x          ,
			Number        obj_factor     ,
			Index         m              ,
			const Number* lambda         ,
			bool          new_lambda     ,
			Index         nele_hess      ,
			Index*        iRow           ,
			Index*        jCol           ,
			Number*       values
		);
		virtual void finalize_solution(
			Ipopt::SolverReturn               status    ,
			Index                             n         ,
			const Number*                     x         ,
			const Number*                     z_L       ,
			const Number*                     z_U       ,
			Index                             m         ,
			const Number*                     g         ,
			const Number*                     lambda    ,
			Number                            obj_value ,
			const Ipopt::IpoptData*           ip_data   ,
			Ipopt::IpoptCalculatedQuantities* ip_cq
		);
	};
} // END_DISMOD_AT_NAMESPACE

# endif
