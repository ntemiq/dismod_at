// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-17 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */

# include <limits>
# include <cmath>
# include <dismod_at/residual_density.hpp>

namespace {
	bool check(
		const dismod_at::residual_struct<double>& residual       ,
		double                                    wres           ,
		double                                    smooth         ,
		double                                    sub_abs        ,
		dismod_at::density_enum                   d_id           ,
		size_t                                    index          ,
		bool                                      difference     )
	{	bool ok    = true;
		double eps = 100. * std::numeric_limits<double>::epsilon();
		ok &= fabs(1.0 - residual.wres / wres) <= eps;
		ok &= fabs(1.0 - residual.logden_smooth / smooth) <= eps;
		if( sub_abs == 0.0 )
			ok &= residual.logden_sub_abs == 0.0;
		else
			ok &= fabs(1.0 - residual.logden_sub_abs / sub_abs) <= eps;
		ok &= residual.density == d_id;
		ok &= residual.index   == index;
		return ok;
	}
}

bool residual_density_xam(void)
{	bool ok    = true;
	double nan = std::numeric_limits<double>::quiet_NaN();
	double pi  = 4.0 * std::atan(1.0);

	using std::sqrt;
	using std::fabs;
	dismod_at::residual_struct<double> residual;
	double wres, smooth, sub_abs, sigma;
	size_t                  index;

	dismod_at::density_enum d_id;
	double z          = 3.0;
	double y          = 2.5;
	double mu         = 2.0;
	double delta      = 1.5;
	double d_eta      = nan;
	double d_nu       = nan;

	// -----------------------------------------------------------------------
	bool   difference = true;
	//
	// uniform
	d_id        = dismod_at::uniform_enum;
	index       = 1;
	residual    = residual_density(
		z, y, mu, delta, d_id, d_eta, d_nu, index, difference
	);
	ok         &= residual.wres           == 0.0;
	ok         &= residual.logden_smooth  == 0.0;
	ok         &= residual.logden_sub_abs == 0.0;
	ok         &= residual.density        == d_id;
	ok         &= residual.index          == index;

	// gaussian
	d_id        = dismod_at::gaussian_enum;
	residual    = residual_density(
		z, y, mu, delta, d_id, d_eta, d_nu, ++index, difference
	);
	wres        = (z - y - mu) / delta;
	smooth      = - log(delta * sqrt(2.0 * pi) ) - wres * wres / 2.0;
	sub_abs     = 0.0;
	ok         &= check(
		residual, wres, smooth, sub_abs, d_id, index, difference
	);

	// laplace
	d_id        = dismod_at::laplace_enum;
	residual    = residual_density(
		z, y, mu, delta, d_id, d_eta, d_nu, ++index, difference
	);
	wres        = (z - y - mu) / delta;
	smooth      = - log(delta * sqrt(2.0) );
	sub_abs     =  sqrt(2.0) * wres;
	ok         &= check(
		residual, wres, smooth, sub_abs, d_id, index, difference
	);

	// log-gaussian
	d_id        = dismod_at::log_gaussian_enum;
	d_eta       = 0.5;
	residual    = residual_density(
		z, y, mu, delta, d_id, d_eta, d_nu, ++index, difference
	);
	sigma       = delta;
	wres        = ( log(z + d_eta) - log(y + d_eta) - mu ) / sigma;
	smooth      = - log(sigma * sqrt(2.0 * pi) ) - wres * wres / 2.0;
	sub_abs     = 0.0;
	ok         &= check(
		residual, wres, smooth, sub_abs, d_id, index, difference
	);

	// log-laplace
	d_id        = dismod_at::log_laplace_enum;
	d_eta       = 3.0;
	residual    = residual_density(
		z, y, mu, delta, d_id, d_eta, d_nu, ++index, difference
	);
	sigma       = delta;
	wres        = ( log(z + d_eta) - log(y + d_eta) - mu) / sigma;
	smooth      = - log(sigma * sqrt(2.0) );
	sub_abs     = sqrt(2.0) * wres;
	ok         &= check(
		residual, wres, smooth, sub_abs, d_id, index, difference
	);

	// -----------------------------------------------------------------------
	difference = false;
	//
	// uniform
	d_id        = dismod_at::uniform_enum;
	residual    = residual_density(
		z, y, mu, delta, d_id, d_eta, d_nu, ++index, difference
	);
	ok         &= residual.wres           == 0.0;
	ok         &= residual.logden_smooth  == 0.0;
	ok         &= residual.logden_sub_abs == 0.0;
	ok         &= residual.density        == d_id;
	ok         &= residual.index          == index;

	// gaussian
	d_id        = dismod_at::gaussian_enum;
	residual    = residual_density(
		z, y, mu, delta, d_id, d_eta, d_nu, ++index, difference
	);
	wres        = (y - mu) / delta;
	smooth      = - log(delta * sqrt(2.0 * pi) ) - wres * wres / 2.0;
	sub_abs     = 0.0;
	ok         &= check(
		residual, wres, smooth, sub_abs, d_id, index, difference
	);

	// laplace
	d_id        = dismod_at::laplace_enum;
	residual    = residual_density(
		z, y, mu, delta, d_id, d_eta, d_nu, ++index, difference
	);
	wres        = (y - mu) / delta;
	smooth      = - log(delta * sqrt(2.0) );
	sub_abs     =  sqrt(2.0) * wres;
	ok         &= check(
		residual, wres, smooth, sub_abs, d_id, index, difference
	);

	// log-gaussian
	d_id        = dismod_at::log_gaussian_enum;
	d_eta       = 0.5;
	residual    = residual_density(
		z, y, mu, delta, d_id, d_eta, d_nu, ++index, difference
	);
	sigma       = log(mu + d_eta + delta) - log(mu + d_eta);
	wres        = ( log(y + d_eta) - log(mu + d_eta) ) / sigma;
	smooth      = - log(sigma * sqrt(2.0 * pi) ) - wres * wres / 2.0;
	sub_abs     = 0.0;
	ok         &= check(
		residual, wres, smooth, sub_abs, d_id, index, difference
	);

	// log-laplace
	d_id        = dismod_at::log_laplace_enum;
	d_eta       = 3.0;
	residual    = residual_density(
		z, y, mu, delta, d_id, d_eta, d_nu, ++index, difference
	);
	sigma       = log(mu + d_eta + delta) - log(mu + d_eta);
	wres        = ( log(y + d_eta) - log(mu + d_eta) ) / sigma;
	smooth      = - log(sigma * sqrt(2.0) );
	sub_abs     = sqrt(2.0) * wres;
	ok         &= check(
		residual, wres, smooth, sub_abs, d_id, index, difference
	);

	return ok;
}
