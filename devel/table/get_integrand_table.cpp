// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rate Estimation as Functions of Age and Time
          Copyright (C) 2014-14 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
/*
$begin get_integrand_table$$
$spell
	struct
	sqlite
	enum
	cpp
	mtexcess
	mtother
	mtwith
	mtall
	mtstandard
	relrisk
	mtspecific
$$

$section C++: Get the Integrand Table Information$$
$index get, integrand table$$
$index integrand, get table$$
$index table, get integrand$$

$head Syntax$$
$icode%integrand_table% = get_integrand_table(%db%)%$$

$head Purpose$$
To read the $cref integrand_table$$ and return it as a C++ data structure.

$head db$$
The argument $icode db$$ has prototype
$codei%
	sqlite3* %db%
%$$
and is an open connection to the database.

$head integrand_struct$$
This is a structure with the following fields
$table
Type $cnext Field $cnext Description
$rnext
$code integrand_enum$$ $cnext $code integrand$$  $cnext
enum corresponding to $cref/integrand_name/integrand_table/integrand_name/$$
$rnext
$code double$$ $cnext $code eta$$  $cnext
	The $cref/eta/integrand_table/eta/$$
$tend

$head integrand_table$$
The return value $icode integrand_table$$ has prototype
$codei%
	CppAD::vector<integrand_struct>  %integrand_table%
%$$
For each $cref/integrand_id/integrand_table/integrand_id/$$,
$codei%
	%integrand_table%[%integrand_id%]
%$$
is the information for the corresponding
$cref/integrand_id/integrand_table/integrand_id/$$.

$head integrand_enum$$
This is an enum type with the following values:
$table
$icode integrand_enum$$ $pre  $$ $cnext $icode integrand_name$$ $rnext
$code incidence_enum$$  $pre  $$ $cnext $code incidence$$     $rnext
$code remission_enum$$  $pre  $$ $cnext $code remission$$     $rnext
$code mtexcess_enum$$   $pre  $$ $cnext $code mtexcess$$      $rnext
$code mtother_enum$$    $pre  $$ $cnext $code mtother$$       $rnext
$code mtwith_enum$$     $pre  $$ $cnext $code mtwith$$        $rnext
$code prevalence_enum$$ $pre  $$ $cnext $code prevalence$$    $rnext
$code mtspecific_enum$$ $pre  $$ $cnext $code mtspecific$$    $rnext
$code mtall_enum$$      $pre  $$ $cnext $code mtall$$         $rnext
$code mtstandard_enum$$ $pre  $$ $cnext $code mtstandard$$    $rnext
$code relrisk_enum$$    $pre  $$ $cnext $code relrisk$$
$tend

$head integrand_enum2name$$
This is a global variable.
If $icode%integrand%$$, is an $code integrand_enum$$ value,
$codei%integrand_enum2name[%integrand%]%$$ is the
$icode integrand_name$$ corresponding to the enum value.

$children%example/devel/table/get_integrand_table_xam.cpp
%$$
$head Example$$
The file $cref get_integrand_table_xam.cpp$$ contains an example that uses
this function.

$end
-----------------------------------------------------------------------------
*/



# include <dismod_at/get_integrand_table.hpp>
# include <dismod_at/get_table_column.hpp>
# include <dismod_at/check_table_id.hpp>
# include <dismod_at/table_error_exit.hpp>

namespace dismod_at { // BEGIN DISMOD_AT_NAMESPACE

// rate names in same order as enum type in get_integrand_table.hpp
const char* integrand_enum2name[] = {
	"incidence",
	"remission",
	"mtexcess",
	"mtother",
	"mtwith",
	"prevalence",
	"mtspecific",
	"mtall",
	"mtstandard",
	"relrisk"
};
CppAD::vector<integrand_struct> get_integrand_table(sqlite3* db)
{	using std::string;

	string table_name  = "integrand";
	size_t n_integrand = check_table_id(db, table_name);

	string column_name =  "integrand_name";
	CppAD::vector<string>  integrand_name;
	get_table_column(db, table_name, column_name, integrand_name);
	assert( n_integrand == integrand_name.size() );

	column_name =  "eta";
	CppAD::vector<double>  eta;
	get_table_column(db, table_name, column_name, eta);
	assert( n_integrand == eta.size() );

	CppAD::vector<integrand_struct> integrand_table(n_integrand);
	for(size_t integrand_id = 0; integrand_id < n_integrand; integrand_id++)
	{	integrand_enum integrand = number_integrand_enum;
		for(size_t j = 0; j < number_integrand_enum; j++)
		{	if( integrand_name[integrand_id] == integrand_enum2name[j] )
				integrand = integrand_enum(j);
		}
		if( integrand == number_integrand_enum )
		{	string s = "integrand_name is not a valid choice.";
			table_error_exit("integrand", integrand_id, s);
		}
		integrand_table[integrand_id].integrand = integrand;
		integrand_table[integrand_id].eta       = eta[integrand_id];
	}
	return integrand_table;
}

} // END DISMOD_AT_NAMESPACE
