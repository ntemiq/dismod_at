# $Id$
#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-15 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
# $begin user_meas_covariate.py$$ $newlinech #$$
# $spell
#	avgint
#	Covariates
#	covariate
#	Integrands
# $$
#
# $section Using Measurement Covariates on Multiple Integrands$$
#
# $code
# $verbatim%
#	example/user/meas_covariate.py
#	%0%# BEGIN PYTHON%# END PYTHON%1%$$
# $$
# $end
# ---------------------------------------------------------------------------
# BEGIN PYTHON
# true values used to simulate data
iota_true        = 0.05
remission_true   = 0.10
n_data           = 51
# ------------------------------------------------------------------------
import sys
import os
import distutils.dir_util
import subprocess
test_program = 'example/user/meas_covariate.py'
if sys.argv[0] != test_program  or len(sys.argv) != 1 :
	usage  = 'python3 ' + test_program + '\n'
	usage += 'where python3 is the python 3 program on your system\n'
	usage += 'and working directory is the dismod_at distribution directory\n'
	sys.exit(usage)
#
# import dismod_at
sys.path.append( os.getcwd() + '/python' )
import dismod_at
#
# change into the build/example/user directory
distutils.dir_util.mkpath('build/example/user')
os.chdir('build/example/user')
# ------------------------------------------------------------------------
def constant_weight_fun(a, t) :
	return 1.0
# note that the a, t values are not used for this case
def fun_rate_child(a, t) :
	return ('prior_gauss_zero', 'prior_gauss_zero', 'prior_gauss_zero')
def fun_iota_parent(a, t) :
	return ('prior_value_parent', 'prior_gauss_zero', 'prior_gauss_zero')
def fun_mulcov(a, t) :
	return ('prior_mulcov', 'prior_gauss_zero', 'prior_gauss_zero')
# ------------------------------------------------------------------------
def example_db (file_name) :
	import copy
	import dismod_at
	import math
	# ----------------------------------------------------------------------
	# age table
	age_list    = [    0.0, 50.0,    100.0 ]
	#
	# time table
	time_list   = [ 1995.0, 2005.0, 2015.0 ]
	#
	# integrand table
	integrand_dict = [
		{ 'name':'Sincidence',  'eta':1e-6 },
		{ 'name':'remission',   'eta':1e-6 }
	]
	#
	# node table: world -> north_america
	#             north_america -> (united_states, canada)
	node_dict = [
		{ 'name':'world',         'parent':'' },
		{ 'name':'north_america', 'parent':'world' },
		{ 'name':'united_states', 'parent':'north_america' },
		{ 'name':'canada',        'parent':'north_america' }
	]
	#
	# weight table: The constant function 1.0 (one age and one time point)
	fun = constant_weight_fun
	weight_dict = [
		{ 'name':'constant',  'age_id':[1], 'time_id':[1], 'fun':fun }
	]
	#
	# covariate table:
	covariate_dict = [
		{'name':'income', 'reference':0.5, 'max_difference':None},
		{'name':'sex',    'reference':0.0, 'max_difference':0.6}
	]
	#
	# mulcov table
	# income has been scaled the same as sex so man use same smoothing
	mulcov_dict = [
		{
			'covariate': 'income',
			'type':      'meas_value',
			'effected':  'Sincidence',
			'smooth':    'smooth_mulcov'
		},{
			'covariate': 'income',
			'type':      'meas_value',
			'effected':  'remission',
			'smooth':    'smooth_mulcov'
		}
	]
	# --------------------------------------------------------------------------
	# data table:
	data_dict = list()
	# values that are the same for all data rows
	row = {
		'node':        'world',
		'density':     'gaussian',
		'weight':      'constant',
		'hold_out':     False,
		'time_lower':   1995.0,
		'time_upper':   1995.0,
		'age_lower':    0.0,
		'age_upper':    0.0
	}
	# values that change between rows:
	mulcov_income    = 1.0
	income_reference = 0.5
	n_integrand      = len( integrand_dict )
	for data_id in range( n_data ) :
		integrand   = integrand_dict[ data_id % n_integrand ]['name']
		income      = data_id / float(n_data-1)
		sex         = ( data_id % 3 - 1.0 ) / 2.0
		meas_value  = iota_true
		if integrand == 'remission' :
			meas_value  = remission_true
			effect      = (income - income_reference) * mulcov_income
			meas_value *= math.exp(effect)
		meas_std    = 0.1 * meas_value
		row['meas_value'] = meas_value
		row['meas_std']   = meas_std
		row['integrand']  = integrand
		row['income']     = income
		row['sex']        = sex
		data_dict.append( copy.copy(row) )
	# --------------------------------------------------------------------------
	# prior_table
	prior_dict = [
		{   # prior_zero
			'name':     'prior_zero',
			'density':  'uniform',
			'lower':    0.0,
			'upper':    0.0,
			'mean':     0.0,
			'std':      None,
			'eta':      None
		},{ # prior_none
			'name':     'prior_none',
			'density':  'uniform',
			'lower':    None,
			'upper':    None,
			'mean':     0.0,
			'std':      None,
			'eta':      None
		},{ # prior_gauss_zero
			'name':     'prior_gauss_zero',
			'density':  'gaussian',
			'lower':    None,
			'upper':    None,
			'mean':     0.0,
			'std':      0.01,
			'eta':      None
		},{ # prior_value_parent
			'name':     'prior_value_parent',
			'density':  'uniform',
			'lower':    0.01,
			'upper':    1.00,
			'mean':     0.1,
			'std':      None,
			'eta':      None
		},{ # prior_mulcov
			'name':     'prior_mulcov',
			'density':  'laplace',
			'lower':    None,
			'upper':    None,
			'mean':     0.0,
			'std':      1.0,
			'eta':      None
		}
	]
	# --------------------------------------------------------------------------
	# smooth table
	middle_age_id  = 1
	middle_time_id = 1
	last_age_id    = 2
	last_time_id   = 2
	smooth_dict = [
		{   # smooth_rate_child
			'name':                     'smooth_rate_child',
			'age_id':                   [ last_age_id ],
			'time_id':                  [ last_time_id ],
			'mulstd_value_prior_name':  '',
			'mulstd_dage_prior_name':   '',
			'mulstd_dtime_prior_name':  '',
			'fun':                      fun_rate_child
		},{ # smooth_rate_parent
			'name':                     'smooth_rate_parent',
			'age_id':                   [ 0, last_age_id ],
			'time_id':                  [ 0, last_time_id ],
			'mulstd_value_prior_name':  '',
			'mulstd_dage_prior_name':   '',
			'mulstd_dtime_prior_name':  '',
			'fun':                       fun_iota_parent
		},{ # smooth_mulcov
			'name':                     'smooth_mulcov',
			'age_id':                   [ middle_age_id ],
			'time_id':                  [ middle_time_id ],
			'mulstd_value_prior_name':  '',
			'mulstd_dage_prior_name':   '',
			'mulstd_dtime_prior_name':  '',
			'fun':                       fun_mulcov
		}
	]
	# --------------------------------------------------------------------------
	# rate table
	rate_dict = [
		{
			'name':          'pini',
			'parent_smooth': '',
			'child_smooth':  ''
		},{
			'name':          'iota',
			'parent_smooth': 'smooth_rate_parent',
			'child_smooth':  'smooth_rate_child'
		},{
			'name':          'rho',
			'parent_smooth': 'smooth_rate_parent',
			'child_smooth':  'smooth_rate_child'
		},{
			'name':          'chi',
			'parent_smooth': '',
			'child_smooth':  ''
		},{
			'name':          'omega',
			'parent_smooth': '',
			'child_smooth':  ''
		}
	]
	# ------------------------------------------------------------------------
	# option_dict
	option_dict = [
		{ 'name':'parent_node_name',       'value':'world'        },
		{ 'name':'number_sample',          'value':'1'            },
		{ 'name':'fit_sample_index',       'value':''                  },
		{ 'name':'ode_step_size',          'value':'10.0'         },
		{ 'name':'random_seed',            'value':'0'            },
		{ 'name':'rate_case',              'value':'iota_pos_rho_pos' },

		{ 'name':'quasi_fixed',            'value':'true'         },
		{ 'name':'derivative_test_fixed',  'value':'first-order'  },
		{ 'name':'max_num_iter_fixed',     'value':'100'          },
		{ 'name':'print_level_fixed',      'value':'0'            },
		{ 'name':'tolerance_fixed',        'value':'1e-7'         },

		{ 'name':'derivative_test_random', 'value':'second-order' },
		{ 'name':'max_num_iter_random',    'value':'100'          },
		{ 'name':'print_level_random',     'value':'0'            },
		{ 'name':'tolerance_random',       'value':'1e-7'         }
	]
	# --------------------------------------------------------------------------
	# avgint table: empty
	avgint_dict = list()
	# --------------------------------------------------------------------------
	# create database
	dismod_at.create_database(
		file_name,
		age_list,
		time_list,
		integrand_dict,
		node_dict,
		weight_dict,
		covariate_dict,
		data_dict,
		prior_dict,
		smooth_dict,
		rate_dict,
		mulcov_dict,
		option_dict,
		avgint_dict
	)
	# -----------------------------------------------------------------------
	n_smooth  = len( smooth_dict )
	rate_true = []
	for rate_id in range( len( data_dict ) ) :
		# for this particular example
		data_id    = rate_id
		meas_value = data_dict[data_id]['meas_value']
		rate_true.append(meas_value)
	#
	return
# ===========================================================================
# Note that this process uses the fit results as the truth for simulated data
# The fit_var table corresponds to fitting with no noise.
# The sample table corresponds to fitting with noise.
file_name      = 'example.db'
example_db(file_name)
program        = '../../devel/dismod_at'
for command in [ 'init', 'start', 'fit' ] :
	cmd = [ program, file_name, command ]
	print( ' '.join(cmd) )
	flag = subprocess.call( cmd )
	if flag != 0 :
		sys.exit('The dismod_at ' + command + ' command failed')
# -----------------------------------------------------------------------
# connect to database
new             = False
connection      = dismod_at.create_connection(file_name, new)
# -----------------------------------------------------------------------
# Results for fitting with no noise
var_dict     = dismod_at.get_table_dict(connection, 'var')
fit_var_dict = dismod_at.get_table_dict(connection, 'fit_var')
#
middle_age_id  = 1
middle_time_id = 1
last_age_id    = 2
last_time_id   = 2
parent_node_id = 0
tol            = 1e-7
#
# check parent iota and remission values
count             = 0
iota_rate_id      = 1
remission_rate_id = 2
for var_id in range( len(var_dict) ) :
	row   = var_dict[var_id]
	match = row['var_type'] == 'rate'
	match = match and row['node_id'] == parent_node_id
	if match and row['rate_id'] == iota_rate_id :
		count += 1
		value = fit_var_dict[var_id]['fit_var_value']
		assert abs( value / iota_true - 1.0 ) < 5.0 * tol
	if match and row['rate_id'] == remission_rate_id :
		count += 1
		value = fit_var_dict[var_id]['fit_var_value']
		assert abs( value / remission_true - 1.0 ) < 5.0 * tol
assert count == 8
#
# check covariate multiplier values
count                   = 0
mulcov_income           = 1.0
remission_integrand_id  = 1
for var_id in range( len(var_dict) ) :
	row   = var_dict[var_id]
	match = row['var_type'] == 'mulcov_meas_value'
	if match :
		integrand_id = row['integrand_id']
		count       += 1
		value        = fit_var_dict[var_id]['fit_var_value']
		if integrand_id == remission_integrand_id :
			assert abs( value / mulcov_income - 1.0 ) < 1e3 * tol
		else :
			assert abs( value ) < 5.0 * tol
assert count == 2
# -----------------------------------------------------------------------------
print('meas_covariate.py: OK')
# -----------------------------------------------------------------------------
# END PYTHON
