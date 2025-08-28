#!/bin/bash
#SBATCH --job-name=VCFmaker_setup
#SBATCH --output=%j.out
#SBATCH --error=%j.err
#SBATCH --mem=4G
#SBATCH --cpus-per-task=1
#SBATCH --partition=general
#SBATCH --qos=general

# Coded by Meghan Myles (TreeGenesDB)

# Install packages
conda install -y -c conda-forge pandas numpy biopython


