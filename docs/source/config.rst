Configuration
=============

There are some elements of TPPS that can be configured to fit your specific needs! After installing TPPS, log into your site as an administrator. Then, from your dashboard, go to Modules --> TPPS --> Configure, or go to <base url>/admin/config/content/tpps. There are a few settings you can customize as an administrator:

* TPPS Admin Email Address: the email address used to send administrative TPPS messages, such as notifications about submission status.
* Use environmental layers from CartograPlant: If CartograPlant is installed, TPPS can add an optional field to the environment section for environment layers, using the layer data pulled in through CartograPlant.
* If the Use environmental layers option is selected, you will be asked to identify which of the layer groups provided by CartograPlant contain environmental data that is relevant to TPPS. These groups will be used to decide which layers to present as environmental layer options to users in TPPS.
* TPPS Genotype Max Group: the maximum number of genotype records TPPS is allowed to try to submit together. Higher max group numbers will mean faster genotype file parsing jobs, but are more likely to cause errors with the Tripal Job daemon.
* Reference Genome Directory: the location of local reference genomes on your server. If left blank, TPPS will skip searching for local reference genomes.
* File upload locations:

   * Author files: the location to store secondary author files
   * Plant Accession files: the location to store plant accession files
   * Genotype files: the location to store .VCF, .FASTA, and genotype assay files
   * Phenotype files: the location to store phenotype data and metadata files

