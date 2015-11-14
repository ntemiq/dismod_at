#! /bin/python3
# $Id:$
#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-15 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
# $begin import_cascade$$ $newlinech #$$
#
# $section Import an IHME Cascade Study$$
#
# $head Syntax$$
# $codei%import_cascade.py %cascade_path% %option_file%$$
#
# $end
# ---------------------------------------------------------------------------
import sys
import os
import csv
import math
import copy
import pdb
import time as timer
import collections
#
sys.path.append( os.path.join( os.getcwd(), 'python' ) )
import dismod_at
# ---------------------------------------------------------------------------
# optiono_csv:       file that contains options to import_cascade
# option_table_in:   a dictionary containing values in option file
#
if sys.argv[0] != 'bin/import_cascade.py' :
	msg  = 'bin/import_cascasde.py: must be executed from its parent directory'
	sys.exit(msg)
#
option_dict = collections.OrderedDict([
('cascade_path','      path to directory where cascade input files are'),
('ode_step_size','     step size of ODE solution in age and time'),
('mtall2mtother','     treat mtall data as if it were mtother [yes/no]'),
('rate_case','         are iota and rho zero or non-zero; see option_table'),
('child_value_std','   value standard deviation for random effects'),
('child_dtime_std','   dtime standard deviation for random effects'),
('time_grid','         the time grid as space seperated values'),
('parent_node_name','  name of the parent node'),
('xi_factor','         factor that multiplies cascade_ode xi value'),
('random_bound','      bound for the random effects, empty text for no bound'),
('fit_covariates','    include or exclude the covariate multipliers from fit')
])
usage = '''bin/import_cascade.py option_csv

option_csv:   a csv file that contains the following (name, value) pairs
'''
usage += 30 * '-' + ' options ' + 40 * '-' + '\n'
for key in option_dict :
	usage += key + ':' + option_dict[key] + '\n'
usage += 79 * '-' + '\n'
n_arg = len(sys.argv)
if n_arg != 2 :
	sys.exit(usage)
#
option_csv   = sys.argv[1]
if not os.path.isfile( option_csv ) :
	msg  = usage + '\n'
	msg += 'import_cascade: ' + option_csv + ' is not a file'
	sys.exit(msg)
# ----------------------------------------------------------------------------
# option_table_in
#
option_table_in = dict()
file_ptr    = open(option_csv)
reader      = csv.DictReader(file_ptr)
for row in reader :
	option_table_in[ row['name'] ] = row['value']
for option in option_dict :
	if option not in option_table_in :
		msg  = usage + '\n'
		msg += option + ' not in ' + option_csv
		sys.exit(msg)
for option in option_table_in :
	if option not in option_dict :
		msg  = usage + '\n'
		msg += option + ' in ' + option_csv + ' is not a valid option'
		sys.exit(msg)
#
ode_step_size = option_table_in['ode_step_size']
if not float(ode_step_size) > 0.0 :
	msg  = usage + '\n'
	msg += 'in ' + option_csv + ' ode_step_size = ' + ode_step_size
	msg += ' is not greater than zero'
	sys.exit(msg)
#
mtall2mtother = option_table_in['mtall2mtother']
if not mtall2mtother in [ 'yes', 'no' ] :
	msg  = usage + '\n'
	msg += 'in ' + option_csv + ' mtall2mtother = "' + mtall2mtother
	msg += '" is not "yes" or "no"'
	sys.exit(msg)
#
fit_covariates = option_table_in['fit_covariates']
if not mtall2mtother in [ 'yes', 'no' ] :
	msg  = usage + '\n'
	msg += 'in ' + option_csv + ' fit_covariates = "' + fit_covariates
	msg += '" is not "yes" or "no"'
	sys.exit(msg)
#
cascade_path = option_table_in['cascade_path']
if not os.path.isdir( cascade_path ) :
	msg  = usage + '\n'
	msg += 'in ' + option_csv + ' cascade_path = ' + cascade_path
	msg += ' is not a directory'
	sys.exit(msg)
#
if len( option_table_in['time_grid'].split() ) < 2 :
	msg = 'in ' + option_csv + ' time_grid does not have two or more elements'
	sys.exit(msg)
#
rate_case = option_table_in['rate_case']
case_list = [
	'iota_pos_rho_zero', 'iota_zero_rho_pos',
	'iota_zero_rho_pos', 'iota_pos_rho_pos'
]
if rate_case not in case_list :
	msg  = usage + '\n'
	msg += 'in ' + option_csv + ' rate_case = ' + rate_case + '\n'
	msg += 'is not one of the following:\n'
	for i in range( len( case_list ) ) :
		msg += case_list[i]
		if i + 1 < len( case_list ) :
			msg += ', '
	sys.exit(msg)
# ----------------------------------------------------------------------------
# cascade_path_dict
#
cascade_path  = option_table_in['cascade_path']
cascade_dir   = os.path.basename(cascade_path)
#
cascade_name_list = [
	'data', 'rate_prior', 'simple_prior', 'effect_prior', 'integrand', 'value'
]
cascade_path_dict = dict()
for name in cascade_name_list :
	path = os.path.join(cascade_path, name + '.csv')
	if not os.path.isfile(path) :
		msg = 'import_cascade: ' + path + ' is not a file'
		sys.exit(msg)
	cascade_path_dict[name] = path
# ---------------------------------------------------------------------------
# make column unique
def make_column_unique(row_list, column) :
	for i in range( len(row_list) ) :
			row          = row_list[i]
			col          = row[column]
			col         += '_' + str(i)
			row[column] = col
# ---------------------------------------------------------------------------
# float_or_none
#
def float_or_none(string) :
	if string in [ 'nan', '_inf', '-inf', 'inf', '+inf' ] :
		return None
	return float(string)
# ---------------------------------------------------------------------------
# gaussian_cascade2at
#
density_name2id = None
def gaussian_cascade2at(prior_name, cascade_prior_row) :
	assert density_name2id != None
	lower = float_or_none( cascade_prior_row['lower'] )
	upper = float_or_none( cascade_prior_row['upper'] )
	mean  = float( cascade_prior_row['mean'] )
	#
	if cascade_prior_row['std'] == 'inf' :
		density_id = density_name2id['uniform']
	else :
		density_id = density_name2id['gaussian']
	#
	#
	eta   = None
	return [ prior_name , lower, upper, mean, std, eta, density_id ]
# ---------------------------------------------------------------------------
# log_gaussian_cascade2at
#
density_name2id = None
def log_gaussian_cascade2at(prior_name, cascade_prior_row, eta) :
	assert density_name2id != None
	lower = float_or_none( cascade_prior_row['lower'] )
	upper = float_or_none( cascade_prior_row['upper'] )
	mean  = float( cascade_prior_row['mean'] )
	if cascade_prior_row['std'] == 'inf' :
		density_id = density_name2id['uniform']
	else :
		density_id = density_name2id['log_gaussian']
	#
	#
	return [ prior_name , lower, upper, mean, std, float(eta), density_id ]
# ---------------------------------------------------------------------------
# db_connection
#
if not os.path.isdir('build') :
	print('mkdir build')
	os.mkdir('build')
#
subdir=os.path.join('build', cascade_dir)
if not os.path.isdir(subdir) :
	print('mkdir ' + subdir)
	os.mkdir(subdir)
#
new = True
file_name        = os.path.join(subdir, cascade_dir + '.db')
db_connection    = dismod_at.create_connection(file_name, new)
# ---------------------------------------------------------------------------
# integrand_table_in: integrand file as a list of dictionaries
# data_table_in:      data file as a list of dictionaries
# rate_prior_in:      rate prior file as a list of dictionaries
# effect_prior_in:    effect file as a list of dictionaries
# simple_prior_in:    simple prior file as a dictionary or dictionaries
# value_table_in:     value file as a single dictionary
#
cascade_data_dict = dict()
for name in cascade_path_dict :
	path      = cascade_path_dict[name]
	file_ptr  = open(path)
	reader    = csv.DictReader(file_ptr)
	#
	cascade_data_dict[name] = list()
	for row in reader :
		cascade_data_dict[name].append(row)
integrand_table_in = cascade_data_dict['integrand']
data_table_in      = cascade_data_dict['data']
rate_prior_in      = cascade_data_dict['rate_prior']
effect_prior_in    = cascade_data_dict['effect_prior']
simple_prior_in    = dict()
for row in cascade_data_dict['simple_prior'] :
	name                    = row['name']
	simple_prior_in[name] =  dict()
	for column in [ 'lower', 'upper', 'mean', 'std'  ] :
		simple_prior_in[name][column] = row[column]
value_table_in = dict()
for row in cascade_data_dict['value'] :
	value_table_in[ row['name'] ] = row['value']
# ---------------------------------------------------------------------------
# Output time table.
# time_list:
#
time_grid  = option_table_in['time_grid']
time_list  = time_grid.split()
for i in range( len(time_list) ) :
	time_list[i] = float( time_list[i] )
assert len(time_list) >= 2
#
col_name = [ 'time' ]
col_type = [ 'real' ]
row_list       = list()
for time in time_list :
	row_list.append([time])
tbl_name = 'time'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
# ---------------------------------------------------------------------------
# Output age table.
# age_list:
#
age_dict = dict()
for rate in [ 'iota', 'rho', 'chi', 'omega' ] :
	drate           = 'd' + rate
	age_dict[rate]  = list()
	age_dict[drate] = list()
	for row in rate_prior_in :
		if row['type'] == rate :
			age_dict[rate].append( float( row['age'] ) )
		if row['type'] == drate :
			age_dict[drate].append( float( row['age'] ) )
#
# This program assumes only one age grid in rate_prior_in
age_list = sorted( age_dict['iota'] )
for rate in [ 'iota', 'rho', 'chi', 'omega' ] :
	drate = 'd' + rate
	assert age_dict[rate] == age_list
	assert age_dict[drate] == age_list[0:-1]
#
col_name = [ 'age' ]
col_type = [ 'real' ]
row_list  = list()
for age in age_list :
	row_list.append([age])
tbl_name = 'age'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
# ---------------------------------------------------------------------------
# rate_prior_in_dict:
#
rate_prior_in_dict = dict()
for rate in [ 'iota', 'rho', 'chi', 'omega' ] :
	drate           = 'd' + rate
	rate_prior_in_dict[rate]  = list()
	rate_prior_in_dict[drate] = list()
	for row in rate_prior_in :
		if row['type'] == rate :
			rate_prior_in_dict[rate].append( row )
		if row['type'] == drate :
			rate_prior_in_dict[drate].append( row )
# ---------------------------------------------------------------------------
# covariate_name2id
#
header        = list( data_table_in[0].keys() )
covariate_name_list = list()
for name in header :
	if name.startswith('r_') or name.startswith('a_') :
		covariate_name_list.append(name)
covariate_name2id = collections.OrderedDict()
for covariate_id in range( len(covariate_name_list) ) :
	name                    = covariate_name_list[covariate_id]
	covariate_name2id[name] = covariate_id
# ---------------------------------------------------------------------------
# Output integrand table.
# integrand_name2id
#
integrand_name2id   = dict()
for integrand_id in range( len(integrand_table_in) ) :
	name                    = integrand_table_in[integrand_id]['integrand']
	integrand_name2id[name] = integrand_id
#
col_name = [ 'integrand_name', 'eta' ]
col_type = [ 'text',           'real']
row_list = list()
for row in integrand_table_in :
	integrand_name = row['integrand']
	if integrand_name == 'incidence' :
		integrand_name = 'Sincidence'
	if integrand_name == 'mtall' and option_table_in['mtall2mtother']=='yes' :
		integrand_name = 'mtother'
	row_list.append( [ integrand_name , float( row['eta'] ) ] )
tbl_name = 'integrand'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
# ---------------------------------------------------------------------------
# Output density table
# density_name2id
#
col_name = [  'density_name'   ]
col_type = [  'text'        ]
row_list = [
	['uniform'],
	['gaussian'],
	['laplace'],
	['log_gaussian'],
	['log_laplace']
]
tbl_name = 'density'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
#
density_name2id = dict()
for density_id in range( len(row_list) ) :
	name                  = row_list[density_id][0]
	density_name2id[name] = density_id
# ---------------------------------------------------------------------------
# Output node table
# node_name2id, node_table_list
#
col_name      = [ 'node_name', 'parent'  ]
col_type      = [ 'text',      'integer' ]
row_list      = list()
node_name2id  = dict()
#
# world
row_list.append( [ 'world', None ] )
node_name2id['world'] = 0
#
for row in data_table_in :
	parent = node_name2id['world']
	for level in [ 'super', 'region', 'subreg', 'atom' ] :
		name = row[level]
		if name != 'none' :
			if not name in node_name2id :
				node_id = len(row_list)
				row_list.append( [ name , parent ] )
				node_name2id[name] = node_id
			parent = node_name2id[name]
tbl_name = 'node'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
node_table_list = row_list
# ---------------------------------------------------------------------------
# Output weight table
# Output weight_grid table
#
col_name =   [ 'weight_name', 'n_age',   'n_time'   ]
col_type =   [ 'text',        'integer', 'integer'  ]
row_list = [ [ 'weight_one',  1,          1         ] ]
tbl_name = 'weight'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
#
col_name =   [  'weight_id', 'age_id',   'time_id',  'weight' ]
col_type =   [  'integer',   'integer',  'integer',  'real'   ]
row_list = [ [  0,           0,          0,           1.0     ] ]
tbl_name = 'weight_grid'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
# ---------------------------------------------------------------------------
# Output data table
#
col_name2type = collections.OrderedDict([
	# required columns
	('integrand_id', 'integer'),
	('density_id',   'integer'),
	('node_id',      'integer'),
	('weight_id',    'integer'),
	('hold_out',     'integer'),
	('meas_value',   'real'   ),
	('meas_std',     'real'   ),
	('age_lower',    'real'   ),
	('age_upper',    'real'   ),
	('time_lower',   'real'   ),
	('time_upper',   'real'   )
] )
col_name = list( col_name2type.keys() )
col_type = list( col_name2type.values() )
for name in covariate_name2id :
	col_name.append( 'x_%s' % covariate_name2id[name] )
	col_type.append( 'real' )
#
mtall_list    = list()
row_list      = list()
for row_in in data_table_in :
	mtall        = row_in['integrand'] == 'mtall'
	integrand_id = integrand_name2id[ row_in['integrand'] ]
	density_id   = density_name2id[ row_in['data_like'] ]
	weight_id    = 0
	hold_out     = row_in['hold_out']
	meas_value   = row_in['meas_value']
	meas_std     = row_in['meas_stdev']
	age_lower    = row_in['age_lower']
	age_upper    = row_in['age_upper']
	time_lower   = row_in['time_lower']
	time_upper   = row_in['time_upper']
	#
	# node_id
	node_id      = node_name2id['world']
	for level in [ 'super', 'region', 'subreg', 'atom' ] :
		if row_in[level] != 'none' :
			node_id = node_name2id[ row_in[level] ]
	#
	# check if node_id is at or below parent_node_id
	parent_node_id = node_name2id[ option_table_in['parent_node_name'] ]
	ok = node_id == parent_node_id
	while node_id != None and not ok :
		parent_index = 1
		node_id = node_table_list[node_id][parent_index]
		ok      = node_id == parent_node_id
	if ok :
		row_out = [
			integrand_id, # 0
			density_id,   # 1
			node_id,      # 2
			weight_id,    # 3
			hold_out,     # 4
			meas_value,   # 5
			meas_std,     # 6
			age_lower,    # 7
			age_upper,    # 8
			time_lower,   # 9
			time_upper    # 10
		]
		if mtall :
			ok = ok and float(age_lower) >= 5.0
			if ok :
				mtall_list.append(row_out)
		else :
			for name in covariate_name2id :
				value        = row_in[name]
				if math.isnan( float(value) ) :
					value = None
				row_out.append(value)
			row_list.append( row_out )
#
# sort the mtall data by age_lower, time_lower, node_id
mtall_list = sorted(mtall_list, key=lambda row: row[2]) # by node_id
mtall_list = sorted(mtall_list, key=lambda row: row[9]) # by time_lower
mtall_list = sorted(mtall_list, key=lambda row: row[7]) # by age_lower
#
# combine the mtall data that has the same node_id, time_lower, age_lower
previous_row  = [None]
for row  in mtall_list :
	if previous_row[0] == None :
		match = False
	else :
		match = True
		for i in [ 7, 9, 2 ] :
			match = match and previous_row[i] == row[i]
	#
	if match :
		n_sum      += 1
		meas_value += float( previous_row[5] )
		meas_std   += float( previous_row[6] )**2
	else :
		if previous_row[0] != None :
			meas_value   = meas_value / n_sum
			meas_std     = math.sqrt( meas_std / n_sum**2  )
			previous_row[5] = meas_value
			previous_row[6] = meas_std
			for name in covariate_name2id :
				previous_row.append(None)
			row_list.append(previous_row)
		n_sum        = 0
		meas_value   = 0.0
		meas_std     = 0.0
	previous_row = row
#
n_sum       += 1
meas_value  += float( previous_row[5] )
meas_std    += float( previous_row[6] )**2
meas_value   = meas_value / n_sum
meas_std     = math.sqrt( meas_std / n_sum**2  )
previous_row[5] = meas_value
previous_row[6] = meas_std
for name in covariate_name2id :
	previous_row.append('0')  # drop the covariates in mtall data
row_list.append(previous_row)
#
#
tbl_name = 'data'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
# ---------------------------------------------------------------------------
# Output covariate table
#
col_name   = [ 'covariate_name',  'reference', 'max_difference' ]
col_type   = [ 'text',             'real',     'real'           ]
row_list   = list()
for name in covariate_name2id :
	if name.startswith('r_') :
		cov_sum   = 0.0
		cov_count = 0
		for row in data_table_in :
			cov_value = float( row[name] )
			if not math.isnan(cov_value) :
				cov_sum   += cov_value
				cov_count += 1
		if cov_count == 0 :
			reference = 0.0
		else :
			reference = cov_sum / cov_count
		max_difference = None
	else :
		assert name.startswith('a_')
		reference = 0.0
		if name == 'a_sex' :
			max_difference = 0.6
		else :
			max_difference = None
	row_list.append( [name , reference, max_difference] )
tbl_name = 'covariate'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
# ---------------------------------------------------------------------------
# Start recording information for prior smooth and smooth_grid tables
# ---------------------------------------------------------------------------
# prior_col_name2type:
# smooth_col_name2type:
# smooth_grid_col_name2type:
prior_col_name2type = collections.OrderedDict([
	('prior_name',     'text'   ),
	('lower',          'real'   ),
	('upper',          'real'   ),
	('mean',           'real'   ),
	('std',            'real'   ),
	('eta',            'real'   ),
	('density_id',     'integer'),
])
smooth_col_name2type = collections.OrderedDict([
	('smooth_name',            'text'   ),
	('n_age',                  'integer'),
	('n_time',                 'integer'),
	('mulstd_value_prior_id',  'integer'),
	('mulstd_dage_prior_id',   'integer'),
	('mulstd_dtime_prior_id',  'integer')
])
#
smooth_grid_col_name2type = collections.OrderedDict([
	('smooth_id',       'integer'),
	('age_id',          'integer'),
	('time_id',         'integer'),
	('value_prior_id',  'integer'),
	('dage_prior_id',   'integer'),
	('dtime_prior_id',  'integer')
])
# --------------------------------------------------------------------------
# Initialize the row list for the prior, smooth, and smooth_grid tables
#
prior_row_list       = list()
smooth_row_list      = list()
smooth_grid_row_list = list()
# --------------------------------------------------------------------------
# child_smooth_id
#
lower      = None
upper      = None
mean       = 0.0
density_id = density_name2id['gaussian']
eta        = None
#
std    = float( option_table_in['child_value_std'] )
name   = 'child_value_piror'
child_value_prior_id = len( prior_row_list )
prior_row_list.append(
		[ name , lower, upper, mean, std, eta, density_id ]
)
#
std    = float( option_table_in['child_dtime_std'] )
name   = 'child_dtime_piror'
child_dtime_prior_id = len( prior_row_list )
prior_row_list.append(
		[ name , lower, upper, mean, std, eta, density_id ]
)
#
name            = 'child_smooth'
n_age           = 1
n_time          = len(time_list)
child_smooth_id = len(smooth_row_list)
smooth_row_list.append(
		[ name , n_age, n_time, None, None, None ]
)
value_prior_id = child_value_prior_id
dage_prior_id  = None
age_id         = 0
for time_id in range( n_time ) :
	if time_id + 1 < n_time :
		dtime_prior_id = child_dtime_prior_id
	else :
		dtime_prior_id = None
	smooth_grid_row_list.append( [
		child_smooth_id,
		age_id,
		time_id,
		value_prior_id,
		dage_prior_id,
		dtime_prior_id
	] )
# --------------------------------------------------------------------------
# child_omega_smooth_id
#
n_age                 = len(age_list)
n_time                = len(time_list)
lower      = None
upper      = None
mean       = 0.0
density_id = density_name2id['gaussian']
eta        = None
#
std    = float( option_table_in['child_dtime_std'] )
std    = std * math.sqrt( n_time / n_age )
name   = 'child_omega_dage_piror'
child_omega_dage_prior_id = len( prior_row_list )
prior_row_list.append(
		[ name , lower, upper, mean, std, eta, density_id ]
)
name                  = 'child_omega_smooth'
child_omega_smooth_id = len(smooth_row_list)
smooth_row_list.append(
		[ name , n_age, n_time, None, None, None ]
)
value_prior_id = child_value_prior_id
for age_id in range( n_age ):
	if age_id + 1 < n_age :
		dage_prior_id = child_omega_dage_prior_id
	else :
		dage_prior_id = None
	#
	for time_id in range( n_time ) :
		if time_id + 1 < n_time :
			dtime_prior_id = child_dtime_prior_id
		else :
			dtime_prior_id = None
		#
		smooth_grid_row_list.append( [
			child_omega_smooth_id,
			age_id,
			time_id,
			value_prior_id,
			dage_prior_id,
			dtime_prior_id
		] )
# --------------------------------------------------------------------------
# rate_dtime_prior_id
#
rate_dtime_prior_id = dict()
avg_delta_time      = (time_list[-1] - time_list[0]) / (len(time_list) - 1)
density_id          = density_name2id['log_gaussian']
lower               = None
upper               = None
mean                = 0.0
std                 = math.log( avg_delta_time / 10. )
for rate in [ 'pini', 'iota', 'rho', 'chi', 'omega' ] :
	if rate == 'pini' :
		eta = 1e-7
	else :
		eta = value_table_in[ 'kappa_' + rate ]
	rate_dtime_prior_id[rate]  = len( prior_row_list )
	prior_row_list.append([
		rate + '_dtime_prior',
		lower,
		upper,
		mean,
		std,
		eta,
		density_id
	])
# --------------------------------------------------------------------------
# pini_smooth_id
#
prior_in         = simple_prior_in['p_zero']
lower            = prior_in['lower']
upper            = prior_in['upper']
if float(upper) == 0.0 :
	pini_smooth_id = None
else :
	name             = 'pini_prior'
	prior_out        = gaussian_cascade2at(name, prior_in)
	pini_prior_id    = len( prior_row_list )
	prior_row_list.append(prior_out)
	#
	n_age          = 1
	n_time         = len( time_list )
	pini_smooth_id = len( smooth_row_list )
	smooth_row_list.append(
		[ 'pini_smooth', n_age, n_time, None, None, None ]
	)
	age_id        = 0
	dage_prior_id = None
	for time_id in range( n_time ) :
		if time_id + 1 < n_time :
			dtime_prior_id = rate_dtime_prior_id['pini']
		else :
			dtime_prior_id = None
		smooth_grid_row_list.append([
			pini_smooth_id,
			age_id,
			time_id,
			pini_prior_id,
			dage_prior_id,
			dtime_prior_id
		])
# --------------------------------------------------------------------------
# rate_is_zero
#
rate_is_zero = { 'omega':False }
rate_is_zero['pini'] = pini_smooth_id == None
rate_is_zero['iota'] = rate_case.find('iota_zero') != -1
rate_is_zero['rho']  = rate_case.find('rho_zero') != -1
#
rate_is_zero['chi'] = True
for row in rate_prior_in :
	if row['type'] == 'chi' and float( row['upper'] ) != 0.0 :
		rate_is_zero['chi'] = False;
# --------------------------------------------------------------------------
# rate_smooth_id
#
rate_case              = option_table_in['rate_case']
rate_smooth_id         = dict()
rate_smooth_id['pini'] = pini_smooth_id
#
delta_age          = (age_list[-1] - age_list[0]) / (len(age_list) - 1)
warn_rate_bounds   = set()
for rate in [ 'iota', 'rho', 'chi', 'omega' ] :
	drate = 'd' + rate
	xi    = float( simple_prior_in['xi_' + rate]['mean'] )
	#
	is_zero = rate_is_zero[rate]
	is_pos  = rate_case.find( rate + '_pos') != -1
	#
	# initialize some lists for this rate
	local_list     = list()
	local_list_id  = list()
	#
	# density same for this dlocal_list
	dlocal_list    = list()
	dlocal_list_id = list()
	#
	# smooth_row_list
	n_age  = len(age_list)
	n_time = len(time_list)
	name   = rate + '_smooth'
	if is_zero :
		rate_smooth_id[rate] = None
	else :
		rate_smooth_id[rate] = len(smooth_row_list)
		smooth_row_list.append(
				[ name, n_age, n_time, None, None, None ]
		)
		# need to fill in smooth_grid entries for this smoothing
		for age_id in range( n_age ) :
			# -----------------------------------------------------------------
			# determine value_prior_id
			name  = rate + '_prior'
			prior_in = rate_prior_in_dict[rate][age_id]
			if rate == 'omega' :
				# The cascade converts all cause mortality to constraints on
				# omega and does not really use its prior for omega, so
				# overide its mean.
				prior_in['mean'] = 0.01
			#
			assert not is_zero
			if is_pos :
				eta   = float( value_table_in['kappa_' + rate] )
				lower = eta / 100.
				prior_in['lower'] = str( lower )
				if float( prior_in['upper'] ) < lower :
					prior_in['mean']  = prior_in['lower']
					prior_in['upper'] = prior_in['lower']
					if rate not in warn_rate_bounds :
						msg = 'option.csv: rate_case = ' + rate_case + '\n'
						msg += 'some bounds for ' + rate + ' were changed '
						msg += 'so that it is positive.'
						print(msg)
						warn_rate_bounds.add(rate)
			#
			prior_at = gaussian_cascade2at(name, prior_in)
			(name, lower, upper, mean, std, eta, density_id) = prior_at
			if lower == upper :
				density_id = density_name2id['uniform']
			#
			# check if this prior already specified
			element = (lower, upper, mean, std)
			if element not in local_list :
				local_list_id.append( len(prior_row_list) )
				local_list.append( element )
				prior_row_list.append( prior_at )
			value_prior_id = local_list_id [ local_list.index(element) ]
			if age_id + 1 < n_age :
				# ----------------------------------------------------------------
				# determine dage_prior_id
				#
				# ignoring this prior
				# prior_in = rate_prior_in_dict[drate][age_id]
				#
				# using these values
				density_id = density_name2id['log_gaussian']
				name       = 'd' + rate + '_prior'
				eta        = value_table_in[ 'kappa_' + rate ]
				lower      = None
				mean       = 0.0
				upper      = None
				xi_factor  = float( option_table_in['xi_factor'] )
				std        = xi_factor * xi * math.sqrt( delta_age )
				#
				# check if this prior already specified
				element = (lower, upper, mean, std)
				if element not in dlocal_list :
					prior_at = [name, lower, upper, mean, std, eta, density_id]
					dlocal_list_id.append( len(prior_row_list) )
					dlocal_list.append( element )
					prior_row_list.append( prior_at )
				dage_prior_id = dlocal_list_id [ dlocal_list.index(element) ]
			else :
				dage_prior_id  = None
			# --------------------------------------------------------------------
			for time_id in range( n_time ) :
				if time_id + 1 < n_time :
					dtime_prior_id = rate_dtime_prior_id[rate]
				else :
					dtime_prior_id = None
				smooth_grid_row_list.append([
					rate_smooth_id[rate],
					age_id,
					time_id,
					value_prior_id,
					dage_prior_id,
					dtime_prior_id
				])
# --------------------------------------------------------------------------
# Output rate table
col_name = [  'rate_name', 'parent_smooth_id', 'child_smooth_id'  ]
col_type = [  'text',      'integer',         'integer'          ]
row_list = list()
#
for rate in [ 'pini', 'iota', 'rho', 'chi' ] :
	if rate_smooth_id[rate] == None :
		row = [ rate , None, None ]
	else :
		row = [ rate, rate_smooth_id[rate], child_smooth_id ]
	row_list.append( row )
rate     = 'omega'
row_list.append( [ rate, rate_smooth_id[rate], child_omega_smooth_id ] )
tbl_name = 'rate'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
# --------------------------------------------------------------------------
# Output mulcov table
col_name2type = collections.OrderedDict([
	('mulcov_type',     'text'    ),
	('rate_id',         'integer' ),
	('integrand_id',    'integer' ),
	('covariate_id',    'integer' ),
	('smooth_id',       'integer' )
])
col_name = list( col_name2type.keys() )
col_type = list( col_name2type.values() )
row_list = list()
#
if option_table_in['fit_covariates'] == 'yes' :
	for row in effect_prior_in :
		effect = row['effect']
		if effect in [ 'zeta', 'beta' ] :
			integrand    = row['integrand']
			covariate    = row['name']
			covariate_id = covariate_name2id[covariate]
			#
			if row['effect'] == 'zeta' :
				mulcov_type  = 'meas_std'
				integrand_id = integrand_name2id[integrand]
				rate_id      = None
				name         = integrand
			elif integrand in [ 'incidence', 'remission', 'mtexcess' ] :
				mulcov_type       = 'rate_value'
				integrand_id      = None
				# rate table order is pini, iota, rho, chi, omega
				if integrand == 'incidence' :
					rate_id = 1
					name    = 'iota'
				if integrand == 'remission' :
					rate_id = 2
					name    = 'rho'
				if integrand == 'mtexcess' :
					rate_id = 3
					name    = 'chi'
			else :
				mulcov_type  = 'meas_value'
				integrand_id = integrand_name2id[integrand]
				rate_id      = None
			#
			prior_name   = name + '_' + covariate + '_prior'
			prior_at     = gaussian_cascade2at(prior_name, row)
			prior_id     = len( prior_row_list )
			prior_row_list.append( prior_at )
			#
			smooth_name  = name + '_' + covariate + '_smooth'
			smooth_id    = len( smooth_row_list )
			smooth_row_list.append(
				[ smooth_name,  1, 1, None, None, None ]
			)
			smooth_grid_row_list.append(
				[ smooth_id, 0, 0, prior_id, None, None ]
			)
			#
			row_list.append(
				[ mulcov_type, rate_id, integrand_id, covariate_id, smooth_id ]
			)
#
tbl_name = 'mulcov'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
#
# --------------------------------------------------------------------------
# Output, prior, smooth, and smooth_grid tables
# --------------------------------------------------------------------------
for row in prior_row_list :
	# row = [ prior_name, lower, upper, mean, std, eta, density_id ]
	density_id = row[6]
	if density_id == density_name2id['uniform'] :
		row[4] = None
		row[5] = None
	if density_id == density_name2id['gaussian'] :
		row[5] = None
	if density_id == density_name2id['laplace'] :
		row[5] = None
col_name = list( prior_col_name2type.keys() )
col_type = list( prior_col_name2type.values() )
row_list = prior_row_list
tbl_name = 'prior'
make_column_unique(row_list, 0)
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
#
col_name = list( smooth_col_name2type.keys() )
col_type = list( smooth_col_name2type.values() )
row_list = smooth_row_list
tbl_name = 'smooth'
make_column_unique(row_list, 0)
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
#
col_name = list( smooth_grid_col_name2type.keys() )
col_type = list( smooth_grid_col_name2type.values() )
row_list = smooth_grid_row_list
tbl_name = 'smooth_grid'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
# ---------------------------------------------------------------------------
# Output option table.
parent_node_id = node_name2id[ option_table_in['parent_node_name'] ]
col_name = [ 'option_name', 'option_value' ]
col_type = [ 'text unique', 'text' ]
random_bound = option_table_in['random_bound']
if random_bound == '' :
	random_bound = None
row_list = [
	[ 'parent_node_id',         str(parent_node_id)              ],
	[ 'ode_step_size',          option_table_in['ode_step_size'] ],
	[ 'number_sample',          '10'                             ],
	[ 'fit_sample_index',       None                             ],
	[ 'random_seed',            str(int( timer.time() ))         ],
	[ 'rate_case',              option_table_in['rate_case']     ],
	[ 'quasi_fixed',            'true'                           ],
	[ 'print_level_fixed',      '5'                              ],
	[ 'print_level_random',     '0'                              ],
	[ 'tolerance_fixed',        '1e-8'                           ],
	[ 'random_bound',           random_bound                     ],
	[ 'tolerance_random',       '1e-8'                           ],
	[ 'max_num_iter_fixed',     '50'                             ],
	[ 'max_num_iter_random',    '50'                             ],
	[ 'derivative_test_fixed',  'none'                           ],
	[ 'derivative_test_random', 'none'                           ]
]
tbl_name = 'option'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
# --------------------------------------------------------------------------
# avgint table
col_name2type = collections.OrderedDict([
	('integrand_id',   'integer'     ),
	('node_id',        'integer'     ),
	('weight_id',      'integer'     ),
	('age_lower',      'real'        ),
	('age_upper',      'real'        ),
	('time_lower',     'real'        ),
	('time_upper',     'real'        )
])
col_name = list( col_name2type.keys() )
col_type = list( col_name2type.values() )
for j in range( len(covariate_name2id) ) :
	col_name.append( 'x_%s' % j )
	col_type.append( 'real' )
weight_id = 0
node_id   = node_name2id['world']
row_list = list()
row      = (7 + len(covariate_name2id) ) * [0]
row[1]  = node_id
row[2]  = weight_id
for time_id in range( len(time_list) ) :
	row[5]  = time_list[time_id]
	row[6]  = time_list[time_id]
	for age_id in range(len(time_list) ) :
		row[3]  = age_list[age_id]
		row[4]  = age_list[age_id]
		for integrand in integrand_name2id :
			row[0]  = integrand_name2id[integrand]
			row_list.append( copy.copy(row) )
tbl_name = 'avgint'
dismod_at.create_table(db_connection, tbl_name, col_name, col_type, row_list)
# --------------------------------------------------------------------------
print('import_cascade.py: OK')
