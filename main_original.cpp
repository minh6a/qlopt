#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <Eigen/LU>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include "misc.h"
#include "nonlinear_odes.h"
#include "numerical_integration.h"
#include "thesis_functions.h"
#include "input.h"
#include "bspline.h"

using namespace thesis;

mat reshape(const mat& U, int n, int m);
void latexOutput(const mat& xn, const vec& u, int p, string buf);
double cond(const mat& A);
bool isNegative(const vec& x);
bool isLessThanOne(const vec& x);

double norm(const mat& M){
	return M.norm();
}

mat inverse(const mat& M){
	return M.inverse();
}

mat noise(const mat& M, double noise){
	double a = noise;
	mat oM = M;
	/*
	double N = 10000;
	for(int i=0; i<oM.rows(); i++){
		for(int j=0; j<oM.cols(); j++){
			oM(i,j) += a*sin(N*(i+1)*(j+1));
		}
	}*/
	
	const gsl_rng_type * T;
	gsl_rng * r;
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc (T);
	
	for(int i=0; i<oM.rows(); i++){
		for(int j=0; j<oM.cols(); j++){
			oM(i,j) += a*gsl_ran_ugaussian(r);
		}
	}
	gsl_rng_free (r);
	
	return oM;
}

int main(int argc, char *argv[])
{
	time_t begin;
	begin = time(NULL);
	
	nonlinearOdes no;
	input in(argc, argv);
	
	//Gather input from the console
	bool reg = in.isRegularized();
	bool noisy = in.isNoisy();
	string system = in.getSystem();

	vec times; 
	times = in.getTimeData(); //vec times(15); //times << 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28;
	
	vec u;
	u = in.getU();
	
	vec yNot;
	yNot = in.getYNot();
	
	vec uNot;
	uNot = in.getUNot();
	
	vec t;
	t = in.getInterval();

	//End input
	
	int lt = t.size();  
	int m = uNot.size();
	int n = yNot.size();
	
	mat measure;
	measure = rungekutta4(no.odeFuncMap[system], times, u, yNot);
	//mat measure(2,15);
	//measure << 20, 55, 65, 95, 55, 5, 15, 50, 75, 20, 25, 50, 70, 30, 15, 10, 15, 55, 60, 20, 15, 10, 60, 60, 10, 5, 25, 40, 25, 5;
	cout.precision(7);
	
	spline msmtRow[n];	
	bspline msmtRows(4, 500, lt);
	mat msmt(n,lt);
	
	if(noisy == true){
		latexOutput(measure.block(0, 0, n, lt), uNot, 100 , " & ");
		measure = noise(measure, in.getNoise());
		for(int i=0; i<n; i++){
			msmtRows.update(times, measure.row(i));
			for(int j = 0; j<lt; j++){
				msmt(i,j) = msmtRows.interpolate(t(j));
			}
		}
	}
	else{
		for(int i=0; i<n; i++){
			msmtRow[i].update(times, measure.row(i));
	
			for(int j = 0; j<lt; j++){
				msmt(i,j) = msmtRow[i].interpolate(t(j));
			}
		}
	}

	//refactor into a function		
	vec zeros(n*m); 
	zeros.fill(0);
	vec lyNot((m+1)*n);
	lyNot << msmt.col(0), zeros;// update where yNot gets its values
	//lyNot << yNot, zeros;// update where yNot gets its values
	//cout << "Ly_0 = " << lyNot << endl;
	
	mat xNminus(n, lt);
	xNminus << msmt; 

	mat bob(lyNot.size(), lt);
	mat U(zeros.size()/* n*m */, n*lt);
	mat A(m,m);
	mat I = mat::Identity(m, m);
	mat B = I;
	/*for(int i=0; i<m-1; i++){
		B(i+1,i) = -1;
	}*/
	vec P(m);
	vec du(m);
	
	int interval = (int)(lt/10+.5);
	if(interval == 0){
		interval = 1;
	}
	
	int l = 0;
	//cout << "N: &" << endl;
	cout << "time: &" << endl;
	while(l<lt){
		cout << t(l) << " & " << endl;
		l += interval;
	}
	cout << "time: &" << endl;
	l=0;
	while(l<lt){
		cout << t(l) << " & " << endl;
		l += interval;
	}
	cout << "\t &" << endl;
	for(int k=0; k<m; k++){
		cout << "$u_{" << k+1 << "}$ & " << endl;
	}
	
	//regs r;
	mat next(n, lt);
	mat last(n, lt);
	double gamma = 1.0;
	vec old_du;
	
	//Run the rest of the iterations
	latexOutput(xNminus, uNot, 0, " & "); 
	
	for(int i = 0; i<1700; i++)
	{
		bob = qLinearRungeKutta4(no.odeFuncMap[system], t, uNot, lyNot, xNminus);
			//bob = rungekutta4(no.qLinFuncMap[system + "_linearization"], t, uNot, lyNot, xNminus); 
			//cout << bob << endl << endl;
		
		U = reshape(bob.bottomRows(zeros.size()/* n*m */), m, n*lt);
		xNminus = bob.topRows(n);
			//cout << measure << endl << endl; cout << msmt << endl << endl; cout << xNminus << endl << endl;	
			//cout << "rel. err:\n" << norm(msmt - xNminus) << endl; //break;

		A = findA(t, U, m); cout << "cond(A) = " << cond(A) << "\tdet(A) = " << A.determinant() << "\trank(A) = " << A.fullPivHouseholderQr().rank() <<endl; //cout << A << endl;
		
		P = findP(t, U, reshape(msmt - xNminus, 1, n*lt).row(0), m); //cout << P << endl; //cout << "deltau\n" << A.inverse()*P << endl;
		old_du = du;
		if((reg) /*&& cond(A) > 1E+6) || isnan(cond(A))*/){
			// The simple solution
			do{
				gamma *= .5;
				du = inverse(A.transpose()*A + gamma*gamma*B.transpose()*B)*A.transpose()*P;
				//du = inverse(A + gamma*gamma*B.transpose()*B)*P;
			}while(norm(A*du-P) > 0.1);
				
		}else{
			du = A.inverse()*P;			//du = A.fullPivHouseholderQr().solve(P);  //A.lu().solve(P);
		}
		cout <<"du:\n" << du << endl;
		
		//while(isNegative(uNot + du))
		//	du *= .5;
		
		//while(isLessThanOne(uNot + du))
		//	du *= .5;
		
		//cout <<"nn_du:\n" << du << endl;
		
		uNot += du;
		
		latexOutput(xNminus, uNot, i+1, " & "); //cout << "n = " << i <<":\n" << xNminus << "\nParameter Estimates\n"<< uNot.transpose() << endl << endl;

		if(false){
			du(0) = norm(msmt - xNminus);
			if( du(0) < 0.0001 || isnan(du(0)) ){
				break;
			}
		}
		else{		
			du(0) = du.norm();
			if(du(0) < 0.00001 || isnan(du(0)) ){
				break;
			}
		}
	}
	latexOutput(msmt, u, -1, " \\\\ ");
	
	time_t end;
	end = time(NULL);
	cout << end - begin << endl;
	return 0;
}

mat reshape(const mat& U, int n, int m)
{
	mat newU(n,m);
	newU.fill(0);
	int olt = U.row(0).size(); 	//old time(row) length
	int on = m/olt;				//on col length
	
	for(int i = 0; i<n; i++){
		for(int j = 0; j<on; j++){
			newU.block(i, j*olt, 1, olt) = U.row(i*on + j);
		}
	}
	
	return newU;
}

void latexOutput(const mat& xn, const vec& u, int p, string buf){
	int n = xn.rows();
	int m = u.size();
	int N = xn.cols();
	
	int interval = (int)(N/10+.5);
	if(interval == 0){
		interval = 1;
	}
	int j;
	for(int i=0; i<n; i++){
		j = 0;
		cout << "$\\vec{x}_{"<< p <<"}(t,u)$ " << buf << endl;
		while(j<N){
			cout << xn(i, j) << buf << endl;
			j+=interval;
		}
	}
	cout << "\t "<< buf << endl;
	for(int k=0; k<m; k++){
		cout << u(k) << buf << endl;
	}
}

double cond(const mat& A){
	return A.norm()*A.inverse().norm();
}

bool isNegative(const vec& x){
	bool value = false;
	for(int i=0; i < x.size(); i++){
		if(x(i) < 0){
			value = true;
			break;
		}
	}
	return value;
}

bool isLessThanOne(const vec& x){
	bool value = false;
	for(int i=0; i < x.size()-1; i++){
		if(x(i) < .9){
			value = true;
			break;
		}
	}
	if(x(x.size()-1) < 1)
			value = true;
			
	return value;
}