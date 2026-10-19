GENOTYPE PROCESSING DOCUMENTATION
=================================

ORDER MATTERS
1.  If there is no genotype data, return to parent function
2.  If SNP Assay is not empty
        IF snp association file is not empty, perform necessary processing / initialization
        IF snp design file is not empty, bypass processing SNP assay file (generate a VCF file) ELSE process the SNP assay file
        IF snp design file is not empty, 
