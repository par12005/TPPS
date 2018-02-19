# Tripal Plant PopGen Submit (TPPS) pipeline
1. [Introduction](#introduction)
2. [Installation](#installation)
3. [Features](#features)
4. [Features in development](#features-in-development)
5. [Resources](#resources)

# Introduction
TPPS is a [Drupal](https://www.drupal.org/) module built to extend the functionality of the [Tripal](http://tripal.info/) toolset. The purpose of the module is to present users with an easy-to-use method of uploading data from association genetics experiments to the [CHADO](http://gmod.org/wiki/Introduction_to_Chado) database schema. This module requires the following Tripal modules:
1. Tripal (v3.x)
2. Tripal Chado

# Installation
1. Click on the green "Clone or download" button on the top right corner of this page to obtain the web URL. Download this module by running ```git clone <URL> ``` on command line. 
2. Place the cloned module folder "TGDR" inside your /sites/all/modules. Then enable the module by running ```drush en TPPS``` (for more instructions, read the [Drupal documentation page](https://www.drupal.org/node/120641)).

# Features
- Support for genotype, phenotype, environmental, and metadata
- Support for population, association, and landscape genetics studies
- Support for ontology standards, including the Minimum Information About a Plant Phenotyping Experiment ([MIAPPE](http://www.miappe.org/))
- Support for standard genotyping file formats, such as .VCF
- Automatically submits data according to the Tripal Chado database schema
- Accepted studies are associated and stored in the database with longterm accessions that can be used in publication
- The studies can be queried or downloaded (flatfiles) through the Tripal interface

# Features in Development
- Location preview window
- File upload column definition
- Progress bar
- View submissions through the user's Tripal account
- Access in-progress submissions through user's Tripal account
- Validation across Tree Accession/VCF/WGS/TSA files
- Unique TreeGenes Accession number


# Resources

[TPPS in action!](https://tgwebdev.cam.uchc.edu/Drupal/master)

[Newest TPPS Flow Concept document](https://docs.google.com/document/d/1fyRlf18j5fq8D2l5Yvx9X-VdUqlrkSvoLC9KGbDnrU8/edit?usp=sharing)

[Old TGDR Submission Pipeline for reference](https://dendrome.ucdavis.edu/tgdr/DataImporter.php)

[Fairly Comprehensive Walkthrough of Drupal Module Dev (Covers Forms, AJAX, and tokens)](https://www.youtube.com/watch?v=bjxML7A19Zs&t=2734s)

[Similar Comprehensive Walkthrough of Drupal Module Dev (Spends some time explining hooks)](https://www.youtube.com/watch?v=dmpSFiCym7c&t=1580s)

[Drupal Form API Reference](https://api.drupal.org/api/drupal/developer%21topics%21forms_api_reference.html/7.x)

[Constructing CHADO queries](http://api.tripal.info/api/tripal/tripal_core%21api%21tripal_core.chado_query.api.inc/group/tripal_chado_query_api/2.x)
