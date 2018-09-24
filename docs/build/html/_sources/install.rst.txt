Installation
============

1. TPPS requires the following modules:

  - Tripal (v3.x)
  - Tripal Chado
  - Tripal Jobs
  - Tripal Jobs Daemon
  - Ultimate Cron

  TPPS will install Ultimate Cron automatically, but Tripal and Tripal core modules must be installed before installing TPPS.

2. Inside your /sites/all/modules directory, download TPPS by running:

  ``git clone https://gitlab.com/TreeGenes/TGDR.git``
 
3. Then, enable TPPS by running:

  ``drush en tpps``

For more instructions, please see the `Drupal documentation page`_.

.. _Drupal documentation page: https://www.drupal.org/node/120641

