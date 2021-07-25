#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-21 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
# BEGIN PYTHON
import time
import sys
import os
import distutils.dir_util
import copy
import statistics
# ---------------------------------------------------------------------------
test_program = 'test/user/sim_data.py'
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
# change into the build/test/user directory
distutils.dir_util.mkpath('build/test/user')
os.chdir('build/test/user')
# ----------------------------------------------------------------------------
def omega_true(age, time) :
	a = min( 100 , max(0, age) )
	result = 0.00 * (100 - a) / 100 + 1.0 * (a   - 0) / 100
	return result
n_data             = 1
# ---------------------------------------------------------------------------
def sim_data(bound, integrand_name) :
	rate    = { 'omega' : omega_true }
	noise   = { 'denisty_name' : 'gaussian', 'meas_std' : 0.0 }
	abs_tol = 1e-6
	return dismod_at.sim_data(rate, integrand_name, bound, noise, abs_tol)
# ---------------------------------------------------------------------------
def example_db (file_name) :
	# note that the a, t values are not used for this case
	def fun_omega(a, t) :
		if a == 0.0 :
			return ('prior_value_0', 'prior_diff', 'prior_diff')
		elif a == 100.0 :
			return ('prior_value_100', 'prior_diff', 'prior_diff')
		else :
			assert False
	# ----------------------------------------------------------------------
	# age table:
	age_list    = [ 0.0, 100.0 ]
	#
	# time table:
	time_list   = [ 2000.0, 2020.0 ]
	#
	# integrand table:
	integrand_table = [
		 { 'name': 'mtother' }
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
		'integrand':   'mtother',
		'density':     'gaussian',
		'meas_std':     meas_std,
	}
	# values that change between rows:
	for data_id in range( n_data ) :
		# age_lower, age_upper
		age_lower  = 0.0
		age_upper  = 100.0
		#
		# time_lower, time_upper
		time_lower  = 2000.0
		time_upper  = time_lower
		#
		bound = {
			'age_lower' : age_lower ,
			'age_upper' : age_upper ,
			'time_lower' : time_lower ,
			'time_upper' : time_upper
		}
		meas_value = sim_data(bound, 'mtother')
		row.update(bound)
		row['meas_value'] = meas_value
		#
		data_table.append( copy.copy(row) )
	#
	# avgint table:
	avgint_table = data_table
	#
	# ----------------------------------------------------------------------
	# prior_table
	prior_table = [
		{ # prior_value_0
			'name':     'prior_value_0',
			'density':  'uniform',
			'lower':    omega_true(0, 2000) / 100.0,
			'upper':    omega_true(0, 2000)  * 100.0,
			'mean':     omega_true(0, 2000)
		},{ # prior_value_100
			'name':     'prior_value_100',
			'density':  'uniform',
			'lower':    omega_true(100, 2000) / 100.0,
			'upper':    omega_true(100, 2000)  * 100.0,
			'mean':     omega_true(100, 2000)
		},{ # prior_diff
			'name':     'prior_diff',
			'density':  'uniform',
			'mean':     0.0
		}
	]
	# ----------------------------------------------------------------------
	# smooth table
	name           = 'smooth_omega'
	fun            = fun_omega
	age_id         = [0, 1]
	time_id        = [0, 1]
	smooth_table = [
		{'name':name, 'age_id':age_id, 'time_id':time_id, 'fun':fun }
	]
	name = 'smooth_omega'
	# ----------------------------------------------------------------------
	# rate table:
	rate_table = [
		{	'name':          'omega',
			'parent_smooth': 'smooth_omega',
			'child_smooth':  None
		}
	]
	# ----------------------------------------------------------------------
	# option_table
	option_table = [
		{ 'name':'rate_case',              'value':'iota_zero_rho_zero' },
		{ 'name':'parent_node_name',       'value':'world'              },
		{ 'name':'ode_step_size',          'value':'1.0'                },

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
dismod_at.system_command_prc([
	program, file_name, 'set', 'truth_var', 'prior_mean'
])
dismod_at.system_command_prc([ program, file_name, 'predict', 'truth_var' ])
#
#
new             = False
connection      = dismod_at.create_connection(file_name, new)
data_table      = dismod_at.get_table_dict(connection, 'data')
avgint_table    = dismod_at.get_table_dict(connection, 'avgint')
predict_table   = dismod_at.get_table_dict(connection, 'predict')
#
assert len(data_table) == n_data
assert len(avgint_table) == n_data
assert len(predict_table) == n_data
for data_id in range( n_data ) :
	data_row   = data_table[data_id]
	avgint_id  = predict_table[data_id]['avgint_id']
	avgint_row = avgint_table[avgint_id]
	for key in avgint_row :
		assert avgint_row[key] == data_row[key]
	#
	avg_integrand = predict_table[data_id]['avg_integrand']
	meas_value    = data_table[data_id]['meas_value']
	#
	relerr = 1.0 - avg_integrand / meas_value
	assert relerr < 1e-5
# ---------------------------------------------------------------------------
print('sim_data.py: OK')
# END PYTHON