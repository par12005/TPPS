<?php

/**
 * @file
 * Contains 'Report' class.
 *
 * Documentation: /reports/README.md.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * How to use?
 *
 * Array and SQL-query (sorting):
 * - Report::print();
 * - Report::print2Level();
 */

/**
 * Manages Drupal Reports.
 *
 * @package tpps
 * @author Vlad Savistky <vlad.savitsky@gmail.com>
 */
class Report {

  // Names of the file groups (and columns in reports).
  const TREE_ACCESSION_FILES = 'tree-accession';
  const PHENOTYPE_FILES = 'phenotype';
  const GENOTYPE_FILES = 'genotype';
  const OTHER_FILES = 'other';

  const MISSING_FILES = 'missing';
  const FILE_ID_LIST = 'id_list';

  /**
   * Clears report's cached data.
   *
   * @param string $name
   *   Report's machine name or function's name (report's callback).
   *   For example:
   *   'tpps_report_organism_synonym_list' or 'organism_synonym_list'.
   *   See methods: getCallbackName(), getList().
   *
   * @return mixed
   *   Returns cached data or NULL if there is no cached data.
   */
  public static function getCache($name) {
    $cid = $name;
    if (strpos($name, 'tpps_report_') === FALSE) {
      // $name contains report name (not function's name).
      $cid = 'tpps_report_' . $name;
    }
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    $cache = cache_get($cid, $cache_bin);
    return $cache->data;
  }

  /**
   * Clears report's cached data.
   *
   * @param string $name
   *   Report's machine name or function's name (report's callback).
   *   For example:
   *   'tpps_report_organism_synonym_list' or 'organism_synonym_list'.
   *   See methods: getCallbackName(), getList().
   *
   * @return bool
   *   Returns TRUE if cache was actually cleared and FALSE otherwise.
   */
  public static function resetCache($name) {
    $cid = $name;
    if (strpos($name, 'tpps_report_') === FALSE) {
      // $name contains report name (not function's name).
      $cid = 'tpps_report_' . $name;
    }
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    $cache = cache_get($cid, $cache_bin);
    // Note: not all repoorts are using caching so we need to check.
    if (!empty($cache) && !empty($cache->data)) {
      $cache->data = NULL;
      cache_set($cid, $cache->data, $cache_bin);
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Values formatter for 'Report::print()'.
   *
   * @param string $name
   *   Required. Field name.
   *   Machine table field name (column name) without table name and dot.
   * @param mixed $value
   *   Field value.
   * @param array $row
   *   Data of the whole table row with hidden fields.
   *   This data could be used to format other fields. For example,
   *   hidden field 'uid' or 'entity_id' could be used to build a link to
   *   user profile page.
   *
   * @return mixed
   *   Returns formatted value.
   *   Empty value will not be formatted and returned as is.
   */
  public static function exampleFormatter(string $name, $value, array $row) {
    $formatted = check_plain($value);
    if (empty($name) || empty($value)) {
      return $formatted;
    }
    $name = check_plain($name);
    switch ($name) {
      case 'publicctestudy_name':
        if (!empty($value)) {
          $formatted = l($formatted, 'tpps-admin-panel/' . $formatted);
        }
        break;
    }
    return $formatted;
  }

  /**
   * Prints sortable table.
   *
   * Supports sorting, formatter.
   *
   * @param array $report
   *   Report's metadata. Required keys:
   *   - 'report_name' string.
   *     Machine report name (with underscores).
   *   - 'header' array
   *     List of the table's header labels.
   *   - 'rows' array
   *     Optional. Required if 'query' item is empty.
   *     Table data to be shown.
   *
   *   - 'query' object
   *     Optional. Required if 'roww' item is empty.
   *     Drupal DB Query object.
   *     WARNING: Requires db_select() not db_query().
   *
   *   - 'empty' string
   *     Optional. Localized string which will be shown when 'rows' is empty.
   *
   *   - 'pager' mixed
   *      Defines where pager will be shown. Possible values:
   *       'both', 'full': at the top and bottom.
   *       'below', 'bottom': at the bottom only.
   *       'above', 'top': at the top only.
   *       'none', FALSE, NULL: hidden.
   *
   *   - 'items_per_page' int
   *     Opitonal. Default is 25.
   *     Number of rows show at single page when pager is used.
   *     Global setting for all reports could be overwritten by Drupal variable:
   *     'tpps_report_items_per_page' at '/admin/config/tpps/reports' page.
   *   - 'css_path' string
   *     Optional. Absolute path to the CSS file. See drupal_add_css().
   *   - 'page_title' string
   *     Page title.
   *
   *   Example of the sortable table:
   *   $report['header'] = [
   *     ['data' => t('Name'), 'field' => 'name'],
   *     // Default sort for this column
   *     ['data' => t('Age'), 'field' => 'age', 'sort' => 'desc'],
   *     ['data' => t('City'), 'field' => 'city'],
   *   ];
   *   $report['rows'] = [
   *     ['name' => 'Alice', 'age' => 30, 'city' => 'New York'],
   *     ['name' => 'Bob', 'age' => 25, 'city' => 'London'],
   *     // ... more data
   *   ];
   *
   *   Regular (flat) table:
   *   - $header is a list of strings,
   *   - $rows is a list of values.
   *   Sortable table:
   *   - $header is an associative array with named keys,
   *   - $rows - each item (row) is an associative array with the same named
   *      keys as $header has.
   *
   * @return string
   *   Returns rendered HTML sortable table with pager.
   */
  public static function print(array $report) {
    // Check required items.
    if (
      // Header is missing.
      empty($report['header'])
      // OR when either query OR rows is not empty.
      || !(isset($report['query']) || isset($report['rows']))
    ) {
      drupal_set_message(t('Required data to build report is missing.'), 'error');
      return '';
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Default values.
    $debug_mode = check_plain($_GET['debug_mode'] ?? $report['debug_mode'] ?? FALSE);
    $pager = check_plain($_GET['pager'] ?? $report['pager'] ?? 'none');
    $report['items_per_page'] = check_plain($_GET['items_per_page']
      ?? $report['items_per_page']
      ?? variable_get('tpps_report_items_per_page', 20));
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Formatter.
    if (
      !empty($report['name'])
      && ($formatter_function = 'tpps_report_' . $report['name'] . '_formatter')
      && function_exists($formatter_function)
    ) {
      $formatter = check_plain($formatter_function);
      $is_formatter = TRUE;
    }
    else {
      $formatter = check_plain($report['formatter'] ?? 'Report::formatter');
      $is_formatter = (!empty($formatter) && function_exists($formatter));
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Data in array.
    if (empty($report['query'])) {
      // Sorting.
      $ts = tablesort_init($report['header']);
      if (isset($ts['sql'])) {
        $sort_field = $ts['sql'];
        uasort($report['rows'], function($a, $b) use ($sort_field) {
          $valA = is_array($a[$sort_field])
            ? implode(' ', $a[$sort_field])
            : (string) $a[$sort_field];
          $valB = is_array($b[$sort_field])
            ? implode(' ', $b[$sort_field])
            : (string) $b[$sort_field];
          return strnatcmp($valA, $valB);
        });
        // Reverse the array if the sort direction is 'desc'.
        if (strtolower($ts['sort']) == 'desc') {
          $report['rows'] = array_reverse($report['rows'], TRUE);
        }
      }

    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Data in SQL-query.
    else {
      $query = $report['query'];
      if (!empty($report['debug_mode'])) {
        $query->addTag('debug');
      }
      // Pager and sorting.
      if ($pager == 'none' || $pager === FALSE) {
        // No pager at all.
        $ts = tablesort_init($report['header']);
        if (isset($ts['sql'])) {
          $query->orderBy($ts['sql'], $ts['sort']);
        }
        $report['rows'] = $query->execute()->fetchAll();
      }
      else {
        $report_query = $query->extend('PagerDefault')
          ->limit($report['items_per_page'])
          ->extend('TableSort')
          ->orderByHeader($report['header']);
        $report['rows'] = $report_query->execute()->fetchAll();
      }
    }

    // Prepare rows for report. Formatter.
    $rows = [];
    foreach ($report['rows'] as $row_id => $row) {
      foreach ($row as $field_id => $field_value) {
        $rows[$row_id][$field_id] = [
          'data' => (!$is_formatter ? $field_value
            : call_user_func($formatter, $field_id, $field_value, (array) $row)
          ),
          'style' => 'vertical-align:top;min-width:10px;',
        ];
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Add 'main' JS and CSS.
    tpps_add_css_js();
    // Cuostom CSS.
    if (!empty($report['css_path'])) {
      drupal_add_css($report['css_path']);
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Pager.
    if ($pager == 'none' || $pager === FALSE) {
      // No pager.
      $rendered_pager = '';
    }
    else {
      // Pager.
      if (!empty($report['query'])) {
        // SQL-query.
        $rendered_pager = theme('pager');
      }
      else {
        // Array.
        $current_page = pager_default_initialize(
          count($rows), $report['items_per_page']
        );
        $chunks = array_chunk($rows, $report['items_per_page'], TRUE);
        $rendered_pager = theme('pager', ['quantity', count($rows) ?? 0]);
        $rows = $chunks[$current_page] ?? [];
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Page title.
    if (!empty($report['page_title'])) {
      // @TODO Remove if possible.
      $content = '<h2>' . $report['page_title'] . '</h2>';
      drupal_set_title($report['page_title']);
    }
    else {
      $content = '';
      $page_title = variable_get('tpps_report_' . ($report['name'] ?? '') . '_title');
      drupal_set_title($page_title);
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Content.
    $content .= theme('table', [
      // @TODO Minor. $report_name now is not string but array of parents.
      // 'caption' => Report::getList()[$report_name]['description'],
      'header' => $report['header'] ?? [],
      'rows' => $rows,
      // To have vertical align to top of the cell.
      'attributes' => ['class' => ['view', 'tpps_table']],
      'empty' => $report['empty'] ?? t('No data found.'),
      'sticky' => TRUE,
    ]);


    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Pager position.
    if (in_array($pager, ['both', 'full']) || is_null($pager)) {
      $content = $rendered_pager . $content . $rendered_pager;
    }
    elseif (in_array($pager, ['below', 'bottom'])) {
      $content = $content . $rendered_pager;
    }
    elseif (in_array($pager, ['above', 'top'])) {
      $content = $rendered_pager . $content;
    }
    if (!empty($report['submission_cache'])) {
      $content .= SubmissionCache::showClearCacheWidget();
    }
    return $content;
  }

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
  public static function printOverviewPage($admin_panel = FALSE) {
    return static::buildOverviewPage(static::getList(), [], $admin_panel);
  }

  /**
   * Builds list of items for report overview page.
   *
   * @param array $list
   *   Array with data.
   * @param array $parents
   *   List of parents.
   * @param bool $admin_panel
   *   Determines if only items for 'Admin Panel' must be returned.
   */
  public static function buildOverviewPage(array $list, array $parents, $admin_panel = FALSE) {
    $items = [];
    foreach ($list as $key => $value) {
      if (empty($value['title'])) {
        $output = static::buildOverviewPage($value, array_merge($parents, [$key]), $admin_panel);
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

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Menu.

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
  public static function addAllMenuItems(array &$items = []) {
    $items = $items ?? [];
    // Add parent menu item.
    $items['admin/reports/tpps'] = [
      'title' => 'TPPS Reports',
      'description' => 'TPPS module reports',
      'page callback' => 'Report::printOverviewPage',
      'access arguments' => ['administer tpps module'],
    ];

    foreach (static::getList() as $name => $item) {
      static::addSubMenuItem($items, [], $name, $item);
    }
    return $items;
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
  public static function addSubMenuItem(array &$items, array $parents, $name, array $item) {
    if (empty($item['title'])) {
      // Create parent menu item.
      // @TODO Show sub-items.
      $path = static::getMenuPath($name, $parents);
      $items['admin/reports/tpps/' . $path] =
      $items['tpps-admin-panel/reports/' . $path] = [
        'title' => check_plain(ucfirst($name)),
        'page callback' => 'Report::showItems',
        'access callback' => 'tpps_access',
        'access arguments' => ['administer tpps module'],
      ];
      // List of sub-items.
      $parents[] = $name;
      foreach ($item as $sub_name => $sub_item) {
        static::addSubMenuItem($items, $parents, $sub_name, $sub_item);
      }
      return;
    }

    $page_callback = static::getCallbackName($name, $parents);
    $path = static::getMenuPath($name, $parents);
    $items['admin/reports/tpps/' . $path] =
    $items['tpps-admin-panel/reports/' . $path] = [
      'title' => $item['title'],
      'description' => $item['description'],
      'page callback' => $page_callback,
      'access callback' => 'tpps_access',
      'access arguments' => ['administer tpps module'],
      'file' => static::getFilePath($name, $parents),
    ];
  }

  /**
   * Gets menu item path.
   *
   * @param string $name
   *   Item name.
   * @param array $parents
   *   List of the menu parents.
   *
   * @return string
   *   Returns path.
   */
  public static function getMenuPath($name, array $parents = []) {
    if (isset($parents[0])) {
      // @TODO Minor. Convert other reports to use the same filename.
      return implode('/', $parents) . '/' . $name;
    }
    else {
      // Outdated reports.
      return $name;
    }
  }

  /**
   * Gets path to the report's file.
   *
   * @param string $name
   *   Menu item name.
   * @param array $parents
   *   Menu item parents.
   *
   * @return string
   *   Returns path to the file which contains helper code to process report.
   */
  public static function getFilePath($name, array $parents = []) {
    return 'reports/' . static::getMenuPath($name, $parents) . '.inc';
  }

  /**
   * Gets name of the report callback.
   *
   * @param string $name
   *   Report name.
   * @param array $parents
   *   Report's parents like ['submission', 'file', 'all'].
   *
   * @return string
   *   Returns callback name.
   */
  public static function getCallbackName($name, array $parents = []) {
    if (isset($parents[0])) {
      return 'tpps_report_' . implode('_', $parents) . '_' . $name;
    }
    else {
      // Outdated reports.
      return 'tpps_admin_' . $name . '_report';
    }
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
  public static function getList() {
    return [
      // Files.
      'file' => [
        'missing' => [
          'title' => variable_get('tpps_report_file_missing_title',
            'Files missing in filesystem'
          ),
          'description' => t("List of all (including submission's) files missing "
          . "in filesystem but present in DB table 'file_managed'."),
          'admin_panel' => TRUE,
        ],
        'not_used' => [
          'title' => variable_get('tpps_report_file_not_used_title',
            'Not used files'
          ),
          'description' => t("List of the files which not used by any study."),
          'admin_panel' => TRUE,
        ],
        'orphan' => [
          'title' => variable_get('tpps_report_file_orphan_title',
            'Orphan files'
          ),
          'description' => t('List of the files which do not belong to any submission.'),
          'admin_panel' => TRUE,
        ],
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
        'list' => [
          'title' => variable_get('tpps_report_synonym_list_title',
            'Phenotype Synonyms'),
          'description' => t('List of all phenotype synonyms.'),
          'admin_panel' => TRUE,
        ],
        'missing' => [
          'title' => variable_get('tpps_report_synonym_missing_title',
            'List of phenotypes without Synonyms'),
          'description' => t('List of phenotypes without synonym.'),
          'admin_panel' => TRUE,
        ],
        'unit_warning' => [
          'title' => variable_get('tpps_report_synonym_unit_warning_title',
          'Unit Warning (list of phenotypes which unit differs from Synonym)'
          ),
          'description' => t("List of Phenotypes which differs from Synonym's Unit."),
          'admin_panel' => TRUE,
        ],
      ],
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Organism.
      'organism' => [
        'all' => [
          'title' => variable_get('tpps_report_organism_all_title',
            'All Organism List'
          ),
          'description' => t('List of the all organisms.'),
          'admin_panel' => FALSE,
        ],
        'taxonomy' => [
          'title' => variable_get('tpps_report_organism_taxonomy_title',
            "All Organism's Taxonomy"
          ),
          'description' => t('List of the all organisms.'),
          'admin_panel' => FALSE,
        ],
        'synonym_list' => [
          'title' => variable_get('tpps_report_organism_synonym_list_title',
            "Organism Synonym List"
          ),
          'description' => t('List of the synonyms for each organism.'),
          'admin_panel' => FALSE,
        ],
        'genus' => [
          'title' => variable_get('tpps_report_organism_genus_title',
            'Organism Genus List'
          ),
          'description' => t("List of all the organism's genuses and number of species in it."),
          'admin_panel' => FALSE,
        ],
        'species' => [
          'title' => variable_get('tpps_report_organism_species_title',
            'Organism Species List'
          ),
          'description' => t("List of all the organism's species in genus."),
          'admin_panel' => FALSE,
        ],
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
        'ncbi' => [
          'all' => [
            'title' => variable_get('tpps_report_organism_ncbi_all_title',
              'All Organisms and their NCBI Taxonomy Ids'),
            'description' => t('List of all organisms and their NCBI Taxonomy Ids.'),
            'admin_panel' => FALSE,
          ],
          'missing' => [
            'title' => variable_get('tpps_report_organism_ncbi_missing_title',
              'Organisms without  NCBI Taxonomy Ids'),
            'description' => t('List of organisms which are missing NCBI Taxonomy Ids.'),
            'admin_panel' => FALSE,
          ],
        ],
        'gbif' => [
          'all' => [
            'title' => variable_get('tpps_report_organism_gbif_all_title',
              'All Organisms and their GBIF Taxonomy Ids'),
            'description' => t('List of all organisms and their GBIF Taxonomy Ids.'),
            'admin_panel' => FALSE,
          ],
          'missing' => [
            'title' => variable_get('tpps_report_organism_gbif_missing_title',
              'Organisms without  GBIF Taxonomy Ids'),
            'description' => t('List of organisms which are missing GBIF Taxonomy Ids.'),
            'admin_panel' => FALSE,
          ],
        ],
      ],
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // DOI.
      'doi' => [
        'submission' => [
          'publication' => [
            'title' => variable_get('tpps_report_doi_submission_publication_title', 'Publication DOI'),
            'description' => t('List of submissions which has Publication DOI.'),
            'admin_panel' => TRUE,
          ],
          'dataset' => [
            'title' => variable_get('tpps_report_doi_submission_dataset_title', 'Dataset DOI'),
            // Note: Zenodo Deposition shown different data.
            'description' => t('List of submissions which has Dataset DOI.'),
            'admin_panel' => TRUE,
          ],
          'all' => [
            'title' => variable_get('tpps_report_doi_submission_all_title', 'All DOI'),
            // Note: Zenodo Deposition shown different data.
            'description' => t('List of submissions which has Publication/Dataset DOI.'),
            'admin_panel' => TRUE,
          ],
        ],
        'publication_data' => [
          'all' => [
            'title' => variable_get(
              'tpps_report_doi_publication_data_all_title',
              'All DOI Publication Data'
            ),
            'description' => t('Shows publication data publication_data from all sources.'),
            'admin_panel' => TRUE,
          ],
          'google_scholar' => [
            'title' => variable_get(
              'tpps_report_doi_publication_data_google_scholar_title',
              'Google Scholar DOI Publication Data'
            ),
            'description' => t('Shows publication data from Gooble Scholar web-site.'),
            'admin_panel' => TRUE,
          ],
          'pubmed' => [
            'title' => variable_get(
              'tpps_report_doi_publication_data_pubmed_title',
              'PubMed DOI Publication Data'
            ),
            'description' => t('Shows publication data from PubMed web-site.'),
            'admin_panel' => TRUE,
          ],
          'pubmed_central' => [
            'title' => variable_get(
              'tpps_report_doi_publication_data_pubmed_central_title',
              'PubMedCentral DOI Publication Data'
            ),
            'description' => t('Shows publication data from PubMedCentral API.'),
            'admin_panel' => TRUE,
          ],
          'crossref' => [
            'title' => variable_get(
              'tpps_report_doi_publication_data_crossref_title',
              'CrossRef DOI Publication Data'
            ),
            'description' => t('Shows publication data from CrossRef API.'),
            'admin_panel' => TRUE,
          ],
          'dryad' => [
            'title' => variable_get(
              'tpps_report_doi_publication_data_dryad_title',
              'Dryad DOI Publication Data'
            ),
            'description' => t('Shows publication data from Dryad API.'),
            'admin_panel' => TRUE,
          ],
          'semantic_scholar' => [
            'title' => variable_get(
              'tpps_report_doi_publication_data_semantic_scholar_title',
              'Semantic Scholar DOI Publication Data'
            ),
            'description' => t('Shows publication data from Semantic Scholar API.'),
            'admin_panel' => TRUE,
          ],
        ],
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
        'version' => [
          'title' => variable_get('tpps_report_submission_version_title', 'Submission Version Changes'),
          'description' => t('Changes in given Submission Version.'),
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
          'type' => [
            'title' => variable_get('tpps_report_submission_file_type_title',
              'Submissions File Type'),
            'description' => t('List of submissions with files of the certain type.'),
            'admin_panel' => TRUE,
          ],
        ],
      ],
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Rest API.
      'rest_api' => [
        'request' => [
          'list' => [
            'title' => variable_get(
              'tpps_report_rest_api_request_list_title',
              'Rest API Request List'
            ),
            'description' => t('List of the all Rest API Requests.'),
            'admin_panel' => TRUE,
          ],
          'single' => [
            'title' => variable_get(
              'tpps_report_rest_api_request_single_title',
              'Rest API Request Body'
            ),
            'description' => t('Rest API Request.'),
            'admin_panel' => FALSE,
          ],
        ],
        'response' => [
          'list' => [
            'title' => variable_get(
              'tpps_report_rest_api_response_list_title',
              'Rest API Response List'
            ),
            'description' => t('List of the all Rest API Responses.'),
            'admin_panel' => TRUE,
          ],
          'body' => [
            'title' => variable_get(
              'tpps_report_rest_api_response_body_title',
              'Rest API Response Body'
            ),
            'description' => t('Rest API Response Body.'),
            'admin_panel' => FALSE,
          ],
        ],
      ],
    ];
  }

  /**
   * Builds single row of the report.
   *
   * @param mixed $name
   *   Report name. E.g., 'all' for /admin/reports/tpps/submission/file/all.
   * @param array $parents
   *   Report's parents. E.g., ['submission', 'file'].
   * @param array $context
   *   Data necessary to build single row.
   *
   * @return array
   *   Returns meta-data which could be used to theme single row.
   */
  public static function buildRow($name, array $parents, array $context = []) {
    $file_name = str_replace('.inc', '', static::getFilePath($name, $parents));
    module_load_include('inc', 'tpps', $file_name);
    $callback = static::getCallbackName($name, $parents)
      . '_build_single_row';

    if (function_exists($callback)) {
      return call_user_func($callback, $context);
    }
    else {
      watchdog('tpps', 'Report callback @callback not found.',
        ['@callback' => $callback], WATCHDOG_WARNING
      );
    }
  }

  /**
   * Builds single row of the report.
   *
   * @param mixed $name
   *   Report name. E.g., 'all' for /admin/reports/tpps/submission/file/all.
   * @param array $parents
   *   Report's parents. E.g., ['submission', 'file'].
   * @param array $context
   *   Data necessary to build single row.
   *
   * @return array
   *   Returns meta-data which could be used to theme single row.
   */
  public static function themeRow($name, array $parents, array $context = []) {
    $file_name = str_replace('.inc', '', static::getFilePath($name, $parents));
    module_load_include('inc', 'tpps', $file_name);
    $callback = static::getCallbackName($name, $parents)
      . '_theme_single_row';
    if (function_exists($callback)) {
      return call_user_func($callback, $context);
    }
  }

  /**
   * Get Cache Key (cid) for report.
   *
   * @param string $name
   *   Report name.
   * @param array $parents
   *   Parents of the report.
   *
   * @return string
   *   Returns Cache Key which used to store data to build a report.
   */
  public static function getCacheKey($name, array $parents = []) {
    if (empty($name)) {
      return '';
    }
    $parent_string = (empty($parents) ? '' : implode('_', $parents) . '_');
    return 'tpps_report_' . $parent_string . $name;
  }

  /**
   * Menu callback for report's parent pages.
   *
   * Shows menu subitems.
   *
   * @return string
   *   Returns rendered list of the menu sub-items.
   */
  public static function showItems() {
    $current_path = arg();
    $report_list = static::getList();
    foreach ($current_path as $item) {
      if (in_array($item, ['admin', 'reports', 'tpps'])) {
        continue;
      }
      // @TODO Implement multiple levels support.
      $sub_item_list = TppsArray::get($report_list, [$item]);
      if (!empty($sub_item_list)) {
        return static::buildOverviewPage($sub_item_list, [$item]);
      }
    }
    return "Empty";
  }

  /**
   * Builds report using nested data structure.
   *
   * 1st level: fieldsets.
   * 2nd level: table's columns. Name is a header of the column.
   *
   * @param array $form
   *   Required. Drupal Form array. Will be updated by reference.
   * @param array $data
   *   Optional. Data for the report.
   *   Data structure is 2 level nested.
   *   Last level has an array of arrays to be compared.
   */
  public static function print2Level(array &$form, array $data = NULL) {
    if (empty($data)) {
      $form['info'] = [
        '#markup' => t('No data'),
      ];
    }
    else {
      foreach ($data as $level1_key => $level1_item) {
        $header = [' '];
        $rows = [];
        foreach ($level1_item as $level2_key => $level2_item) {
          $header[] = $level2_key;
          foreach ($level2_item as $key => $value) {
            if (empty($rows[$key])) {
              $rows[$key][] = '<strong>' . $key . '</strong>';
            }
            $rows[$key][] = $value;
          }
        }
        $form[$level1_key] = [
          '#type' => 'fieldset',
          '#title' => $level1_key,
          '#collapsible' => TRUE,
          '#collapsed' => 0,
        ];
        $form[$level1_key][$level2_key] = [
          '#markup' => theme('table', [
            'header' => $header,
            'rows' => $rows,
            'empty' => t('No data'),
          ]),
        ];
      }
    }
  }

}
