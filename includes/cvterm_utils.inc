<?php

/**
 * @file
 * Defines utilities for managing TPPS cvterms.
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
 * @return object
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
  $response = @tpps_ols_call(array(
    'ontologies',
    $id,
  ));
  if (empty($response)) {
    return NULL;
  }
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
    'cvname' => $name,
    'namespace' => $response->config->annotations->{'default-namespace'}[0],
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
 * @return object
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

  print_r('OLS URL:' . $url . "\n");
  try {
    if(function_exists('tpps_job_logger_write')) {
      tpps_job_logger_write('[INFO] -- ' . 'OLS URL:' . $url);
    }
  }
  catch (Exception $ex) {

  }
  return $url;
}

/**
 * Load a cv based on an OLS ontology id or cv_info object.
 *
 * @param array|string $info
 *   Either an OLS ontology id or a cv_info object.
 *
 * @return object|null
 *   The resulting cv object or NULL on failure.
 */
function tpps_load_cv($info) {
  $id = $info;
  if (!is_string($info)) {
    $id = $info['id'];
  }
  $id = strtolower($id);

  $cache = cache_get('tpps_cv')->data ?? array();
  // print_r($cache);

  // Try to load cv_id from cache.
  $cv_id = $cache[$id] ?? NULL;
  if (!empty($cv_id)) {
    return chado_get_cv(array(
      'cv_id' => $cv_id,
    ));
  }

  $cv_info = $info;
  if (is_string($info)) {
    $cv_info = tpps_ols_ontology_info($info);
  }
  if (empty($cv_info)) {
    $cv_info = array(
      'cvname' => $info,
      'namespace' => $info,
    );
  }

  $query = array(
    'name' => array(
      'op' => 'ILIKE',
      'data' => $cv_info['cvname'],
    ),
  );

  $cv = chado_generate_var('cv', $query);
  if (!$cv and !empty($cv_info['namespace'])) {
    $query['name']['data'] = $cv_info['namespace'];
    $cv = chado_generate_var('cv', $query);
  }

  if (is_array($cv)) {
    // This is a critical temporary patch until confirmation from Emily (slack: 10/19/2021 with Rish)
    if($query['name']['data'] == 'OBI') {
      $query['name']['data'] = strtolower($query['name']['data']); // for the use of lowercase OBI
    }
    $query['name']['op'] = 'LIKE';
    $cv = chado_generate_var('cv', $query);
  }

  if (!$cv) {
    return NULL;
  }

  $cache[$id] = $cv->cv_id;
  cache_set('tpps_cv', $cache);
  return $cv;
}

/**
 * Load a cvterm based on a key from tpps_get_cvterms_v<version>().
 *
 * If the key does not appear in tpps_get_cvterms_v<version>(), then we just
 * load the cvterm using the provided key as the name.
 *
 * @param string $term
 *   The key of the cvterm we are trying to load.
 * @param array $options
 *   Optional additional options to be passed to chado_get_cvterm().
 * @param string $version
 *   Optional TPPS cvterm version.
 *
 * @return object|false
 *   The resulting cvterm object, or FALSE on failure.
 */
function tpps_load_cvterm($term, array $options = array(), $version = NULL, $refresh_cache = FALSE) {
  // [VS]
  $check_cache = variable_get('tpps_load_cvterm_' . md5($term), NULL);
  if ($refresh_cache == TRUE) {
    $check_cache = NULL;
  }
  if (isset($_GET['refresh_cache']) && $_GET['refresh_cache'] == 'true') {
    $check_cache = NULL;
  }
  // [/VS]
  if ($check_cache == NULL) {
    $cvt_v = variable_get('tpps_cvterms_version', '1_1');
    $get_cvt = 'tpps_get_cvterms_v' . ($version ?? $cvt_v);
    $terms = $get_cvt();
    $info = $terms[$term] ?? NULL;
    if (empty($info)) {
      $info = ":$term";
    }
    // dpm($info);
    // drupal_set_message('info:'. $info);
    preg_match('/^([^:]*):([^:]+):?([^:]+)?$/', $info, $matches);
    $ontology = $matches[1];
    $name = $matches[2];
    $alt_ontology = $matches[3] ?? NULL;

    $identifiers = array(
      'name' => $term,
      'is_obsolete' => 0,
    );

    if (!empty($ontology)) {
      if ($ontology == 'local') {
        $identifiers['definition'] = $name;
      }
      else {
        $identifiers['name'] = $name;
        $identifiers['cv_id'] = tpps_load_cv($ontology)->cv_id;
      }
    }
    $result = chado_get_cvterm($identifiers, $options);

    // Cater for CV types like GO: Biological Process.
    if (empty($result)) {
      preg_match('/^([\w]*):([\s]+[\w\s]*):([\s\w]*):?([\s\w]*)?$/', $info, $matches2);
      if (count($matches2) > 0) {
        // dpm($matches2);
        $ontology = $matches2[1] . ":" . $matches2[2];
        // drupal_set_message('ONTOLOGY:' . $ontology);
        $name = $matches2[3];
        $alt_ontology = $matches2[4] ?? NULL;
        if (!empty($ontology)) {
          if ($ontology == 'local') {
            $identifiers['definition'] = $name;
          }
          else {
            $identifiers['name'] = $name;
            $identifiers['cv_id'] = tpps_load_cv($ontology)->cv_id;
          }
        }
        $result = chado_get_cvterm($identifiers, $options);
      }
    }

    if (empty($result) && $ontology == 'local') {
      // Added by Rish (1/5/2022)
      unset($identifiers['definition']);
      $identifiers['name'] = $name;
      $identifiers['cv_id'] = tpps_load_cv($ontology)->cv_id;
      $result = chado_get_cvterm($identifiers, $options);
    }
    // drupal_set_message('Identifiers: ' . var_export($identifiers,true));
    // drupal_set_message('CVTERM_ID: ' . $result->cvterm_id);
    // echo var_dump($result);

    if (empty($result) and $ontology != 'local' and !empty($alt_ontology)) {
      $identifiers['cv_id'] = tpps_load_cv($alt_ontology)->cv_id;
      $result = chado_get_cvterm($identifiers, $options);
    }

    if (empty($result)) {
      // Try searching our TreeGenes Internal Ontology (desperate measures: eg study_type)
      $identifiers['cv_id'] = tpps_load_cv('TreeGenes Internal Ontology')->cv_id;
      $result = chado_get_cvterm($identifiers, $options);
    }

    if (empty($result)) {
      // Try searching our TreeGenes Internal Ontology (desperate measures: eg study_type)
      $identifiers['cv_id'] = tpps_load_cv('TreeGenes Internal Ontology')->cv_id;
      $result = chado_get_cvterm($identifiers, $options);
    }

    if (empty($result)) {
      if (!empty($options['job'])) {
        $options['job']->logMessage('[WARNING] Failed to load cvterm: @term', array('@term' => $term), TRIPAL_WARNING);
        throw new Exception('FATAL - after extensive searching, cvterm ' . $term . ' could not be found. This will cause insertion errors later on so it needs to be fixed');
      }
      else {
        $trace = debug_backtrace()[1];
        $job = NULL;
        if (!empty($trace['args']) and gettype($trace['args']) == 'array') {
          foreach ($trace['args'] as $key => $arg) {
            if (gettype($arg) == 'object' and get_class($arg) == 'TripalJob') {
              $job = &$trace['args'][$key];
            }
          }
          if (!empty($job)) {
            $job->logMessage('[WARNING] Failed to load cvterm: @term', array('@term' => $term), TRIPAL_WARNING);
          }
        }
        throw new Exception('FATAL - after extensive searching, cvterm ' . $term . ' could not be found. This will cause insertion errors later on so it needs to be fixed');
      }
    }


    variable_set('tpps_load_cvterm_' . md5($term), serialize($result));
  }
  else {
    $result = unserialize($check_cache);
  }
  return $result;
}

/**
 * Returns nothing, this is an administration function
 *
 * To speed up the process cvterms and even the cv (ontologies)
 * are cached using various methods. This will refresh the cache
 * and update the necessary links in the cache.
 *
 * @return array
 *   Array of cvterms that TPPS will be using.
 */
function tpps_cvterms_clear_cache(TripalJob $job = NULL) {
  if($job) {
    $job->logMessage('[INFO] Initializing cvterms cache refresh...');
  }
  // Reset the cv ids
  cache_set('tpps_cv', NULL);

  $cvt_v = variable_get('tpps_cvterms_version', '1_1');
  $get_cvt = 'tpps_get_cvterms_v' . ($version ?? $cvt_v);
  $terms = $get_cvt();
  foreach ($terms as $term => $ontology_and_term) {
    tpps_load_cvterm($term, array(), $cvt_v, TRUE);
  }
  if($job) {
    $job->logMessage('[INFO] Complete!');
  }
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
    // Local terms.
    'assession_number' => 'local:Number of times the trees were assessed, on average.',
    'temperature_high' => 'local:Average high temperature of the environment.',
    'temperature_low' => 'local:Average low temperature of the environment.',
    'rooting_type' => 'local:Type of rooting. Aeroponics, Hydroponics, or Soil.',
    'organism 4 letter code' => 'local:4 letter abbreviation of species. Usually first two letters of genus + first two letters of species, may be different if that code already exists (use next letters from species if possible).',
    'source_description' => 'local:A textual description of study type and comments for a source.',
    'email' => 'local:Email address',
    'number_samples' => 'local:Number of samples',
    'gps_type' => 'local:Type of GPS coordinates. Exact, Approximate, or Site-based.',
    'gps_precision' => 'local:Precision of GPS coordinates.',

    // PAG 2022 request by Emily
    'precipitation' => 'local:precipitation',
    'absorbance' => 'local:absorbance',
    'water use efficiency' => 'local:water use efficiency',
    'delta' => 'ncit:Delta',
    'carbon-13 atom' => 'chebi_ontology:carbon-13 atom',
    'rate' => 'pato:rate',
    'chlorophyll' => 'chebi:chlorophyll',
    'transpiration' => 'GO: Biological Process:transpiration',
    'photosynthesis' => 'GO: Biological Process:photosynthesis',
    'degrees celsius per millimeter' => 'local:degrees celsius per millimeter',
    'centimeters per day' => 'local:centimeters per day',
    'kilograms per meter cubed' => 'local:kilograms per meter cubed',
    'no unit' => 'local:no unit',
    'log(centimeters per day)' => 'local:log(centimeters per day)',
    'log(centimeters cubed per day)' => 'local:log(centimeters cubed per day)',
    'micromoles carbon dioxide per meter squared per second' => 'local:micromoles carbon dioxide per meter squared per second',
    'micromoles carbon dioxide per gram per second' => 'local:micromoles carbon dioxide per gram per second',
    'absorbance unit' => 'local:absorbance unit',
    'number' => 'local:number',
    'milligrams per millimeter' => 'local:milligrams per millimeter',
    'milligrams per millimeter squared' => 'local:milligrams per millimeter squared',
    'milligrams per milligram' => 'local:milligrams per milligram',
    'micromoles carbon dioxide per gram Nitrogen per second' => 'local:micromoles carbon dioxide per gram Nitrogen per second',
    'micromoles carbon dioxide per millimole water' => 'local:micromoles carbon dioxide per millimole water',
    'moles water per meter squared per second' => 'local:moles water per meter squared per second',

    // Tripal terms.
    'article' => 'tripal_pub:Journal Article',
    'abstract' => 'tripal_pub:Abstract',
    'authors' => 'tripal_pub:Authors',
    'person' => 'tripal_contact:Person',
    'contact_part_of' => 'tripal_contact:part of',
    'organization' => 'tripal_contact:Organization',
    'country' => 'tripal_contact:Country',
    'state' => 'tripal_contact:State',
    'analysis_type' => 'analysis_property:Analysis Type',


    // TREEGENES TO specific
    'gravity' => 'tree_genes_to:specific gravity',

    // PPEO terms.
    'experiment_location' => 'ppeo:location',
    'file_path' => 'ppeo:data file',

    // NCIT terms.
    'study_start' => 'ncit:Study Start Date',
    'district' => 'ncit:Locality',
    'study_end' => 'ncit:Study Completion Date',
    'assession_season' => 'ncit:Season',
    'soil_container' => 'ncit:Container',
    'gps_latitude' => 'ncit:Latitude',
    'gps_longitude' => 'ncit:Longitude',
    'location' => 'ncit:Location',
    'association_results_type' => 'ncit:Data Type',
    'county' => 'ncit:County',
    'read_depth' => 'ncit:Read Depth',
    'format' => 'ncit:Format',
    'minimum' => 'ncit:Minimum',
    'maximum' => 'ncit:Maximum',
    'indel' => 'ncit:Indel Mutation',
    'p_value' => 'ncit:P-Value',
    'lambda' => 'ncit:Lambda',
    'bonferroni' => 'ncit:Adjusted Bonferroni Correction',
    'boolean' => 'ncit:Boolean',

    // PATO terms.
    'age' => 'pato:age',
    'alive' => 'pato:alive',
    'amount' => 'pato:amount',
    'angle' => 'pato:angle',
    'area' => 'pato:area',
    'bent' => 'pato:bent',
    'circumference' => 'pato:circumference',
    'color' => 'pato:color',
    'composition' => 'pato:composition',
    'concentration_of' => 'pato:concentration of',
    'damage' => 'pato:damage',
    'description' => 'pato:description',
    'diameter' => 'pato:diameter',
    'distance' => 'pato:distance',
    'growth_quality_of_occurrent' => 'pato:growth quality of occurrent',
    'growth_rate' => 'pato:growth rate',
    'has_number_of' => 'pato:has number of',
    'height' => 'pato:height',
    'humidity_level' => 'pato:humidity',
    'intensity' => 'pato:intensity',
    'length' => 'pato:length',
    'lesioned' => 'pato:lesioned',
    'maturity' => 'pato:maturity',
    'pH_level' => 'pato:acidity',
    'position' => 'pato:position',
    'pressure' => 'pato:pressure',
    'proportionality_to' => 'pato:proportionality to',
    'qualitative' => 'pato:qualitative',
    'rate' => 'pato:rate',
    'rough' => 'pato:rough',
    'shape' => 'pato:shape',
    'size' => 'pato:size',
    'temperature' => 'pato:temperature',
    'texture' => 'pato:texture',
    'thickness' => 'pato:thickness',
    'time' => 'pato:time',
    'volume' => 'pato:volume',
    'weight' => 'pato:weight',
    'width' => 'pato:width',
    'sex' => 'pato:phenotypic sex',

    // PO terms.
    'whole plant' => 'po:whole plant:plant_ontology',
    'bark' => 'po:bark:plant_anatomy',
    'branch' => 'po:branch:plant_anatomy',
    'bud' => 'po:bud:plant_anatomy',
    'catkin_inflorescence' => 'po:catkin inflorescence:plant_anatomy',
    'endocarp' => 'po:endocarp:plant_anatomy',
    'floral_organ' => 'po:floral organ:plant_anatomy',
    'flower' => 'po:flower:plant_anatomy',
    'flower_bud' => 'po:flower bud:plant_anatomy',
    'flower_fascicle' => 'po:flower fascicle:plant_anatomy',
    'fruit' => 'po:fruit:plant_anatomy',
    'leaf' => 'po:leaf:plant_anatomy',
    'leaf_rachis' => 'po:leaf rachis:plant_anatomy',
    'leaflet' => 'po:leaflet:plant_anatomy',
    'nut_fruit' => 'po:nut fruit:plant_anatomy',
    'petal' => 'po:petal:plant_anatomy',
    'petiole' => 'po:petiole:plant_anatomy',
    'phloem' => 'po:phloem:plant_anatomy',
    'plant_callus' => 'po:plant callus:plant_anatomy',
    'primary_thickening_meristem' => 'po:primary thickening meristem:plant_anatomy',
    // 'root' => 'po:root:plant_anatomy',
    'root' => 'plant_anatomy:root',
    'secondary_xylem' => 'po:secondary xylem:plant_anatomy',
    'seed' => 'po:seed:plant_anatomy',
    'shoot_system' => 'po:shoot system:plant_anatomy',
    'stem' => 'po:stem:plant_anatomy',
    'stomatal_complex' => 'po:stomatal complex:plant_anatomy',
    'strobilus' => 'po:strobilus:plant_anatomy',
    'terminal_bud' => 'po:terminal bud:plant_anatomy',
    'vascular_leaf' => 'po:vascular leaf:plant_anatomy',

    // PECO terms.
    'co2_control' => 'plant_experimental_conditions_ontology:carbon dioxide exposure',
    'humidity_control' => 'plant_experimental_conditions_ontology:humidity exposure',
    'light_control' => 'plant_experimental_conditions_ontology:light intensity exposure',
    'pH_control' => 'plant_experimental_conditions_ontology:pH exposure',
    'treatment' => 'plant_experimental_conditions_ontology:plant exposure',
    'salinity_control' => 'plant_experimental_conditions_ontology:salt exposure',
    'biotic_environment' => 'plant_experimental_conditions_ontology:biotic plant exposure',
    'study_type' => 'plant_experimental_conditions_ontology:study type',

    // ENVO terms.
    'co2_level' => 'envo:atmospheric carbon dioxide',
    'light_level' => 'envo:visible spectrum radiation',
    'soil_type' => 'envo:soil',
    'environment' => 'envo:environmental system',
    'atmosphere' => 'envo:atmosphere',

    // CHEBI terms.
    'salinity_level' => 'chebi:salt',

    // AGRO terms.
    'irrigation_type' => 'agro:irrigation process',

    // SO terms.
    'reference_genome' => 'sequence:reference_genome',
    'genotype' => 'sequence:genotype',
    'sequence_variant' => 'sequence:sequence_variant',
    'snp' => 'sequence:SNP',
    'ssr' => 'sequence:microsatellite',
    'genetic_marker' => 'sequence:genetic_marker',
    'quality_value' => 'sequence:quality_value',
    'allelic_frequency' => 'sequence:allelic_frequency',
    'synonymous' => 'sequence:synonymous',
    'clone' => 'sequence:clone',
    'supercontig' => 'sequence:supercontig',
    'scaffold' => 'sequence:chromosome',
    'has_part' => 'sequence:has_part',

    // EDAM terms.
    'filter' => 'edam:Sequence contamination filtering:operation',

    // UO terms.
    'unit' => 'uo:unit',
    'centimeter' => 'uo:centimeter',
    'cubic_centimeter' => 'uo:cubic centimeter',
    'day' => 'uo:day',
    'degrees_celsius' => 'uo:degree Celsius',
    'degrees_fahrenheit' => 'uo:degree Fahrenheit',
    'grams_per_square_meter' => 'uo:gram per square meter',
    'gram' => 'uo:gram',
    'luminous_intensity_unit' => 'uo:luminous intensity unit',
    'kilogram' => 'uo:kilogram',
    'kilogram_per_cubic_meter' => 'uo:kilogram per cubic meter',
    'liter' => 'uo:liter',
    'cubic_meter' => 'uo:cubic meter',
    'pascal' => 'uo:pascal',
    'meter' => 'uo:meter',
    'milligram' => 'uo:milligram',
    'milliliter' => 'uo:milliliter',
    'millimeter' => 'uo:millimeter',
    'micrometer' => 'uo:micrometer',
    'percent' => 'uo:percent',
    // 'ratio' => 'uo:ratio',
    'square_micrometer' => 'uo:square micrometer',
    'square_millimeter' => 'uo:square millimeter',
    'watt_per_square_meter' => 'uo:watt per square meter',
    'year' => 'uo:year',

    // SBO terms.
    'phenotype' => 'sbo:phenotype',

    // OBI terms.
    'organism' => 'obi:organism',
    'q_value' => 'obi:q-value',
    'p_adj_fwe' => 'obi:FWER adjusted p-value',

    // TAXRANK terms.
    'family' => 'taxonomic_rank:family',
    'subkingdom' => 'taxonomic_rank:subkingdom',
    'order' => 'taxonomic_rank:order',
    'speciesaggregate' => 'taxonomic_rank:speciesaggregate',

    // NCBI terms.
    'common name' => 'ncbitaxon:common name',

    // Abandoned terms.
    'phenotype_binary_type' => NULL,
    'contact photo' => NULL,
    'cpSSR' => NULL,
    'SSR' => NULL,
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
    'study_start' => 'local:The month and year that the study began.',
    'study_end' => 'local:The month and year that the study ended.',
    'experiment_location' => 'local:Geographic location of the experiment.',
    'assession_season' => 'local:Season the trees were assessed.',
    'assession_number' => 'local:Number of times the trees were assessed, on average.',
    'co2_control' => 'local:Whether or not the co2 level of the environment was controlled. True or False.',
    'co2_level' => 'local:Must have an associated co2_control property. If co2_control is True, this term describes the co2 level the environment was kept at. If co2_control is False, this term describes the average measured co2 value in the environment.',
    'humidity_control' => 'local:Whether or not the air humidity level of the environment was controlled. True or False.',
    'humidity_level' => 'local:Must have an associated humidity_control property. If humidity_control is True, this term describes the air humidity level the environment was kept at. If humidity_control is False, this term describes the average measured air humidity value in the environment.',
    'light_control' => 'local:Whether or not the light intensity level of the environment was controlled. True or False.',
    'light_level' => 'local:Must have an associated light_control property. If light_control is True, this term describes the light intensity level the environment was kept at. If light_control is False, this term describes the average measured light intensity value in the environment.',
    'temperature_high' => 'local:Average high temperature of the environment.',
    'temperature_low' => 'local:Average low temperature of the environment.',
    'rooting_type' => 'local:Type of rooting. Aeroponics, Hydroponics, or Soil.',
    'soil_type' => 'local:Type of soil. For example: Sand, Peat, Mixed, etc.',
    'soil_container' => 'local:Type of soil container.',
    'pH_control' => 'local:Whether or not the pH level of the environment was controlled. True or False.',
    'pH_level' => 'local:Must have an associated pH_control property. If pH_control is True, this term describes the pH level the environment was kept at. If pH_control is False, this term describes the average measured pH value in the environment.',
    'treatment' => 'local:Describes the treatment of the trees during the study. For example, air or soil temperature regimes, chemical administration, fertilizer, non-mineral nutrient, or rainfall regimes, disease status, etc.',
    'irrigation_type' => 'local:Describes the type of irrigation. For example, drip irrigation, irrigation from the top, no irrigation, etc.',
    'salinity_control' => 'local:Whether or not the salinity level of the environment was controlled. True or False.',
    'salinity_level' => 'local:Must have an associated salinity_control property. If salinity_control is True, this term describes the salinity level the environment was kept at. If salinity_control is False, this term describes the average measured salinity value in the environment.',
    'biotic_environment' => 'local:Describes the biotic environment.',
    'study_type' => 'local:Describes the type of environment the trees were in during the study. Possible values are Natural Population (Landscape), Growth Chamber, Greenhouse, Experimental/Common Garden, Plantation.',
    'phenotype_binary_type' => 'local:Describes one type of a binary phenotype.',
    'file_path' => 'schema:url',
    'contact photo' => 'local:A profile photo for a contact record',
    'gps_latitude' => 'local:Coordinate that specifies north-south position on Earth\'s surface',
    'gps_longitude' => 'local:Coordinate that specifies east-west position on Earth\'s surface',
    'association_results_type' => 'local:The type of the association results.',
    'county' => 'local:A political and administrative division of a state',
    'district' => 'local:A distinct area of a geographic entity, such as a country or city',
    'organism 4 letter code' => 'local:4 letter abbreviation of species. Usually first two letters of genus + first two letters of species, may be different if that code already exists (use next letters from species if possible).',
    'cpSSR' => 'local:Chloroplast simple sequence repeats.',
    'SSR' => 'local:Simple sequence repeats. Microsatellite.',
    'time' => 'local:A quality in which events occur in sequence.',
    'source_description' => 'local:A textual description of study type and comments for a source.',
    'location' => 'nd_geolocation_property:Location',
    'format' => 'sep:format',
    'environment' => 'biomaterial_property:climate_environment',
    'description' => 'schema:description',
  );
}

/**
 * Adds a controlled vocabulary to chado based on an OLS ontology id.
 *
 * @param string $ontology_id
 *   The OLS ontology id we will be adding.
 *
 * @return object|bool
 *   The resulting cv object, or FALSE on failure.
 */
function tpps_ols_add_cv($ontology_id) {
  $cv_info = tpps_ols_ontology_info($ontology_id);
  $query = array(
    'name' => array(
      'op' => 'ILIKE',
      'data' => $cv_info['cvname'],
    ),
  );

  $db = chado_get_db($query);
  if (!$db) {
    drupal_set_message("Adding db $ontology_id", 'status');
    chado_insert_db(array(
      'name' => $cv_info['cvname'],
      'description' => $cv_info['description'],
      'url' => $cv_info['url'],
      'urlprefix' => $cv_info['urlprefix'],
    ));
  }

  $cv = tpps_load_cv($cv_info);

  if (!$cv) {
    drupal_set_message("Adding vocabulary $ontology_id", 'status');
    $cv = chado_insert_cv($cv_info['name'], $cv_info['description']);
  }
  if (!$cv) {
    drupal_set_message("There was an error adding vocabulary $ontology_id", 'error');
  }
  return $cv;
}

/**
 * Adds a controlled vocabulary term to chado based on an OLS term object.
 *
 * @param object $term
 *   The decoded OLS JSON term object returned by the OLS API.
 *
 * @return object|bool
 *   The resulting cvterm object, or FALSE on failure
 */
function tpps_ols_add_cvterm($term) {
  $cv = tpps_load_cv($term->ontology_name);

  $query = array(
    'name' => $term->label,
    'cv_id' => $cv->cv_id,
  );

  $cvt = chado_get_cvterm($query);
  if (!$cvt) {
    drupal_set_message("Adding cvterm {$term->label}, description: {$term->description[0]} to ontology {$term->ontology_prefix}", 'status');
    $cvterm = array(
      'id' => $term->obo_id,
      'name' => $term->label,
      'definition' => $term->description[0],
      'cv_name' => $cv->name,
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
 * @return object|string|bool
 *   The resulting cvterm object, or "local", or FALSE on failure.
 */
function tpps_ols_install_term($info) {
  // print_r('tpps_ols_install_term:');
  // print_r('overall_info:' . $info . "\n");
  $parts = explode(':', $info);
  $ontology = $parts[0];
  if ($ontology == 'local') {
    return 'local';
  }
  $term = $parts[1];
  $alt_ontology = $parts[2] ?? NULL;

  $cv = tpps_load_cv($ontology);
  if (!$cv) {
    $cv = tpps_ols_add_cv($ontology);
  }


  // print_r('term:' . $term . "\n");
  // print_r('cv_id:' . $cv->cv_id . "\n");
  $cvt = chado_get_cvterm(array(
    'name' => $term,
    'cv_id' => $cv->cv_id,
  ));

  if (!$cvt and !empty($alt_ontology)) {
    $cv = tpps_load_cv($alt_ontology);
    if ($cv) {
      $cvt = chado_get_cvterm(array(
        'name' => $term,
        'cv_id' => $cv->cv_id,
      ));
    }
  }

  if(!$cvt) {
    // Double check original term
    try {
      // Since tpps_load_cvterm is used else where for require data
      // we had set this to throw Exceptions if a cvterm isn't found
      // so this is why it is placed in a try clause
      $cvt = tpps_load_cvterm($term);
    }
    catch (Exception $ex) {

    }
  }

  // Try searching for the term with the lowercase of the term
  // Added by Rish on 02/20/2023
  if(!$cvt) {
    try {
      $cvt = tpps_load_cvterm(strtolower($term));
    }
    catch (Exception $ex) {

    }
  }

  if (!$cvt) {
    print_r("Searching OLS search for term: $term with ontology: $ontology\n");
    try {
      tpps_job_logger_write("[INFO] -- Searching OLS search for term: $term with ontology: $ontology");
    }
    catch (Exception $ex) {

    }
    $response = tpps_ols_search($term, $ontology)->response;
    $doc_index = 0; // default value of 0 (index from OLS search for first value found)
    if ($response->numFound == 0) {
      return FALSE;
    }
    if ($response->numFound != 1) {
      $docs = array();
      foreach ($response->docs as $doc) {
        $docs[$doc->iri] = $doc;
      }
      print_r($response->docs);
      if (count($docs) != 1) {
        // RISH: 10/06/2022 - I coded this for po:root but it is probably better
        // to make sure the term exists in cvterms than use this method
        // also update the function with the term, the function name is tpps_get_cvterms_v1_2()
        //
        // // try to find a defining ontology based on the json results, if one can be found, use that
        // $found_alternative = false;
        // for($doc_index = 0; $doc_index < count($response->docs); $doc_index++) {
        //   $doc = $response->docs[$doc_index];
        //   print_r('doc:');
        //   print_r($doc);
        //   if (!empty($doc->is_defining_ontology)) {
        //     // we found a possible legit term (this was tested with po:root)
        //     if($doc->is_defining_ontology == 'true') {
        //       print_r('Found a valid alternative.' . "\n");
        //       $found_alternative = true;
        //       break;
        //     }
        //   }
        // }
        // if($found_alternative == false) {
        //   throw new \Exception("conflict found for cvterm $term from ontology $ontology: $response->numFound terms found");
        // }
        throw new \Exception("conflict found for cvterm $term from ontology $ontology: $response->numFound terms found");
      }
      $response->docs[$doc_index] = current($docs);
    }
    $cvt = tpps_ols_add_cvterm($response->docs[$doc_index]);
  }
  return $cvt;
}

/**
 * Installs a local TPPS cvterm based on a provided key.
 *
 * This function should be used if the tpps_ols_install_term() function returns
 * "local".
 *
 * @param string $key
 *   The key of the TPPS cvterm being installed.
 * @param string $version
 *   The TPPS cvterm version to use.
 */
function tpps_local_install_term($key, $version = NULL) {
  if (empty($version)) {
    $version = TPPS_MAJOR_VERSION . '_' . TPPS_MINOR_VERSION;
  }
  $get_cvt = 'tpps_get_cvterms_v' . $version;
  $terms = $get_cvt();
  $info = $terms[$key] ?? NULL;
  if (empty($info)) {
    throw new \Exception("Error installing term $key: Term info could not be found for TPPS cvterm version $version.");
  }

  $local_db = variable_get('tpps_local_db');
  $parts = explode(':', $info);
  $term = $parts[1];
  $cvt = chado_get_cvterm(array(
    'name' => $key,
    'definition' => $term,
    'is_obsolete' => 0,
  ));

  if (!$cvt) {
    drupal_set_message("Creating local cvterm $key", 'status');
    $cvt = chado_insert_cvterm(array(
      'id' => "{$local_db->name}:$key",
      'name' => $key,
      'definition' => $term,
      'cv_name' => 'local',
    ));

    if (!$cvt) {
      drupal_set_message("Error creating local cvterm $key", 'error');
    }
  }

  return $cvt;
}

/**
 * Adds a cvtermsynonym record for two matching cvterms.
 *
 * This is useful for migrating from an older TPPS cvterm version to a new one.
 *
 * @param object $old_cvt
 *   The old cvterm object returned by chado_get_cvterm().
 * @param object $new_cvt
 *   The new cvterm object returned by chado_get_cvterm().
 */
function tpps_match_cvterms($old_cvt, $new_cvt) {
  if (empty($old_cvt->name)) {
    return;
  }
  $result = db_select('chado.cvtermsynonym', 'cs')
    ->fields('cs', array('cvtermsynonym_id'))
    ->condition('cvterm_id', $new_cvt->cvterm_id)
    ->condition('synonym', $old_cvt->name)
    ->range(0, 1)
    ->execute()->fetchObject() ?? NULL;

  if (empty($result)) {
    db_insert('chado.cvtermsynonym')
      ->fields(array(
        'cvterm_id' => $new_cvt->cvterm_id,
        'synonym' => $old_cvt->name,
        'type_id' => chado_get_cvterm(array(
          'name' => 'exact',
          'cv_id' => array(
            'synonym_type',
          ),
        ))->cvterm_id,
      ))
      ->execute();
  }

  $old_synonyms = db_select('chado.cvtermsynonym', 'c')
    ->fields('c', array('synonym', 'type_id'))
    ->condition('cvterm_id', $old_cvt->cvterm_id)
    ->execute();

  while (($syn = $old_synonyms->fetchObject())) {
    $result = db_select('chado.cvtermsynonym', 'cs')
      ->fields('cs', array('cvtermsynonym_id'))
      ->condition('cvterm_id', $new_cvt->cvterm_id)
      ->condition('synonym', $syn->synonym)
      ->range(0, 1)
      ->execute()->fetchObject() ?? NULL;

    if (empty($result)) {
      db_insert('chado.cvtermsynonym')
        ->fields(array(
          'cvterm_id' => $new_cvt->cvterm_id,
          'synonym' => $syn->synonym,
          'type_id' => $syn->type_id,
        ))
        ->execute();
    }
  }
}

/**
 * Migrate TPPS cvterms from an old version to a new version.
 *
 * This is useful if a term that was previously in a local ontology is being
 * moved to a standardized ontology from OLS. New cvterms inherit all of the
 * same synonyms as the old cvterms. If version numbers are left blank, then
 * the old version number will be imported from the tpps_cvterms_version
 * variable and the new version number will be imported by the
 * TPPS_MAJOR_VERSION and TPPS_MINOR_VERSION constants defined in tpps.module.
 *
 * @param int $old_major
 *   Major version of the old cvterms.
 * @param int $old_minor
 *   Minor version of the old cvterms.
 * @param int $new_major
 *   Opotional major version of the new cvterms.
 * @param int $new_minor
 *   Opotional minor version of the new cvterms.
 */
function tpps_migrate_cvterms($old_major = NULL, $old_minor = NULL, $new_major = TPPS_MAJOR_VERSION, $new_minor = TPPS_MINOR_VERSION) {
  $old_v = variable_get('tpps_cvterms_version', '1_1');
  if (!empty($old_major) and !empty($old_minor)) {
    $old_v = $old_major . '_' . $old_minor;
  }
  $new_v = $new_major . '_' . $new_minor;
  if ($new_v === $old_v) {
    return;
  }
  $old_term_func = 'tpps_get_cvterms_v' . $old_v;
  $new_term_func = 'tpps_get_cvterms_v' . $new_v;
  $old_terms = $old_term_func();
  $new_terms = $new_term_func();
  $transaction = db_transaction();

  try {
    foreach ($old_terms as $key => $old_info) {
      $new_info = $new_terms[$key] ?? NULL;
      if (!isset($new_info) or !isset($old_info)) {
        continue;
      }

      if ($old_info == $new_info) {
        continue;
      }

      preg_match('/^([^:]*):(.*)$/', $old_info, $matches);
      $old_ont = $matches[1];
      $old_term = $matches[2];

      if ($old_ont == 'local') {
        $old_cvt = chado_get_cvterm(array(
          'name' => $key,
          'definition' => $old_term,
        ));
      }
      else {
        $old_cvt = chado_get_cvterm(array(
          'name' => $old_term,
          'cv_id' => tpps_load_cv($old_ont)->cv_id,
        ));
      }

      $new_cvt = tpps_ols_install_term($new_info);
      if ($new_cvt === FALSE) {
        throw new \Exception("No OLS terms found for term $new_info");
      }
      unset($new_terms[$key]);
      tpps_match_cvterms($old_cvt, $new_cvt);
    }
    foreach ($new_terms as $key => $info) {
      if (!empty($info)) {
        // print_r($info);
        // echo "\n";
        $result = tpps_ols_install_term($info);
        if ($result === 'local') {
          tpps_local_install_term($key, $new_v);
        }
      }
    }

    variable_set('tpps_cvterms_version', $new_v);
  }
  catch (\Exception $e) {
    $transaction->rollback();
    throw $e;
  }
}
