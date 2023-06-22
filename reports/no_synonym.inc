<?php

/**
 * @file
 * Generates list of Phenotypes without Synonyms.
 */

/**
 * Menu callback. Shows list of all phenotypes without synonym.
 */
function tpps_admin_no_synonym_report() {
  $table = 'chado.phenotype';
  $table_alias = str_replace('.', '', $table);
  $page_title = variable_get('tpps_report_no_synonym_title');

  // Query to get Study name.
  $sub_query = db_select('chado.phenotype', 'cte')->fields('cte', []);
  $sub_query->addExpression(
    "substring(cte.uniquename, 0, strpos(cte.uniquename, '-'))",
    'study_name'
  );
  // Get data.
  $base_query = db_select($table, $table_alias)->fields($table_alias, ['name']);
  // 'chado.phenotype_to_synonym'.
  $base_query->leftJoin(
    'chado.phenotype_to_synonym',
    'chadophenotype_to_synonym',
    $table_alias . '.phenotype_id = chadophenotype_to_synonym.phenotype_id'
  );
  $base_query->fields('chadophenotype_to_synonym', ['phenotype_synonyms_id']);
  // Common Table Expression.
  $base_query->addJoin(
    'left outer',
    $sub_query,
    // Full Table Name.
    'public.cte',
    $table_alias . '.phenotype_id = publiccte.phenotype_id'
  );
  // Exclude all names without 'TGDR' in it.
  $base_query->having(
    "strpos(publiccte.study_name, 'TGDR') > 0"
  );
  // Show only phenotypes which do not have synonyms.
  $base_query->having(
    "COALESCE(chadophenotype_to_synonym.phenotype_synonyms_id, 0) = 0"
  );
  // WARNING:
  // Do not add extra fields because grouping by 'study_name' won't work.
  $base_query->groupBy('publiccte.study_name');
  $base_query->groupBy('chadophenotype.name');
  $base_query->groupBy('chadophenotype_to_synonym.phenotype_synonyms_id');

  return '<h2>' . $page_title . '</h2>' . easy_report([
    'formatter' => 'easy_report_formatter',
    'debug_mode' => FALSE,
    'css_path' => drupal_get_path('module', 'tpps') . TPPS_CSS_PATH,
    'page_title' => $page_title,
    'table_attributes' => ['class' => ['view' , 'tpps_table']],
    'header' => [
      [
        'data' => t('Phenotype Name'),
        'field' => $table_alias . '.name',
      ],
      [
        'data' => t('Study'),
        'field' => 'publiccte.study_name',
      ],
    ],
    'base_query' => $base_query,
  ]);
}
