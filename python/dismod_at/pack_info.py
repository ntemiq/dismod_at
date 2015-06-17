# $Id:$
#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-15 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
# 	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
# $begin pack_info_ctor$$ $newlinech #$$
# $spell
#	dismod
#	mulcov
# $$
#
#
# $section Variable Packing Information: Python Constructor$$
#
# $head Syntax$$
# $icode%pack_object% = dismod_at.pack_info(
#	%n_integrand%, %n_child%, %smooth_dict%, %mulcov_dict%, %rate_dict%
# %$$
# $icode%size% = %pack_object%.size()%$$
#
# $head See Also$$
# see $cref get_table_dict$$.
#
# $head n_integrand$$
# This is a non-negative $code int$$
# that is equal to the number of rows in the
# $cref integrand_table$$.
# If the $cref mulcov_table$$ has size zero,
# then $icode n_integrand$$ can be zero (a case used for testing purposes).
#
# $head n_child$$
# This is a non-negative $code int$$
# that is the number of children; i.e., the size of
# $cref/child group/node_table/parent/Child Group/$$
# corresponding to the
# $cref/parent_node/fit_table/parent_node_id/$$.
#
# $head smooth_dict$$
# This is a list of dictionaries.
# For each primary key value $icode smooth_id$$
# and column name $icode col_name$$ in the $cref smooth_table$$,
# $icode%smooth_dict%[%smooth_id%][%col_name%]%$$
# is the corresponding value. Note that
# $codei%
#	%smooth_dict%[%smooth_id%][smooth_id]% == %smooth_id%
# %$$
#
# $subhead Remark$$
# Only the $cref/n_age/smooth_table/n_age/$$
# and $cref/n_time/smooth_table/n_time/$$ columns are used.
#
# $head mulcov_dict$$
# This is a list of dictionaries.
# For each primary key value $icode mulcov_id$$
# and column name $icode col_name$$ in the $cref mulcov_table$$,
# $icode%mulcov_dict%[%mulcov_id%][%col_name%]%$$
# is the corresponding value. Note that
# $codei%
#	%mulcov_dict%[%mulcov_id%][mulcov_id]% == %mulcov_id%
# %$$
#
# $head rate_dict$$
# This is a list of dictionaries.
# For each primary key value $icode rate_id$$
# and column name $icode col_name$$ in the $cref rate_table$$,
# $icode%rate_dict%[%rate_id%][%col_name%]%$$
# is the corresponding value. Note that
# $codei%
#	%rate_dict%[%rate_id%][rate_id]% == %rate_id%
# %$$
#
# $head size$$
# this return value is a $code int$$ equal to the number of
# $cref/model_variables/model_variable/$$.
#
# $end
# ----------------------------------------------------------------------------
# $begin pack_info_mulstd$$ $newlinech #$$
# $spell
#	var
#	mulstd
#	dage
#	dtime
#	const
#	dismod
# $$
#
# $section Pack Variables: Standard Deviations Multipliers$$
#
# $head Syntax$$
# $icode%offset% = %pack_object%.mulstd_offset(%smooth_id%)%$$
#
# $head pack_object$$
# This object was constructed using $cref pack_info_ctor$$.
#
# $head smooth_id$$
# This is an $code int$$ that specifies the
# $cref/smooth_id/smooth_table/smooth_id/$$ for this multiplier.
#
# $head offset$$
# The return value is the $code int$$
# offset (index) in the packed variable list
# where the three variables for this smoothing begin.
# The three variables for each smoothing are the
# value, dage, and dtime standard deviation multipliers
# (and should always be used in that order).
#
# $head Example$$
# See $cref/pack_info Example/pack_info/Example/$$.
#
# $end
# ----------------------------------------------------------------------------
# $begin pack_info_rate$$ $newlinech #$$
# $spell
# $$
#
# $section Pack Variables: Rates$$
#
# $head Syntax$$
# $icode%info% = %pack_object%.rate_info(%rate_id%, %j%)%$$
#
# $head pack_object$$
# This object was constructed using $cref pack_info_ctor$$.
#
# $head rate_id$$
# This is an $code int$$ that specifies the
# $cref/rate_id/rate_table/rate_id/$$ for the rate values.
#
# $head j$$
# This is an $code int$$.
# If $icode%j% < %n_child%$$, these rates are for the corresponding
# child node.
# If $icode%j% == %n_child%$$, these rates are for the
# parent node.
#
# $head info$$
# The return value is the $code info$$
# is a dictionary with the following keys:
#
# $subhead smooth_id$$
# $icode%info%['smooth_id']%$$ is an $code int$$ equal to the
# $cref/smooth_id/smooth_table/smooth_id/$$ for the rate.
# Note that the smoothing is the same for all child rates.
#
# $subhead n_var$$
# $icode%info%['n_var']%$$ is an $code int$$ equal to the
# the number of variables for this $icode rate_id$$ and index $icode j$$.
#
# $subhead offset$$
# $icode%info%['offset']%$$ is an $code int$$ equal to the
# the index in the packed variable list where the variables begin.
#
# $head Example$$
# See $cref/pack_info Example/pack_info/Example/$$.
#
# $end
# ----------------------------------------------------------------------------
class pack_info :
	# ------------------------------------------------------------------------
	def __init__(
		self, n_integrand, n_child, smooth_dict, mulcov_dict, rate_dict
	) :
		# initialize offset
		offset = 0;

		# number_rate_enum_ (pini, iota, rho, chi, omega)
		self.number_rate_enum_ = 5

		# n_smooth_
		self.n_smooth_ = len(smooth_dict)

		# n_integrand_
		self.n_integrand_ = n_integrand

		# n_child_
		self.n_child_ = n_child

		# mulstd_offset_
		self.mulstd_offset_ = offset
		offset        += 3 * self.n_smooth_

		# rate_info_
		self.rate_info_ = list()
		for rate_id in range( self.number_rate_enum_ ) :
			self.rate_info_.append( list() )
			for j in range(n_child + 1) :
				self.rate_info_[rate_id].append( dict() )
				if j < n_child :
					smooth_id = rate_dict[rate_id]['child_smooth_id']
				else :
					smooth_id = rate_dict[rate_id]['parent_smooth_id']
				n_age  = smooth_dict[smooth_id]['n_age']
				n_time = smooth_dict[smooth_id]['n_time']
				n_var  = n_age * n_time
				self.rate_info_[rate_id][j]['smooth_id'] = smooth_id
				self.rate_info_[rate_id][j]['n_var']     = n_var
				self.rate_info_[rate_id][j]['offset']    = offset
				offset += n_var
				#
				# check assumption about pini smoothing
				assert rate_id != 0 or n_age == 1

		# meas_mean_mulcov_info_ and meas_std_mulcov_info_
		self.meas_mean_mulcov_info_ = list()
		self.meas_std_mulcov_info_ = list()
		for integrand_id in range(n_integrand) :
			for mulcov_id in range( len(mulcov_dict) ) :
				match  = mulcov_dict[mulcov_id]['mulcov_type'] == 'meas_mean'
				match |= mulcov_dict[mulcov_id]['mulcov_type'] == 'meas_std'
				tmp_id = int( mulcov_dict[mulcov_id]['integrand_id'] )
				match &= tmp_id == integrand_id
				if match :
					mulcov_type  = mulcov_dict[mulcov_id]['mulcov_type']
					covaraite_id = mulcov_dict[mulcov_id]['covariate_id']
					smooth_id    = mulcov_dict[mulcov_id]['smooth_id']
					n_age        = smooth_dict[smooth_id]['n_age']
					n_time       = smooth_dict[smooth_id]['n_time']
					info         = {
						'covariate_id' : covaraite_id ,
						'smooth_id'    : smooth_id ,
						'n_var'        : n_age * n_time,
						'offset'       : offset
					}
					if mulcov_type == 'meas_mean' :
						info_list = self.meas_mean_mulcov_info_
					elif mulcov_type == 'meas_std' :
						info_list = self.meas_std_mulcov_info_
					for j in range( len(info_list) ) :
						if info_list[j]['covariate_id'] == covaraite_id :
							msg  = 'mulcov_dict: '
							msg += 'covariate_id appears twice with '
							msg += 'mulcov_type equal to ' + mulcov_type
							sys.exit(msg)
					info_list.append(info)
					offset += info['n_var']

		# rate_mean_mulcov_info_
		self.rate_mean_mulcov_info_ = list()
		for integrand_id in range(n_integrand) :
			for mulcov_id in range( len(mulcov_dict) ) :
				match  = mulcov_dict[mulcov_id]['mulcov_type'] == 'rate_mean'
				tmp_id = int( mulcov_dict[mulcov_id]['integrand_id'] )
				match &= tmp_id == integrand_id
				if match :
					covaraite_id = mulcov_dict[mulcov_id]['covariate_id']
					smooth_id    = mulcov_dict[mulcov_id]['smooth_id']
					n_age        = smooth_dict[smooth_id]['n_age']
					n_time       = smooth_dict[smooth_id]['n_time']
					info         = {
						'covariate_id' : covaraite_id ,
						'smooth_id'    : smooth_id ,
						'n_var'        : n_age * n_time,
						'offset'       : offset
					}
					info_list = rate_mean_mulcov_info_
					for j in range( len(info) ) :
						if info_list[j].covariate_id == covaraite_id :
							msg  = 'mulcov_dict: '
							msg += 'covariate_id appears twice with '
							msg += 'mulcov_type equal to rate_mean'
							sys.exit(msg)
					info_list.append(info)
					offset += info['n_var']

		# size is final offset
		self.size_ = offset
	# ------------------------------------------------------------------------
	def size(self) :
		return self.size_
	# ------------------------------------------------------------------------
	def mulstd_offset(self, smooth_id) :
		assert smooth_id < self.n_smooth_
		return self.mulstd_offset_ + 3 * smooth_id
	# ------------------------------------------------------------------------
	def rate_info(self, rate_id, j) :
		assert j < self.n_child_
		return self.rate_info_[rate_id][j]
	# ------------------------------------------------------------------------
	def meas_mean_mulcov_n_cov(self, integrand_id) :
		return len(self.meas_mean_mulcov_info_)
	def meas_mean_mulcov_info(self, integrand_id, j) :
		return self.meas_mean_mulcov_info[integrand_id][j]
	# ------------------------------------------------------------------------
	def meas_std_mulcov_n_cov(self, integrand_id) :
		return len(self.meas_std_mulcov_info_)
	def meas_std_mulcov_info(integrand_id, j) :
		return meas_std_mulcov_info[integrand_id][j]
	# ------------------------------------------------------------------------
	def rate_mean_mulcov_n_cov(self, integrand_id) :
		return len(rate_mean_mulcov_info_)
	def rate_mean_mulcov_info(integrand_id, j) :
		return rate_mean_mulcov_info[integrand_id][j]
# ----------------------------------------------------------------------------



