#include <qlopt.h>
#include <string>
#include <fstream>

//The namespace for the objects found in qlopt.h
using namespace thesis;

std::vector<std::string> split(const std::string &s, char delim);
mat getCsvData(std::string data);
mat rungekutta4(odefunction fhandle, const vec& t, const vec& u,
                const vec& y0, const vec& input);

// Construct the ODE system it should have the same structure/arguments
vec benchmark(const double& t, const vec& x, const vec& p, const vec& input)
{
	(void) t;
	double P = input(0);
	double S = input(1);

	vec result(8);
	result << p(1-1)/(1 + pow(P/p(2-1), p(3-1))   + pow(p(4-1)/S, p(5-1)))  - p(6-1)*x(1-1),
			  p(7-1)/(1 + pow(P/p(8-1), p(9-1))   + pow(p(10-1)/x(7-1), p(11-1))) - p(12-1)*x(2-1),
			  p(13-1)/(1 + pow(P/p(14-1),p(15-1)) + pow(p(16-1)/x(8-1), p(17-1))) - p(18-1)*x(3-1),
			  p(19-1)*x(1-1)/(p(20-1) + x(1-1)) - p(21-1)*x(4-1),
		 	  p(22-1)*x(2-1)/(p(23-1) + x(2-1)) - p(24-1)*x(5-1),
			  p(25-1)*x(3-1)/(p(26-1) + x(3-1)) - p(27-1)*x(6-1),
			  p(28-1)*x(4-1)*(S - x(7-1))/(p(29-1)*(1 + S/p(29-1) + x(7-1)/p(30-1))) - p(31-1)*x(5-1)*(x(7-1) - x(8-1))/(p(32-1)*(1 + x(7-1)/p(32-1) + x(8-1)/p(33-1))),
			  p(31-1)*x(5-1)*(x(7-1) - x(8-1))/(p(32-1)*(1 + x(7-1)/p(32-1) + x(8-1)/p(33-1))) -  p(34-1)*x(6-1)*(x(8-1) - P)/(p(35-1)*(1 + x(8-1)/p(35-1) + P/p(36-1)));

	return result;
}

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	/*
		Construct the input parameters for qlopt method. The struct has default
		values, but some values still need to be adjusted. All the elements of
		the struct will be displayed and some changed.
	*/
	inputStruct params;
	std::vector<outputStruct> results;
	//Tolerance parameter.
	params.tol.absparam = 1E-7; //Change optional, default value given
	params.tol.relparam = 1E-7; //Change optional, default value given
	params.tol.absobj = 1E-7; 	//Change optional, default value given
	params.tol.relobj = 1E-7; 	//Change optional, default value given
	params.tol.maxiter = 150; 	//Change optional, default value given

	//Data parameters.
	params.dat.spacing = "uniform";	//Options: "uniform", "nonuniform"
	params.dat.initialTime = 0.0; 	//Should be set to proper value
	params.dat.endTime = 4.0;		//Should be set to proper value
	params.dat.timeIncrement = .1;	//Should be set to proper value
	params.dat.numOfDataSets = 1;

	//Regularization parameters.
	params.reg.type = 5; 	// 0 - none,
  // 1 - Type 1,
  // 2 - Type 2
	params.reg.alpha = 1.0E-5;
	//Initial value problem parameters.
	params.ivp.solver = "mine";	// "mine", "boost_rk4", "cvodes", ...

	//General parameters.
	params.gen.numOfStates = 8;	//Should be set to proper value
	params.gen.numOfParams = 36;	//Should be set to proper value
	params.gen.divisions = 1;		//Change optional, default value given
	params.gen.finitediff = true;	//Change optional, default value given

	//Inputs and data
	std::vector<vec> input(16/*params.dat.numOfDataSets*/, vec::Zero(2));
	std::vector<mat> data(params.dat.numOfDataSets);
	vec t(21);
  vec t2;
  t2 = vec::LinSpaced(41,0.0,4.0);
  //cout << t2 << endl << endl;
  vec u(params.gen.numOfParams);
	vec u0(params.gen.numOfParams);
	vec y0(params.gen.numOfStates);

	t << 0,6,12,18,24,30,36,42,48,
		54,60,66,72,78,84,90,96,102,
		108,114,120;

	input[0](0) = 10; 		input[0](1) = 0.05;
	input[1](0) = 0.1; 		input[1](1) = 0.13572;
	input[2](0) = 0.1; 		input[2](1) = 0.3684;
	input[3](0) = 0.1; 		input[3](1) = 1.0;
	input[4](0) = 0.46416; 	input[4](1) = 0.05;
	input[5](0) = 0.46416; 	input[5](1) = 0.13572;
	input[6](0) = 0.46416; 	input[6](1) = 0.3684;
	input[7](0) = 0.46416; 	input[7](1) = 1.0;
	input[8](0) = 2.1544; 	input[8](1) = 0.05;
	input[9](0) = 2.1544; 	input[9](1) = 0.13572;
	input[10](0) = 2.1544; 	input[10](1) = 0.3684;
	input[11](0) = 2.1544; 	input[11](1) = 1.0;
	input[12](0) = 10; 		input[12](1) = 0.05;
	input[13](0) = 10; 		input[13](1) = 0.13572;
	input[14](0) = 10; 		input[14](1) = 0.3684;
	input[15](0) = 10; 		input[15](1) = 1.0;

  y0 << 6.6667e-1, 5.7254e-1, 4.1758e-1, 4.0e-1,
    3.6409e-1, 2.9457e-1, 1.419    , 9.3464e-1;

  //y0.fill(.5);

  u << 1.0,1.0,2.0,1.0,2.0,1.0,1.0,1.0,2.0,1.0,2.0,1.0,1.0,1.0,2.0,1.0,2.0,
		1.0,0.1,1.0,0.1,0.1,1.0,0.1,0.1,1.0,0.1,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0;

  u0.fill(2.5);
	//u0 << 1.0,1.0,2.0,1.0,2.0,1.0,1.0,1.0,2.0,1.0,2.0,1.0,1.0,1.0,2.0,1.0,2.0,
	//	1.0,0.1,1.0,0.1,0.1,1.0,0.1,0.1,1.0,0.1,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0;

	/**/
	for(size_t i = 0; i<params.dat.numOfDataSets; i++)
    {
      //data[i] = getCsvData("data/benchmark_" + std::to_string(i+1) + ".csv");
    
      data[i] = rungekutta4(benchmark, t2, u, y0, input[i]);
      //cout << data[i]<< endl<< endl;
    }

	/*
		This structure contains the many outputs of the method. Which the user
		can manipulate to get the desired graphics and numerical summaries.
	*/
	//qlopt(benchmark, t, u0, u, y0, input, data, params);
  results.push_back(qlopt(benchmark, t2, u0, u, y0, input, data, params));
  //cout << u.transpose() << endl;
	return 0;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> tokens;
    while (std::getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

mat getCsvData(std::string data)
{
	string filename = data;

	std::ifstream csvfile;
	std::string line;
	std::vector<std::string> dataAsStr, headers, dataRow;
	csvfile.open(filename);
	if(csvfile.is_open())
    {
      while(std::getline(csvfile, line))
        {
          dataAsStr.push_back(line);
        }
      csvfile.close();
    }
	else
    {
      std::cerr << "ERROR: Unable to open file";
    }

	headers = split(dataAsStr[0], ',');
	dataAsStr.erase(dataAsStr.begin());

	size_t n = headers.size();
	size_t lt = dataAsStr.size();
	mat featureData(n, lt);

	for(size_t i=0; i<lt; i++)
    {
      dataRow = split(dataAsStr[i], ',');
      for(size_t j = 0; j<n; j++)
        {
          featureData(j,i) = std::stod(dataRow[j]);
        }
    }

	return featureData;
}

mat rungekutta4(odefunction fhandle, const vec& t, const vec& u,
                const vec& y0, const vec& input)
{
  const unsigned int n = y0.size();
  const unsigned int N = t.size();
  double h = 0.0;

  mat w(n, N);
  w.fill(0.0);
	w.col(0) = y0;

	vec k1(n), k2(n), k3(n), k4(n);

	for (size_t i = 0; i<N-1; i++)
    {
      h = t(i+1)-t(i);

      k1 = h*fhandle(t(i), 		 w.col(i), 		  u, input);
      k2 = h*fhandle(t(i) + h/2, w.col(i) + k1/2, u, input);
      k3 = h*fhandle(t(i) + h/2, w.col(i) + k2/2, u, input);
      k4 = h*fhandle(t(i) + h, 	 w.col(i) + k3,   u, input);

      w.col(i+1) = w.col(i) + (k1 + 2*(k2 + k3) + k4)/6;
    }

	return w;
}
