Project Structure
=================

.. code-block:: CSS

   - admin
       - config.php
       - panel.php
   - ajax
       - tpps_ajax.php
   - css
       - tpps.css
   - forms
       - build
           - front.php
           - page_1.php
           - page_1_ajax.php
           - page_1_helper.php
           - page_2.php
           - page_2_ajax.php
           - page_2_helper.php
           - page_3.php
           - page_3_ajax.php
           - page_3_helper.php
           - page_4.php
           - page_4_ajax.php
           - page_4_helper.php
           - summary.php
       - validate
           - page_1.php
           - page_2.php
           - page_3.php
           - page_4.php
       - submit
           - submit_all.php
   - includes
       - accession_coordinates.inc
       - compare_files.inc
       - completed_display.inc
       - create_record.inc
       - cron.inc
       - file_parsing.inc
       - flatten.inc
       - get_env_data.inc
       - init_project.inc
       - manage_doi.inc
       - parse_xlsx.inc
       - save_file_columns.inc
       - standard_coord.inc
       - status_bar.inc
       - submissions.inc
       - submit_email.inc
       - tab_create.inc
       - table_display.inc
       - validate_columns.inc
       - zenodo.inc
   - js
       - tpps.js
   - tests
       - bootstrap.php
       - DataFactory.php
       - example.env
       - ProjectInitTest.php
   - tpps.info
   - tpps.module
   - tpps.install
   - README.md
   - LICENSE
   - composer.json
   - composer.lock
   - phpunit.xml

The admin/ folder contains code that build the forms to manage TPPS settings and TPPS submissions.
The ajax/ folder contains code for ajax callback functions that need to be accessible to any part of the TPPS module.
The css/ folder contains stylesheets for TPPS, and the js/ folder contains JavaScript that needs to be accessible to any part of the TPPS module.
The forms/ folder contains code for functions that build, validate, and submit the main TPPS form.
The includes/ folder contains code for helper functions that are used throughout the TPPS module.
The tests/ module contains code for unit tests that are run automatically by TravisCI every time the code is pushed. The TPPS project on TravisCI can be found at `this link`_.


.. _this link: https://travis-ci.org/par12005/TPPS

