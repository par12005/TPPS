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
  $query->join('chado.contact_relationship', 'cr', 'c.contact_id = cr.object_id');
  $query->join('chado.project_contact', 'pc', 'pc.contact_id = cr.subject_id');
  $query->fields('pc', array('project_id'));
  $or = tpps_api_query_or($orgs, array(
    'c.name',
  ));
  $and = db_and()
    ->condition('c.type_id', chado_get_cvterm(array(
      'name' => 'Organization',
      'cv_id' => array(
        'name' => 'tripal_contact',
      ),
    ))->cvterm_id)
    ->condition($or);
  $query->condition($and);
  return $query->execute();
}

/**
 * 
 */
function tpps_api_project_query($pids, $entities = FALSE) {
  if (!$entities) {
    return $pids;
  }

  return tpps_api_record_entities('Project', $pids);
}

/**
 * 
 */
function tpps_api_project_author_query($pids, $entities = FALSE) {
  $query = db_select('chado.project_pub', 'pp');
  $query->join('chado.pubauthor', 'pa', 'pa.pub_id = pp.pub_id');
  $query->fields('pa', array('pubauthor_id'));
  $or = tpps_api_query_or($pids, array(
    'pp.project_id',
  ), '=');
  $query->condition($or);
  $query = $query->execute();
  return $query->fetchCol();
}

/**
 * 
 */
function tpps_api_project_organism_query($pids, $entities = FALSE) {
  $query = db_select('chado.project_organism', 'po')
    ->fields('po', array('organism_id'));
  $or = tpps_api_query_or($pids, array(
    'po.project_id',
  ), '=');
  $query->condition($or);
  $query = $query->execute();

  $results = $query->fetchCol();

  if (!$entities) {
    return $results;
  }

  return tpps_api_record_entities('Organism', $results);
}

/**
 * 
 */
function tpps_api_query_or($vals, $fields, $override_op = NULL) {
  $or = db_or();
  foreach ($vals as $val) {
    $op = '~*';
    if (preg_match('/^(.*)\[(.+)\]$/', $val, $matches)) {
      $val = $matches[1];
      $op = $matches[2];
    }
    $op = !empty($override_op) ? $override_op : $op;
    foreach ($fields as $field) {
      $or->condition($field, $val, $op);
    }
  }
  return $or;
}

/**
 * 
 */
function tpps_api_record_entities($label, $record_ids) {
  $bundle = tripal_load_bundle_entity(array('label' => $label));
  $details = @tripal_get_bundle_details($bundle->name);

  $entity_ids = array();
  foreach ($record_ids as $id) {
    $entity_ids[] = chado_get_record_entity_by_bundle($bundle, $id);
  }

  $field_ids = array();
  if (!empty($details['fields'])) {
    foreach ($details['fields'] as $field) {
      $field_ids[] = field_info_field($field['name'])['id'];
    }
  }

  return tripal_load_entity('TripalEntity', $entity_ids, FALSE, $field_ids);
}
