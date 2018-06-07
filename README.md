# Tripal Plant PopGen Submit (TPPS) pipeline
1. [Introduction](#introduction)
2. [Installation](#installation)
3. [Features](#features)
4. [Login and profile](#login-and-profile)
5. [Publication and Species Interface](#publication-and-species-interface)
6. [Study Design](#study-design)
7. [Tree Accession](#tree-accession)
8. [Genotype, Phenotype, and Environment](#genotype-phenotype-and-environment)
9. [Input Validation](#input-validation)
10. [Data Submission](#data-submission)
11. [Features in development](#features-in-development)
12. [Resources](#resources)

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

# Login and Profile
Users can only access the TPPS form while they are logged into their Tripal account. This is so that a user can pick up where they left off, should they need to make their submission over multiple sessions. For example, if a user is submitting data through TPPS, then realizes that they need to collect some additional information before completing their submission, they can leave the TreeGenes site to collect their additional information, and when they return, all of the data that user entered previously will be stored on TreeGenes, and the user will not need to fill out all of the form fields again.

If the user is not logged in, they are redirected to the login page, where they can login as an existing user or sign up to create a new account.

![alt text](/screenshots/TPPS_landing.png)

# Publication and Species Interface
The first set of fields in TPPS is the publication and species interface, where users upload data about their publication and up to 5 studied species. The form fields and their properties are as follows:

- User Info: set of form fields
  - Primary Author: textfield that auto-populates with the name registered to their Tripal account. If the primary author is changed, autocomplete options are provided from the chado.contact table.
  - Organization: textfield with autocomplete options from the chado.contact table.
- Publication: set of form fields
  - Publication status: drop-down menu with options 'In Preparation or Submitted', 'In press', and 'Published'
  - Secondary Authors: set of form fields
     - Secondary Author **x**: textfield with autocomplete options from the chado.contact table
     - >30 Secondary Authors: checkbox
     - Secondary Authors file: file upload for a spreadsheet of secondary authors. This field is only visible if the '>30 Secondary Authors' checkbox is checked.
  - Publication Year: drop-down menu with options '1970' to '2018'
  - Publication Title: textfield
  - Publication Abstract: text area
  - Publication Journal: textfield with autocomplete options from chado.pub table
- Tree Species: set of form fields
  - Species **x**: textfield with autocomplete options from the chado.organism table
  - Up to 5 different species are allowed per submission

![alt text](/screenshots/TPPS_author_species.png)

# Study Design
The second set of fields in TPPS is the Study Design section, where users upload metadata about their experiment. The form fields and their properties are as follows:

- Study Start Date: set of form fields
  - Start Date Year: drop-down menu with options '1970' to '2018'
  - Start Date Month: drop-down menu with options 'January' through 'December'
- Study End Date: set of form fields
  - End Date Year: drop-down menu with options '1970' to '2018'
  - End Date Month: drop-down menu with options 'January' through 'December'
- Study Location: set of form fields
  - Study Location Format: drop-down menu with options 'WGS 84', 'NAD 83', 'ETRS 89', and 'Custom Location'
  - Study Location Latitude: textfield - only visible if user selects 'WGS 84', 'NAD 83', or 'ETRS 89' from 'Study Location Format'
  - Study Location Longitude: textfield - only visible if user selects 'WGS 84', 'NAD 83', or 'ETRS 89' from 'Study Location Format'
  - Study Location Country: textfield - only visible if user selects 'Custom Location' from 'Study Location Format'
  - (Optional) Study Location Region: textfield - only visible if user selects 'Custom Location' from 'Study Location Format'
- Data Type: drop-down menu with options 'Genotype', 'Genotype x Phenotype', 'Genotype x Environment', 'Genotype x Phenotype x Environment', 'Phenotype x Environment'
- Study Type: drop-down menu with options 'Natural Population', 'Growth Chamber', 'Greenhouse', 'Experimental/Common Garden', 'Plantation'
- Natural Population: set of form fields - only visible if the user selects 'Natural Population' from 'Study Type'
  - Season: checkboxes with options 'Spring', 'Summer', 'Fall', 'Winter'
  - Assessions: textfield
- Growth Chamber: set of form fields - only visible if the user selects 'Growth Chamber' from 'Study Type'
  - CO2 Info: set of form fields
     - CO2 Control: drop-down menu with options 'controlled', 'uncontrolled'
     - CO2 Value: textfield
  - Humidity Info: set of form fields
     - Humidity Control: drop-down menu with options 'controlled', 'uncontrolled'
     - Humidity Value: textfield
  - Light Intensity Info: set of form fields
     - Light Intensity Control: drop-down menu with options 'controlled', 'uncontrolled'
     - Light Intensity Value: textfield
  - Temperature Info: set of form fields
     - Average High Temperature: textfield
     - Average Low Temperature: textfield
  - Rooting Info: set of form fields
     - Rooting Type: drop-down menu with options 'Aeroponics', 'Hydroponics', and 'Soil'
     - Soil: set of form fields - only visible if the user selects 'Soil' from 'Rooting Type'
         - Soil Type: drop-down menu with opitons 'Sand', 'Peat', 'Clay', 'Mixed', 'Other'
         - Custom Soil Type: textfield - only visible if the user selects 'Other' from 'Soil Type'
         - Soil Container Type: textfield
     - PH: set of form fields
         - PH Control: drop-down menu with options 'controlled', 'uncontrolled'
         - PH Value: textfield
     - Treatments: checkboxes with options 'Seasonal Environment', 'Air temperature regime', 'Soil Temperature regime', 'Antibiotic regime', 'Chemical administration', 'Disease status', 'Fertilizer regime', 'Fungicide regime', 'Gaseous regime', 'Gravity Growth hormone regime', 'Mechanical treatment', 'Mineral nutrient regime', 'Humidity regime', 'Non-mineral nutrient regime', 'Radiation (light, UV-B, X-ray) regime', 'Rainfall regime', 'Salt regime', 'Watering regime', 'Water temperature regime', 'Pesticide regime', 'pH regime', 'other perturbation'
     - Treatments Description: if an option from 'Treatments' is selected, users must provide a description of the treatment in a textfield
- Greenhouse: set of form fields - only visible if the user selects 'Greenhouse' from 'Study Type'
  - Humidity Info: set of form fields
     - Humidity Control: drop-down menu with options 'controlled', 'uncontrolled'
     - Humidity Value: textfield - only visible if the user selects 'controlled' from 'Humidity Control'
  - Light Intensity Info: set of form fields
     - Light Intensity Control: drop-down menu with options 'controlled', 'uncontrolled'
     - Light Intensity Value: textfield - only visible if the user selects 'controlled' from 'Light Intensity Control'
  - Temperature Info: set of form fields
     - Average High Temperature: textfield
     - Average Low Temperature: textfield
  - Rooting Info: set of form fields
     - Soil: set of form fields
        - Soil Type: drop down menu with options 'Sand', 'Peat', 'Clay', 'Mixed', 'Other'
        - Custom Soil Type: textfield - only visible if the user selects 'Other' from 'Soil Type'
        - Soil Container Type: textfield
     - PH: set of form fields
        - PH Control: drop-down menu with options 'controlled', 'uncontrolled'
        - PH Value: textfield - only visible if the user selects 'controlled' from 'PH Control'
     - Treatments: checkboxes with options 'Seasonal Environment', 'Air temperature regime', 'Soil Temperature regime', 'Antibiotic regime', 'Chemical administration', 'Disease status', 'Fertilizer regime', 'Fungicide regime', 'Gaseous regime', 'Gravity Growth hormone regime', 'Mechanical treatment', 'Mineral nutrient regime', 'Humidity regime', 'Non-mineral nutrient regime', 'Radiation (light, UV-B, X-ray) regime', 'Rainfall regime', 'Salt regime', 'Watering regime', 'Water temperature regime', 'Pesticide regime', 'pH regime', 'other perturbation'
     - Treatments Description: if an option from 'Treatments' is selected, users must provide a description of the treatment in a textfield
- Common Garden: set of form fields - only visible if the user selects 'Experimental/Common Garden' from 'Study Type'
  - Irrigation: set of form fields
     - Irrigation Type: drop-down menu with options 'Irrigation from top', 'Irrigation from bottom', 'Drip Irrigation', 'Other', 'No Irrigation'
     - Custom Irrigation Type: textfield - only visible if the user selects 'Other' from 'Irrigation Type'
  - Salinity Info: set of form fields
     - Salinity Control: drop-down menu with options 'controlled', 'uncontrolled'
     - Salinity Value: textfield
  - Biotic Environment Info: set of form fields
     - Biotic Environment Type: drop-down menu with options 'Herbivores', 'Mutilists', 'Pathogens', 'Endophyts', 'Other', 'None'
     - Custom Biotic Environment Type: textfield - only visible if the user selects 'Other' from 'Biotic Environment Type'
  - Season: checkboxes with options 'Spring', 'Summer', 'Fall', 'Winter'
  - Treatments: checkboxes with options 'Seasonal environment', 'Antibiotic regime', 'Chemical administration', 'Disease status', 'Fertilizer regime', 'Fungicide regime', 'Gaseous regime', 'Gravity Growth hormone regime', 'Herbicide regime', 'Mechanical treatment', 'Mineral nutrient regime', 'Non-mineral nutrient regime', 'Salt regime', 'Watering regime', 'Pesticide regime', 'pH regime', 'Other perturbation'
  - Treatments Description: if an option from 'Treatments' is selected, users must provide a description of the treatment in a textfield
- Plantation: set of form fields - only visible if the user selects 'Plantation' from 'Study Type'
  - Season: checkboxes with options 'Spring', 'Summer', 'Fall', 'Winter'
  - Assessions: textfield
  - Treatments: checkboxes with options 'Seasonal environment', 'Antibiotic regime', 'Chemical administration', 'Disease status', 'Fertilizer regime', 'Fungicide regime', 'Gaseous regime', 'Gravity Growth hormone regime', 'Herbicide regime', 'Mechanical treatment', 'Mineral nutrient regime', 'Non-mineral nutrient regime', 'Salt regime', 'Watering regime', 'Pesticide regime', 'pH regime', 'Other perturbation'
  - Treatments Description: if an option from 'Treatments' is selected, users must provide a description of the treatment in a textfield

![alt text](/screenshots/TPPS_exp_cond.png)

# Tree Accession
The third set of fields in TPPS is the Tree Accession section, where users upload information about where trees are located. The form fields and their properties are as follows:

- Tree Accession: set of form fields - only visible if number of species in [Publication and Species Interface](#publication-and-species-interface) is 1, or if the 'Separate Tree Accession' checkbox is unchecked.
  - Tree Accession File: file upload for a spreadsheet of tree locations
  - Tree Accession Columns: set of drop-down menus, allowing the user to define which of their columns contain the Tree ID, and which of their columns contain location information
- Separate Tree Accession: checkbox
- Separate Tree Accession Information: set of form fields - only visible if the 'Separate Tree Accession' checkbox is checked
  - Tree Accession File **x**: file upload for a spreadsheet of tree locations of tree species **x**
  - Tree Accession Columns **x**: set of drop-down menus, allowing the user to define which of their columns contain the Tree ID, and which of their columns contain location information

![alt text](/screenshots/TPPS_accession.png)

# Genotype, Phenotype, and Environment
The fourth set of fields in TPPS is the Genotype, Phenotype, and Environment section, where users upload Genotypic, Phenotypic, and Environmental data and metadata. The form fields and their properties are as follows:

- Tree Species **x**: set of form fields
  - Phenotype Information: set of form fields - only visible if the user selects 'Genotype x Phenotype', 'Genotype x Phenotype x Environment', or 'Phenotype x Environment' from 'Data Type' in [Study Design](#study-design)
     - Phenotype **x**: set of form fields
         - Phenotype Name: text field
         - Environmental Data: checkbox
         - Phenotype Description: text area
         - (Optional) Phenotype Units: drop-down menu with options 'mm', 'cm', 'm', 'Degrees Celsius', 'Degrees Fahrenheit', 'Other'
         - Phenotype Custom Unit: text field - only visible if the user selects 'Other' from 'Phenotype Units'
         - Phenotype Type: drop-down menu with options 'Binary', 'Quantitative', 'Qualitative' - only visible if the 'Environmental Data' checkbox is unchecked
         - Phenotype Binary: set of form fields - only visible if the user selects 'Binary' from 'Phenotype Type'
              - Binary Type 1: text field
              - Binary Type 2: text field
         - Phenotype Quantitative: set of form fields - only visible if the user selects 'Quantitative' from 'Phenotype Type'
              - Phenotype Minimum: text field
              - Phenotype Maximum: text field
         - Phenotype Structure Ontology: drop-down menu with options from the chado.phenotype_structure_cvterm table
         - Phenotype Developmental Ontology: drop-down menu with options from the chado.phenotype_cvterm table
     - >30 Phenotypes: checkbox
     - Phenotype Metadata file: file upload for a spreadsheet of metadata about each phenotype - only visible if the '>30 Phenotypes' checkbox is checked
  - Phenotype File: file upload for a spreadsheet of phenotypes
  - Phenotype File Columns: set of drop-down menus, allowing the user to define which of their columns contain the Tree ID
  - Genotype Information: set of form fields - only visible if the user selects 'Genotype', 'Genotype x Phenotype', 'Genotype x Environment', or 'Genotype x Phenotype x Environment' from 'Data Type' in [Study Design](#study-design)
     - Genotype Marker Type: checkboxes with options 'SNPs', 'SSRs/cpSSRs', 'Other'
     - Genotype SNPs: set of form fields - only visible if the user selects 'SNPs' from 'Genotype Marker Type'
         - SNPs Genotyping Design: drop-down menu with options 'GBS', 'Targeted Capture', 'Whole Genome Resequencing', 'RNS-Seq', 'Genotyping Array'
         - SNPs GBS: set of form fields - only visible if the user selects 'GBS' from 'SNPs Genotyping Design'
              - GBS Type: drop-down menu with options 'RADSeq', 'ddRAD-Seq', 'NextRAD', 'RAPTURE', 'Other'
              - GBS Custom Type: text field - only visible if the user selects 'Other' from 'GBS Type'
         - SNPs Targeted Capture: set of form fields - only visible if the user selects 'Targeted Capture' from 'SNPs Genotyping Design'
              - Targeted Capture Type: drop-down menu with options 'Exome Capture', 'Other'
              - Targeted Capture Custom Type: text field - only visible if the user selects 'Other' from 'Targeted Capture Type'
         - SNPs BioProject ID: text field - only visible if the 'Manual Assembly' checkbox is unchecked
         - Auto Assembly Numbers: checkboxes with options from NCBI - only visible if the 'Manual Assembly' checkbox is unchecked
         - Manual Assembly File: file upload for a WGS/TSA assembly file - only visible if the 'Manual Assembly' checkbox is checked
         - Manual Assembly: checkbox
     - Genotype SSRs/cpSSRs Type: text field - only visible if the user selects 'SSRs/cpSSRs' from 'Genotype Marker Type'
     - Genotype Other Marker Type: text field - only visible if the user selects 'Other' from 'Genotype Marker Type'
  - Genotype File: file upload for a spreadsheet or .VCF file of genotypes
  - Genotype File Columns: set of drop-down menus, allowing the user to define which of their columns contain the Tree ID

![alt text](/screenshots/TPPS_data.png)

# Input Validation
Once the 4 sets of fields have been filled out by the user, their data is validated to ensure completion and integrity. All fields except for those marked (Optional) above are required when they are visible, and are therefore checked for completeness. Tree IDs from Phenotype and Genotype files are checked against Tree IDs in Tree Accession files to ensure there is not any data without trees. Users are allowed to upload trees in Tree Accession files without Genotype/Phenotype data, but not Genotype/Phenotype data without tree locations. Input validation is broken up into steps, once after each set of fields. This is so that the user cannot continue to the next set of fields if they have incomplete or invalid data in their current set, and so that a user does not need to go back to previous sets of fields to correct data later.

# Data Submission
Once the Input validation is completed for each of the 4 sets of fields, the data is organized and submitted to the database according to the CHADO schema.

Initially, the data will be submitted to temporary tables which are mirrored versions of the real CHADO tables. This is so that an administrator on the system can either approve or reject a data submission before it is written to the public tables. The temporary mirrored tables will be created and destroyed on installation and uninstallation, respectively.

After the data has been submitted and approved, the user will recieve an email with an accession number, which they can use in publications or to link to later. Incomplete submissions and submissions pending approval will still have accession numbers so that a user can leave the site and continue in the same place as their form later, but these accession numbers will not be released to them until after the submission has been completed and approved.

# Features in Development
- View submissions through the user's Tripal account
- Admin Interface for submission approval
- Latitude/Longitude formats for file uploads
- Latitude/Longitude format examples
- Status bar conditions/detection needs to be revised
- Tree ID in phenotype file compared to tree accession
- Phenotype name in phenotype file compared to phenotype metadata file OR phenotype fields

# Resources

[TPPS on TreeGenes!](https://treegenesdb.org/Drupal/master)

[Newest TPPS Flow Concept document](https://docs.google.com/document/d/1fyRlf18j5fq8D2l5Yvx9X-VdUqlrkSvoLC9KGbDnrU8/edit?usp=sharing)

[Old TGDR Submission Pipeline for reference](https://dendrome.ucdavis.edu/tgdr/DataImporter.php)

[Fairly Comprehensive Walkthrough of Drupal Module Dev (Covers Forms, AJAX, and tokens)](https://www.youtube.com/watch?v=bjxML7A19Zs&t=2734s)

[Similar Comprehensive Walkthrough of Drupal Module Dev (Spends some time explining hooks)](https://www.youtube.com/watch?v=dmpSFiCym7c&t=1580s)

[Drupal Form API Reference](https://api.drupal.org/api/drupal/developer%21topics%21forms_api_reference.html/7.x)

[Constructing CHADO queries](http://api.tripal.info/api/tripal/tripal_core%21api%21tripal_core.chado_query.api.inc/group/tripal_chado_query_api/2.x)
