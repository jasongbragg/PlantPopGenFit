library(PlantPopGenFIT)


gt = matrix(c(2,0,0, 0,2,0,2,2,2),nrow=3)
gt = matrix(c(2,0,0, 0,2,0,0,0,0),nrow=3)

loc_eff = c(1.3,-1.3,0)

dom_eff = c(0,0,0)

epi_eff  = matrix(c(0,0,0, 0,0,0,0,0,0),nrow=3)
epi_eff[1,3] = 1

collapse_locus_interactions <- function(m) {

   c = 0 
   for (i in 1:nrow(m)) {
      for (j in 1:ncol(m)) {

         if (m[i,j] != 0) {
            if (c != 0 ) {
               tmp = c(i, j, m[i,j])
               out = rbind(out, tmp)
               c = c+1 
            }    
            if (c == 0 ) {
               out = c(i, j, m[i,j])
               c = c+1 
            }

         }

      }
   }
   out = matrix(out,ncol=3)
   rownames(out) <- paste0("I",1:nrow(out))
   return(out)
}

epi_table = collapse_locus_interactions(epi_eff)



#NumericVector assign_phenotype_quantitative_epistatic(IntegerMatrix gt, NumericVector locus_effect, NumericVector dominance_effect, IntegerMatrix epistatic_effect)


assign_phenotype_quantitative_epistatic(gt, loc_eff, dom_eff, epi_table)

