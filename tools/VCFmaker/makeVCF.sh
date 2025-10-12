#!/bin/bash
#SBATCH --job-name=makeVCF
#SBATCH --output=%j.out
#SBATCH --error=%j.err
#SBATCH --partition=general
#SBATCH --qos=general
#SBATCH --mem=10G
#SBATCH -c 1

# Coded by Meghan Myles (TreeGenesDB)

module load python/3.10.1
module load tabix/0.2.6
module load htslib/1.20
module load samtools/1.20
module load bcftools/1.20

python makeVCF.py "$@"

# Zip and index (warning: will zip and index all *vcf in vcf output directory)
for vcf in ./*.vcf; do
    bgzip -f "$vcf"  # compress to .vcf.gz
    bcftools sort "${vcf}.gz" -Oz -o "${vcf}.gz.tmp" 2>/dev/null  # sort into temp file
    mv "${vcf}.gz.tmp" "${vcf}.gz"  # replace original with sorted
    tabix -f -p vcf "${vcf}.gz"  # index
done

# Remove .out and .err files if they are empty
[[ ! -s "${SLURM_JOB_ID}.out" ]] && rm -f "${SLURM_JOB_ID}.out"
[[ ! -s "${SLURM_JOB_ID}.err" ]] && rm -f "${SLURM_JOB_ID}.err"
