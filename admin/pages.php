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

/**
 * This function will check study submission state from database
 * and compare with another study submission state for differences
 * 
 */
function tpps_admin_state_compare_page(array $form, array &$form_state, $study_accession1 = NULL, $study_accession2 = NULL) {
  
  // print_r($study_accession1);
  // print_r($study_accession2);

  $results1 = chado_query('SELECT * FROM tpps_submission WHERE accession = :accession', [
    ':accession' => $study_accession1
  ]);

  $results2 = chado_query('SELECT * FROM tpps_submission WHERE accession = :accession', [
    ':accession' => $study_accession2
  ]);  

  $submission1 = [];
  foreach ($results1 as $row) {
    $submission1 = unserialize($row->submission_state);
  }

  $submission2 = [];
  foreach ($results2 as $row) {
    $submission2 = unserialize($row->submission_state);
  }  

  $markup = "";
  $markup .= '<div style="width: 80%"><table width="800px">';
  $markup .= '<tr>';
  $markup .= '<td colspan="1" style="width: 30%; display: inline-block; vertical-align: top; word-wrap: break-word;">';
  $markup .= '<h3>' . $study_accession1 . '</h3>';
  $markup .= print_r($submission1['saved_values'], true);
  $markup .= '</td>';
  $markup .= '<td colspan="1" style="width: 30%; display: inline-block; vertical-align: top; word-wrap: break-word;">';
  $markup .= '<h3>' . $study_accession2 . '</h3>';
  $markup .= print_r($submission2['saved_values'], true);
  $markup .= '</td>';
  $markup .= '<td colspan="1" style="width: 30%; display: inline-block; vertical-align: top; word-wrap: break-word;">';
  $markup .= '<h3>Differences</h3>';
  // $markup .= print_r(array_diff($submission1['saved_values'], $submission2['saved_values']), true);
  $markup .= print_r(tpps_arrayRecursiveDiff($submission1['saved_values'], $submission2['saved_values']), true);
  $markup .= '</td>';
  $markup .= '</tr>';
  $markup .= '</table></div>';
  $form['markup'] = array(
    '#type' => 'markup',
    '#markup' => $markup
  );
  return $form;
}

// https://stackoverflow.com/questions/3876435/recursive-array-diff
function tpps_arrayRecursiveDiff($aArray1, $aArray2) {
  $aReturn = array();

  foreach ($aArray1 as $mKey => $mValue) {
    if (array_key_exists($mKey, $aArray2)) {
      if (is_array($mValue)) {
        $aRecursiveDiff = tpps_arrayRecursiveDiff($mValue, $aArray2[$mKey]);
        if (count($aRecursiveDiff)) { $aReturn[$mKey] = $aRecursiveDiff; }
      } else {
        if ($mValue != $aArray2[$mKey]) {
          $aReturn[$mKey] = $mValue;
        }
      }
    } else {
      $aReturn[$mKey] = $mValue;
    }
  }
  return $aReturn;
} 

// https://stackoverflow.com/questions/5911067/compare-object-properties-and-show-diff-in-php
function tpps_objDiff($obj1, $obj2):array { 
  $a1 = (array)$obj1;
  $a2 = (array)$obj2;
  return tpps_arrDiff($a1, $a2);
}

function tpps_arrDiff(array $a1, array $a2):array {
  $r = array();
  foreach ($a1 as $k => $v) {
      if (array_key_exists($k, $a2)) { 
          if ($v instanceof stdClass) { 
              $rad = objDiff($v, $a2[$k]); 
              if (count($rad)) { $r[$k] = $rad; } 
          }else if (is_array($v)){
              $rad = arrDiff($v, $a2[$k]);  
              if (count($rad)) { $r[$k] = $rad; } 
          // required to avoid rounding errors due to the 
          // conversion from string representation to double
          } else if (is_double($v)){ 
              if (abs($v - $a2[$k]) > 0.000000000001) { 
                  $r[$k] = array($v, $a2[$k]); 
              }
          } else { 
              if ($v != $a2[$k]) { 
                  $r[$k] = array($v, $a2[$k]); 
              }
          }
      } else { 
          $r[$k] = array($v, null); 
      } 
  } 
  return $r;     
}
?>