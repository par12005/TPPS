.. image:: ../_static/images/miappe_logo.png
   :alt: Description of your PNG
    
============================

The TreeGenes Plant Phenotype Submission (TPPS) system has been updated to support submissions that are compliant with the `MIAPPE`_ (Minimum Information About a Plant Phenotyping Experiment) standard. This document outlines the necessary steps and considerations for preparing and submitting MIAPPE-compliant data through TPPS. A lot of MIAPPE information is based on the `ISA Abstract Model`_ that consists of three core entities: Investigation, Study, Assay.

.. image:: ../_static/images/miappe_owl.png
   :alt: MIAPPE Entity Relationship Diagram

.. note::

    colors in flowchart correspond to requriements

Investigation
-------------

Investigations in MIAPPE correspond to TGDRxxxx studies in TPPS. 

.. image:: ../_static/images/miappe_investigation2chado.png
   :alt: Mappings between MIAPPE Investigation and Chado tables

Study
-----

An investigation can have one or more studies. Where a study corresponds to one experiment and its location and duration. Additional information like experimental design, cultural practices, and growth facility.

Biological Material
-------------------

It is recommended to follow the `MCPD`_ (Multi-Crop Passport Descriptors) guidelines plus a unique identifier provided by the institute when preparing biological material information for submission. This also needs to include fields like genus, species, and infraspecific name alongside geographical coordinates. The biological material also should include pretreatments (to the seeds, tree cuttings) prior to starting the experiment. The proveneance information like, forest wild site, laboratory-specific population and any relevant DOI's to gene banks should also be included. 

Biological Unit
-----------------


Biological Variable
--------------------


Integration With Chado
----------------------

.. image:: ../_static/images/chado_mermaid.svg
   :alt: Chado schema

At the investigation level, we can start including relationships between MIAPPE version and study duration (end) with publication accession in dbxref table.  


Exchange Formats To BioSample
------------------------------

EBI BioSample provides a MIAPPE-compliant schema for plant phenotyping data submissions. The schema can be found `EBI_BioSample_Miappe_Schema`_. When preparing data for submission to EBI BioSample, ensure that the data adheres to this schema to facilitate smooth integration and compliance with MIAPPE standards. Once a sample is submitted to BioSample you can view it here: `here`_.


.. image:: ../_static/images/BioSample_schema_ui.jpeg
   :alt: EBI BioSample MIAPPE Schema UI

.. _MIAPPE: https://www.miappe.org/
.. _MCPD: https://openknowledge.fao.org/server/api/core/bitstreams/52a8b5bc-0a5f-47e2-a6c3-3e93434057ae/content
.. _EBI_BioSample_Miappe_Schema: https://www.ebi.ac.uk/biosamples/schemas/certification/plant-miappe.json
.. _EBI_BioStudies_Example: https://www.ebi.ac.uk/biostudies/arrayexpress/studies/E-GEOD-32551
