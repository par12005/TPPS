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
  // print_r($serialized_data['saved_values']);

  // print_r($serialized_data['ids']);
  $project_id = $serialized_data['ids']['project_id'];


  $project_file_ids = [];
  // get file_ids from project_id
  $results = chado_query('SELECT * FROM tpps_project_file_managed WHERE project_id = :project_id', [
    ':project_id' => $project_id
  ]);
  foreach ($results as $row) {
    // print_r($row);
    array_push($project_file_ids, $row->fid);
  }
  // print_r($project_file_ids);
  sort($project_file_ids);

  $saved_values = $serialized_data['saved_values'];
  // print_r($saved_values);
  $file_ids = [];
  $organism_count = $saved_values['1']['organism']['number'];
  // print_r($organism_count);
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
        if(isset($saved_values[$i]['organism-' . $j]['genotype']['files']['ssrs'])) {
          array_push($file_ids, $saved_values[$i]['organism-' . $j]['genotype']['files']['ssrs']);
        } 
        if(isset($saved_values[$i]['organism-' . $j]['genotype']['files']['ssrs_extra'])) {
          array_push($file_ids, $saved_values[$i]['organism-' . $j]['genotype']['files']['ssrs_extra']);
        }                             
      }
    }
  }
  // print_r($file_ids);
  sort($file_ids);
  
  $markup .= '<div style="font-size: 12px;">';
  $markup .= '<div style="display: inline-block; width: 30%; vertical-align: top;">';
  $markup .= '<h4>Project ID Managed</h4>';
  foreach ($project_file_ids as $fid) {
    $markup .= '<div>' . $fid . '-';
    $file = file_load($fid);
    if ($file) {
      $file_url = check_plain(file_create_url($file->uri));
      $markup .= "<a href='$file_url' target='blank'>$file->filename</a>";
    }
    $markup .= '</div>';
  }
  $markup .= '</div>';

  $markup .= '<div style="display: inline-block; width: 30%; vertical-align: top;">';
  $markup .= '<h4>Submission state</h4>';
  foreach ($file_ids as $fid) {
    $markup .= '<div>' . $fid . '-';
    $file = file_load($fid);
    if ($file) {
      $file_url = check_plain(file_create_url($file->uri));
      $markup .= "<a href='$file_url' target='blank'>$file->filename</a>";
    }
    $markup .= '</div>';
  }
  $markup .= '</div>'; 
  
  $markup .= '<div style="display: inline-block; width: 30%; vertical-align: top;">';
  $markup .= '<h4>History/State files</h4>';
  $overall_file_ids = $serialized_data['files'];
  sort($overall_file_ids);
  foreach ($overall_file_ids as $fid) {
    $markup .= '<div>' . $fid . '-';
    $file = file_load($fid);
    if ($file) {
      $file_url = check_plain(file_create_url($file->uri));
      $markup .= "<a href='$file_url' target='blank'>$file->filename</a>";
    }
    $markup .= '</div>';
    
  }
  $markup .= '</div>'; 
  $markup .= '</div>'; 

  $form['markup'] = array(
    '#type' => 'markup',
    '#markup' => $markup
  );

  return $form;
}
?>