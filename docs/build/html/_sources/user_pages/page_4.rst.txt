************************************
Genotype, Phenotype, and Environment
************************************

The fourth set of fields in TPPS is the Genotype, Phenotype, and Environment section. Here you will be asked to provide the data and metadata for your phenotypes and genotypes. Each of the fields on this page is asked once per tree species:

Phenotype
=========

Phenotype fields are only visible when 'Genotype x Phenotype', 'Genotype x Phenotype x Environment', or 'Phenotype x Environment' was selected from 'Data type' in `Study Design`_.

* Phenotype Metadata: select one or both of the following methods to provide phenotype metadata:

   * Textfields: click the 'Add Phenotype' or 'Remove Phenotype' buttons to add or remove phenotypes, respecitively. The following fields are required once per phenotype:

      * Phenotype Name: text field - The name of the phenotype.
      * Phenotype Attribute: text field - The attribute that the phenotype is describing. For example, "amount", "width", "mass", "age", "density", "color", "time" would be phenotype attributes.
      * Phenotype Description: text field - A brief description of the phenotype.
      * Phenotype Units: text field - The units of the phenotype. For example, "meters", "cm", "inches", "°C", "Degrees Fahrenheit" would be phenotype units.
      * Phenotype Structure: text field - The structure that the phenotype is describing. If your phenotype has a structure, you can click 'Phenotype has a structure descriptor' and provide a structure. Structure can refer to a tissue type or to a biological process.
      * Phenotype Value Range: text fields - A maximum and minimum value for the phenotype. If your phenotype is binary or has a range, you can click 'Phenotype has a value range' and provide a value range.

   * File: Click the 'I would like to upload a phenotype metadata file' checkbox to upload a phenotype file. You will be shown a table with several drop-down menus, along with the names of your column headers, and the first few rows of data in your file. You will then be asked to select what type of data each of your columns holds: 'Name/Identifier', 'Attribute', 'Description', 'Units', 'Structure', 'Minimum Value', 'Maximum Value', or 'N/A'. Columns marked 'N/A' will still be kept in the flat file, but will not be recorded in the database with the other data from the file. Columns that hold 'Name/Identifier', 'Attribute', 'Description', and 'Units' must be defined before continuing.

* Phenotype Data: file upload - The phenotype data. This file should contain the 'Tree Id' of the tree that the phenotype is describing, the 'Name/Identifier' of the phenotype, and the 'Value' that was actually measured for this phenotype.

.. _`Study Design`: page_2.html

.. image:: ../../../screenshots/TPPS_data.png


