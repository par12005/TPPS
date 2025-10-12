***************
Plant Accession
***************

Study Location
==============

Only common garden studies will have access to these fields.

* Coordinate Projection: drop-down menu - The coordinate projection of the location of the common garden: 'WGS 84', 'NAD 83', 'ETRS 89', 'Custom Location'. If you don't know your coordinate projection, then it is probably WGS 84.
* Study Location(s): Use add/remove location buttons to add/remove locations. Depending on the choice for coordinate projection, this field may only accept properly formatted coordinate locations.

Accession Information
=====================

The third set of fields in TPPS is the Plant Accession section. Here you will be asked to submit a file or files that describe each plant with a unique identifier and a geographic location.

* These plants may have been studied in the past: checkbox - If this box is checked, TPPS will try to find plants with matching IDs around the same location as the ones being provided. If it finds them successfully, it will mark them as the same plant in the database.
* Skip location validation (ignore location information): checkbox - Available to administrators only. Checking this box will skip validation of location information for the entire study.

* Plant Accession File: file upload - The plant accession file. This file must have at least a column for Plant ID, and columns describing the location of the plants. There are several options for the location column formats, including GPS coordinates and country/state/district. Files that include more than one species will also need to provide columns for the genus and species of each plant. When the file is uploaded, you will need to define the contents of the file. A table with the header 'Define Data' should appear, where you can select which columns describe the required data. If you do not define the required columns, you will not be able to continue.
* Coordinate Projection: drop-down menu - The coordinate projection of coordinate locations in the plant accession file: 'WGS 84', 'NAD 83', 'ETRS 89'. This feature is currently only available for accession files with GPS coordinate locations.
* After uploading the plant accession file and selecting the correct coordinate projection, you can click the button 'Click here to update map' to view the locations of the plants you described on Google Maps. This can be useful to verify that there are no drastic errors in the locations. This feature is currently only available for accession files with GPS coordinate locations.

* Separate Plant Accession: checkbox - If you would like to upload a separate plant accession file for each species, click this checkbox. This field is only available for studies with more than one species.
* Separate Plant Accession Files: file uploads - The plant accession files. These fields are simply duplicates of the 'Plant Accession File' field above.

* The provided GPS coordinates are exact: checkbox - If the coordinates provided in the Accession file are exact, then check this box. Otherwise, leave it uncheck and fill out the following field.
* Coordinates accuracy: textfield - The precision of the provided coordinates. For example, if a plant could be up to 10m away from the provided coordinates, then the accuracy would be "10m".

A screenshot of the Plant Accession page can be seen below:

.. image:: ../../../images/TPPS_accession.png


