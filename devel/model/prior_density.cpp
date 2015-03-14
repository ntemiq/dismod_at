// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rate Estimation as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
/*
$begin prior_density$$
$spell
	logden
	pack_vec
	CppAD
	const
	subvectors
	std
	struct
	fabs
	var
$$

$section Compute Prior Density for the Fixed and Random Effects$$

$head Syntax$$
$icode%logden% = prior_density(
	%pack_object%, %pack_vec%,
	%age_table%, %time_table%, %prior_table%, %s_info_vec%
)%$$

$head Purpose$$
The joint prior density for the fixed effects $latex \theta$$
and random effects $latex u$$ is
$latex \[
	\B{p} ( u | \theta ) \B{p} ( \theta )
\] $$
See
$cref/p(u|theta)/random_prior/p(u|theta)/$$ and
$cref/p(theta)/fixed_prior/p(theta)/$$.

$head Float$$
The type $icode Float$$ must be one of the following:
$code double$$, $code AD<double>$$, $code AD< AD<double> >$$,
where $code AD$$ is $code CppAD::AD$$.

$head pack_object$$
This argument has prototype
$codei%
	const pack_info& %pack_object%
%$$
and is the $cref pack_info$$ information corresponding to
$icode pack_vec$$.

$head pack_vec$$
This argument has prototype
$codei%
	const CppAD::vector<%Float%>& %pack_vec%
%$$
and is a vector of values for all of the
$cref/model variables/model_variable/$$.
Only the following subvectors of $icode pack_vec$$ are used:
$cref pack_info_rate$$,
$cref pack_info_rate_mulcov$$.

$head age_table$$
This argument has prototype
$codei%
	const CppAD::vector<double>&  %age_table%
%$$
and is the $cref/age_table/get_age_table/age_table/$$.

$head time_table$$
This argument has prototype
$codei%
	const CppAD::vector<double>&  %time_table%
%$$
and is the $cref/time_table/get_time_table/time_table/$$.

$head prior_table$$
This argument has prototype
$codei%
	const CppAD::vector<prior_struct>&  %prior_table%
%$$
and is the $cref/prior_table/get_prior_table/prior_table/$$.
Only to following fields are used
$cref/density_id/prior_table/density_id/$$,
$cref/mean/prior_table/mean/$$,
$cref/std/prior_table/std/$$,
$cref/eta/prior_table/eta/$$.

$head s_info_vec$$
This argument has prototype
$codei%
	const CppAD::vector<smooth_info>& %s_info_vec%
%$$
For each $cref/smooth_id/smooth_table/smooth_id/$$,
$codei%
	%s_info_vec%[ %smooth_id% ]
%$$
is the corresponding $cref smooth_info$$ information.

$head logden$$
The return value has prototype
$codei%
	prior_density_struct<%Float%> %logden%
%$$
The log of the density function (see $cref model_density$$)
is represented by
$codei%
	%Float%                %smooth%   = %logden%.smooth
	CppAD::vector<%Float%> %sub_abs%  = %logden%.sub_abs
%$$
The value $icode smooth$$ and components of $icode logden_sub_abs$$
are infinitely differentiable with
respect to the model variables $cref/pack_vec/prior_density/pack_vec/$$;
i.e., smooth.
The log of the prior density for all the
$cref/model_variables/model_variable/$$ is
$codei%
	%sum% = %smooth%;
	for(size_t %i% = 0; %i% < %sub_abs%.size(); %i%++)
		%sum% -= fabs( %sub_abs% );
%$$.

$children%example/devel/model/prior_density_xam.cpp
%$$
$head Example$$
The file $cref prior_density_xam.cpp$$ contains an example and test
of using this routine.

$end
*/

# include <utility>
# include <dismod_at/prior_density.hpp>
# include <dismod_at/residual_density.hpp>
# include <dismod_at/prior_density.hpp>

namespace {
	using dismod_at::residual_density_struct;
	using dismod_at::prior_density_struct;
	using dismod_at::prior_struct;
	using dismod_at::smooth_info;

	using dismod_at::density_enum;
	using dismod_at::laplace_enum;
	using dismod_at::log_laplace_enum;
	using dismod_at::number_density_enum;

	// log_prior_density
	template <class Float>
	residual_density_struct<Float> log_prior_density(
		const prior_struct&   prior    ,
		const Float&          variable )
	{	assert ( 0 <= prior.density_id  );
		assert ( prior.density_id < number_density_enum );

		density_enum density = density_enum(prior.density_id);
		Float        mean    = Float(prior.mean);
		Float        std     = Float(prior.std);
		Float        eta     = Float(prior.eta);
		return residual_density( density, variable, mean, std, eta );
	}

	// add_to_logden
	template <class Float>
	void add_to_logden(
		prior_density_struct<Float>&    logden      ,
		int                             density_id  ,
		residual_density_struct<Float>& wres_logden )
	{	logden.smooth += wres_logden.logden_smooth;
		if( (density_id == laplace_enum) | (density_id == log_laplace_enum) )
			logden.sub_abs.push_back( wres_logden.logden_sub_abs );
	}


	// log_prior_on_grid
	template <class Float>
	void log_prior_density_on_grid(
		prior_density_struct<Float>&           logden          ,
		size_t                                 offset          ,
		const CppAD::vector<Float>&            pack_vec         ,
		const CppAD::vector<double>&           age_table       ,
		const CppAD::vector<double>&           time_table      ,
		const CppAD::vector<prior_struct>&     prior_table     ,
		const smooth_info&                     s_info          )
	{	size_t n_age  = s_info.age_size();
		size_t n_time = s_info.time_size();

		// used to get results from log_prior_density
		residual_density_struct<Float> wres_logden;

		// value smoothing
		for(size_t i = 0; i < n_age; i++)
		{	for(size_t j = 0; j < n_time; j++)
			{	Float  var      = Float(pack_vec[offset + i * n_time + j]);
				size_t prior_id           = s_info.value_prior_id(i, j);
				const prior_struct& prior = prior_table[prior_id];
				wres_logden               = log_prior_density(prior, var);
				add_to_logden(logden, prior.density_id, wres_logden);
			}
		}
		// age difference smoothing
		for(size_t i = 0; i < (n_age-1); i++)
		{	double a0 = age_table[ s_info.age_id(i) ];
			double a1 = age_table[ s_info.age_id(i+1) ];
			assert( a1 > a0 );
			for(size_t j = 0; j < n_time; j++)
			{	Float  v0       = pack_vec[offset + i * n_time + j];
				Float  v1       = pack_vec[offset + (i+1) * n_time + j];
				Float  dv_da    = Float((v1 - v0) / (a1 - a0));
				size_t prior_id = s_info.dage_prior_id(i, j);
				const prior_struct& prior = prior_table[prior_id];
				wres_logden               = log_prior_density(prior, dv_da);
				add_to_logden(logden, prior.density_id, wres_logden);
			}
		}
		// time difference smoothing
		for(size_t j = 0; j < (n_time-1); j++)
		{	double t0 = time_table[ s_info.time_id(j) ];
			double t1 = time_table[ s_info.time_id(j+1) ];
			assert( t1 > t0 );
			for(size_t i = 0; i < n_age; i++)
			{
				Float  v0       = pack_vec[offset + i * n_time + j];
				Float  v1       = pack_vec[offset + i * n_time + j + 1];
				Float  dv_dt    = Float((v1 - v0) / (t1 - t0));
				size_t prior_id = s_info.dtime_prior_id(i, j);
				const prior_struct& prior = prior_table[prior_id];
				wres_logden               = log_prior_density(prior, dv_dt);
				add_to_logden(logden, prior.density_id, wres_logden);
			}
		}
		return;
	}



}

namespace dismod_at { // BEGIN_DISMOD_AT_NAMESPACE

// prior_density
template <class Float>
prior_density_struct<Float> prior_density(
	const pack_info&                        pack_object        ,
	const CppAD::vector<Float>&            pack_vec         ,
	const CppAD::vector<double>&           age_table       ,
	const CppAD::vector<double>&           time_table      ,
	const CppAD::vector<prior_struct>&     prior_table     ,
	const CppAD::vector<smooth_info>&      s_info_vec      )
{
	// initialize the log of the prior density as zero
	prior_density_struct<Float> logden;
	logden.smooth = 0.0;
	assert( logden.sub_abs.size() == 0 );

	// used to get results from log_prior_density
	residual_density_struct<Float> wres_logden;

	// number of smoothings
	size_t n_smooth = s_info_vec.size();

	// smoothing multipliers
	for(size_t smooth_id = 0; smooth_id < n_smooth; smooth_id++)
	{	// offset for this smoothing
		size_t offset = pack_object.mulstd_offset(smooth_id);

		// multiplier for value smoothing
		Float mulstd = Float(pack_vec[offset + 0]);

		// prior index for this multilier
		size_t prior_id           = s_info_vec[smooth_id].mulstd_value();
		const prior_struct* prior = &prior_table[prior_id];

		// add prior density for this multipliers value
		wres_logden         = log_prior_density(*prior, mulstd);
		add_to_logden(logden, prior->density_id, wres_logden);

		// add multiplier for age difference smoothing
		mulstd              = pack_vec[offset + 1];
		prior_id            = s_info_vec[smooth_id].mulstd_dage();
		prior               = &prior_table[prior_id];
		wres_logden         = log_prior_density(*prior, mulstd);
		add_to_logden(logden, prior->density_id, wres_logden);

		// multiplier for time difference smoothing
		mulstd              = pack_vec[offset + 2];
		prior_id            = s_info_vec[smooth_id].mulstd_dtime();
		prior               = &prior_table[prior_id];
		wres_logden         = log_prior_density(*prior, mulstd);
		add_to_logden(logden, prior->density_id, wres_logden);
	}

	// rates
	pack_info::subvec_info info;
	size_t n_child = pack_object.child_size();
	for(size_t rate_id = 0; rate_id < number_rate_enum; rate_id++)
	{	// for all children and parent
		for(size_t child = 0; child <= n_child; child++)
		{	info = pack_object.rate_info(rate_id, child);
			size_t smooth_id          = info.smooth_id;
			size_t offset             = info.offset;
			const smooth_info& s_info = s_info_vec[smooth_id];

			log_prior_density_on_grid(
				logden      ,
				offset      ,
				pack_vec     ,
				age_table   ,
				time_table  ,
				prior_table ,
				s_info
			);
		}
	}

	// rate covariate multipliers
	for(size_t rate_id = 0; rate_id < number_rate_enum; rate_id++)
	{	size_t n_cov = pack_object.rate_mean_mulcov_n_cov( rate_id );
		for(size_t cov = 0; cov < n_cov; cov++)
		{	info = pack_object.rate_mean_mulcov_info(rate_id, cov);
			size_t smooth_id          = info.smooth_id;
			size_t offset             = info.offset;
			const smooth_info& s_info = s_info_vec[smooth_id];

			log_prior_density_on_grid(
				logden      ,
				offset      ,
				pack_vec     ,
				age_table   ,
				time_table  ,
				prior_table ,
				s_info
			);
		}
	}

	// measurement covariate multipliers
	size_t n_integrand = pack_object.integrand_size();
	for(size_t integrand_id = 0; integrand_id < n_integrand; integrand_id++)
	{	size_t n_cov = pack_object.meas_mean_mulcov_n_cov( integrand_id );
		for(size_t cov = 0; cov < n_cov; cov++)
		{	info = pack_object.meas_mean_mulcov_info(integrand_id, cov);
			size_t smooth_id          = info.smooth_id;
			size_t offset             = info.offset;
			const smooth_info& s_info = s_info_vec[smooth_id];

			log_prior_density_on_grid(
				logden      ,
				offset      ,
				pack_vec     ,
				age_table   ,
				time_table  ,
				prior_table ,
				s_info
			);
		}
		n_cov = pack_object.meas_std_mulcov_n_cov( integrand_id );
		for(size_t cov = 0; cov < n_cov; cov++)
		{	info = pack_object.meas_std_mulcov_info(integrand_id, cov);
			size_t smooth_id          = info.smooth_id;
			size_t offset             = info.offset;
			const smooth_info& s_info = s_info_vec[smooth_id];

			log_prior_density_on_grid(
				logden      ,
				offset      ,
				pack_vec     ,
				age_table   ,
				time_table  ,
				prior_table ,
				s_info
			);
		}
	}
	//
	return logden;
}

# define DISMOD_AT_INSTANTIATE_PRIOR_DENSITY(Float)          \
	template                                                 \
	prior_density_struct<Float> prior_density<Float>(        \
		const pack_info&                        pack_object    , \
		const CppAD::vector<Float>&            pack_vec     , \
		const CppAD::vector<double>&           age_table   , \
		const CppAD::vector<double>&           time_table  , \
		const CppAD::vector<prior_struct>&     prior_table , \
		const CppAD::vector<smooth_info>&      s_info_vec    \
	);

// instantiations
DISMOD_AT_INSTANTIATE_PRIOR_DENSITY(double)
DISMOD_AT_INSTANTIATE_PRIOR_DENSITY( CppAD::AD<double> )
DISMOD_AT_INSTANTIATE_PRIOR_DENSITY( CppAD::AD< CppAD::AD<double> > )

} // BEGIN_DISMOD_AT_NAMESPACE

