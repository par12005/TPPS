<?php

function tpps_custom_cvterm($term, $definition){
    
    $dbxref = db_query('select d.accession from chado.dbxref d where d.db_id=95 and d.accession ~ \'^\\d+$\' order by cast(d.accession as bigint) desc');
    
    foreach ($dbxref as $row){
        $accession = $row->accession + 1;
        break;
    }
    
    if (!isset($accession)){
        $accession = 1;
    }
    
    $dbxref = db_insert('chado.dbxref')
        ->useDefaults(array(
          'version',
          'description',
          'dbxref_id'
        ))
        ->fields(array(
          'db_id' => '95',
          'accession' => "$accession"
        ))
        ->execute();
    
    $cvterm = db_insert('chado.cvterm')
        ->useDefaults(array(
          'cvterm_id',
          'is_obsolete',
          'is_relationshiptype'
        ))
        ->fields(array(
          'cv_id' => '61',
          'name' => $term,
          'definition' => $definition,
          'dbxref_id' => $dbxref
        ))
        ->execute();
    
    return $cvterm;
}
