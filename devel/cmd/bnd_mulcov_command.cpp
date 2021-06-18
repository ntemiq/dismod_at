/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-21 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
         GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */

# include <gsl/gsl_randist.h>
# include <cppad/utility/to_string.hpp>
# include <cppad/mixed/manage_gsl_rng.hpp>
# include <dismod_at/exec_sql_cmd.hpp>
# include <dismod_at/create_table.hpp>
# include <dismod_at/get_mulcov_table.hpp>
# include <dismod_at/get_data_subset.hpp>
# include <dismod_at/get_bnd_mulcov_table.hpp>
# include <dismod_at/bnd_mulcov_command.hpp>

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE
/*
-----------------------------------------------------------------------------
$begin bnd_mulcov_command$$
$spell
	Covariate
	dismod
	bnd_mulcov
	covariates
	mul
	cov
	py
$$

$section Bound The Covariate Multiplier Absolute Data Effect$$

$head Syntax$$
$codei%dismod_at %database% bnd_mulcov %max_abs_effect%$$

$head Purpose$$
This command is used to set the maximum absolute effect,
in the model for the data values,
for all the covariate multipliers.
This is done by changing the lower and upper bounds
for the covariate multipliers (ignoring bounds in the corresponding priors).
The $cref/meas_noise/mulcov_table/mulcov_type/meas_noise/$$
covariates are not included.

$head database$$
Is an
$href%http://www.sqlite.org/sqlite/%$$ database containing the
$code dismod_at$$ $cref input$$ tables which are not modified.

$head max_abs_effect$$
A covariate multiplier is defined by a row of the $cref mulcov_table$$.
We use the notation $icode mul_value$$ for a value of the multiplier.
There is a corresponding covariate for each multiplier.
We use the notation $icode cov_value$$ for a value of the
$cref/covariate/data_table/Covariates/$$ in the data table.
We use the notation $icode cov_ref$$ for the
$cref/reference/covariate_table/reference/$$ for the covariate.
The maximum effect condition is
$codei%
	| %mul_value% * (%cov_value% - %cov_ref%) | <= %max_abs_effect%
%$$

$head bnd_mulcov_table$$
The table $cref bnd_mulcov_table$$ is output by this command.
It contains the maximum upper bound and minimum lower bound,
for each covariate multiplier, such that the inequality above
is satisfied for all $icode mul_value$$ between the lower and upper bound.
$list number$$
Only the subset of the data table specified by the $cref data_subset_table$$
table are included in this calculation.
$lnext
The upper and lower bounds are set to zero in the special case where,
for a covariate, $icode cov_value$$ is equal to $icode cov_ref$$
for all the point in the data_subset table.
$lnext
The lower (upper) bound for
The $cref/meas_noise/mulcov_table/mulcov_type/meas_noise/$$
covariate multipliers are set to null which is interpreted as
minus infinity (plus infinity) and hence has no effect.
$lend

$comment 2DO: create the user_bnd_mulcov.py example$$
$head Example$$
The file $code user_bnd_mulcov.py$$ contains an example and test
using this command.

$end
*/
void bnd_mulcov_command(
	sqlite3*                                      db                ,
	std::string&                                  max_abs_effect    ,
	const CppAD::vector<double>&                  data_cov_value    ,
	const CppAD::vector<data_subset_struct>&      data_subset_table ,
	const CppAD::vector<covariate_struct>&        covariate_table   ,
	const CppAD::vector<mulcov_struct>&           mulcov_table      )
{	using std::string;
	using CppAD::vector;
	double inf = std::numeric_limits<double>::infinity();
	double nan = std::numeric_limits<double>::quiet_NaN();
	//
	// max_abs_effect
	double max_effect = std::atoi( max_abs_effect.c_str() );
	//
	// n_data
	size_t n_subset = data_subset_table.size();
	//
	// n_covariate
	size_t n_covariate = covariate_table.size();
	//
	// n_mulcov
	size_t n_mulcov    = mulcov_table.size();
	//
	// initialize bnd_mulcov_table
	CppAD::vector<bnd_mulcov_struct> bnd_mulcov_table(n_mulcov);
	for(size_t mulcov_id = 0; mulcov_id < n_mulcov; ++mulcov_id)
	{	bnd_mulcov_table[mulcov_id].lower = nan;
		bnd_mulcov_table[mulcov_id].upper = nan;
	}
	//
	// covariate loop
	for(size_t covariate_id = 0; covariate_id < n_covariate; ++covariate_id)
	{	double cov_min_diff = + inf;
		double cov_max_diff = - inf;
		double cov_ref = covariate_table[covariate_id].reference;
		for(size_t subset_id = 0; subset_id < n_subset; ++subset_id)
		{	size_t data_id = data_subset_table[subset_id].data_id;
			size_t index   = data_id * n_covariate + covariate_id;
			if( ! std::isnan( data_cov_value[index] ) )
			{	double diff    = data_cov_value[index] - cov_ref;
				cov_min_diff   = std::min(cov_min_diff, diff);
				cov_max_diff   = std::max(cov_max_diff, diff);
			}
		}
		double upper = + inf;
		double lower = - inf;
		if( cov_max_diff > 0 )
		{	upper = std::min(upper, + max_effect / cov_max_diff);
			lower = std::max(lower, - max_effect / cov_max_diff);
		}
		if( cov_min_diff < 0 )
		{	upper = std::min(upper, - max_effect / cov_min_diff);
			lower = std::max(upper, + max_effect / cov_min_diff);
		}
		if( upper == + inf )
		{	// This covariate is equal to its reference for all values
			lower = 0.0;
			upper = 0.0;
		}
		for(size_t mulcov_id = 0; mulcov_id < n_mulcov; ++mulcov_id)
		if( size_t( mulcov_table[mulcov_id].covariate_id ) == covariate_id )
		if( mulcov_table[mulcov_id].mulcov_type != meas_noise_enum )
		{	bnd_mulcov_table[mulcov_id].lower = lower;
			bnd_mulcov_table[mulcov_id].upper = upper;
		}
	}
	//
	// drop old bnd_mulcov table
	string sql_cmd    = "drop table bnd_mulcov";
	exec_sql_cmd(db, sql_cmd);
	//
	// write new data_subset table
	string table_name = "bnd_mulcov";
	size_t n_col      = 2;
	vector<string> col_name(n_col), col_type(n_col);
	vector<string> row_value(n_col * n_mulcov);
	vector<bool>   col_unique(n_col);
	//
	col_name[0]       = "lower";
	col_type[0]       = "real";
	col_unique[0]     = false;
	//
	col_name[1]       = "upper";
	col_type[1]       = "real";
	col_unique[1]     = false;
	//
	for(size_t mulcov_id = 0; mulcov_id < n_mulcov; mulcov_id++)
	{	double lower = bnd_mulcov_table[mulcov_id].lower;
		double upper = bnd_mulcov_table[mulcov_id].upper;
		row_value[n_col * mulcov_id + 0] = CppAD::to_string( lower );
		row_value[n_col * mulcov_id + 1] = CppAD::to_string( upper );
	}
	create_table(
		db, table_name, col_name, col_type, col_unique, row_value
	);
	return;
}
} // END_DISMOD_AT_NAMESPACE
