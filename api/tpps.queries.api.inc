<?php

/**
 * @file
 */

/**
 * 
 */
function tpps_api_author_pid_query($authors) {
  $query = db_select('chado.pubauthor', 'pa');
  $query->join('chado.project_pub', 'pp', 'pp.pub_id = pa.pub_id');
  $query->fields('pp', array('project_id'));
  $or = tpps_api_query_or($orgs, array(
    'pa.givennames',
    'pa.surname',
  ));
  $query->condition($or);
  return $query->execute();
}

/**
 * 
 */
function tpps_api_organism_pid_query($orgs) {
  $query = db_select('chado.organism', 'o');
  $query->join('chado.project_organism', 'po', 'po.organism_id = o.organism_id');
  $query->fields('po', array('project_id'));
  $or = tpps_api_query_or($orgs, array(
    'o.genus',
    'o.species',
  ));
  $query->condition($or);
  return $query->execute();
}

/**
 * 
 */
function tpps_api_organization_pid_query($orgs) {
  $query = db_select('chado.contact', 'c');
  $query->join('chado.project_contact', 'pc', 'pc.contact_id = c.contact_id');
  $query->fields('pc', array('project_id'));
  $or = tpps_api_query_or($orgs, array(
    'c.name',
  ));
  $and = db_and()
    ->condition('type_id', chado_get_cvterm(array(
      'name' => 'Organization',
      'cv_id' => array(
        'name' => 'tripal_contact',
      ),
    ))->cvterm_id)
    ->condition($or);
}

/**
 * 
 */
function tpps_api_query_or($vals, $fields) {
  $or = db_or();
  foreach ($vals as $val) {
    $op = '~*';
    if (preg_match('/^(.*)\[(.+)\]$/', $val, $matches)) {
      $val = $matches[1];
      $op = $matches[2];
    }
    foreach ($fields as $field) {
      $or->condition($field, $val, $op);
    }
  }
  return $or;
}
