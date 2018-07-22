#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-18 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
# $begin csv2db$$ $newlinech #$$
# $spell
#	Csv
#	Dismod
#	str
#	pini
#	integrands
#	Sincidence
#	mtexcess
#	mtother
#	Tincidence
#	mtspecific
#	mtall
#	mtstandard
#	relrisk
#	std
#	mtwith
# $$
#
# $section Example Conversion of a Csv File to a Dismod_at Database$$
#
# $head Under Construction$$
# This example is under construction.
#
# $head Syntax$$
# $codei% import csv2db
# %$$
# $codei%csv2db.csv2db(%option_csv%, %data_csv%)%$$
#
# $head option_csv$$
# is an $code str$$ containing the option file name and must end with the
# $code .csv$$ extension.
# The first row contains the following column names
# $code name$$, $code value$$.
# Column names that begin with $code c_$$ are comments
# and will not be used by future versions of $code csv2db$$.
# The options are documented by the corresponding name value below
#
# $subhead database_name$$
# The $icode value$$ in this row is the name of the
# dismod_at database that is written by $code csv2db$$
# There is no default value for this name; i.e., it must appear.
#
# $subhead non_zero_rates$$
# The $icode value$$ in this row is a list rates that are non-zero
# in the model.
# The possible rates are
# $code pini$$, $code iota$$, $code rho$$, $code chi$$, $code omega$$.
# The rates in the list are separated by a single space
# and $code omega$$ must appear in the list.
# There is no default value for this name; i.e., it must appear.
#
# $head data_csv$$
# is an $code str$$ containing the data file name
# and must end with the $code .csv$$ extension.
# Each row of the data file corresponds to one data point.
# The first row of the file contains the column names.
# The other rows correspond to data points.
# Each column of the necessary columns in the data file is documented
# under its column name below.
# Column names that begin with $code c_$$ are comments
# and will not be used by future versions of $code csv2db$$.
#
# $head integrand$$
# This column of $icode data_csv$$ contains
# one of the following integrands:
#
# $subhead Sincidence$$
# The incidence rate relative to susceptible population:
# $latex \iota $$.
#
# $subhead remission$$
# The remission rate:
# $latex \rho $$.
#
# $subhead mtexcess$$
# The excess mortality rate:
# $latex \chi $$.
#
# $subhead mtother$$
# The other cause mortality rate:
# $latex \omega $$.
# Other cause mortality data is special in that it must satisfy
# $icode%age_lower% == %age_upper%$$,
# $icode%time_lower% == %time_upper%$$.
# In addition, it must be specified on a rectangular grid; i.e.,
# each age that appears must appear once and only once for every time, and
# each time that appears must appear once and only once for every age.
# The $code mtother$$ data is converted to a constraint on $latex \omega$$.
#
# $subhead mtwith$$
# The with condition mortality rate:
# $latex \omega + \chi $$.
#
# $subhead susceptible$$
# The susceptible fraction of the population:
# $latex S $$.
#
# $subhead withC$$
# The with condition fraction of the population:
# $latex C $$.
#
# $subhead prevalence$$
# The prevalence of the condition:
# $latex P = C / [ S + C ] $$.
#
# $subhead Tincidence$$
# The incidence rate relative to the total population:
# $latex \iota [ 1 - P ] $$.
#
# $subhead mtspecific$$
# The cause specific mortality rate:
# $latex \chi P $$.
#
# $subhead mtall$$
# The all cause mortality rate:
# $latex \omega + \chi P $$.
# We assume the $cref/mtother/csv2db/integrand/mtother/$$
# data is a surrogate
# for the $code mtall$$ data.
# The $code mtall$$ data is included as a check that the desired values
# are satisfied (by checking residuals in a data fit).
# For this reason, $cref/hold_out/csv2db/hold_out/$$ must be
# one for all the $code mtall$$ data.
#
# $subhead mtstandard$$
# The standardized mortality ratio:
# $latex [ \omega + \chi ] / [ \omega + \chi P ] $$.
#
# $subhead relrisk$$
# The relative risk:
# $latex [ \omega + \chi ] / \omega $$.
#
# $head age_lower$$
# This column of $icode data_csv$$ contains
# The initial age for averaging the integrand for this row; $latex b$$.
#
# $head age_upper$$
# This column of $icode data_csv$$ contains
# the final age for averaging the integrand for this row; $latex c$$.
#
# $head time_lower$$
# This column of $icode data_csv$$ contains
# the initial time for averaging the integrand for this row; $latex r$$.
#
# $head time_upper$$
# This column of $icode data_csv$$ contains
# the final time for averaging the integrand for this row; $latex s$$.
#
# $head meas_value$$
# This column of $icode data_csv$$ contains
# the value of the average integrand plus measurement noise
# $latex \[
#	e + \frac{1}{c-b} \frac{1}{s-r} \int_b^c \int_r^s I(a , t) \; da \; dt
# \] $$
# where $latex e$$ is the measurement noise,
# $latex a$$ and $latex t$$ are the age and time integration variables,
# and $latex I(a, t)$$ is the value of the integrand for this row.
#
# $head meas_std$$
# This column of $icode data_csv$$ contains
# the standard deviation of the measurement noise $latex e$$.
#
# $head hold_out$$
# This column of $icode data_csv$$ contains
# either zero or one. If it is one, this row is included
# dismod_at fits. Otherwise it is excluded.
# In either case, the residuals are computed for this row.
#
# $end
# -----------------------------------------------------------------------------
def constant_weight_fun(a, t) :
	return 1.0
#
def uniform10_fun(a, t) :
	v  = 'uniform01'
	da = None
	dt = None
	return (v, da, dy)
#
def omega_constraint_fun(a, t, age_grid, time_grid, omega_grid) :
	i  = age_grid.index(a)
	j  = time_grid.index(t)
	v  = omega[i, j]
	da = None
	dv = None
	return (v, da, dv)
# -----------------------------------------------------------------------------
def csv2db(option_csv, data_csv) :
	import sys
	import os
	import dismod_at
	import csv
	import copy
	import numpy
	# -------------------------------------------------------------------------
	# option_csv, data_csv
	#
	for file_name in option_csv data_csv :
		if not file_name.endswith('.csv') :
			msg  = 'csv2db: option_csv file ' + file_name
			msg += ' does not end with .csv'
			sys.exit(msg)
		if not os.path.isfile(file_name) :
			msg = 'csv2db: cannot find option_csv file ' + file_name
			sys.exit(msg)
	# -------------------------------------------------------------------------
	# file_option
	file_ptr    = open(option_csv, 'r')
	reader      = data_csv.DictReader(file_ptr)
	file_option = dict()
	for row in reader :
		name              = row['name']
		value             = row['value'
		file_option[name] = value
	file_ptr.close()
	required = [ 'database_name', 'non_zero_rates' ] :
	for name in required :
		if not name in file_option :
			msg = 'csv2db: ' + name + ' not in option_csv file ' + option_csv
			sys.exit(msg)
	if 'omega' not in file_option['non_zero_rates'] :
		msg = 'csv2db: omega not in non_zero_rates in ' + option_csv
		sys.exit(msg)
	# -------------------------------------------------------------------------
	# file_data
	file_ptr   = open(data_csv, 'r')
	reader     = data_csv.DictReader(file_ptr)
	file_data  = list()
	for row in reader :
		file_data.append( copy.copy(row) )
	file_ptr.close()
	#
	line  = 1
	mtother_found = False
	for row in file_data :
		line += 1
		row['age_lower']  = float( row['age_lower']  )
		row['age_upper']  = float( row['age_upper']  )
		row['time_lower'] = float( row['time_lower'] )
		row['time_upper'] = float( row['time_upper'] )
		row['meas_value'] = float( row['meas_value'] )
		row['meas_std']   = float( row['meas_std']   )
		row['hold_out']   = bool(  row['hold_out']   )
		#
		if row['integrand'] == 'mtother' :
			found = True
			if row['age_lower'] != row['age_upper'] :
				msg  = 'csv2db: line ' + str(line)
				msg += ' of data_csv ' + data_csv + '\n'
				msg += 'age_lower not equal age_upper for mtother data.'
				sys.exit(msg)
			if row['time_lower'] != row['time_upper'] :
				msg  = 'csv2db: line ' + str(line)
				msg += ' of file ' + data_csv + '\n'
				msg += 'time_lower not equal time_upper for mtother data.'
				sys.exit(msg)
		if row['integrand'] == 'mtall' :
			if not row['hold_out'] :
				msg  = 'csv2db: line ' + str(line)
				msg += ' of file ' + data_csv + '\n'
				msg += 'hold_out is not 1 for mtall data.'
				sys.exit(msg)

	if not mtother_found
		msg = 'csv2db: no mtother data in ' + data_csv
		sys.exit(msg)
	# -------------------------------------------------------------------------
	# age_grid, time_grid, omega_grid
	age_set  = set()
	time_set = set()
	for row in file_data :
		if row['integrand'] == 'mtother' :
			age_set.add(  row['age_lower']  )
			time_set.add( row['time_lower'] )
	age_grid  = sorted(age_set)
	time_grid = sorted(time_set)
	n_age          = len(age_grid)
	n_time         = len(time_grid)
	count          = numpy.zeros( (n_age, n_time), dtype=int )
	omega_grid     = numpy.zeros( (n_age, n_time), dtype=float )
	for row in file_data :
		if row['integrand'] == 'mtother' :
			age        = row['age_lower']
			time       = row['time_lower']
			age_index  = age_grid.index(age)
			time_index = time_grid.index(time)
			omega_grid[age_index, time_index] = row['meas_value']
			count[age_index, time_index] += 1
	if not (count == 1).all() :
		msg = 'csv2db: the mtother data in ' + data_csv
		msg += ' is not on a rectangular grid'
		sys.exit(msg)
	# -------------------------------------------------------------------------
	# db_file
	file_name = os.path.basename(data_csv)[0:-4] + '.db'
	dir_name  = os.path.dirname(data_csv)
	db_file   = os.path.join(dir_name, file_name)
	# -------------------------------------------------------------------------
	# age_min, age_max
	age_min = + float('inf')
	age_max = - float('inf')
	for row in file_data :
		age_min = min(age_min, row['age_lower'], row['age_upper'])
		age_max = max(age_min, row['age_lower'], row['age_upper'])
	# -------------------------------------------------------------------------
	# time_min, time_max
	time_min = + float('inf')
	time_max = - float('inf')
	for row in file_data :
		time_min = min(time_min, row['time_lower'], row['time_upper'])
		time_max = max(time_min, row['time_lower'], row['time_upper'])
	# -------------------------------------------------------------------------
	# age_list, time_list
	age_list  = copy.copy( age_grid )
	time_list = copy.copy( time_grid )
	#
	if age_min < age_list[0] :
		age_list.insert(0, age_min)
	if time_min < time_list[0] :
		time_list.insert(0, time_min)
	#
	if age_max > age_list[-1] :
		age_list.append(age_max)
	if time_max > time_list[-1] :
		time_list.append(time_max)
	# -------------------------------------------------------------------------
	# integrand_table
	integrand_table = [
		'name': 'Sincidence',
		'name': 'remission',
		'name': 'mtexcess',
		'name': 'mtother',
		'name': 'mtwith',
		'name': 'susceptible',
		'name': 'withC',
		'name': 'prevalence',
		'name': 'Tincidence',
		'name': 'mtspecific',
		'name': 'mtall',
		'name': 'mtstandard',
		'name': 'relrisk'
	]
	# -------------------------------------------------------------------------
	# node_table
	node_table = [ { 'name':'no_name', 'parent': '' } ]
	# -------------------------------------------------------------------------
	# weight_table
	fun          = constant_weight_fun
	weight_table = [
		{ 'name':'constant',  'age_id':[0], 'time_id':[0], 'fun':fun }
	]
	# -------------------------------------------------------------------------
	# covariate_table
	covariate_table = list()
	# -------------------------------------------------------------------------
	# avgint_table
	rate2integrand = {
		'iota ' : 'Sincidence',
		'rho'   : 'remission',
		'chi'   : 'mtexcess',
		'omega' : 'mtother'
	]
	non_zero_rates = file_option['non_zero_rates'].split() :
	avgint_table   = [ 'prevalence' ]
	for rate in non_zero_rates :
		if rate != 'pini' :
			integrand = rate2integrand[rate]
			for age in age_grid :
				for time in time_grid :
					row = {
							'integrand': integrand,
							'node':      'no_name',
							'weight':    'constant',
							'age_lower':  age,
							'age_upper':  age,
							'time_lower': time,
							'time_lower': time
					}
					avgint_table.append(row)
	# -------------------------------------------------------------------------
	# data_table
	data_table = list()
	copy_column  = [
		'integrand',
		'hold_out',
		'age_lower',
		'age_upper',
		'time_lower',
		'time_upper',
		'meas_value',
		'meas_std'
	]
	for row in file_data :
		if row['integrand'] != 'mtother' :
			data_row = dict()
			for field in copy_column
				data_row[field] = row[field]
		data_row['node']    = 'no_name'
		data_row['weight']  = 'constant'
		data_row['density'] = 'gaussian'
	# -------------------------------------------------------------------------
	# prior_table
	prior = {
		'name': 'uniform01',
		'lower': 0.0,
		'upper': 1.0,
		'density': 'uniform'
	}
	prior_table = [ prior ]
	# -------------------------------------------------------------------------
	# smooth_table
	start = 0
	if age_list[0] != age_grid[0] :
		start = 1
	age_id = range(start, len(age_grid) + 1)
	#
	start = 0
	if time_list[0] != time_grid[0] :
		start = 1
	time_id = range(start, len(time_grid) + 1)
	#
	# omega
	def omega_fun(a, t) :
		return omega_constrant_fun(a, t, age_grid, time_grid, omega_grid)
	smooth_table  = list()
	non_zero_rates = file_option['non_zero_rates'].split()
	for rate in non_zero_rates :
		if rate == 'omega' :
			smooth_table.append( {
				'name':     rate,
				'age_id':   age_id,
				'time_id':  time_id,
				'fun':      omega_fun,
		} ]
		elif rate == 'pini' :
			smooth_table.append( {
				'name':     rate,
				'age_id':	[ age_id[0] ],
				'time_id':  time_id,
				'fun':      uniform01_fun
			} )
		else :
			smooth_table.append( {
				'name':     rate,
				'age_id':   age_id,
				'time_id':  time_id,
				'fun':      uniform01_fun,
		} ]
	# -------------------------------------------------------------------------
	# rate_table
	non_zero_rates = file_option['non_zero_rates'].split()
	rate_table     = list()
	for rate in [ 'pini', 'iota', 'rho', 'chi', 'omega' ] :
		smooth = None
		for i in len(smooth_table) :
			if smooth_table[i].name == rate :
				smooth = smooth_table[i]
		rate_table.append( {
			'name':          rate,
			'parent_smooth': smooth
		} )
	# -------------------------------------------------------------------------
	# empty tables
	nslist_table = list()
	mulcov_table = list()
	option_table = list()
	# -------------------------------------------------------------------------
	create_database(
	     file_name,
	     age_list
	     time_list
	     integrand_table,
	     node_table,
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
