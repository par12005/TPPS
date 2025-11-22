#!/bin/bash

# CODE CONTRIBUTION BY MEGHAN MYLES - THANK YOU!
# MODIFIED BY RISHARDE RAMNATH FOR USE WITH STUDY_DETAILS.INC (3/4/2025)
# module load bcftools/1.12

# Default param
PAGE=1

# Command line args
while getopts "f:p:" opt; do
    case $opt in
        f) VCF_FILE="$OPTARG" ;;
        p) PAGE="$OPTARG" ;;
        *) exit 1 ;;
    esac
done

# Get specific page of genotypes
query_vcf() {
    local page=$1

    #local start_line=$(( (page - 1) * 20 + 1 ))
    #local end_line=$(( page * 20 ))
    start_line=$(( (page - 1) * 20 + 1 ))
    end_line=$(( page * 20 ))
    
    # Get and map genotypes
    bcftools query -f '[%SAMPLE\t%ID\t%REF\t%ALT\t%GT\n]' "$VCF_FILE" \
    | head -n $end_line \
    | tail -n +$start_line \
    | awk -F'\t' '{
        split($4, alts, ",")
        if ($5 == "./.") {
            final_gt = "./."
        } else {
            split($5, gt, "/")
            allele1 = (gt[1] == ".") ? "." : (gt[1] == "0") ? $3 : alts[int(gt[1])]
            allele2 = (gt[2] == ".") ? "." : (gt[2] == "0") ? $3 : alts[int(gt[2])]
            final_gt = allele1 "/" allele2
        }
        print $1, $2, final_gt
    }'
}

# Call the function
query_vcf "$PAGE"
exit 0