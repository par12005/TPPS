## Overview

TPPS module has own report system which allows to quickly add new reports.

## How to add new report?

File *.install:
1. Come up with report title.
2. Add  variable which stores report's title to ```tpps_get_variable_list()```.
3. Use ```hook_update_N()``` to set value of the variable which holds report's title.

File /src/Report.class.inc:
4. Create report's metadata in Report::getList().

Report file:
5. Create new or clone existing (and update) report to have a report callback.
   * for simple db-query: /reports/orgnaism/_list.inc
   * for table (no sorting): /reports/doi/duplicates.inc
6. Update code of this new file.
   * Menu callback name must have 'tpps_report_' prefix.

7. Add variable name for new report title to ```tpps_reports_settings_form()```
   as a commented string for searching purposes.
8. To show on TPPS Admin Panel:
   Register report and it's URL in ```tpps_admin_panel_get_reports()```.
9. Clear cache.


## Configuration

* URL: admin/config/tpps/reports'

## Naming rules

1. Machine name uses underscores. E.g., 'organism_list'.
2. URL uses hyphens. E.g., 'organism-list'.
 . Variable which holds report's title must have prefix 'tpps_report_' and suffix '_title'.
4. Page callback must have prefix 'tpps_report_'.
   E.g., 'tpps_report_file_orphan',
5. Default value of the report title is required.
6. Title stored in non-localized English in variable.


## Metatdata example

Example of the item in function ```Report::getList()```:
```
    'organism' => [
      'new' => [
        'title' => variable_get('tpps_report_organism_new_title', 'New Organisms'),
        'description' => t('List of Studies with new organisms (missing in NCBI database).'),
        'admin_panel' => TRUE,
      ],
    ],
```

## Report types

+ ---------------------------------------_+----------------------------------+
| Description                            | Example                           |
+ ---------------------------------------_+----------------------------------+
| simpliest db query without formatter   | tpps_report_organism_list()       |
| db-query with subqueries and formatter | tpps_report_doi_missing()         |
| no-query table with caching            | tpps_report_submission_file_all() |
+ ---------------------------------------_+----------------------------------+
