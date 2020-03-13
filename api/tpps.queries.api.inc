<?php

/**
 * @file
 * Defines queries for the TPPS API.
 *
 * Most queries get information based on a list of project ids, but some can
 * produce a list of project ids based on some conditions. Query functions with
 * names like "tpps_api_*_pid_query" will produce a list of pids based on the
 * provided conditions. For example, the function "tpps_api_author_pid_query"
 * will return a list of pids with authors that match at least one of the
 * provided names. Query functions with names like "tpps_api_project_*_query"
 * will produce a list of ids or entities based on a list of pids. For example,
 * the function "tpps_api_project_organism_query" will return a list of
 * organism ids or, if the entities parameter is set to true, will return a
 * list of organism entities associated with the provided pids.
 */

/**
 * Returns array of project ids associated with provided authors.
 *
 * @param array $authors
 *   An array of author names.
 *
 * @return array
 *   An array of associated project ids.
 */
function tpps_api_author_pid_query(array $authors) {
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
 * Returns array of project ids associated with provided species.
 *
 * @param array $orgs
 *   An array of species names.
 *
 * @return array
 *   An array of associated project ids.
 */
function tpps_api_organism_pid_query(array $orgs) {
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
 * Returns array of project ids associated with provided organizations.
 *
 * @param array $orgs
 *   An array of organization names.
 *
 * @return array
 *   An array of associated project ids.
 */
function tpps_api_organization_pid_query(array $orgs) {
  $query = db_select('chado.contact', 'c');
  $query->join('chado.contact_relationship', 'cr', 'c.contact_id = cr.object_id');
  $query->join('chado.project_contact', 'pc', 'pc.contact_id = cr.subject_id');
  $query->fields('pc', array('project_id'));
  $or = tpps_api_query_or($orgs, array(
    'c.name',
  ));
  $and = db_and()
    ->condition('c.type_id', tpps_load_cvterm('organization')->cvterm_id)
    ->condition($or);
  $query->condition($and);
  return $query->execute();
}

/**
 * Returns array of projects associated with provided project ids.
 *
 * @param array $pids
 *   An array of project ids.
 * @param bool $entities
 *   Whether or not to return entities.
 *
 * @return array
 *   An array of project ids or entities.
 */
function tpps_api_project_query(array $pids, $entities = FALSE) {
  if (!$entities) {
    return $pids;
  }

  return tpps_api_record_entities('Project', $pids);
}

/**
 * Returns array of publications associated with provided project ids.
 *
 * @param array $pids
 *   An array of project ids.
 * @param bool $entities
 *   Whether or not to return entities.
 *
 * @return array
 *   An array of publication ids or entities.
 */
function tpps_api_project_publication_query(array $pids, $entities = FALSE) {
  $query = db_select('chado.project_pub', 'pp')
    ->fields('pp', array('pub_id'));
  $or = tpps_api_query_or($pids, array(
    'pp.project_id',
  ), '=');
  $query->condition($or);
  $query = $query->execute();

  $results = $query->fetchCol();

  if (!$entities) {
    return $results;
  }

  return tpps_api_record_entities('Publication', $results);
}

/**
 * Returns array of authors associated with provided project ids.
 *
 * This function currently does not support returning Tripal Entities.
 *
 * @param array $pids
 *   An array of project ids.
 * @param bool $entities
 *   (Placeholder) Whether or not to return entities.
 *
 * @return array
 *   An array of pubauthor table record ids.
 */
function tpps_api_project_author_query(array $pids, $entities = FALSE) {
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
 * Returns array of species associated with provided project ids.
 *
 * @param array $pids
 *   An array of project ids.
 * @param bool $entities
 *   Whether or not to return entities.
 *
 * @return array
 *   An array of organism ids or entities.
 */
function tpps_api_project_organism_query(array $pids, $entities = FALSE) {
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
 * Returns array of design information associated with provided project ids.
 *
 * This function does not support returning Tripal Entities.
 *
 * @param array $pids
 *   An array of project ids.
 *
 * @return array
 *   An array of study design information.
 */
function tpps_api_project_design_query(array $pids) {
  $query = db_select('chado.projectprop', 'pp');
  $query->join('chado.cvterm', 'c', 'c.cvterm_id = pp.type_id');
  $query->fields('c', array('name'))
    ->fields('pp', array('value'));
  $or = tpps_api_query_or($pids, array(
    'pp.project_id',
  ), '=');
  $query->condition($or);
  $query = $query->execute();

  $results = array();
  while (($result = $query->fetchObject())) {
    $results[$result->name][] = $result->value;
  }

  return $results;
}

/**
 * Returns array of stocks associated with provided project ids.
 *
 * @param array $pids
 *   An array of project ids.
 * @param bool $entities
 *   Whether or not to return entities.
 *
 * @return array
 *   An array of stock ids or entities.
 */
function tpps_api_project_stock_query(array $pids, $entities = FALSE) {
  $query = db_select('chado.project_stock', 'p')
    ->fields('p', array('stock_id'));
  $or = tpps_api_query_or($pids, array(
    'p.project_id',
  ), '=');
  $query->condition($or);
  $query = $query->execute();

  $results = $query->fetchCol();

  if (!$entities) {
    return $results;
  }

  return tpps_api_record_entities('Stock', $results);
}

/**
 * Returns array of phenotype ids associated with provided project ids.
 *
 * This function currently does not support returning Tripal Entities.
 *
 * @param array $pids
 *   An array of project ids.
 * @param bool $entities
 *   (Placeholder) Whether or not to return entities.
 *
 * @return array
 *   An array of study design information.
 */
function tpps_api_project_phenotype_query(array $pids, $entities = FALSE) {
  $query = db_select('chado.project_stock', 'ps');
  $query->join('chado.stock_phenotype', 'sp', 'sp.stock_id = ps.stock_id');
  $or = tpps_api_query_or($pids, array(
    'ps.project_id',
  ), '=');
  $query->fields('sp', array('phenotype_id'))
    ->condition($or);
  $query = $query->execute();

  $results = $query->fetchCol();
  return $results;
}

/**
 * Returns array of genotypes associated with provided project ids.
 *
 * @param array $pids
 *   An array of project ids.
 * @param bool $entities
 *   Whether or not to return entities.
 *
 * @return array
 *   An array of genotype ids or entities.
 */
function tpps_api_project_genotype_query(array $pids, $entities = FALSE) {
  $query = db_select('chado.project_stock', 'ps');
  $query->join('chado.stock_genotype', 'sg', 'sg.stock_id = ps.stock_id');
  $or = tpps_api_query_or($pids, array(
    'ps.project_id',
  ), '=');
  $query->fields('sg', array('genotype_id'))
    ->condition($or);
  $query = $query->execute();

  $results = $query->fetchCol();

  if (!$entities) {
    return $results;
  }
  return tpps_api_record_entities('Genotype', $results);
}

/**
 * Returns array of submission statistics based on provided submission states.
 *
 * This function does not support returning Tripal Entities.
 *
 * @param array $states
 *   An array of TPPS submission states.
 *
 * @return array
 *   An array of TPPS submission statistics.
 */
function tpps_api_project_stats_query(array $states) {
  $results = array();
  foreach ($states as $state) {
    $results[] = $state['stats'];
  }
  return $results;
}

/**
 * Constructs a db_or() condition object for use with other TPPS API functions.
 *
 * The default comparison operator is '~*', but this can be overriden by either
 * specifying a different argument in square brackets following the value being
 * compared, or by specifying a condition-wide operator in the override_op
 * function parameter.
 *
 * @param array $vals
 *   The values being compared in this condition.
 * @param array $fields
 *   The columns that the values should be compared against.
 * @param string $override_op
 *   The condition-wide overriding operator.
 *
 * @return object
 *   The constructed db_or() condition object.
 */
function tpps_api_query_or(array $vals, array $fields, $override_op = NULL) {
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
 * Loads and returns entities that refer to the provided record ids.
 *
 * This function takes a Tripal Entity label and an array of record ids and
 * attempts to load the associated Tripal Entities. It will also attach fields
 * that are not automatically attached by the Tripal Entity loader.
 *
 * @param string $label
 *   The label of the Tripal Entity type.
 * @param array $record_ids
 *   The array of database record ids referred to by the Tripal Entities.
 *
 * @return array
 *   An array of associated Tripal Entities.
 */
function tpps_api_record_entities($label, array $record_ids) {
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
