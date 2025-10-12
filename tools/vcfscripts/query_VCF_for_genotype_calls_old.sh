#!/bin/bash
# Original code contribution by Meghan Myles - THANK YOU!
# Modified by Risharde Ramnath for use with study_details.inc (3/30/2025)
# module load bcftools/1.12

# VCF file path
# VCF_FILE="/core/labs/Wegrzyn/meghan_work/query_VCF_for_genotypes/Geraldes_2013.v2.vcf"
# VCF_FILE="/core/labs/Wegrzyn/VCF/tpps_genotype_web_uploads/tiny_fake_Ptrich_4.vcf"
VCF_FILE=$2

# Query specific page of markers
query_vcf() {
    local page=$1
    local chunk_size=20

    # Calculate start and end marker indices
    start_line=$(( (page - 1) * chunk_size + 1 ))
    end_line=$(( page * chunk_size ))

    # Extract markers with reference and alternate alleles, then convert
    bcftools query \
        -f '[%SAMPLE\t%ID\t%REF\t%ALT\t%GT\n]' \
        "$VCF_FILE" \
        | awk -v start="$start_line" -v end="$end_line" 'NR >= start && NR <= end {
            # Split alternate alleles
            split($4, alts, ",")
            
            # Split genotype
            split($5, gt, "/")
            
            # Determine alleles based on genotype
            if (gt[1] == "0" && gt[2] == "0") {
                $5 = $3 "/" $3
            } else if (gt[1] == "0" || gt[2] == "0") {
                # One allele is reference
                $5 = (gt[1] == "0" ? $3 : alts[int(gt[1])]) "/" (gt[2] == "0" ? $3 : alts[int(gt[2])])
            } else {
                # Both alleles are alternate
                $5 = alts[int(gt[1])] "/" alts[int(gt[2])]
            }
            
            # Remove the REF and ALT columns, keeping only the converted genotype
            print $1, $2, $5
        }'
}

# Call the function with the first argument passed to the script
query_vcf "${1:-1}"
