<?php

/**
 * @file
 * Manages reports.
 */

/**
 * Menu callback. Shows list of TPPS Reports.
 *
 * See
 * tpps_menu_add_report_pages();
 * tpps_admin_panel_reports();
 *
 * @param bool $admin_panel
 *   Determines if only items for 'Admin Panel' must be returned.
 *
 * @return string
 *   Returns rendered list of links to report pages with descriptions.
 */
function tpps_reports_overview_page($admin_panel = FALSE) {
  return tpps_report_build_overview_page(tpps_report_get_list(), [], $admin_panel);
}

/**
 * Builds list of items for report overview page.
 *
 * @param array $list
 *   Array with data.
 * @param bool $admin_panel
 *   Determines if only items for 'Admin Panel' must be returned.
 */
function tpps_report_build_overview_page(array $list, array $parents, $admin_panel = FALSE) {
  $items = [];
  foreach ($list as $key => $value) {
    if (empty($value['title'])) {
      $output = tpps_report_build_overview_page($value, array_merge($parents, [$key]), $admin_panel);
      // This a list and it requires some text before to be shown correctly.
      $items[] = '<strong>' . check_plain(strtoupper($key)) . ':</strong> ' . $output;
    }
    else {
      // Build URL.
      $path = $key;
      if (isset($parents[0])) {
        $path = implode('/', $parents) . '/' . $key;
      }

      $output = l(t($value['title']), 'admin/reports/tpps/' . $path,
        ['attributes' => ['title' => $value['description']]]
      );
      if (!$admin_panel) {
        $output = '<strong>' . $output . '</strong><br /><em>' . $value['description'] . '</em>';
      }
      $items[] = $output;
    }
  }
  return theme('item_list', ['items' => $items]);
}

/**
 * Adds report menu items for hook_menu().
 *
 * To add new item: see reports/README.md.
 *
 * @param array $items
 *   Menu items.
 *
 * @todo Minor. Rename folder 'reports' to 'report'.
 */
function tpps_menu_add_report_pages(array &$items = []) {
  $items = $items ?? [];
  // Add parent menu item.
  $items['admin/reports/tpps'] = [
    'title' => 'TPPS Reports',
    'description' => 'TPPS module reports',
    'page callback' => 'tpps_reports_overview_page',
    'access arguments' => ['administer tpps module'],
  ];

  foreach (tpps_report_get_list() as $name => $item) {
    tpps_report_add_submenu_item($items, [], $name, $item);
  }
}

/**
 * Recursively adds report menu items.
 *
 * @param array $items
 *   Drupal menu item list.
 * @param array $parents
 *   List of parent element names.
 * @param string $name
 *   Current element name.
 * @param array $item
 *   Item. Could be a single item metadata or list of subelements.
 */
function tpps_report_add_submenu_item(array &$items, array $parents, $name, array $item) {
  if (empty($item['title'])) {
    // List of sub-items.
    $parents[] = $name;
    foreach ($item as $sub_name => $sub_item) {
      tpps_report_add_submenu_item($items, $parents, $sub_name, $sub_item);
    }
    return;
  }

  if (isset($parents[0])) {
    // @TODO Minor. Convert other reports to use the same filename.
    $path = implode('/', $parents) . '/' . $name;
    $page_callback = 'tpps_report_' . implode('_', $parents) . '_' . $name;
  }
  else {
    // Outdated reports.
    $path = $name;
    $page_callback = 'tpps_admin_' . $name . '_report';
  }
  $items['admin/reports/tpps/' . $path] =
  $items['tpps-admin-panel/reports/' . $path] = [
    'title' => $item['title'],
    'description' => $item['description'],
    'page callback' => $page_callback,
    'access callback' => 'tpps_access',
    'access arguments' => ['administer tpps module'],
    'file' => 'reports/' . $path . '.inc',
  ];
}

/**
 * Gets reports meta-data.
 *
 * See tpps_reports_overview_page() which builds list of report links.
 * WARNING: Do not use 's' in the report's metadata.
 *
 * To add new item:
 * 1. Insert item below.
 * 2. Add variable name into tpps_reports_settings_form().
 *
 * @todo Use nested list instead of plain and remove 'url' item.
 */
function tpps_report_get_list() {
  return [
    // Files.
    'missing_files' => [
      'url' => 'missing-files',
      'title' => variable_get('tpps_report_missing_files_title',
        'Files missing in filesystem'
      ),
      'description' => t("List of all (including submission's) files missing "
      . "in filesystem but present in DB table 'file_managed'."),
      'admin_panel' => TRUE,
    ],
    // Misc.
    'variable_list' => [
      'url' => 'variable-list',
      'title' => variable_get('tpps_report_variable_list_title',
        'TPPS module variables'
      ),
      'description' => t("List of TPPS module's variables with prefix 'tpps'."),
      'admin_panel' => FALSE,
    ],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Synonym.
    'synonym' => [
      'missing' => [
        'title' => variable_get('tpps_report_synonym_missing_title',
          'List of phenotypes without Synonyms'),
        'description' => t('List of phenotypes without synonym.'),
        'admin_panel' => TRUE,
      ],
      'unit_warning' => [
        'title' => variable_get('tpps_report_unit_warning_title',
        'Unit Warning (list of phenotypes which unit differs from Synonym)'
        ),
        'description' => t("List of Phenotypes which differs from Synonym's Unit."),
        'admin_panel' => TRUE,
      ],
    ],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Organism.
    'organism' => [
      'list' => [
        'title' => variable_get('tpps_report_organism_list_title',
          'Organism List'
        ),
        'description' => t('List of organisms per study.'),
        'admin_panel' => FALSE,
      ],
      'new' => [
        'title' => variable_get('tpps_report_organism_new_title', 'New Organisms'),
        'description' => t('List of Studies with new organisms (missing in NCBI database).'),
        'admin_panel' => TRUE,
      ],
      'order_family_not_exist' => [
        'title' => variable_get('tpps_report_organism_order_family_not_exist_title',
          'Order/Family not exist'),
        'description' => t('Lists non-existing order/family.'),
        'admin_panel' => TRUE,
      ],
    ],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // DOI.
    'doi' => [
      'duplicates' => [
        'title' => variable_get('tpps_report_doi_duplicates_title', 'DOI Duplicates'),
        'description' => t('List of submissions with the same Publication DOI.'),
        'admin_panel' => TRUE,
      ],
      'missing' => [
        'title' => variable_get('tpps_report_doi_missing_title', 'Missing DOI'),
        'description' => t('List of Studies without DOI attached.'),
        'admin_panel' => TRUE,
      ],
    ],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Zenodo.
    'zenodo' => [
      'deposition' => [
        'title' => variable_get('tpps_report_zenodo_deposition_title', 'Zenodo Depositions'),
        'description' => t('List of Zenodo Depositions related to current account.'),
        'admin_panel' => TRUE,
      ],
    ],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Submission.
    'submission' => [
      'imported' => [
        'title' => variable_get('tpps_report_submission_imported_title', 'Imported Submissions'),
        'description' => t('List of accession of imported submissions.'),
        'admin_panel' => TRUE,
      ],
      'zenodo' => [
        'title' => variable_get('tpps_report_submission_zenodo_title', 'Submissions with Zenodo Deposit'),
        'description' => t('List of Submissions which has Zenodo Depositions.'),
        'admin_panel' => TRUE,
      ],
      'file' => [
        'all' => [
          'title' => variable_get('tpps_report_submission_file_all_title',
            "Submission's All Files"
          ),
          'description' => t('List of all (existing and missing) File Ids for ALL submissions'),
          'admin_panel' => TRUE,
        ],
        'none' => [
          'title' => variable_get('tpps_report_submission_file_none_title',
            'Submissions without Files'),
          'description' => t('List of submissions which has no files.'),
          'admin_panel' => TRUE,
        ],
        'missing' => [
          'title' => variable_get('tpps_report_submission_file_missing_title',
            "Submission's Missing Files"
          ),
          'description' => t('List submissions which has files but those files '
          . 'are missing in filesystem.'),
          'admin_panel' => TRUE,
        ],
      ],
    ],
  ];
}
