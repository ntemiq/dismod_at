#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-21 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
# $begin user_sim_data.py$$ $newlinech #$$
# $spell
#	init
#	covariate
#	std
#	sim
#	cv
# $$
#
# $section Using the Python sim_data Utility$$
#
# $head See Also$$
# $cref user_sim_data.py$$
#
# $head Random Effects$$
# There are no random effects in this example.
#
# $head Priors$$
# The priors do not matter for this example except for the fact that
# the $cref truth_var_table$$ values for the $cref model_variables$$
# must satisfy the lower and upper limits in the corresponding priors.
#
# $head Simulation$$
# The simulation value for $icode iota$$ is bi-linear function with
# the following values:
# $table
# iota $cnext age  $cnect time $rnext
# 0.01  $cnext   0  $cnext 2000 $rnext
# 0.02  $cnext  100 $cnext 2000 $rnext
# 0.03  $cnext    0 $cnext 2020 $rnext
# 0.04  $cnext  100 $cnext 2020 $rnext
# $tend
# All the other rates are zero for this simulation.
#
# $head Model$$
# The only non-zero rate in this model is the parent iota.
# The (age, time) grid for the iota model are
# (0,2000), (100,2000), (0, 2020), (100, 2020).
# This, if there is no noise in the measurements, the model should
# fit the data perfectly.
#
# $head Data$$
# There are $icode n_data$$ measurements of prevalence and
# each at a randomly selected age between 0 and 100 and random time
# between 2000 and 2020.
# There is no measurement noise in the simulated data, but it is modeled
# as having measurement noise.
#
# $head Source Code$$
# $srcthisfile%0%# BEGIN PYTHON%# END PYTHON%1%$$
# $end
# ---------------------------------------------------------------------------
# BEGIN PYTHON
import time
import sys
import os
import distutils.dir_util
import copy
import random
import statistics
# ---------------------------------------------------------------------------
test_program = 'example/user/sim_data.py'
if sys.argv[0] != test_program  or len(sys.argv) != 1 :
	usage  = 'python3 ' + test_program + '\n'
	usage += 'where python3 is the python 3 program on your system\n'
	usage += 'and working directory is the dismod_at distribution directory\n'
	sys.exit(usage)
print(test_program)
#
# import dismod_at
local_dir = os.getcwd() + '/python'
if( os.path.isdir( local_dir + '/dismod_at' ) ) :
	sys.path.insert(0, local_dir)
import dismod_at
#
# change into the build/example/user directory
distutils.dir_util.mkpath('build/example/user')
os.chdir('build/example/user')
# ----------------------------------------------------------------------------
def iota_true(age, time) :
	a = min( 100 , max(0, age) )
	t = min( 2020 , max(2000, time) )
	result = \
		0.01 * (100 - a) * (2020 - t) / ( 100 * 20 ) + \
		0.02 * (a   - 0) * (2020 - t) / ( 100 * 20 ) + \
		0.03 * (100 - a) * (t - 2000) / ( 100 * 20 ) + \
		0.04 * (a   - 0) * (t - 2000) / ( 100 * 20 )
	return result
n_data             = 10
random_seed        = int( time.time() )
# ---------------------------------------------------------------------------
def sim_data(a, t, integrand_name) :
	rate    = { 'iota' : iota_true }
	noise   = { 'denisty_name' : 'gaussian', 'meas_std' : 0.0 }
	bound           = {
		'age_lower' : a, 'age_upper' : a, 'time_lower' : t, 'time_upper' : t
	}
	return dismod_at.sim_data(rate, integrand_name, bound, noise)
# ---------------------------------------------------------------------------
def example_db (file_name) :
	# note that the a, t values are not used for this case
	def fun_iota(a, t) :
		return ('prior_value', 'prior_diff', 'prior_diff')
	# ----------------------------------------------------------------------
	# age table:
	age_list    = [ 0.0, 100.0 ]
	#
	# time table:
	time_list   = [ 2000.0, 2020.0 ]
	#
	# integrand table:
	integrand_table = [
		 { 'name': 'prevalence' }
	]
	#
	# node table:
	node_table = [ { 'name':'world', 'parent':'' } ]
	#
	# weight table:
	weight_table = list()
	#
	# covariate table:
	covariate_table = list()
	#
	# mulcov table:
	mulcov_table = list()
	#
	# avgint table: empty
	avgint_table = list()
	#
	# nslist_table:
	nslist_table = dict()
	# ----------------------------------------------------------------------
	# data table:
	data_table = list()
	# values that are the same for all data rows
	meas_std  = 0.01
	row = {
		'weight':      '',
		'hold_out':     False,
		'node':        'world',
		'subgroup':    'world',
		'integrand':   'prevalence',
		'density':     'gaussian',
		'meas_std':     meas_std,
	}
	# values that change between rows:
	for data_id in range( n_data ) :
		age        = random.uniform(0, 100)
		time       = random.uniform(2000, 2020)
		meas_value = sim_data(age, time, 'prevalence')
		row['age_lower']  = age
		row['age_upper']  = age
		row['time_lower'] = time
		row['time_upper'] = time
		row['meas_value'] = meas_value
		data_table.append( copy.copy(row) )
	#
	# ----------------------------------------------------------------------
	# prior_table
	iota_list = list()
	for (age, time) in [ (0,2000), (100,2000), (0,2020), (100,2020) ] :
		iota_list.append( sim_data(age, time, 'Sincidence') )
	prior_table = [
		{ # prior_value
			'name':     'prior_value',
			'density':  'uniform',
			'lower':    min( iota_list ) / 100.0,
			'upper':    max( iota_list)  * 100.0,
			'mean':     statistics.mean( iota_list)
		},{ # prior_diff
			'name':     'prior_diff',
			'density':  'uniform',
			'mean':     0.0
		}
	]
	# ----------------------------------------------------------------------
	# smooth table
	name           = 'smooth_iota'
	fun            = fun_iota
	age_id         = [0, 1]
	time_id        = [0, 1]
	smooth_table = [
		{'name':name, 'age_id':age_id, 'time_id':time_id, 'fun':fun }
	]
	name = 'smooth_iota'
	# ----------------------------------------------------------------------
	# rate table:
	rate_table = [
		{	'name':          'iota',
			'parent_smooth': 'smooth_iota',
			'child_smooth':  None
		}
	]
	# ----------------------------------------------------------------------
	# option_table
	option_table = [
		{ 'name':'rate_case',              'value':'iota_pos_rho_zero' },
		{ 'name':'parent_node_name',       'value':'world'             },
		{ 'name':'random_seed',            'value':str(random_seed)    },

	]
	# ----------------------------------------------------------------------
	# subgroup_table
	subgroup_table = [ { 'subgroup':'world', 'group':'world' } ]
	# ----------------------------------------------------------------------
	# create database
	dismod_at.create_database(
		file_name,
		age_list,
		time_list,
		integrand_table,
		node_table,
		subgroup_table,
		weight_table,
		covariate_table,
		avgint_table,
		data_table,
		prior_table,
		smooth_table,
		nslist_table,
		rate_table,
		mulcov_table,
		option_table
	)
	# ----------------------------------------------------------------------
	return
# ===========================================================================
# Create database
file_name = 'example.db'
example_db(file_name)
program = '../../devel/dismod_at'
#
# fit fixed
dismod_at.system_command_prc([ program, file_name, 'init' ])
dismod_at.system_command_prc([ program, file_name, 'fit', 'fixed' ])
#
#
new             = False
connection      = dismod_at.create_connection(file_name, new)
var_table       = dismod_at.get_table_dict(connection, 'var')
fit_var_table   = dismod_at.get_table_dict(connection, 'fit_var')
age_table       = dismod_at.get_table_dict(connection, 'age')
time_table      = dismod_at.get_table_dict(connection, 'time')
#
assert len(var_table) == 4
for (var_id, row) in enumerate( var_table ) :
	var_type = row['var_type']
	assert var_type == 'rate'
	#
	age_id         = row['age_id']
	time_id        = row['time_id']
	fit_var_value  = fit_var_table[var_id]['fit_var_value']
	#
	age            = age_table[age_id]['age']
	time           = time_table[time_id]['time']
	true_var_value = iota_true(age, time)
	#
	rel_err        = 1.0 - fit_var_value / true_var_value
	print(rel_err)