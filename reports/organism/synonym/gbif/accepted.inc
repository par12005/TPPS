<?php

/**
 * @file
 * Shows list of the accepted organism synonym names.
 */

/**
 * Menu callback. Shows list of the accepted organism synonym names.
 *
 * URL: /admin/reports/tpps/organism/synonym/gbif/accepted.
 *
 * @param bool $reset
 *   Flag is cache must be resetted. Default if FALSE.
 */
function tpps_report_organism_synonym_gbif_accepted($reset = FALSE) {
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  if (isset($_GET['clear'])) {
    GbifSpeciesApi::clearAcceptedSynonymList();
  }
  module_load_include('inc', 'tpps', 'reports/organism/synonym/ncbi/accepted');
  return tpps_report_organism_synonym_accepted_helper(
    'organism_synonym_gbif_accepted',
    GbifSpeciesApi::getAcceptedSynonymList(),
    $reset
  );
}

/**
 * Values formatter for tpps_report_organism_taxonomy().
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
function tpps_report_organism_synonym_gbif_accepted_formatter(string $name, $value, array &$row) {
  $formatted = check_plain($value);
  if (empty($name) || empty($value)) {
    return $formatted;
  }
  $name = check_plain($name);
  switch ($name) {
    case 'synonym_id':
    case 'primary_id':
      $formatted = l($formatted, 'admin/reports/db/chado.organism',
        ['query' => ['chado.organism.organism_id' => $formatted]]
      );
      if ($value <= 2616 && $name == 'primary_id') {
        $formatted = '[ <em title="Existing">' . $formatted . '</em> ]';
      }
      break;

    case 'synonym_name':
    case 'primary_name':
      // Add Taxonomy Id in brackets.
      if (!empty($value)) {
        $response = GbifSpeciesApi::getInfo($formatted);
        if (($response['diagnostics']['matchType'] ?? NULL) != 'NONE') {
          $taxonomy_id = $response['usage']['key'];
          $formatted = $formatted . ' (<em>'
            . l($taxonomy_id, GbifSpeciesApi::getUrl($taxonomy_id))
            . '</em>)';
        }
      }
      break;
  }

  return $formatted;
}
