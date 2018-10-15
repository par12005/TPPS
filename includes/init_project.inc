<?php

function tpps_init_project($form, &$form_state){
    
    //Get next dbxref_id and accession # for project
    $and = db_and()
        ->condition('d.accession', db_like('TGDR') . '%', 'LIKE')
        ->condition('d.db_id', '95');
    
    $accession_results = db_select('chado.dbxref', 'd')
        ->fields('d', array('accession'))
        ->condition($and)
        ->orderBy('d.accession', 'DESC')
        ->range(0,1)
        ->execute()
        ->fetchAssoc();
    
    if ($accession_results){
        $accession = $accession_results['accession'];
    }
    
    if ($accession === NULL){
        $accession = "TGDR001";
    }
    else {
        $accession = substr($accession, 4) + 1;
    }
    
    while (strlen($accession) < 3){
        $accession = "0" . $accession;
    }
    if (substr($accession, 0, 4) != 'TGDR'){
        $accession = "TGDR" . $accession;
    }
    
    //make new record in temp table
    $values = array(
      'db_id' => '95',
      'accession' => $accession
    );
    
    $dbxref_id = db_insert('chado.dbxref')
        ->fields($values)
        ->execute();
    
    $form_state['dbxref_id'] = $dbxref_id;
    $form_state['accession'] = $accession;
    $form_state['saved_values']['frontpage']['accession'] = $accession;
    
    //project will be created when name is provided, after the first page is complete.
    return $dbxref_id;
}
