<?php
/**
 * This function will check study submission state from database
 * find the file ids and also check the managed tables to see what is
 * missing. This will thus detect old files.
 */
function tpps_admin_files_diagnostics_page(array $form, array &$form_state, $study_accession = NULL) {
  $markup = "";
  
  $results = chado_query('SELECT * FROM tpps_submission WHERE accession = :accession', [
    ':accession' => $study_accession
  ]);

  $serialized_data = "";
  foreach($results as $row) {
    $serialized_data = unserialize($row->submission_state);
  }
  print_r($serialized_data['saved_values']);
  $saved_values = $serialized_data['saved_values'];
  $file_ids = [];
  $organism_count = $saved_values['1']['organism']['number'];
  print_r($organism_count);
  for($i=1; $i<=4; $i++) {
    if ($i == 3) {
      for($j=1; $j<=$organism_count; $j++) {
        if(isset($saved_values[$i]['tree-accession']['species-' . $j]['file'])) {
          array_push($file_ids, $saved_values[$i]['tree-accession']['species-' . $j]['file']);
        }
      }
    }
    if ($i == 4) {
      // Phenotype files
      for($j=1; $j<=$organism_count; $j++) {
        if(isset($saved_values[$i]['organism-' . $j]['phenotype']['file'])) {
          array_push($file_ids, $saved_values[$i]['organism-' . $j]['phenotype']['file']);
        }
        if(isset($saved_values[$i]['organism-' . $j]['phenotype']['metadata'])) {
          array_push($file_ids, $saved_values[$i]['organism-' . $j]['phenotype']['metadata']);
        }        
      }
      for($j=1; $j<=$organism_count; $j++) {
        if(isset($saved_values[$i]['organism-' . $j]['genotype']['files']['snps-assay'])) {
          array_push($file_ids, $saved_values[$i]['organism-' . $j]['genotype']['files']['snps-assay']);
        }
        if(isset($saved_values[$i]['organism-' . $j]['genotype']['files']['snps-association'])) {
          array_push($file_ids, $saved_values[$i]['organism-' . $j]['genotype']['files']['snps-association']);
        }
        if(isset($saved_values[$i]['organism-' . $j]['genotype']['files']['vcf'])) {
          array_push($file_ids, $saved_values[$i]['organism-' . $j]['genotype']['files']['vcf']);
        }                
      }
    }
  }
  print_r($file_ids);

  $form['markup'] = array(
    '#type' => 'markup',
    '#markup' => $markup
  );

  return $form;
}
?>