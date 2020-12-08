Input Validation
================

Once the 4 sets of fields have been filled out by the user, their data is validated to ensure completion and integrity.

* All fields are required when they are visible, and are therefore checked for completeness.
* Plant IDs from Phenotype and Genotype files are checked against Plant IDs in Plant Accession files to ensure there is not any data without plants.
* Scaffold/Chromosome IDs from .VCF files or Genotype assay files are checked against scaffold/chromosome IDs in assembly files to ensure there are no scaffolds without position.
* Phenotype names/IDs from phenotype data files are checked against phenotype names/IDs in phenotype metadata files to ensure there are no undefined phenotypes.
* Users are allowed to upload plants in Plant Accession files without Genotype/Phenotype data, but not Genotype/Phenotype data without plant locations.

Input validation is broken up into steps, once after each set of fields. This is so that the user cannot continue to the next set of fields if they have incomplete or invalid data in their current set, and so that a user does not need to go back to previous sets of fields to correct data later.

