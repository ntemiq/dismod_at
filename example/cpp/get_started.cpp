/*
$begin get_started.cpp$$

$section C++ Getting Started$$ 

$end
-----------------------------------------------------------------------------
*/

# include <dismod_at/dismod_at.hpp>

int main(int argc, const char* argv[])
{
	assert( argc == 2 );
	std::string file_name = argv[1];
	bool new_file         = false;
	sqlite3* db           = dismod_at::open_connection(file_name, new_file);
	//
	dismod_at::input_table_struct input_table;
	dismod_at::get_input_table(db, input_table);
	//
	std::cout << "get_started.cpp: OK" << std::endl;
	return 0;
}
