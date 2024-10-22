## Overview

TPPS module has own report system which allows to quickly add new reports.

Main steps to add new report:
1. Come up with report title.
2. Add  variable which stores report's title to install/uninstall hooks.
3. Create report's metadata.
4. Create new or clone existing (and update) report to have a report callback.


## Configuration

* URL: admin/config/tpps/reports'

## Rules

1. Machine name uses underscores. E.g., 'organism_list'.
2. URL uses hyphens. E.g., 'organism-list'.
3. Variable which holds report's title must have prefix 'tpps_report_' and suffix '_title'.
4. Page callback must have prefix 'tpps_' and suffix '_report'.
   E.g., 'tpps_new_organisms_report',
5. Default value of the report title is required.
6. Title stored in non-localized English in variable.

## How to add new report

1. Clone existing report to speed-up creation:
   * for simple db-query: /reports/orgnaism_list.inc
   * for table (no sorting): /reports/doi_duplicates.inc
2. Update code of this new file.
3. Add variables which stores report title to hook_install/uninstall.
4. Add metadata to ```tpps_report_get_list()```.
5. Add ability to change report title at 'admin/config/tpps/reports'.
   See function ```tpps_reports_settings_form()```.
6. Add commented new title variable name to ```tpps_admin_panel_get_reports()```.
7. Use ```hook_update_N()``` to set value of the variable which holds report's title.
8. Clear cache.

Example of the item in function ```tpps_report_get_list()```:
```
    'new_organisms' => [
      'url' => 'new-organisms',
      'title' => variable_get('tpps_report_new_organisms_title', 'New Organisms'),
      'description' => t('List of Studies with new organisms (missing in NCBI database).'),
      'page callback' => 'tpps_new_organisms_report',
      'admin_panel' => TRUE,
    ],
```

## Report types

+ ---------------------------------------_+-------------------------------------------+
| Description                            | Example                                   |
+ ---------------------------------------_+-------------------------------------------+
| simpliest db query without formatter   | tpps_admin_organism_list_report()         |
| db-query with subqueries and formatter | tpps_admin_missing_doi_report()           |
| no-query table with caching            | tpps_admin_submissions_all_files_report() |
+ ---------------------------------------_+-------------------------------------------+
