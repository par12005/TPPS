<?php

/**
 * @file
 * Shows list of studies without DOI attached.
 */

/**
 * Menu callback. Shows studies without DOI attached.
 */
function tpps_admin_new_organisms_report() {
  $name = 'new_organisms';
  $page_title = variable_get('tpps_report_new_organisms_title', 'New Organisms');

  // Get all organisms.
  $table = 'chado.organism';
  $query = db_select($table, 't')->fields('t', ['organism_id']);
  $query->addExpression("CONCAT(t.genus, ' ', t.species)", 'organism_name');
  $query->range(0, 10)->orderBy('organism_id');
  $result = $query->execute()->fetchAllKeyed();

  foreach ($result as $organism_id => $organism_name) {
    $xml = tpps_ncbi_get_taxon_id_v2($organism_name);
    // @TODO Fix 'Too many requests' error.
    // Use Organism::requestNcbiTaxonomyId().
    sleep(1);
    if (is_string($xml)) {
      $id = '-';
    }
    else {
      // Convert Simple XML Object into PHP's array.
      $ncbi_response = json_decode(json_encode($xml), TRUE);
      $id = check_plain($ncbi_response[0]) ?? FALSE;
    }
    //if (empty($id)) {
      $rows[] = [
        $organism_id,
        $organism_name,
        $id,
      ];
    //}
  }


  //dpm(print_r($result, 1));
  //
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output.
  //$page_title = variable_get('tpps_report_' . $name . '_title', 'DOI Duplicates');
  //drupal_set_title($page_title);
  $header = [t('Organism Id'), t('Organism Name'), t('NCBI Taxonomy Id')];
  return tpps_report_show_table([
    'report_name' => $name,
    'header' => $header,
    'rows' => $rows,
  ]);
}
