TGDR 

Directory Location:
Server: 155.37.254.147
Path:   /var/www/Drupal/sites/all/modules/CustomModule/custom_module
All contents are in this directory, currently no need to look elsewhere.

Status:
All necessary jQeury functionality and error-checking has been successfully implemented.

To do:
Populate forms with information from the database.
Implement login functionality.
Allow users to submit a new species to the database.
Implement a filtering mechanism so users don't have to scroll through thousands of possible options

Issues:
Edge case issues with adding authors.

Resources:

Old TGDR Submission Pipeline for reference: https://dendrome.ucdavis.edu/tgdr/DataImporter.php

Fairly Comprehensive Walkthrough of Drupal Module Dev (Covers Forms, AJAX, and tokens):
https://www.youtube.com/watch?v=bjxML7A19Zs&t=2734s

Similar Comprehensive Walkthrough of Drupal Module Dev (Spends some time explining hooks):
https://www.youtube.com/watch?v=dmpSFiCym7c&t=1580s

Drupal Form API Reference:
https://api.drupal.org/api/drupal/developer%21topics%21forms_api_reference.html/7.x#select

Programmatically populate the contents of select dropdown:
https://www.drupal.org/node/1783904

Constructing CHADO queries:
http://api.tripal.info/api/tripal/tripal_core%21api%21tripal_core.chado_query.api.inc/group/tripal_chado_query_api/2.x
