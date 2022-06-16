// load Rcpp
#include <Rcpp.h>
using namespace Rcpp;


// determine reproductive (flowering) intensity
// generate relative value of each at random
// [[Rcpp::export]]
NumericVector assign_reproductive_intensity_random (int num_ind, double mean, double sigma) {
   NumericVector ri = rnorm(num_ind, mean, sigma);
   return ri;
}


// identify individuals to die in this time step
// rate of mortality depends on age class
// [[Rcpp::export]]
IntegerVector assign_death_randomly_by_age_class(NumericVector mortality, IntegerMatrix demo) {

   // determine index of mortality to be applied to 
   // all subsequent age classes
   int s = mortality.size() - 1;

   int n = demo.nrow();
   IntegerVector mi(n);  

    for (int i=0; i<n; ++i) {

        // get age of individual
        int  age = demo(i,1);

        // if age greater than s, set to s
        if (age > s) {
           age = s;
        }

        // get mortality rate for age
        double m = mortality(age);

        // determine if individual dies
        if (R::runif(0,1) < m) {
           mi(i) = 1;
        }
    }

   return mi;
}


// sample with replacement based on relative weights 
// [[Rcpp::export]]
int sample_with_replacement_based_on_relative_weights (NumericVector weights) {

   int    n = weights.size();
   double s = sum(weights); 
 
   double cumisum = 0.0 ;
   double randval = R::runif(0,1) * s;
   int    si = -9; 

   for (int i=0; i<n; ++i) {

      double cumisumi = cumisum + weights[i];
      if (cumisum < randval && cumisumi > randval) {
         si = i;
      }
      cumisum = cumisumi;
    }

   if (si == -9) {
      Rprintf(" Error: sample_with_replacement_based_on_relative_weights did not return index ");
   }
   return si;
}


// determines the number of recruits needed for each population to
// reach carrying capacity in the next step
// Note: K can be different to population size at last step 
// [[Rcpp::export]]
IntegerMatrix recruit_to_carrying_capacity (IntegerMatrix demo, IntegerVector mortality_individuals, IntegerVector K) {

   int number_populations = K.size();
   int carrying_capacity  = sum(K);
   int total_mortality    = sum(mortality_individuals);
   int total_previous     = demo.nrow(); 

   IntegerVector population_mortality(number_populations);
   IntegerVector population_previous(number_populations);
   IntegerVector population_recruits(number_populations);

   // make vectors of population numbers
   for (int i=0; i<total_previous; ++i) {

      int ipop = demo(i, 0);
      population_previous(ipop) = population_previous(ipop) + 1;

      if (mortality_individuals(i) == 1) {
         population_mortality(ipop) = population_mortality(ipop) + 1; 
      }
   }

   // calculate number recruits for each population 
   for (int i=0; i<number_populations; ++i) {
      int irec = K(i) - population_previous(i) + population_mortality(i) ;

      // in case population is falling...
      if ( irec < 0 ) {
         irec = 0;
      }

      // necessary?
      if ( irec > K(i) ) {
         irec = K(i);
      }

      population_recruits(i) = irec ;
   }

   int total_recruits = sum(population_recruits); 
   IntegerMatrix rd(total_recruits, 2);

   int k = 0;
   for (int i=0; i<number_populations; ++i) {
      int prec = population_recruits(i);
      for (int j=0; j<prec; ++j) {
         rd(k, 0) = i;
         k = k+1;
      }
   }

   return rd;
}


// Assign mothers to new recruits
// function of reproductuve intensity, dispersal
// [[Rcpp::export]]
IntegerVector assign_mothers_of_recruits (IntegerMatrix demo, IntegerMatrix recruit_demo, NumericVector reproductive_intensity, NumericMatrix dispersal) {

   int total_recruits = recruit_demo.nrow();
   int total_number   = demo.nrow();

   IntegerVector mi(total_recruits);

   for (int i=0; i<total_recruits; ++i) {

      NumericVector m_fact_vec(total_number);
      int    p_rec_i = recruit_demo(i,0);

      for (int j=0; j<total_number; ++j) {

         int   p_j  = demo(j,0);
         double r_fact  = reproductive_intensity(j);
         double p_fact  = dispersal(p_rec_i, p_j);
         m_fact_vec[j]  = r_fact * p_fact;

         //Rprintf("    p_rec_i: %i\n", p_rec_i);
         //Rprintf("    p_j    : %i\n", p_j);
         //Rprintf("    p_fact : %f\n", p_fact);

      }

      int i_mother = sample_with_replacement_based_on_relative_weights ( m_fact_vec );
      mi[i] = i_mother;

      // Rprintf(" Mfactor... \n" ); 
      // Rf_PrintValue( m_fact_vec );
      // Rf_PrintValue( mi );
   }
 
   return mi;
}

// Assign fathers to new recruits
// function of reproductuve intensity, dispersal, mother, selfing
// [[Rcpp::export]]
IntegerVector assign_fathers_of_recruits (IntegerMatrix demo, IntegerMatrix recruit_demo, NumericVector reproductive_intensity, NumericMatrix dispersal, IntegerVector mother_index, double selfing) {

   int total_recruits = recruit_demo.nrow();
   int total_number   = demo.nrow();

   IntegerVector fi(total_recruits);

   for (int i=0; i<total_recruits; ++i) {

      NumericVector f_fact_vec(total_number);
      int    p_mother_of_rec_i = mother_index(i);
      int    p_mother_i        = demo(p_mother_of_rec_i,0);

      for (int j=0; j<total_number; ++j) {

         int   p_j  = demo(j,0);
         double r_fact  = reproductive_intensity(j);
         double p_fact  = dispersal(p_mother_i, p_j);

         double s_fact  = 1.0;
         if (i == j) {
            s_fact = selfing;
         }

         f_fact_vec[j]  = r_fact * p_fact * s_fact;

      }

      int i_father = sample_with_replacement_based_on_relative_weights ( f_fact_vec );
      fi[i] = i_father;

   }
 
   return fi;

}


// assign alleles to new recruits, based on parent genotypes
// [[Rcpp::export]]
IntegerMatrix assign_alleles_to_recruits (IntegerMatrix gt, IntegerVector mother_index, IntegerVector father_index) {

   int number_recruits = mother_index.size();
   int number_loci     = gt.ncol();

   IntegerMatrix rgt(number_recruits, number_loci);
   

   for (int i=0; i<number_recruits; ++i) {

      // get indices for parents
      int i_mother = mother_index(i);
      int i_father = father_index(i);

      // loop through loci
      for (int j=0; j<number_loci; ++j) {

         // get maternal allele
         int allele_from_mother = 0;
         int mother_genotype = gt(i_mother,j);

         if (mother_genotype == 1) {
            double mrandval = R::runif(0,1);
            if ( mrandval > 0.5) {
               allele_from_mother = 1;
            }
         }

         if (mother_genotype == 2) {
            allele_from_mother = 1;
         }


         // get paternal allele
         int allele_from_father = 0;
         int father_genotype = gt(i_father,j);

         if (father_genotype == 1) {
            double frandval = R::runif(0,1);
            if ( frandval > 0.5) {
               allele_from_father = 1;
            }
         }

         if (father_genotype == 2) {
            allele_from_father = 1;
         }


         // determine recruit genotype
         rgt(i,j) = allele_from_mother + allele_from_father;

      }
   }

   return rgt;
}

// rbind!
// [[Rcpp::export]]
IntegerMatrix row_bind_integer_matrices(IntegerMatrix a, IntegerMatrix b) {

   int a_nrow = a.nrow(); 
   int b_nrow = b.nrow(); 
   int a_ncol = a.ncol(); 
   int b_ncol = b.ncol(); 

   int c_nrow = a_nrow + b_nrow;
   int c_ncol = a_ncol;

   IntegerMatrix c(c_nrow, c_ncol);

   int k = 0;
   for (int i=0; i<a_nrow; ++i) 
   {
      for (int j=0; j<c_ncol; ++j) {
         c(k,j) = a(i,j);
      }
      k = k+1;
   }

   for (int i=0; i<b_nrow; ++i) 
   {
      for (int j=0; j<b_ncol; ++j) {
         c(k,j) = b(i,j);

      }
      k = k+1;
   }

   return c;
}


// makes a demo table for t=t+1 using recruits and survivors from time t
// [[Rcpp::export]]
IntegerMatrix update_demo(IntegerMatrix demo, IntegerVector mortality_individuals, IntegerMatrix recruit_demo) {

   int number_previous  = mortality_individuals.size();
   int number_recruits  = recruit_demo.nrow();
   int number_mortality = sum(mortality_individuals);
   int number_survived  = number_previous - number_mortality;

   int number_new = number_survived + number_recruits;

   IntegerMatrix new_demo(number_new, 2);
   IntegerMatrix survivor_demo(number_survived,2);

   // make demo table for survivors
   int j = 0;
   for (int i=0; i<number_previous; ++i) 
   {
      int imort = mortality_individuals(i); 
      if ( imort == 0 ) {
         survivor_demo(j,0) = demo(i,0);
         survivor_demo(j,1) = demo(i,1) + 1;
         j = j + 1;
      }
   }

   new_demo = row_bind_integer_matrices(recruit_demo, survivor_demo);

   return new_demo;
}


// makes a gt table for t=t+1 using recruits and survivors from time t
// [[Rcpp::export]]
IntegerMatrix update_gt(IntegerMatrix gt, IntegerVector  mortality_individuals, IntegerMatrix recruit_gt) {

   int number_previous  = mortality_individuals.size();
   int number_recruits  = recruit_gt.nrow();
   int number_mortality = sum(mortality_individuals);
   int number_survived  = number_previous - number_mortality;

   int number_new = number_survived + number_recruits;

   int number_loci = gt.ncol();

   IntegerMatrix new_gt(number_new, number_loci);
   IntegerMatrix survivor_gt(number_survived, number_loci);

   // make gt table for survivors
   int j = 0;
   for (int i=0; i<number_previous; ++i) 
   {
      int imort = mortality_individuals(i); 
      if ( imort == 0 ) {
         for (int n=0; n<number_loci; ++n) {
            survivor_gt(j,n) = gt(i,n);
         }
         j = j + 1;
      }
   }

   new_gt = row_bind_integer_matrices(recruit_gt, survivor_gt);

   return new_gt;

}

// Phenotype values from genotypes, and locus and dominance effects
// [[Rcpp::export]]
NumericVector assign_phenotype_quantitative(IntegerMatrix gt, NumericVector locus_effect, NumericVector dominance_effect) {

   int number_individuals = gt.nrow();
   int number_loci        = gt.ncol();
   
   NumericVector p(number_individuals);

   for (int i=0; i<number_individuals; ++i) 
   {
      double p_i = 0.0;

      for (int j=0; j<number_loci; ++j) {

         int gt_ij = gt(i,j);
         double loc = locus_effect(j);
         double dom = dominance_effect(j);

         double p_ij = 0.0;

         if (gt_ij == 2) {
            p_ij = loc;
         }

         if (gt_ij == 1) {
            p_ij = loc * dom;
         }

         p_i = p_i + p_ij;
      }

      p(i) = p_i;
   }
   
   return p;
}


// Phenotype values from genotypes, and locus, dominance and epistatic effects
// [[Rcpp::export]]
NumericVector assign_phenotype_quantitative_epistatic(IntegerMatrix gt, NumericVector locus_effect, NumericVector dominance_effect, IntegerMatrix epistatic_effect) {

   int number_individuals = gt.nrow();
   int number_loci        = gt.ncol();
   int number_epi         = epistatic_effect.nrow();   

   NumericVector p(number_individuals);

   for (int i=0; i<number_individuals; ++i) 
   {

      // get the additive effects in a vector, p_ij_vec
      double p_i = 0.0;
      NumericVector p_ij_vec(number_loci);

      for (int j=0; j<number_loci; ++j) {

         int gt_ij = gt(i,j);
         double loc = locus_effect(j);
         double dom = dominance_effect(j);

         double p_ij = 0.0;

         if (gt_ij == 2) {
            p_ij = loc;
         }

         if (gt_ij == 1) {
            p_ij = loc * dom;
         }

         p_ij_vec(j) = p_ij;
      }

      // modulate additive effects with epistatic effects

      for (int k=0; k<number_epi; ++k) {

         // modulated
         int ind_med_locus  = epistatic_effect(k,0) -1 ;
         // modulating
         int ind_ming_locus = epistatic_effect(k,1) -1 ;
         // modval
         int modval = epistatic_effect(k,2);

         double dom = dominance_effect(ind_ming_locus);
         int gt_ming_locus = gt(i, ind_ming_locus) ;

         if (gt_ming_locus == 2) {
            p_ij_vec(ind_med_locus) = p_ij_vec(ind_med_locus) + modval;
         }

         if (gt_ming_locus == 1) {
            p_ij_vec(ind_med_locus) = p_ij_vec(ind_med_locus) + modval * dom;
         }

      }

      p(i) = sum(p_ij_vec);
   }
   
   return p;
}



// assign reproductive intensity (RI) to individuals based on their phenotype, and a parameterized dependence of RI on phenotype
// [[Rcpp::export]]
NumericVector assign_reproductive_intensity_phenotype(IntegerMatrix demo, NumericVector phenotype_value, NumericVector phenotype_opt, double n_p, double n_k) {

   int number_individuals = phenotype_value.size();
   NumericVector ri(number_individuals);

   for (int i=0; i<number_individuals; ++i) 
   {
      double p    = phenotype_value(i);
      int    pop  = demo(i,0);
      double popt = phenotype_opt(pop);
      double delp = fabs( p-popt);
      ri(i) = pow(n_k,n_p) / ( pow(n_k, n_p) + pow(delp,n_p) );
   }

   return ri;

}

// Mutate genotypes, looping over all
// [[Rcpp::export]]
IntegerMatrix mutate_genotypes(IntegerMatrix gt, double mutation) {

   int number_individuals = gt.nrow();
   int number_loci        = gt.ncol();

   for (int i=0; i<number_individuals; ++i) 
   {
      for (int j=0; j<number_loci; ++j) 
      {
         int gt_ij = gt(i,j);
         double mrandval = R::runif(0,1);

         // if rand small enough, apply mutation
         if ( mrandval < mutation ) {

            // if homo, make het
            if (gt_ij == 0) {
               gt(i,j) = 1;
            }

            // if het, make homo -- 50% chance of either allele
            if (gt_ij == 1) {
               if ( R::runif(0,1) > 0.5 ) {
                  gt(i,j) = 0;
               } else {
                  gt(i,j) = 2;
               }
            }

            // if homo, make het
            if (gt_ij == 2) {
               gt(i,j) = 1;
            }

         } // if mutation
      }  // for j
   } // for i
   return gt;
}


/* 
// Under development -- 
// Mutate genotypes, estimate number by binomial, choose loci at random
// [[Rcpp::export]]
IntegerMatrix mutate_genotypes_fast(IntegerMatrix gt, double mutation) {

   int number_individuals = gt.nrow();
   int number_loci        = gt.ncol();
 
   int total_size         = number_individuals * number_loci; 
   int number_mutations   = R::rbinom( total_size, mutation );

   if (number_mutations == 0) {
      return gt;
   } else {
      for (int i=0; i<number_mutations; ++i) 
      {
         double randval  = R::runif(0,1) * total_size;
         int    mut_i    = floor(randval);
         int    gt_mut   = gt[mut_i];

         // if homo, make het
         if (gt_mut == 0) {
            gt[mut_i] = 1;
         }

            // if het, make homo -- 50% chance of either allele
         if (gt_mut == 1) {

            if ( R::runif(0,1) > 0.5 ) {
               gt[mut_i] = 0;
            } else {
               gt[mut_i] = 2;
            }
         }

         // if homo, make het
         if (gt_mut == 2) {
            gt[mut_i] = 1;
         }

      } // for i in mutations
      return gt;
   } // if gt 0 mutations

}

*/


