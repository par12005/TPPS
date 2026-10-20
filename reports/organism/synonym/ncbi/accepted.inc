<?php

/**
 * @file
 * Shows list of the accepted organism synonym names.
 */

/**
 * Menu callback. Shows list of the accepted organism synonym names.
 *
 * URL: /admin/reports/tpps/organism/synonym/ncbi/accepted.
 *
 * @param bool $reset
 *   Flag is cache must be resetted. Default if FALSE.
 */
function tpps_report_organism_synonym_ncbi_accepted($reset = FALSE) {
  $reset = isset($_GET['reset']) ? TRUE : ($reset ?? FALSE);
  $rows = [];

  // Clear list.
  if (isset($_GET['clear'])) {
    NcbiTaxonomy::clearAcceptedSynynymList();
  }
  // Get list.
  $list = NcbiTaxonomy::getAcceptedSynynymList();

  if (!empty($list)) {
    // Mapping.
    $fields = [
      // DB field name => Class property name.
      'organism_id' => 'id',
      'genus' => 'genus',
      'species' => 'species',
      'infraspecific_name' => 'infraspecificName',
      'type_id' => 'infraspecificRank',
    ];

    // Get all (synonym and primary) organism names.
    $organism_id_list = array_merge(array_keys($list), array_values($list));
    $names = db_select('chado.organism', 'co')
      ->condition('co.organism_id', $organism_id_list, 'IN')
      ->fields('co', array_keys($fields))
      ->execute()->fetchAllAssoc('organism_id', PDO::FETCH_ASSOC);

    // Build table.
    $i = 0;
    foreach ($list as $synonym_id => $primary_id) {
      $id_list = [
        'synonym' => $synonym_id,
        'primary' => $primary_id,
      ];
      $row['#'] = ++$i;
      foreach ($id_list as $type => $id) {
        if (empty($names[$id])) {
          $row[$type . '_id'] = NULL;
          $row[$type . '_name'] = NULL;
          continue;
        }
        $organism = new Organism();
        foreach ($fields as $field_name => $property_name) {
          if ($field_name == 'type_id') {
            if (!empty($rank = $names[$id][$field_name])) {
              $organism->{$property_name} = InfraspecificRank::getName($rank);
            }
            else {
              // Just skip.
            }
          }
          else {
            $organism->{$property_name} = ($names[$id][$field_name] ?? NULL);
          }
        }
        $organism->generateName();
        $row[$type . '_id'] = $id;
        $row[$type . '_name'] = $organism->name;
      }
      $rows[] = $row;
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  $header = [
    ['data' => '#', 'field' => '#'],
    ['data' => t('Synonym Id'), 'field' => 'synonym_id'],
    ['data' => t('Synonym Name'), 'field' => 'syonym_name'],
    ['data' => t('Primary Id'), 'field' => 'primary_id'],
    ['data' => t('Primary Name'), 'field' => 'primary_name'],
  ];
  return Report::print([
    'name' => 'organism_synonym_ncbi_accepted',
    'header' => $header,
    'rows' => $rows,
    'pager' => 'both',
  ], $reset);
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
function tpps_report_organism_synonym_ncbi_accepted_formatter(string $name, $value, array &$row) {
$debug_mode = 0;
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
        $xml = NcbiTaxonomy::getInfo($formatted);
        if (is_array($xml)) {
          if ($debug_mode ?? FALSE) {
            dpm(var_export(NcbiTaxonomy::printXml($xml), 1));
          }
        }
        else {
          if (
            strpos($xml, '<OutputMessage>No items found.</OutputMessage>') === FALSE
          ) {
            // Name was found.
            preg_match('/<Id>(\d+)<\/Id>/', $xml, $matches);
            if (!empty($taxonomy_id = ($matches[1] ?? NULL))) {
              $formatted = l(
                $formatted . ' (' . $taxonomy_id . ')',
                NcbiTaxonomy::getUrl($taxonomy_id)
              );
              $formatted = '<em>' . $formatted . '</em>';
            }
            else {
              if ($debug_mode ?? FALSE) {
                dpm(var_export(NcbiTaxonomy::printXml($xml), 1));
              }
            }
          }
          else {
            // Name wasn't found.
          }
        }
      }
      break;
  }

  return $formatted;
}
