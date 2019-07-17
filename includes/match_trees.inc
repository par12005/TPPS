<?php

/*
 * @file
 */

/**
 * 
 *
 * @param type $project_id
 */
function tpps_matching_trees($project_id) {
  $eps = variable_get('tpps_gps_epsilon', .001);

  $lat_id = chado_get_cvterm(array('name' => 'gps_latitude', 'is_obsolete' => 0))->cvterm_id;
  $lng_id = chado_get_cvterm(array('name' => 'gps_longitude', 'is_obsolete' => 0))->cvterm_id;
  
  $query_trees_1 = db_select('chado.stock', 's');
  $query_trees_1->join('chado.stockprop', 'lat', "lat.stock_id = s.stock_id AND lat.type_id = $lat_id");
  $query_trees_1->join('chado.stockprop', 'lng', "lng.stock_id = s.stock_id AND lng.type_id = $lng_id");
  $query_trees_1->addField('s', 'stock_id', 'stock_id');
  $query_trees_1->addField('s', 'type_id', 'type_id');
  $query_trees_1->addField('s', 'organism_id', 'organism_id');
  $query_trees_1->addExpression('substring(s.uniquename, 9)', 'uniquename');
  $query_trees_1->addExpression('cast(lat.value as float)', 'lat');
  $query_trees_1->addExpression('cast(lng.value as float)', 'lng');
  
  $query_trees_2 = clone $query_trees_1;

  $query_trees_1->join('chado.project_stock', 'p', "p.project_id = $project_id AND p.stock_id = s.stock_id");
  
  $query_trees_2->join('chado.project_stock', 'p', "p.project_id != $project_id AND p.stock_id = s.stock_id");

  $query = db_select($query_trees_1, 's1');
  $query->join($query_trees_2, 's2', "s1.uniquename = s2.uniquename AND s1.type_id = s2.type_id AND s1.organism_id = s2.organism_id AND abs(s1.lat - s2.lat) < $eps AND abs(s1.lng - s2.lng) < $eps");
  $query->addfield('s1', 'stock_id', 'stock_id1');
  $query->addfield('s2', 'stock_id', 'stock_id2');
  
  $results = $query->execute();

  while (($record = $results->fetchObject())) {
    tpps_chado_insert_record('stock_relationship', array(
      'subject_id' => $record->stock_id1,
      'object_id' => $record->stock_id2,
      'type_id' => array(
        'name' => 'synonymous',
        'cv_id' => array(
          'name' => 'sequence',
        ),
        'is_obsolete' => 0,
      )
    ));
  }
}
