// load Rcpp
#include <Rcpp.h>
using namespace Rcpp;

NumericVector assign_reproductive_intensity_random (int num_ind, double mean, double sigma) ;
IntegerVector assign_death_randomly_by_age_class(NumericVector mortality, IntegerMatrix demo) ;
IntegerMatrix recruit_to_carrying_capacity (IntegerMatrix demo, IntegerVector mortality_individuals, IntegerVector K) ;
IntegerVector assign_mothers_of_recruits (IntegerMatrix demo, IntegerMatrix recruit_demo, NumericVector reproductive_intensity, NumericMatrix dispersal) ;      
IntegerVector assign_fathers_of_recruits (IntegerMatrix demo, IntegerMatrix recruit_demo, NumericVector reproductive_intensity, NumericMatrix dispersal, IntegerVector mother_index, double selfing) ;
IntegerMatrix assign_alleles_to_recruits (IntegerMatrix gt, IntegerVector mother_index, IntegerVector father_index) ;
IntegerMatrix update_demo(IntegerMatrix demo, IntegerVector mortality_individuals, IntegerMatrix recruit_demo) ;
IntegerMatrix update_gt(IntegerMatrix gt, IntegerVector mortality_individuals, IntegerMatrix recruit_gt);
NumericVector assign_phenotype_quantitative(IntegerMatrix gt, NumericVector locus_effect, NumericVector dominance_effect);
NumericVector assign_reproductive_intensity_phenotype(IntegerMatrix demo, NumericVector phenotype_value, NumericVector phenotype_opt, double n_p, double n_k);
IntegerMatrix mutate_genotypes(IntegerMatrix gt, double mutation);
