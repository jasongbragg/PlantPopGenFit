// load Rcpp
#include <Rcpp.h>
#include "simulation_functions.h"

using namespace Rcpp;

// [[Rcpp::export]]
List simulation(int tsteps, NumericVector mortality, IntegerMatrix demo, IntegerMatrix gt, IntegerVector K, NumericMatrix dispersal, double selfing, NumericVector locus_effect, NumericVector dominance_effect, NumericVector phenotype_opt, double n_p, double n_k, double mutation) {

   int num_ind = demo.nrow();
   int num_loc = gt.ncol();

   int t = 0;

   // main loop
   while (t < tsteps) {

      // Apply mutations to genotypes
      gt   = mutate_genotypes(gt, mutation);

      // Calculate phenotype values
      NumericVector phenotype_value        = assign_phenotype_quantitative(gt, locus_effect, dominance_effect);
      //Rf_PrintValue( phenotype_value  );

      // Reproductive Intensity for each individual 
      // NumericVector reproductive_intensity = assign_reproductive_intensity_random( num_ind, 1, 0.25 );
      NumericVector reproductive_intensity = assign_reproductive_intensity_phenotype(demo, phenotype_value, phenotype_opt, n_p, n_k);
      //Rf_PrintValue( reproductive_intensity  );

      // Identify dead, this time step
      IntegerVector mortality_individuals  = assign_death_randomly_by_age_class(mortality, demo);

      // Find number of recruits for each population
      IntegerMatrix recruit_demo           = recruit_to_carrying_capacity(demo, mortality_individuals, K); 

      // Assign mothers of recruits 
      IntegerVector mother_index           = assign_mothers_of_recruits(demo, recruit_demo, reproductive_intensity, dispersal);

      // Assign fathers of recruits
      IntegerVector father_index           = assign_fathers_of_recruits(demo, recruit_demo, reproductive_intensity, dispersal, mother_index, selfing);

      // Assign alleles to recruits from assigned parents   
      IntegerMatrix recruit_gt             = assign_alleles_to_recruits(gt, mother_index, father_index);

      // Concatenate recruits and survivors 
      demo = update_demo(demo, mortality_individuals, recruit_demo);
      gt   = update_gt(gt,   mortality_individuals, recruit_gt);

      t = t + 1;

    } // end main loop 

    NumericVector final_phenotype = assign_phenotype_quantitative(gt, locus_effect, dominance_effect);

    // bundle stuff into a list and return
    List result =
        List::create(demo, gt, final_phenotype);

    return result;
  
}


