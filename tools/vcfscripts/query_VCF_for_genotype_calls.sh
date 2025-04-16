#!/bin/bash
# CODE CONTRIBUTION BY MEGHAN MYLES - THANK YOU!
# MODIFIED BY RISHARDE RAMNATH FOR USE WITH STUDY_DETAILS.INC (3/4/2025)
# module load bcftools/1.12

# Default parameters
CHUNK_SIZE=20
PAGE=1
VCF_FILE=""

# Parse command line arguments without verbose output
while getopts "f:p:c:h" opt; do
    case $opt in
        f) VCF_FILE="$OPTARG" ;;
        p) PAGE="$OPTARG" ;;
        c) CHUNK_SIZE="$OPTARG" ;;
        h) 
            echo "Usage: $0 -f VCF_FILE [-p PAGE] [-c CHUNK_SIZE]"
            echo "  -f VCF_FILE    Path to the VCF file (required)"
            echo "  -p PAGE        Page number (default: 1)"
            echo "  -c CHUNK_SIZE  Number of markers per page (default: 20)"
            exit 0
            ;;
        *) exit 1 ;;
    esac
done

# Query specific page of markers
query_vcf() {
    local page=$1
    local chunk_size=$2

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

            # Handle genotype conversion
            if ($5 == "./.") {
                # Missing genotype stays as ./.
                final_gt = "./."
            } else {
                # Split genotype
                split($5, gt, "/")

                # First allele
                if (gt[1] == ".") {
                    allele1 = "."
                } else if (gt[1] == "0") {
                    allele1 = $3
                } else {
                    allele1 = alts[int(gt[1])]
                }

                # Second allele
                if (gt[2] == ".") {
                    allele2 = "."
                } else if (gt[2] == "0") {
                    allele2 = $3
                } else {
                    allele2 = alts[int(gt[2])]
                }

                final_gt = allele1 "/" allele2
            }

            # Print sample, ID, and converted genotype
            print $1, $2, final_gt
        }'
}

# Call the function and exit immediately after completion
query_vcf "$PAGE" "$CHUNK_SIZE"
exit 0