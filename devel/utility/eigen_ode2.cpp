// $Id$
/* --------------------------------------------------------------------------
dismod_at: Estimating Disease Rates as Functions of Age and Time
          Copyright (C) 2014-15 University of Washington
             (Bradley M. Bell bradbell@uw.edu)

This program is distributed under the terms of the
	     GNU Affero General Public License version 3.0 or later
see http://www.gnu.org/licenses/agpl.txt
-------------------------------------------------------------------------- */
/*
$begin eigen_ode2$$
$spell
	tf
	eigen
	const
	yi
	yf
	cpp
$$

$section Eigen Vector Solution of ODE with Two Components$$

$head Syntax$$
$icode%yf% = eigen_ode2(%b%, %yi%, %tf%)
%$$

$head Purpose$$
Given $latex y( 0 ) \in \B{R}^2$$,
$latex B \in \B{R}^{2 \times 2}$$,
and $latex t_f \in \B{R}_+$$,
this routine uses the eigen vectors of $latex B$$ to solve for
$latex y( t_f )$$ where
$latex \[
	y' (t) = B y(t)
\]$$
Furthermore, the CppAD floating point operation sequence
does not depend on the input values.

$head Float$$
The type $icode Float$$ must be one of the following:
$code double$$, $code AD<double>$$, or $cref a2_double$$.


$head b$$
This argument has prototype
$codei%
	const CppAD::vector<%Float%>& %b%
%$$
and size four.
It specifies the matrix $latex B$$
in row-major order; i.e.,
$latex \[
B
=
\left( \begin{array}{cc}
	B_{0,0}  & B_{0,1} \\
	B_{1,0}  & B_{1,1}
\end{array} \right)
=
\left( \begin{array}{cc}
	b_0  & b_1 \\
	b_2  & b_3
\end{array} \right)
\] $$

$head Assumption$$
We restrict our attention to the case where the off-diagonal elements
of $latex  B$$ have the same sign; i.e.
$latex \[
	B_{0,1} B_{1,0} = b_1 b_2 \geq 0
\] $$

$head yi$$
This argument has prototype
$codei%
	const CppAD::vector<%Float%>& %yi%
%$$
and size two.
It specifies the vector $latex y( 0 )$$, to be specific,
$pre
	$$ $latex y_0 ( 0 ) =$$ $icode%yi%[0]%$$
$pre
	$$ $latex y_1 ( 0 ) =$$ $icode%yi%[1]%$$.

$head tf$$
This argument has prototype
$codei%
	const %Float%& %tf%
%$$
It specifies the final time; i.e. $latex t_f$$.

$head yf$$
The return value has prototype
$codei%
	CppAD::vector<%Float%>& %yf%
%$$
and size two and contains the solution of the ODE; i.e.,
$codei%
	%yf%[0]%$$ $latex = y_0 ( t_f )$$
$codei%
	%yf%[1]%$$ $latex = y_1 ( t_f )$$.

$head Method$$

$subhead Case One$$
The case where $latex b_1 = 0$$ and $latex b_2 = 0$$
has the following solution
$latex \[
\begin{array}{rcl}
	y_0 ( t_f ) & = & y_0 (0) \exp( b_0 t )
	\\
	y_1 ( t_f ) & = & y_1 (0) \exp( b_3 t )
\end{array}
\]$$

$subhead Case Two$$
The case $latex b_1 = 0$$ and $latex b_2 \neq 0$$
has the following solution
$latex \[
\begin{array}{rcl}
y_0 ( t )   & = & y_0 ( 0 ) \exp ( b_0 t )
\\
y_1 ' ( t ) & = & b_3 y_1 (t) + b_2 y_0 ( t )
\\
y_1 (t)     & = & y_1 ( 0 ) \exp ( b_3 t ) +
	\int_0^t \exp [ b_3 ( t - s ) ] b_2 y_0 (s) \B{d} s
\\
y_1 (t)     & = & y_1 ( 0 ) \exp ( b_3 t ) +
	b_2 y_0 ( 0 ) \int_0^t \exp [ b_3 ( t - s ) + b_0 s  ] \B{d} s
\\
y_1 (t)     & = & y_1 ( 0 ) \exp ( b_3 t ) +
	b_2 y_0 ( 0 ) \exp ( b_3 t ) \int_0^t \exp [ ( b_0 - b_3 ) s ] \B{d} s
\end{array}
\] $$

$subhead Case Three$$
The case $latex b_1 \neq 0$$ and $latex b_2 = 0$$
can be converted to case two by switching the rows and columns
of the matrix and the elements of the vectors.

$subhead Case Four$$
The case where $latex b_1 \neq 0$$ and $latex b_2 \neq 0$$
has the following solution
$latex \[
	(b_0 - b_3)^2 + b_1 b_2 > 0
\] $$
A value $latex \lambda$$
is an eigen value of $latex B$$ if and only if:
$latex \[
\begin{array}{rcl}
0 & = & ( b_0 - \lambda ) ( b_3 - \lambda )  - b_1 b_2
\\
0 & = & \lambda^2 - (b_0 + b_3) \lambda + b_0 b_3 - b_1 b_2
\\
\lambda & = &  \frac{
	(b_0 + b_3) \pm \sqrt{ (b_0 + b_3)^2 - 4 b_0 b_3 + 4 b_1 b_2 }
}{2}
\\
\lambda & = &  \frac{
	(b_0 + b_3) \pm \sqrt{ (b_0 - b_3)^2 + 4 b_1 b_2 }
}{2}
\end{array}
\] $$
We can express an arbitrary left eigenvector of $latex B$$
as $latex (1, u)$$ where $latex u \in \B{R}$$,
$latex \[
\begin{array}{rcl}
	\lambda & = & b_0  + b_2 u
	\\
	\lambda u & = & b_1  + b_3 u
\end{array}
\] $$
and $latex \lambda$$ is an eigenvalue of $latex B$$.
Using the first equation to solve for $latex u$$, we have
$latex \[
\lambda \; [ 1 \; , \; ( \lambda - b_0 ) / b_2  ]
=
[ 1 \; , \; ( \lambda - b_0 ) / b_2  ] \; B
\] $$
where
$latex \[
\lambda =  \frac{
	(b_0 + b_3) \pm \sqrt{ (b_0 - b_3)^2 + 4 b_1 b_2 }
}{2}
\] $$
Define $latex \lambda_+$$ and $latex \lambda_-$$ as corresponding
to the plus and minus the square root above and note
$latex \lambda_+ \neq \lambda_-$$ because $latex b_1 b_2 > 0$$.
We also define
$latex \[
\begin{array}{rcl}
	u_\pm & = & ( \lambda_\pm - b_0 ) / b_2
	\\
	z_\pm (t) & = & y_0 (t) + u_\pm y_1 (t)
\end{array}
\] $$
It follows that
$latex \[
\begin{array}{rcl}
z_\pm (t) & = &  (1, u_\pm ) y(t)
\\
z_\pm ' (t)
& = &  (1, u_\pm ) y'(t) =  (1, u_\pm ) B y (t) = \lambda_\pm z(t)
\\
z_\pm (t) & = & z_\pm (0) \exp( \lambda_\pm t )
\end{array}
\] $$
Which enables us to compute $latex z_\pm (t)$$.
Furthermore
$latex \[
\begin{array}{rcl}
y_1 (t) & = & [ z_+ (t) - z_- (t) ] / ( u_+ - u_- )
\\
& = & [ z_+ (t) - z_- (t) ] b_2 / ( \lambda_+ - \lambda_- )
\\
& = & [ z_+ (t) - z_- (t) ] b_2 / \sqrt{ (b_0 - b_3)^2 + 4 b_1 b_2 }
\\
y_0(t) & = & z_+ (t) - u_+ y_1 (t)
\end{array}
\] $$

$children%
	example/devel/utility/eigen_ode2_xam.cpp
%$$
$head Example$$
The file $cref eigen_ode2_xam.cpp$$ contains
and example and test of $code eigen_ode2$$.
It returns true for success and false for failure.

$end
---------------------------------------------------------------------------
*/
# include <cppad/cppad.hpp>
# include <dismod_at/eigen_ode2.hpp>
# include <dismod_at/a2_double.hpp>

namespace dismod_at { // BEGIN DISMOD_AT_NAMESPACE

namespace {
	// solution corresponding to b1 = 0, b2 = 0
	template <class Float>
	CppAD::vector<Float> both_zero(
		const CppAD::vector<Float>&  b           ,
		const CppAD::vector<Float>&  yi          ,
		const Float&                 tf          )
	{	using CppAD::exp;
		CppAD::vector<Float> yf(2);

		yf[0] = yi[0] * exp( b[0] * tf );
		yf[1] = yi[1] * exp( b[3] * tf );

		return yf;
	}
	// solution corresponding to b1 = 0 , b2 != 0
	template <class Float>
	CppAD::vector<Float> b1_zero(
		const CppAD::vector<Float>&  b           ,
		const CppAD::vector<Float>&  yi          ,
		const Float&                 tf          )
	{	using CppAD::exp;
		CppAD::vector<Float> yf(2);
		Float eps = Float(std::sqrt( std::numeric_limits<double>::epsilon()));
		Float diff_03 = b[0] - b[3];
		//
		// y_0 ( tf )
		yf[0] = yi[0] * exp( b[0] * tf );
		//
		// exp[ (b0 - b3) * tf ] / (b0 - b3);
		Float term = expm1( diff_03 * tf ) / diff_03;
		term = CppAD::CondExpLt(abs(diff_03), eps, tf, term);
		//
		// y_1 ( tf )
		yf[1] = exp( b[3] * tf ) * ( yi[1] + b[2] * yi[0] * term );
		//
		return yf;
	}
	// solution corresponding to b1 != 0 , b2 == 0
	template <class Float>
	CppAD::vector<Float> b2_zero(
		const CppAD::vector<Float>&  b           ,
		const CppAD::vector<Float>&  yi          ,
		const Float&                 tf          )
	{	using CppAD::exp;
		CppAD::vector<Float> yf(2);
		Float eps = Float(std::sqrt( std::numeric_limits<double>::epsilon()));
		Float diff_30 = b[3] - b[0];
		//
		// y_1 ( tf )
		yf[1] = yi[1] * exp( b[3] * tf );
		//
		// exp[ (b3 - b0) * tf ] / (b3 - b0);
		Float term = expm1( diff_30 * tf ) / diff_30;
		term = CppAD::CondExpLt(abs(diff_30), eps, tf, term);
		//
		// y_0 ( tf )
		yf[0] = exp( b[0] * tf ) * ( yi[0] + b[1] * yi[1] * term );
		//
		return yf;
	}
	// solution corresponding to b1 != 0, b2 != 0
	template <class Float>
	CppAD::vector<Float> both_nonzero(
		const CppAD::vector<Float>&  b           ,
		const CppAD::vector<Float>&  yi          ,
		const Float&                 tf          )
	{	using CppAD::exp;
		CppAD::vector<Float> yf(2);
		// discriminant in the quadratic equation for eigen-values
		Float disc = (b[0] - b[3])*(b[0] - b[3]) + 4.0*b[1]*b[2];
		Float root_disc = Float( CppAD::sqrt( disc ));

		// use eigen vectors
		Float lambda_p  = (b[0] + b[3] + root_disc) / 2.0;
		Float lambda_m  = (b[0] + b[3] - root_disc) / 2.0;
		//
		Float u_p       = (lambda_p - b[0]) / b[2];
		Float u_m       = (lambda_m - b[0]) / b[2];
		//
		Float zi_p      = yi[0] + u_p * yi[1];
		Float zi_m      = yi[0] + u_m * yi[1];
		//
		Float zf_p      = zi_p * exp( lambda_p * tf );
		Float zf_m      = zi_m * exp( lambda_m * tf );
		//
		yf[1]           = (zf_p - zf_m) * b[2] / root_disc;
		yf[0]           = zf_p - u_p * yf[1];
		//
		return yf;
	}
}

template <class Float>
CppAD::vector<Float> eigen_ode2(
	const CppAD::vector<Float>&  b           ,
	const CppAD::vector<Float>&  yi          ,
	const Float&                 tf          )
{	using CppAD::abs;
	using CppAD::vector;
	assert( b.size() == 4 );
	assert( yi.size() == 2 );
	vector<Float>abs_b(4),
		both_zero_y(2), b2_zero_y(2), b1_zero_y(2), both_nonzero_y(2), yf(2);
	//
	// square root of machine epsilon
	Float eps = Float( std::sqrt( std::numeric_limits<double>::epsilon() ) );
	//
	//
	Float norm = Float(0.0);
	for(size_t i = 0; i < b.size(); i++)
	{	abs_b[i] = abs(b[i]);
		norm    += abs_b[i];
	}
	//
	// solution corresponding to b1 = 0, b2 = 0
	both_zero_y = both_zero(b, yi, tf);
	// solution corresponding to b1 != 0, b2 = 0
	b2_zero_y = b2_zero(b, yi, tf);
	// solution corresponding to b1 = 0, b2 != 0
	b1_zero_y = b1_zero(b, yi, tf);
	// solution corresponding to b1 ! 0, b2 != 0
	both_nonzero_y = both_nonzero(b, yi, tf);
	//
	Float eps_norm    = eps * norm;
	Float diff_abs_12 = abs(abs_b[1] - abs_b[2]);
	Float min_abs_12 = (abs_b[1] + abs_b[2] - diff_abs_12) / 2.0;
	Float max_abs_12 = (abs_b[1] + abs_b[2] + diff_abs_12) / 2.0;
	for(size_t i = 0; i < yi.size(); i++)
	{	// if |b1| < |b2|, b1_zero, else b2_zero
		Float b1_or_b2_zero_yi = CppAD::CondExpLt(
			abs_b[1], abs_b[2], b1_zero_y[i], b2_zero_y[i]
		);
		// if max(|b1|,|b2|) < eps_norm, both_zero, else b1_or_b2_zero
		Float one_or_both_zero_yi = CppAD::CondExpLt(
			max_abs_12, eps_norm, both_zero_y[i], b1_or_b2_zero_yi
		);
		// if min(|b1|,|b2|) > eps_norm, both_nonzero, else, one_or_both_zero
		yf[i] = CppAD::CondExpGt(
			min_abs_12, eps_norm, both_nonzero_y[i], one_or_both_zero_yi
		);
	}
	return yf;
}

// instantiation macro
# define DISMOD_AT_INSTANTIATE_EIGEN_ODE2(Float)       \
	template CppAD::vector<Float> eigen_ode2<Float>(   \
		const CppAD::vector<Float>&  b           ,     \
		const CppAD::vector<Float>&  yi          ,     \
		const Float&                 tf                \
	);

// instantiations
DISMOD_AT_INSTANTIATE_EIGEN_ODE2(double)
DISMOD_AT_INSTANTIATE_EIGEN_ODE2( CppAD::AD<double> )
DISMOD_AT_INSTANTIATE_EIGEN_ODE2( a2_double )

} // END DISMOD_AT_NAMESPACE
