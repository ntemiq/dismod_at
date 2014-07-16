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
$begin weight_grid_xam.cpp$$
$spell
	xam
$$

$section C++ weight_grid: Example and Test$$
$index example, C++ weight_grid$$
$index weight_grid, C++ example$$

$code
$verbatim%example/devel/table/weight_grid_xam.cpp%0%// BEGIN C++%// END C++%1%$$
$$

$end
*/
// BEGIN C++
# include <dismod_at/dismod_at.hpp>

bool weight_grid_xam(void)
{
	bool   ok = true;
	using  std::string;
	using  CppAD::vector;	

	string   file_name = "example.db";
	bool     new_file  = true;
	sqlite3* db        = dismod_at::open_connection(file_name, new_file);

	// sql commands
	const char* sql_cmd[] = { 
	"create table weight_grid("
		" weight_grid_id integer primary key,"
		" weight_id      integer, "
		" age_id         integer,"
		" time_id        integer,"
		" weight         real)",
	//                 weight_grid_id, weight_id,age_id,time_id, weight
	"insert into weight_grid values(0,         0,     1,      0,    1.00)",
	"insert into weight_grid values(1,         1,     0,      0,    0.50)",
	"insert into weight_grid values(2,         1,     2,      0,    0.75)",
	"insert into weight_grid values(3,         1,     4,      0,    1.00)",
	"insert into weight_grid values(4,         1,     0,      1,    1.00)",
	"insert into weight_grid values(5,         1,     2,      1,    1.25)",
	"insert into weight_grid values(6,         1,     4,      1,    1.50)",
	};
	size_t n_command = sizeof(sql_cmd) / sizeof(sql_cmd[0]);
	for(size_t i = 0; i < n_command; i++)
		dismod_at::exec_sql_cmd(db, sql_cmd[i]);

	// get the weight_grid table
	vector<dismod_at::weight_grid_struct> 
		weight_grid_table = dismod_at::get_weight_grid(db);

	// extract the weight_grid information
	size_t weight_id = 1;
	dismod_at::weight_grid  wg(weight_id, weight_grid_table);

	ok  &= wg.age_size()  == 3;
	ok  &= wg.time_size() == 2;
	//
	ok  &= wg.age_id(0)    == 0;
	ok  &= wg.age_id(1)    == 2;
	ok  &= wg.age_id(2)    == 4;
	//
	ok  &= wg.time_id(0)   == 0;
	ok  &= wg.time_id(1)   == 1;
	//
	ok  &= wg.weight(0,0)  == 0.50;
	ok  &= wg.weight(1,0)  == 0.75;
	ok  &= wg.weight(2,0)  == 1.00;
	ok  &= wg.weight(0,1)  == 1.00;
	ok  &= wg.weight(1,1)  == 1.25;
	ok  &= wg.weight(2,1)  == 1.50;
 	//
	// close database and return
	sqlite3_close(db);
	return ok;
}
// END C++
