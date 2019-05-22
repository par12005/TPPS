<?php

/**
 * @file
 */

/**
 *
 */
function save_file_columns(&$form, &$form_state) {

  if ($form_state['stage'] == TPPS_PAGE_3) {
    if ($form_state['saved_values'][TPPS_PAGE_1]['organism']['number'] == '1' or $form_state['values']['tree-accession']['check'] == '0') {
      if (($file = file_load($form_state['values']['tree-accession']['file'])) and $form_state['values']['tree-accession']['file'] != 0) {
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
      }

      $form_state['values']['tree-accession']['file-columns'] = array();

      foreach ($form['tree-accession']['file']['columns'] as $col => $val) {
        if (isset($form['tree-accession']['file']['columns'][$col]['#value']) and $col[0] != '#') {
          $form_state['values']['tree-accession']['file-columns'][$col] = $form['tree-accession']['file']['columns'][$col]['#value'];
        }
      }
    }
    else {
      for ($i = 1; $i <= $form_state['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {
        if (($file = file_load($form_state['values']['tree-accession']["species-$i"]['file'])) and $form_state['values']['tree-accession']["species-$i"]['file'] != 0) {
          file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
        }

        $form_state['values']['tree-accession']["species-$i"]['file-columns'] = array();

        if (isset($form['tree-accession']["species-$i"]['file']['#value']['columns'])) {
          foreach ($form['tree-accession']["species-$i"]['file']['#value']['columns'] as $col => $val) {
            if ($col[0] != '#') {
              $form_state['values']['tree-accession']["species-$i"]['file-columns'][$col] = $form['tree-accession']["species-$i"]['file']['#value']['columns'][$col];
            }
          }
        }
      }
    }
  }
  elseif ($form_state['stage'] == TPPS_PAGE_4) {
    if (isset($form['organism-1']['genotype'])) {
      for ($i = 1; $i <= $form_state['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {

        if ($form_state['values']["organism-$i"]['genotype']['marker-type']['Other'] != '0' or $form_state['values']["organism-$i"]['genotype']['marker-type']['SSRs/cpSSRs'] != '0') {
          if (($file = file_load($form_state['values']["organism-$i"]['genotype']['file'])) and $form_state['values']["organism-$i"]['genotype']['file'] != 0) {
            file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
          }

          $form_state['values']["organism-$i"]['genotype']['file-columns'] = array();

          if (isset($form["organism-$i"]['genotype']['file']['#value']['columns'])) {
            foreach ($form["organism-$i"]['genotype']['file']['#value']['columns'] as $col => $val) {
              if ($col[0] != '#') {
                $form_state['values']["organism-$i"]['genotype']['file-columns'][$col] = $form["organism-$i"]['genotype']['file']['#value']['columns'][$col];
              }
            }
          }
        }
      }
    }

    if (isset($form['organism-1']['phenotype'])) {
      for ($i = 1; $i <= $form_state['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {
        if (($file = file_load($form_state['values']["organism-$i"]['phenotype']['file'])) and $form_state['values']["organism-$i"]['phenotype']['file'] != 0) {
          file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
        }

        $form_state['values']["organism-$i"]['phenotype']['file-columns'] = array();

        if (isset($form["organism-$i"]['phenotype']['file']['#value']['columns'])) {
          foreach ($form["organism-$i"]['phenotype']['file']['#value']['columns'] as $col => $val) {
            if ($col[0] != '#') {
              $form_state['values']["organism-$i"]['phenotype']['file-columns'][$col] = $form["organism-$i"]['phenotype']['file']['#value']['columns'][$col];
            }
          }
        }

        if ($form_state['values']["organism-$i"]['phenotype']['check'] != '0') {
          if (($file = file_load($form_state['values']["organism-$i"]['phenotype']['metadata'])) and $form_state['values']["organism-$i"]['phenotype']['metadata'] != 0) {
            file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
          }

          $form_state['values']["organism-$i"]['phenotype']['metadata-columns'] = array();

          if (isset($form["organism-$i"]['phenotype']['metadata']['#value']['columns'])) {
            foreach ($form["organism-$i"]['phenotype']['metadata']['#value']['columns'] as $col => $val) {
              if ($col[0] != '#') {
                $form_state['values']["organism-$i"]['phenotype']['metadata-columns'][$col] = $form["organism-$i"]['phenotype']['metadata']['#value']['columns'][$col];
              }
            }
          }
        }
      }
    }
  }
  return;
}
