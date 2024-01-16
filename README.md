# Tripal Plant PopGen Submit (TPPS) pipeline
[![Build Status](https://app.travis-ci.com/par12005/TPPS.svg?branch=7.x-1.x)](https://app.travis-ci.com/par12005/TPPS)
[![Documentation Status](https://readthedocs.org/projects/tpps/badge/?version=latest)](https://tpps.readthedocs.io/en/latest/?badge=latest)
[![Maintainability](https://api.codeclimate.com/v1/badges/81cd654f6b83fa66f079/maintainability)](https://codeclimate.com/github/par12005/TPPS/maintainability)
[![Tripal Rating Silver Status](https://tripal.readthedocs.io/en/7.x-3.x/_images/Tripal-Silver.png)](https://tripal.readthedocs.io/en/7.x-3.x/extensions/module_rating.html#Silver)
[![DOI](https://zenodo.org/badge/189632031.svg)](https://zenodo.org/badge/latestdoi/189632031)

## Introduction
In this past 5 years alone, over 1200 papers have been published on association genetics and/or landscape genomics of forest trees. Very little of this data is formally collected as georeferenced accessions with full integration of genotype and phenotype. TreeGenes has developed the Tripal Plant Pop-Gen Submit pipeline (TPPS), an open-source [Drupal](https://www.drupal.org/) module built to extend the functionality of the [Tripal](http://tripal.info/) toolset, to specifically capture data and metadata describing genotype, phenotype and environmental studies associated with landscape genomics or association genetics investigations. The workflow relies on a series of questions to properly describe the experimental design, including location, replication and treatments. These questions also guide the system on the types of raw and intermediate data to request. Raw sequence data and reference genomes are sent to the primary repositories and linked back to TreeGenes via [NCBI](https://www.ncbi.nlm.nih.gov/)/EBI accession numbers. Intermediate deliverables, such as assemblies and genotypes, are accepted within the TPPS workflow. TPPS is able to accommodate a wide range of designs common to forest genetics studies: landscape sampling, breeding plots, common gardens and growth chamber experiments.

TPPS enforces minimal reporting standards and associated biological ontologies to provide reusable data. The module employs standards established as the Minimal Information About a Plant Phenotyping Experiment ([MIAPPE](http://www.miappe.org/)) to guide the collection of phenotypic data as well as the overall experimental design. The MIAPPE standards were developed from the objectives of transPLANT, European Plant Phenotyping Network and ELIXIR-EXCELERATE projects with the goal of developing reporting requirements to describe plant phenotyping experiments. MIAPPE integrates its minimal reporting standards with existing ontological frameworks. TreeGenes has implemented five of these: Plant Ontology (PO), Chemical Entities of Biological Interest ([ChEBI](https://www.ebi.ac.uk/chebi/)), Trait Ontology (TO), Crop Ontology (CO), Phenotype And Trait Ontology ([PATO](https://github.com/pato-ontology/pato)), and a custom TreeGenes ontology that serves to hold traits in transition to established ontologies. Plant structure, development and trait terms are integrated with PATO and supporting ontologies via the Planteome project which enables comparative biology across the omics.

In addition to the traits and their associated plant structures, genotypic values are collected through TPPS. Currently, TPPS can accommodate both SNPs and microsatellites (SSRs), as well as other user-defined marker types. This marker data is collected in the context of the sequencing design, which may include genotyping assays, genotype-by-sequencing (GBS) approaches, transcriptomic AQ4 sequencing and whole genome resequencing. Community standards, such as the Variant Common Format, are preferred. However, alternatives consistent with minimal reporting are accepted and will be converted and stored for re-distribution in standard file formats. For all submissions with genotypic values, TPPS strongly encourages the user to reference a genome and version or provide an intermediate assembly (transcriptomic or genomic) from which the SNP calls were derived. In a final, optional step, environmental data for the georeferenced trees can be loaded directly from the layers used or as independent measurements conducted by the investigators. Following acceptance and validation of the data in the TPPS module, data is organized and submitted to a database implementing the [CHADO](http://gmod.org/wiki/Introduction_to_Chado) database schema, and an accession number is supplied to the user that provides a long-term reference to the entire dataset. TreeGenes works closely with journals focused on tree genetics to encourage researchers to submit these data at the time of publication. Accepted studies are available in TreeGenes under the ‘Tripal Plant Pop-Gen Submissions’ page, where users can download the associated flat files, organized by content type.

## Documentation
Documentation about TPPS for both users and administrators can be found [here](http://tpps.rtfd.io).



## TPPSc: Tripal Plant PopGen Submit - Curate
[![Documentation Status](https://readthedocs.org/projects/tppsc/badge/?version=latest)](https://tppsc.readthedocs.io/en/latest/?badge=latest)
[![DOI](https://zenodo.org/badge/194678174.svg)](https://zenodo.org/badge/latestdoi/194678174)

## Introduction
TPPSc is a [Drupal](https://www.drupal.org/) module built to extend the functionality of the [Tripal](http://tripal.info/) toolset. The purpose of this tripal module is to permit internal curation of manuscripts available on public sites with supplemental data, such as Dryad. This module will allow members of the database team to load existing flat files and select the data that is relevant. This module does not currently require any other modules.

## Documentation
Documentation for TPPSc can be found [here](http://tppsc.rtfd.io).

## Tutorial
A detailed tutorial for TPPSc can be found [here](https://tppsc.readthedocs.io/en/latest/tutorial.html). The tutorial features screenshots and information about how to interact with the TPPSc form.

