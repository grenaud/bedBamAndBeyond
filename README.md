bedBamAndBeyond is a lightweight tool to visualize genomic ranges from a BED/BAM file. You need:

- libbamtools
- libharu.

Add the correct path in the makefile, type make. Just type:

./bedBamAndBeyond   out.pdf in.bam 

you can also have a coverage plot:

./bedBamAndBeyond  --cov  out.pdf in.bam 

or

./bedBamAndBeyond out.pdf  in.bam  
