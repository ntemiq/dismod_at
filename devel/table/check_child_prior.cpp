// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-17 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
/*
$begin check_child_prior$$
$spell
	sqlite
	std
	dage
	dtime
	const
	CppAD
	struct
$$

$section Check Priors in Child Smoothing$$

$head syntax$$
$codei%check_child_prior(%db%, %rate_table%, %smooth_grid%, %prior_table%)%$$

$head Purpose$$
Checks the
$cref/child smoothing assumptions/rate_table/child_smooth_id/$$.
Note that a $code null$$ prior for
$cref/value_prior_id/smooth_grid_table/value_prior_id/$$,
$cref/dage_prior_id/smooth_grid_table/dage_prior_id/$$ and
$cref/dtime_prior_id/smooth_grid_table/dtime_prior_id/$$
is considered OK.

$list number$$
The $cref/density_id/prior_table/density_id/$$ must correspond
to a Gaussian density.
$lnext
The $cref/mean/prior_table/mean/$$ must be zero.
$lnext
The $cref/std/prior_table/std/$$ must be greater than zero.
$lnext
The $cref/lower/prior_table/lower/$$ limit must be minus infinity.
$lnext
The $cref/upper/prior_table/upper/$$ limit must be plus infinity.
$lend

$head db$$
This argument has prototype
$codei%
	sqlite3* %db%
%$$
and is the database connection for $cref/logging/log_message/$$ errors.

$head rate_table$$
This argument has prototype
$codei%
	const CppAD::vector<rate_struct>& %rate_table%
%$$
and it is the
$cref/rate_table/get_rate_table/rate_table/$$.
For this table,
only the $code child_smooth_id$$ field is used.

$head smooth_grid$$
This argument has prototype
$codei%
	const CppAD::vector<smooth_grid_struct>& %smooth_grid%
%$$
and it is the
$cref/smooth_grid/get_smooth_grid/smooth_grid/$$.
For this table, only the
$code value_prior_id$$, $code dage_prior_id$$, and $code dtime_prior_id$$
fields are used.

$head prior_table$$
This argument has prototype
$codei%
	const CppAD::vector<prior_struct>& %prior_table%
%$$
and it is the
$cref/prior_table/get_prior_table/prior_table/$$.
For this table,
only the
$code value_prior_id$$,
$code dage_prior_id$$, and
$code dtime_prior_id$$,
field are used.

$end
*/
# include <dismod_at/check_child_prior.hpp>
# include <dismod_at/get_density_table.hpp>
# include <dismod_at/error_exit.hpp>
# include <dismod_at/null_int.hpp>
# include <cppad/utility/to_string.hpp>

namespace dismod_at { // BEGIN DISMOD_AT_NAMESPACE

void check_child_prior(
	sqlite3*                                 db            ,
	const CppAD::vector<rate_struct>&        rate_table    ,
	const CppAD::vector<smooth_grid_struct>& smooth_grid   ,
	const CppAD::vector<prior_struct>&       prior_table   )
{	assert( rate_table.size()   == number_rate_enum );
	using std::endl;
	using std::string;
	using CppAD::to_string;
	string msg;
	//
	for(size_t rate_id = 0; rate_id < rate_table.size(); rate_id++)
	{
		int child_smooth_id  = rate_table[rate_id].child_smooth_id;
		for(size_t grid_id = 0; grid_id < smooth_grid.size(); grid_id++)
		if( smooth_grid[grid_id].smooth_id == child_smooth_id )
		{	CppAD::vector<int> prior_id(3);
			CppAD::vector<string> name(3);
			prior_id[0] = smooth_grid[grid_id].value_prior_id;
			name[0]     = "child value prior";
			prior_id[1] = smooth_grid[grid_id].dage_prior_id;
			name[1]     = "child dage prior";
			prior_id[2] = smooth_grid[grid_id].dtime_prior_id;
			name[2]     = "child dtime prior";
			// skip dage and dtime priors for last age and last time
			for(size_t i = 0; i < 3; i++)
			if( prior_id[i] != DISMOD_AT_NULL_INT )
			{
				int    density_id = prior_table[prior_id[i]].density_id;
				double mean       = prior_table[prior_id[i]].mean;
				double std        = prior_table[prior_id[i]].std;
				double lower      = prior_table[prior_id[i]].lower;
				double upper      = prior_table[prior_id[i]].upper ;
				//
				// check for an error
				msg = "";
				if( density_id != int( gaussian_enum ) )
				{	msg += "density not gaussian";
				}
				if( mean != 0.0 )
				{	if( msg != "" )
						msg += ", ";
					msg += "mean not zero";
				}
				if( std <= 0.0 )
				{	if(msg != "" )
						msg += ", ";
					msg += "std not greater than zero";
				}
				double inf = std::numeric_limits<double>::infinity();
				if( lower != -inf )
				{	if(msg != "" )
						msg += ", ";
					msg += "lower not minus infinity";
				}
				if( upper != +inf )
				{	if(msg != "" )
						msg += ", ";
					msg += "upper not plus infinity";
				}
				if( msg != "" )
				{
					msg = name[i]
					+ ": child_smooth_id = " + to_string(child_smooth_id)
					+ ", smooth_grid_id = " + to_string(grid_id)
					+ ", prior_id = " + to_string( prior_id[i] )
					+ ": " + msg;
					string table_name  = "rate";
					error_exit(msg,  table_name, rate_id);
				}
			}
		}
	}
}

} // END DISMOD_AT_NAMESPACE
