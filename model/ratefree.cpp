/*
 * ratefree.cpp
 *
 *  Created on: Nov 3, 2014
 *      Author: minh
 */

#include "phylotree.h"
#include "ratefree.h"


const double MIN_FREE_RATE = 0.0001;
const double MAX_FREE_RATE = 0.9999;
const double TOL_FREE_RATE = 0.0001;

// Modified by Thomas on 13 May 2015
//const double MIN_FREE_RATE_PROP = 0.0001;
//const double MAX_FREE_RATE_PROP = 0.9999;
const double MIN_FREE_RATE_PROP = 0.001;
const double MAX_FREE_RATE_PROP = 1000;

RateFree::RateFree(int ncat, double start_alpha, string params, bool sorted_rates, PhyloTree *tree) : RateGamma(ncat, start_alpha, false, tree) {
	fix_params = false;
	prop = NULL;
    this->sorted_rates = sorted_rates;
	setNCategory(ncat);

	if (params.empty()) return;
	DoubleVector params_vec;
	try {
		convert_double_vec(params.c_str(), params_vec);
		if (params_vec.size() != ncategory*2)
			outError("Number of parameters for FreeRate model must be twice number of categories");
		int i;
		double sum, sum_prop;
		for (i = 0, sum = 0.0, sum_prop = 0.0; i < ncategory; i++) {
			prop[i] = params_vec[i*2];
			rates[i] = params_vec[i*2+1];
			sum += prop[i]*rates[i];
			sum_prop += prop[i];
		}
		if (fabs(sum_prop-1.0) > 1e-5)
			outError("Sum of category proportions not equal to 1");
		for (i = 0; i < ncategory; i++)
			rates[i] /= sum;
		fix_params = true;
	} catch (string &str) {
		outError(str);
	}
}

void RateFree::setNCategory(int ncat) {

    // initialize with gamma rates
    RateGamma::setNCategory(ncat);
	if (prop) delete [] prop;
	prop  = new double[ncategory];

    int i;
	for (i = 0; i < ncategory; i++)
        prop[i] = 1.0/ncategory;
    
//	double sum_prop = (ncategory)*(ncategory+1)/2.0;
//	double sum = 0.0;
//	int i;
	// initialize rates as increasing
//	for (i = 0; i < ncategory; i++) {
//		prop[i] = (double)(ncategory-i) / sum_prop;
//        prop[i] = 1.0 / ncategory;
//		rates[i] = (double)(i+1);
//		sum += prop[i]*rates[i];
//	}
//	for (i = 0; i < ncategory; i++)
//		rates[i] /= sum;

	name = "+R";
	name += convertIntToString(ncategory);
	full_name = "FreeRate";
	full_name += " with " + convertIntToString(ncategory) + " categories";
}


RateFree::~RateFree() {
	if (prop) delete [] prop;
	prop = NULL;
}

string RateFree::getNameParams() {
	stringstream str;
	str << "+R" << ncategory << "{";
	for (int i = 0; i < ncategory; i++) {
		if (i > 0) str << ",";
		str << prop[i]<< "," << rates[i];
	}
	str << "}";
	return str.str();
}

double RateFree::targetFunk(double x[]) {
	getVariables(x);
	phylo_tree->clearAllPartialLH();
	return -phylo_tree->computeLikelihood();
}



/**
	optimize parameters. Default is to optimize gamma shape
	@return the best likelihood
*/
double RateFree::optimizeParameters(double gradient_epsilon) {

	int ndim = getNDim();

	// return if nothing to be optimized
	if (ndim == 0)
		return phylo_tree->computeLikelihood();

	if (verbose_mode >= VB_MAX)
		cout << "Optimizing " << name << " model parameters by BFGS..." << endl;

	//if (freq_type == FREQ_ESTIMATE) scaleStateFreq(false);

	double *variables = new double[ndim+1];
	double *upper_bound = new double[ndim+1];
	double *lower_bound = new double[ndim+1];
	bool *bound_check = new bool[ndim+1];
	double score;

	// by BFGS algorithm
	setVariables(variables);
	setBounds(lower_bound, upper_bound, bound_check);

	score = -minimizeMultiDimen(variables, ndim, lower_bound, upper_bound, bound_check, max(gradient_epsilon, TOL_FREE_RATE));

	getVariables(variables);
    
    // sort the rates in increasing order
    if (sorted_rates)
        quicksort<double,double>(rates, 0, ncategory-1, prop);

	phylo_tree->clearAllPartialLH();

	delete [] bound_check;
	delete [] lower_bound;
	delete [] upper_bound;
	delete [] variables;

	return score;
}

void RateFree::setBounds(double *lower_bound, double *upper_bound, bool *bound_check) {
	if (getNDim() == 0) return;
	int i;
	for (i = 1; i < ncategory; i++) {
		lower_bound[i] = MIN_FREE_RATE_PROP;
		upper_bound[i] = MAX_FREE_RATE_PROP;
		bound_check[i] = false;
	}
	for (i = ncategory; i <= 2*ncategory-2; i++) {
		lower_bound[i] = MIN_FREE_RATE;
		upper_bound[i] = MAX_FREE_RATE;
		bound_check[i] = false;
	}
}


void RateFree::setVariables(double *variables) {
	if (getNDim() == 0) return;
	int i;

	// Modified by Thomas on 13 May 2015
	// --start--
	/*
	variables[1] = prop[0];
	for (i = 2; i < ncategory; i++)
		variables[i] = variables[i-1] + prop[i-1];
	*/
	for (i = 0; i < ncategory-1; i++)
		variables[i+1] = prop[i] / prop[ncategory-1];
	// --end--

	for (i = 0; i < ncategory-1; i++)
		variables[i+ncategory] = rates[i] / rates[ncategory-1];
}

void RateFree::getVariables(double *variables) {
	if (getNDim() == 0) return;
	int i;

	// Modified by Thomas on 13 May 2015
	// --start--
	/*
	double *y = new double[2*ncategory+1];
	double *z = y+ncategory+1;
	//  site proportions: y[0..c] <-> (0.0, variables[1..c-1], 1.0)
	y[0] = 0; y[ncategory] = 1.0;
	memcpy(y+1, variables+1, (ncategory-1) * sizeof(double));
	std::sort(y+1, y+ncategory);

	// category rates: z[0..c-1] <-> (variables[c..2*c-2], 1.0)
	memcpy(z, variables+ncategory, (ncategory-1) * sizeof(double));
	z[ncategory-1] = 1.0;
	//std::sort(z, z+ncategory-1);

	double sum = 0.0;
	for (i = 0; i < ncategory; i++) {
		prop[i] = (y[i+1]-y[i]);
		sum += prop[i] * z[i];
	}
	for (i = 0; i < ncategory; i++) {
		rates[i] = z[i] / sum;
	}

	delete [] y;
	*/

	double sum = 1.0;
	for (i = 0; i < ncategory-1; i++) {
		sum += variables[i+1];
	}
	for (i = 0; i < ncategory-1; i++) {
		prop[i] = variables[i+1] / sum;
	}
	prop[ncategory-1] = 1.0 / sum;

	sum = prop[ncategory-1];
	for (i = 0; i < ncategory-1; i++) {
		sum += prop[i] * variables[i+ncategory];
	}
	for (i = 0; i < ncategory-1; i++) {
		rates[i] = variables[i+ncategory] / sum;
	}
	rates[ncategory-1] = 1.0 / sum;
	// --end--

}

/**
	write information
	@param out output stream
*/
void RateFree::writeInfo(ostream &out) {
	out << "Site proportion and rates: ";
	for (int i = 0; i < ncategory; i++)
		out << " (" << prop[i] << "," << rates[i] << ")";
	out << endl;
}

/**
	write parameters, used with modeltest
	@param out output stream
*/
void RateFree::writeParameters(ostream &out) {
	for (int i = 0; i < ncategory; i++)
		out << "\t" << prop[i] << "\t" << rates[i];

}

