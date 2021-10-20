************************************
Genotype, Phenotype, and Environment
************************************

The fourth set of fields in TPPS is the Genotype, Phenotype, and Environment section, where users upload Genotypic, Phenotypic, and Environmental data and metadata about each organism. In multi-species submissions, users can reuse data from the previous species by checking the '<Genotype/Phenotype/Environmental> information for <organism i> is the same as <genotype/phenotype/environmental> information for <organism i-1>.' box, which is only available after the first organism. The form fields and their properties are as follows:

* Species **x**: ``fieldset``

Phenotype
=========

  * Phenotype Information: ``fieldset`` - only visible if the user selects 'Genotype x Phenotype', 'Genotype x Phenotype x Environment', or 'Phenotype x Environment' from 'Data Type' in `Study Design`_

     * Phenotype **x**: ``fieldset``

         * Phenotype Name: ``textfield`` - autocomplete options from ``chado.phenotype`` table
         * Phenotype Attribute: ``textfield`` -  autocomplete options from ``chado.phenotype`` table
         * Phenotype Description: ``textarea``
         * Phenotype Units: ``textfield`` - autocomplete options from ``chado.phenotypeprop`` table
         * Phenotype Structure: ``textfield`` - autocomplete options from ``chado.phenotype`` table - only visible if the 'Phenotype **x** has a structure descriptor' checkbox is checked
         * Phenotype Value Range : ``textfield`` - autocomplete options from ``chado.phenotypeprop`` table - only visible if the 'Phenotype **x** has a value range' checkbox is checked

     * Phenotype Metadata file: ``managed_file`` - spreadsheet of metadata about each phenotype - only visible if the 'I would like to upload a phenotype metadata file' checkbox is checked
     * Phenotype Metadata File Columns: ``fieldset`` of ``select`` elements - user will define which of their columns contain the Phenotype Name/Identifier, Phenotype Attribute, Phenotype Description, Phenotype Units, Phenotype Structure, Max/Min Phenotype Values - only visible if the 'I would like to upload a phenotype metadata file' checkbox is checked
     * Phenotype File: ``managed_file`` - spreadsheet of phenotypes
     * Phenotype File Columns: ``fieldset`` of ``select`` elements - user will define which of their columns contain the Plant ID, Phenotype Name/Identifier, and Phenotype value

A screenshot of the manual phenotype information fields can be seen below:

.. image:: ../../../images/TPPS_phenotype_manual.png

A screenshot of the phenotype metadata file field can be seen below:

.. image:: ../../../images/TPPS_phenotype_meta.png

A screenshot of the phenotype data file field can be seen below:

.. image:: ../../../images/TPPS_phenotype_data.png

Genotype
========

  * Genotype Information: ``fieldset`` - only visible if the user selects 'Genotype', 'Genotype x Phenotype', 'Genotype x Environment', or 'Genotype x Phenotype x Environment' from 'Data Type' in `Study Design`_

     * Genotype Marker Type: ``checkboxes`` - options 'SNPs', 'SSRs/cpSSRs', 'Other'
     * Genotype SNPs: ``fieldset`` - only visible if the user selects 'SNPs' from 'Genotype Marker Type'

         * SNPs Genotyping Design: ``select`` - options 'GBS', 'Targeted Capture', 'Whole Genome Resequencing', 'RNS-Seq', 'Genotyping Array'
         * GBS Type: ``select`` - options 'RADSeq', 'ddRAD-Seq', 'NextRAD', 'RAPTURE', 'Other' - only visible if the user selects 'GBS' from 'SNPs Genotyping Design'
         * GBS Custom Type: ``textfield`` - only visible if the user selects 'Other' from 'GBS Type'
         * Targeted Capture Type: ``select`` - options 'Exome Capture', 'Other' - only visible if the user selects 'Targeted Capture' from 'SNPs Genotyping Design'
         * Targeted Capture Custom Type: ``textfield`` - only visible if the user selects 'Other' from 'Targeted Capture Type'

     * Genotype SSRs/cpSSRs Type: ``textfield`` - only visible if the user selects 'SSRs/cpSSRs' from 'Genotype Marker Type'
     * Genotype Other Marker Type: ``textfield`` - only visible if the user selects 'Other' from 'Genotype Marker Type'

     * Reference Genome: ``select`` - stored reference genomes, as well as 'I can provide a URL to the website of my reference file(s)', 'I can provide a GenBank accession number (BioProject, WGS, TSA) and select assembly file(s) from a list', 'I can upload my own reference genome file', 'I can upload my own reference transcriptome file', 'I am unable to provide a reference assembly'
     * BioProject Accession: ``textfield`` - only visible if the user selects 'I can provide a GenBank accession number (BioProject, WGS, TSA) and select assembly file(s) from a list' from 'Reference Genome'

         * NCBI Assembly Accessions: ``checkboxes`` - options pulled directly from NCBI

     * URL or Manual Assembly File: ``fieldset`` - Tripal FASTA Loader fields - only visible if the user selects 'I can provide a URL to the website of my reference file(s)', 'I can upload my own reference genome file', or 'I can upload my own reference transcriptome file' from 'Reference Genome'

     * Genotype File Type: ``checkboxes`` - options 'Genotype Spreadsheet/Assay', 'Assay Design', 'VCF'. 'Assay Design' only visible if the user selects 'SNPs' from 'Genotype Marker Type'.
     * Genotype VCF File: ``managed_file`` - .VCF file of genotypes - only visible if the user selects 'VCF' from 'Genotype File Type'
     * Genotype File: ``managed_file`` - spreadsheet of genotypes - only visible if the user selects 'Genotype Spreadsheet/Assay' from 'Genotype File Type'
     * Genotype File Columns: ``fieldset`` of ``select`` elements - user will define which of their columns contain the Plant ID and Genotype Data
     * Assay Design File: ``managed_file`` - Assay design file - only visible if the user selects 'Assay Design' from 'Genotype File Type'

A screenshot of the genotype marker type fields can be seen below:

.. image:: ../../../images/TPPS_genotype_marker.png

A screenshot of the genotype reference fields can be seen below:

.. image:: ../../../images/TPPS_genotype_ref.png

A screenshot of the genotype file fields can be seen below:

.. image:: ../../../images/TPPS_genotype_file.png

Environment
===========

  * Environment Information: ``fieldset`` - only visible if the user selects 'Environment', 'Phenotype x Environment', 'Genotype x Environment', or 'Genotype x Phenotype x Environment' from 'Data Type' in `Study Design`_

     * CartograPlant Environmental Layers: ``fieldset`` - only visible if CartograPlant Layers are enabled in TPPS admin configuration and the 'I used environmental layers in my study that are indexed by CartograPlant.' checkbox is checked.

         * CartograPlant Environmental Layer **x**: ``checkbox`` - Indicates if the CartograPlant Environmental Layer **x** was used.

     * CartograPlant Environmental Layer Parameters: ``fieldset`` - only visible if CartograPlant Layers are enabled in TPPS admin configuration and the 'I used environmental layers in my study that are indexed by CartograPlant.' checkbox is checked.

         * CartograPlant Environmental Layer **x** Parameters: ``checkboxes`` - options of possible parameter types for the selected CartograPlant Environmental Layer. Each CartograPlant Environmental Layer **x** Parameters checkboxes set is only visible if that layer was selected in 'CartograPlant Environmental Layers'.
     
     * Custom Environmental Data: ``fieldset`` - only visible if the 'I have environmental data that I collected myself.' checkbox was checked.

         * Environmental Data **x**: ``fieldset``
     
              * Environmental Data Name: ``textfield``
              * Environmental Data Description: ``textfield``
              * Environmental Data Units: ``textfield``
              * Environmental Data Value: ``textifled``

.. _`Study Design`: page_2.html

