#include <least_squares.h>

namespace thesis{
	lsquares::lsquares(const size_t n, const size_t p)
	{
		init(n, p);
	}
			
	void lsquares::init(const size_t n, const size_t p)
	{
		num_obs = n;
		order = p;
		mws = gsl_multifit_linear_alloc(num_obs, order);
		X = gsl_matrix_alloc(num_obs, order);
		gslx = gsl_vector_alloc(num_obs);
		gsly = gsl_vector_alloc(num_obs);
		c = gsl_vector_alloc(order);
		cov = gsl_matrix_alloc(order, order);
	}

	lsquares::~lsquares()
	{
		gsl_multifit_linear_free(mws);
		gsl_matrix_free(X);
		gsl_matrix_free(cov);
		gsl_vector_free(gslx);
		gsl_vector_free(gsly);
		gsl_vector_free(c);
	}
	
	void lsquares::update(const vec& x, const vec& y)
	{
		double chisq;
		vecToGslVec(x, gslx);
		vecToGslVec(y, gsly);
		generateX(X,gslx);
		gsl_multifit_linear(X, gsly, c, cov, &chisq, mws);
	}
	
	void lsquares::vecToGslVec(const vec& v, gsl_vector* gslv)
	{	
		for (size_t i=0; i<gslv->size; i++){
			gsl_vector_set(gslv, i, v(i));
		}
	}

	void lsquares::generateX(gsl_matrix *M, gsl_vector* gslv)
	{
		gsl_vector *temp = gsl_vector_alloc(order);
		for (size_t i=0; i<M->size1; i++){
			generate_xi(gsl_vector_get(gslv, i), temp);
			gsl_matrix_set_row(M, i, temp);
		}
	}
	
	void lsquares::generate_xi(double xi, gsl_vector *gslv)
	{
		for (size_t i=0; i<order; i++){
			gsl_vector_set(gslv, i, pow(xi,i));
		}
	}
	
	double lsquares::interpolate(double xi)
	{
		double yi, yerr;
		gsl_vector *temp = gsl_vector_alloc(order);
		generate_xi(xi, temp);
		gsl_multifit_linear_est(temp, c, cov, &yi, &yerr);
		
		return yi;
	}
}