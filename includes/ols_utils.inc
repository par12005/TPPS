<?php

/**
 * @file
 * Defines utilities for using the OLS API.
 *
 * Also defines some utilities for managing TPPS cvterms.
 */

/**
 * Searches OLS for a term, possibly from a specific ontology.
 *
 * @param string $query
 *   The term we are looking for.
 * @param string|null $ontology
 *   Optional ontology to search in.
 * @param bool $exact
 *   Whether or not we should be looking for an exact match.
 *
 * @return stdClass
 *   Decoded JSON object returned by the OLS API.
 */
function tpps_ols_search($query, $ontology = NULL, $exact = TRUE) {
  $exact = $exact ? 'true' : 'false';
  $args = array();
  $args[] = "q=" . urlencode($query);
  $args[] = "exact=$exact";
  $args[] = "queryFields=label";
  if (!empty($ontology)) {
    $args[] = "ontology=$ontology";
  }
  return tpps_ols_call('search', $args);
}

/**
 * Retrieves information about an ontology from OLS based on the ontology id.
 *
 * @param string $id
 *   The id of the ontology.
 *
 * @return array
 *   An array containing some minimal information about the specified ontology.
 */
function tpps_ols_ontology_info($id) {
  $response = tpps_ols_call(array(
    'ontologies',
    $id,
  ));
  $name = $response->config->preferredPrefix;
  $base_uri = $response->config->baseUris[0] ?? NULL;
  if (substr($base_uri, -1 * (strlen($name) + 1)) == "_$name") {
    $url_prefix = substr($base_uri, 0, -5) . '{db}_{accession}';
  }

  return array(
    'id' => $response->ontologyId,
    'name' => $name,
    'description' => $response->config->title,
    'url' => $response->config->homepage,
    'urlprefix' => $url_prefix ?? NULL,
  );
}

/**
 * Makes a call to the OLS API.
 *
 * @param string|array $type
 *   Type(s) to pass to tpps_ols_url().
 * @param array $args
 *   Optional additional args to pass to tpps_ols_url().
 *
 * @return stdClass
 *   Decoded JSON object returned by the OLS API.
 */
function tpps_ols_call($type, array $args = array()) {
  $url = tpps_ols_url($type, $args);
  $data = file_get_contents($url);
  return json_decode($data);
}

/**
 * Constructs a URL for querying the OLS API based on supplied parameters.
 *
 * @param string|array $types
 *   Type(s) to specify the type of API call being made.
 * @param array $args
 *   Optional additional parameters to be passed to the API.
 *
 * @return string
 *   The constructed URL for the desired API call.
 */
function tpps_ols_url($types, array $args = array()) {
  $url = TPPS_OLS_BASE;

  if (!is_array($types)) {
    $url .= "$types/";
  }
  else {
    foreach ($types as $type) {
      $url .= "$type/";
    }
  }

  if (!empty($args)) {
    $url .= "?" . implode('&', $args);
  }

  return $url;
}

/**
 * Returns an array of cvterms that TPPS version 1.2 will be using.
 *
 * This function is called during installation of TPPS to add these cvterms via
 * the Ontology Lookup Service (https://www.ebi.ac.uk/ols/). Older versions of
 * this function are kept for the purposes of migrating cvterms that TPPS may
 * have defined in the past.
 *
 * @return array
 *   Array of cvterms that TPPS will be using.
 */
function tpps_get_cvterms_v1_2() {
  return array(
    'study_start' => 'ncit:Study Start Date',
    'study_end' => 'ncit:Study Completion Date',
    'experiment_location' => 'ncit:Location',
    'assession_season' => 'ncit:Season',
    'assession_number' => 'local:Number of times the trees were assessed, on average.',
    'co2_control' => 'peco:carbon dioxide exposure',
    'co2_level' => 'envo:atmospheric carbon dioxide',
    'humidity_control' => 'peco:humidity exposure',
    'humidity_level' => 'envo:humidity',
    'light_control' => 'peco:light intensity exposure',
    'light_level' => 'envo:visible spectrum radiation',
    'temperature_high' => 'local:Average high temperature of the environment.',
    'temperature_low' => 'local:Average low temperature of the environment.',
    'rooting_type' => 'local:Type of rooting. Aeroponics, Hydroponics, or Soil.',
    'soil_type' => 'envo:soil',
    'soil_container' => 'ncit:Container',
    'pH_control' => 'peco:pH exposure',
    'pH_level' => 'pato:acidity',
    'treatment' => 'peco:plant exposure',
    'irrigation_type' => 'agro:irrigation process',
    'salinity_control' => 'peco:salt exposure',
    'salinity_level' => 'envo:salt',
    'biotic_environment' => 'peco:biotic plant exposure',
    'study_type' => 'peco:study type',
    'phenotype_binary_type' => NULL,
    'file_path' => 'ncit:Pathname',
    'contact photo' => NULL,
    'gps_latitude' => 'ncit:Latitude',
    'gps_longitude' => 'ncit:Longitude',
    'association_results_type' => 'ncit:Data Type',
    'county' => 'ncit:County',
    'district' => 'ncit:Locality',
    'organism 4 letter code' => 'local:4 letter abbreviation of species. Usually first two letters of genus + first two letters of species, may be different if that code already exists (use next letters from species if possible).',
    'cpSSR' => NULL,
    'SSR' => 'so:microsatellite',
    'time' => 'pato:time',
    'source_description' => 'local:A textual description of study type and comments for a source.',
  );
}

/**
 * Returns an array of cvterms that TPPS version 1.1 will be using.
 *
 * This function is called during installation of TPPS to add these cvterms via
 * the Ontology Lookup Service (https://www.ebi.ac.uk/ols/). Older versions of
 * this function are kept for the purposes of migrating cvterms that TPPS may
 * have defined in the past.
 *
 * @return array
 *   Array of cvterms that TPPS will be using.
 */
function tpps_get_cvterms_v1_1() {
  return array(
    'study_start' => 'The month and year that the study began.',
    'study_end' => 'The month and year that the study ended.',
    'experiment_location' => 'Geographic location of the experiment.',
    'assession_season' => 'Season the trees were assessed.',
    'assession_number' => 'Number of times the trees were assessed, on average.',
    'co2_control' => 'Whether or not the co2 level of the environment was controlled. True or False.',
    'co2_level' => 'Must have an associated co2_control property. If co2_control is True, this term describes the co2 level the environment was kept at. If co2_control is False, this term describes the average measured co2 value in the environment.',
    'humidity_control' => 'Whether or not the air humidity level of the environment was controlled. True or False.',
    'humidity_level' => 'Must have an associated humidity_control property. If humidity_control is True, this term describes the air humidity level the environment was kept at. If humidity_control is False, this term describes the average measured air humidity value in the environment.',
    'light_control' => 'Whether or not the light intensity level of the environment was controlled. True or False.',
    'light_level' => 'Must have an associated light_control property. If light_control is True, this term describes the light intensity level the environment was kept at. If light_control is False, this term describes the average measured light intensity value in the environment.',
    'temperature_high' => 'Average high temperature of the environment.',
    'temperature_low' => 'Average low temperature of the environment.',
    'rooting_type' => 'Type of rooting. Aeroponics, Hydroponics, or Soil.',
    'soil_type' => 'Type of soil. For example: Sand, Peat, Mixed, etc.',
    'soil_container' => 'Type of soil container.',
    'pH_control' => 'Whether or not the pH level of the environment was controlled. True or False.',
    'pH_level' => 'Must have an associated pH_control property. If pH_control is True, this term describes the pH level the environment was kept at. If pH_control is False, this term describes the average measured pH value in the environment.',
    'treatment' => 'Describes the treatment of the trees during the study. For example, air or soil temperature regimes, chemical administration, fertilizer, non-mineral nutrient, or rainfall regimes, disease status, etc.',
    'irrigation_type' => 'Describes the type of irrigation. For example, drip irrigation, irrigation from the top, no irrigation, etc.',
    'salinity_control' => 'Whether or not the salinity level of the environment was controlled. True or False.',
    'salinity_level' => 'Must have an associated salinity_control property. If salinity_control is True, this term describes the salinity level the environment was kept at. If salinity_control is False, this term describes the average measured salinity value in the environment.',
    'biotic_environment' => 'Describes the biotic environment.',
    'study_type' => 'Describes the type of environment the trees were in during the study. Possible values are Natural Population (Landscape), Growth Chamber, Greenhouse, Experimental/Common Garden, Plantation.',
    'phenotype_binary_type' => 'Describes one type of a binary phenotype.',
    'file_path' => 'Specifies the path to a file',
    'contact photo' => 'A profile photo for a contact record',
    'gps_latitude' => 'Coordinate that specifies north-south position on Earth\'s surface',
    'gps_longitude' => 'Coordinate that specifies east-west position on Earth\'s surface',
    'association_results_type' => 'The type of the association results.',
    'county' => 'A political and administrative division of a state',
    'district' => 'A distinct area of a geographic entity, such as a country or city',
    'organism 4 letter code' => '4 letter abbreviation of species. Usually first two letters of genus + first two letters of species, may be different if that code already exists (use next letters from species if possible).',
    'cpSSR' => 'Chloroplast simple sequence repeats.',
    'SSR' => 'Simple sequence repeats. Microsatellite.',
    'time' => 'A quality in which events occur in sequence.',
    'source_description' => 'A textual description of study type and comments for a source.',
  );
}

/**
 * Adds a controlled vocabulary to chado based on an OLS ontology id.
 *
 * @param string $ontology_id
 *   The OLS ontology id we will be adding.
 *
 * @return stdClass|bool
 *   The resulting cv object, or FALSE on failure.
 */
function tpps_ols_add_cv($ontology_id) {
  $cv_info = tpps_ols_ontology_info($ontology_id);
  $cv_query = array(
    'name' => $cv_info['name'],
  );

  $db = chado_get_db(array(
    'name' => $cv_info['name'],
  ));
  if (!$db) {
    drupal_set_message("Adding db $ontology_id", 'status');
    chado_insert_db($cv_info);
  }

  $cv = chado_get_cv($cv_query);
  if (!$cv) {
    drupal_set_message("Adding vocabulary $ontology_id", 'status');
    $cv = chado_insert_cv($cv_info['name'], $cv_info['definition']);
  }
  if (!$cv) {
    drupal_set_message("There was an error adding vocabulary $ontology_id", 'error');
  }
  return $cv;
}

/**
 * Adds a controlled vocabulary term to chado based on an OLS term object.
 *
 * @param stdClass $term
 *   The decoded OLS JSON term object returned by the OLS API.
 *
 * @return stdClass|bool
 *   The resulting cvterm object, or FALSE on failure
 */
function tpps_ols_add_cvterm($term) {
  $query = array(
    'name' => $term->label,
    'cv_id' => array(
      'name' => $term->ontology_prefix,
    ),
  );

  $cvt = chado_get_cvterm($query);
  if (!$cvt) {
    drupal_set_message("Adding cvterm {$term->label}, description: {$term->description[0]} to ontology {$term->ontology_prefix}", 'status');
    $cvterm = array(
      'id' => $term->obo_id,
      'name' => $term->label,
      'definition' => $term->description[0],
      'cv_name' => $term->ontology_prefix,
    );
    $cvt = chado_insert_cvterm($cvterm);
  }

  if (!$cvt) {
    drupal_set_message("There was an error adding cvterm {$term->label} to ontology {$term->ontology_prefix}", 'error');
  }

  return $cvt;
}

/**
 * Install a single cvterm to chado from OLS.
 *
 * @param string $info
 *   String of the format '<ontology>:<term name>'.
 *
 * @return stdClass|bool
 *   The resulting cvterm object, or FALSE on failure.
 */
function tpps_ols_install_term($info) {
  $parts = explode(':', $info);
  $ontology = $parts[0];
  if ($ontology == 'local') {
    return 'local';
  }
  $term = $parts[1];

  tpps_ols_add_cv($ontology);
  $response = tpps_ols_search($term, $ontology)->response;
  if ($response->numFound != 1) {
    drupal_set_message("conflict found for cvterm $term from ontology $ontology: $response->numFound terms found", 'error');
    return FALSE;
  }
  return tpps_ols_add_cvterm($response->docs[0]);
}
