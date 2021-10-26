


a <- matrix (sample(c(  rep(1,10), rep(0,90) )), nrow=10 )


collapse_locus_interactions <- function(m) {

   c = 0 
   for (i in 1:nrow(m)) {
      for (j in 1:ncol(m)) {

         if (m[i,j] != 0) {
            if (c == 0 ) {
               out = c(i, j, m[i,j])
               c = c+1 
            }
            if (c != 0 ) {
               tmp = c(i, j, m[i,j])
               out = rbind(out, tmp)
               c = c+1 
            }    
         }

      }
   }
   rownames(out) <- paste0("I",1:nrow(out))
   return(out)
}

epistatic_effect = collapse_locus_interactions(a)

