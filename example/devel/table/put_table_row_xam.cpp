// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
/*
$begin put_table_row_xam.cpp$$
$spell
	xam
$$

$section C++ put_table_row: Example and Test$$

$code
$verbatim%example/devel/table/put_table_row_xam.cpp%0%// BEGIN C++%// END C++%1%$$
$$

$end
*/
// BEGIN C++
# include <dismod_at/put_table_row.hpp>
# include <dismod_at/get_fit_table.hpp>
# include <dismod_at/exec_sql_cmd.hpp>
# include <dismod_at/open_connection.hpp>

bool put_table_row_xam(void)
{
	bool   ok = true;
	using  std::string;
	using  CppAD::vector;

	string   file_name = "example.db";
	bool     new_file  = true;
	sqlite3* db        = dismod_at::open_connection(file_name, new_file);

	// create the fit table
	const char* sql_cmd[] = {
	"create table fit"
	"(fit_id                 integer primary key,"
		" parent_node_id     integer,"
		" ode_step_size      real,"
		" tolerance          real,"
		" max_num_iter       integer)",
	};
	size_t n_command = sizeof(sql_cmd) / sizeof(sql_cmd[0]);
	for(size_t i = 0; i < n_command; i++)
		dismod_at::exec_sql_cmd(db, sql_cmd[i]);

	// setup for put_table_row
	std::string table_name = "fit";
	CppAD::vector<std::string> col_name_vec(4), row_val_vec(4);

	// column names as a vector
	col_name_vec[0] = "parent_node_id";
	col_name_vec[1] = "ode_step_size";
	col_name_vec[2] = "tolerance";
	col_name_vec[3] = "max_num_iter";

	// insert first row in the fit table
	row_val_vec[0]   = "4";
	row_val_vec[1]   = "0.4";
	row_val_vec[2]   = "1e-8";
	row_val_vec[3]   = "400";
	size_t fit_id = dismod_at::put_table_row(
		db, table_name, col_name_vec, row_val_vec
	);
	ok &= fit_id == 0;

	// insert second row in the fit table
	row_val_vec[0]  = "5";
	row_val_vec[1]  = "0.5";
	row_val_vec[2]  = "1e-8";
	row_val_vec[3]  = "500";
	fit_id = dismod_at::put_table_row(
		db, table_name, col_name_vec, row_val_vec
	);
	ok &= fit_id == 1;

	// get the fit table
	vector<dismod_at::fit_struct>
		fit_table = dismod_at::get_fit_table(db);
	ok  &= fit_table.size() == 2;
	//
	ok  &= fit_table[0].parent_node_id     == 4;
	ok  &= fit_table[0].ode_step_size      == 0.4;
	ok  &= fit_table[0].tolerance          == 1e-8;
	ok  &= fit_table[0].max_num_iter       == 400;
	//
	ok  &= fit_table[1].parent_node_id     == 5;
	ok  &= fit_table[1].ode_step_size      == 0.5;
	ok  &= fit_table[1].tolerance          == 1e-8;
	ok  &= fit_table[1].max_num_iter       == 500;
	//
	// close database and return
	sqlite3_close(db);
	return ok;
}
// END C++
