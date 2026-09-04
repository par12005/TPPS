<?php

/**
 * @file
 * Contains 'Organism' class.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Usage:
 *
 * To check if organism exists in DB:
 *   tpps_organism($id)->doesExists();
 * To add new organism:
 *   tpps_organism($name)->save();
 *
 * To get organism id by name:
 *   tpps_organism($organism_name)->id;
 * To get organism name by id:
 *   tpps_organism($organism_id)->name;
 *
 * To get NCBI Taxonomy Id:
 *   tpps_organism($id)->requestNcbiTaxonomyId()->ncbiTaxonomyId;
 * To get GBIF Taxonomy Id:
 *   tpps_organism($id)->requestGbifTaxonomyId()->gbifTaxonomyId;
 * To load organism from DB:
 *   tpps_organism($organism_id);
 *   tpps_organism($organism_name);
 *
 * Not implemented:
 *
 * To validate organism name:
 *   tpps_organism($name)->isValid();
 */

/**
 * Manages TPPS Organism.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class Organism {

  /**
   * Name of the table where data is stored.
   *
   * @todo Rename to TABLE.
   */
  const TABLE_ORGANISM = 'chado.organism';

  /**
   * NCBI Taxonomy Id.
   *
   * Stored in NcbiTaxonomy::TABLE.id field.
   * See tpps_ncbi_get_taxon_id().
   *
   * @var int
   */
  protected $ncbiTaxonomyId;

  /**
   * GBIF Taxonomy Id.
   *
   * Stored in GbifSpeciesApi::TABLE.id field.
   *
   * @var int
   */
  protected $gbifTaxonomyId;

  /**
   * Canonical full organism name.
   *
   * @var string
   */
  protected $name;

  /**
   * Parsed Organism Name.
   *
   * See getEmptyParsedName() method for keys.
   *
   * @var array
   */
  protected $parsedName = NULL;

  /**
   * Organism Id.
   *
   * @var int
   */
  protected $id;

  /**
   * Organism Genus. Required 1st part of the full name.
   *
   * DB: chado.organism.genus.
   *
   * @var string
   */
  protected $genus;

  /**
   * Organism Species. Required 2nd part of the full name.
   *
   * DB: chado.organism.species.
   *
   * @var string
   */
  protected $species;

  /**
   * Organism infraspecific Rank.
   *
   * DB: chado.organism.type_id.
   *
   * @var string
   * @todo Which cvterm should be used?
   */
  protected $infraspecificRank;

  /**
   * Organism infraspecific name. Optional 3rd part of the full name.
   *
   * DB: chado.organism.infraspecific_name.
   *
   * @var string
   */
  protected $infraspecificName;

  /**
   * Class constructor.
   *
   * @param mixed $organism_key
   *   Organism Name or Organism Id.
   */
  public function __construct($organism_key = NULL) {
    $this->parsedName = Organism::getEmptyParsedName();
    if (!empty($organism_key)) {
      if (is_numeric($organism_key)) {
        $this->id = $organism_key;
      }
      else {
        // Key is a full organism name.
        // @todo Minor. Validate organism name.
        $this->setName($organism_key);
      }
      $this->load();
    }
    return $this;
  }

  /**
   * Magic setter.
   *
   * @param string $name
   *   Class property name.
   * @param mixed $value
   *   Value to be set.
   *
   * @throws Exception
   *   When requested non-existing property.
   */
  public function __set($name, $value) {
    // Allow to overwrite magic with custom setter.
    $func = tpps_snake_to_camel_case('set_' . $name);
    if (method_exists($this, $func)) {
      $this->$func($value);
    }
    else {
      if (property_exists($this, $name)) {
        $this->$name = $value;
      }
      else {
        throw new Exception("Inexistent property: $name");
      }
    }
    return $this;
  }

  /**
   * Magic getter.
   *
   * @param string $name
   *   Class property name.
   *
   * @return mixed
   *   Returns property value.
   *
   * @throws Exception
   *   When requested non-existing property.
   */
  public function &__get($name) {
    // Allow to overwrite magic with custom getter.
    $func = tpps_snake_to_camel_case('get_' . $name);
    if (method_exists($this, $func)) {
      return $this->$func();
    }
    else {
      if (property_exists($this, $name)) {
        return $this->$name;
      }
      else {
        throw new Exception("Inexistent property: $name");
      }
    }
    return $this;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Custom setter for $name property.
   *
   * @param string $value
   *   Organism name.
   *
   * @return object
   *   Returns $this to allow method chaining.
   */
  public function setName($value) {
    if (empty($value)) {
      throw new Exception('Empty organism name.');
    }
    $this->name = $value;
    $this->parsedName = Organism::parseName($value);
    $this->genus = $this->parsedName['genus'] ?? NULL;
    if (empty($this->genus)) {
      throw new Exception('Invalid organism name.');
    }
    $this->species = $this->parsedName['species'] ?? NULL;
    $this->infraspecificRank = $this->parsedName['infraspecific_rank'] ?? NULL;
    $this->infraspecificName = $this->parsedName['infraspecific_name'] ?? NULL;

    return $this;
  }

  /**
   * Reload organism from database by organism name.
   *
   * WARNING:
   * Currently unsaved data will be lost and overwritten by data from DB.
   */
  public function reload() {
    if (!empty($this->id)) {
      unset($this->name);
    }
    if (!empty($this->name)) {
      unset($this->id);
    }
    $this->load();
  }

  /**
   * Load organism from database by organism name.
   *
   * It's safe to call this method because existing data won't be overwritten.
   * Note: data from db table 'chado.organism' will be loaded.
   */
  public function load() {
    if ($this->isLoaded()) {
      // Do not load multiple times. See $this->reload().
      return;
    }
    $query = db_select('chado.organism', 'co')
      ->fields('co')
      ->range(0, 1);
    if (empty($this->id)) {
      // Load by name.
      if (!empty($this->name)) {
        if (!empty($this->genus)) {
          $query->condition('genus', $this->genus);
        }
        if (!empty($this->species)) {
          $query->condition('species', $this->species);
        }
        if (!empty($this->infraspecificName)) {
          // @TODO Major. Some names has only genus + authority.
          // @TODO infraspecificRank must be not part of the name but
          // stored in 'type_id' field and cvterm_id must be used.
          $infraspecific_name = (empty($this->infraspecificRank)
            ? '' : $this->infraspecificRank . ' ') . $this->infraspecificName;
          $query->condition('infraspecific_name', $infraspecific_name);
        }
      }
      else {
        $message = t("Can't load organism because there are no organism id and name.");
        watchdog('tpps', $message, [], WATCHDOG_ERROR);
        return;
      }
    }
    else {
      // Load by id.
      $query->condition('organism_id', $this->id);
    }

    $organism = $query->execute()->fetchAssoc();
    // Set properties.
    if (empty($this->id)) {
      // Loaded by name.
      $this->id = $this->id ?? $organism['organism_id'] ?? NULL;
    }
    else {
      // Loaded by 'organism_id'.
      $this->genus = $organism['genus'] ?? NULL;
      $this->species = $organism['species'] ?? NULL;
      $this->infraspecificName = $organism['infraspecific_name'] ?? NULL;
    }
    // There is also columns:
    // abbreviation,
    // common_name,
    // type_id,
    // comment.
    $this->generateName();
    return $this;
  }

  /**
   * Checks if organism was loaded from DB.
   *
   * @return bool
   *   Returns TRUE if organism was loaded and FALSE otherwise.
   */
  public function isLoaded() {
    // When organism loaded then property 'name' will be generated and there
    // is no other way to generate. This could be used to check if organism
    // was loaded.
    return (bool) (!empty($this->name) && !empty($this->id));
  }

  /**
   * Builds full organism name using name parts from DB table.
   */
  public function generateName() {
    // @todo Minor. Use $this->parsedName if not empty.
    $name_parts = [
      $this->genus,
      $this->species,
      $this->infraspecificRank,
      $this->infraspecificName,
    ];
    $name = trim(implode(' ', $name_parts));
    // Replace multiple spaces with a single space.
    $name = preg_replace('/\s+/', ' ', $name);

    // Check if $this->name and new name differs.
    if (!empty($this->name) && $this->name != $name) {
      watchdog('tpps', 'Existing organism name and generated name differs:'
        . '<br />Existing name: "@existing_name" (@existing_name_len).'
        . '<br />Generated name: "@existing_name" (@new_name_len).',
        [
          '@existing_name' => json_encode($this->name),
          '@new_name' => json_encode($name),
          '@existing_name_len' => strlen($this->name),
          '@new_name_len' => strlen($name),
        ],
        WATCHDOG_WARNING
      );
    }
    $this->setName($name);
    return $this;
  }

  /**
   * Extracts genus, species and extra parts from the organism name.
   *
   * @param string $name
   *   Organism name.
   *
   * @return mixed
   *   When called statically returns an accosiative array with result of the
   *   parsing.
   *   When called from object returns $this to allow chaining and result
   *   will be stored in the class's properties.
   *
   * @throws Exception
   *   When organism name is empty.
   */
  public static function parseName($name) {
    $result = Organism::getEmptyParsedName();
    $name = trim($name);
    // Check if namei is valid.
    // First genus letter is '?' or in lowercase.
    // Crosses which has ' x ' substring.
    if (
      preg_match('/^([a-z\?])/', $name, $matches)
      || preg_match('/(\sx\s)/', $name, $matches)
    ) {
      return $result;
    }

    // Handle cases where there's no species (just genus).
    if (preg_match('/^([A-Z][a-z]+)$/', $name, $matches)) {
      $result['genus'] = $matches[1];
      return $result;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // 'Genus species' or
    // 'Genus species 2nd-word'.
    $pattern = '/^
      ([A-Z][a-z]+)                  # 1. Genus
      \s+
      ([a-z-]+(?:\s*[a-z-.]+)?)      # 2. Species (1 or 2 words).
      $/xu';
    if (preg_match($pattern, $name, $matches)) {
      $result['genus'] = $matches[1] ?? NULL;
      $result['species'] = $matches[2] ?? NULL;
      return $result;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Allow optional dot after rank
    $pattern = '/\s
      (
        (?:subsp|subspecies|var|variety|f|forma|subvar|subf|proles|nothosubsp|nothovar|spp)
        \.*
      )\s/x';
    preg_match($pattern, $name, $matches);
    $infraspecific_rank = $matches[1] ?? NULL;
    if (!empty($infraspecific_rank)) {
      $result['infraspecific_rank'] = $infraspecific_rank;
      // Split by rank.
      $parts = explode($matches[0], $name, 2);
    }
    elseif (strpos($name, '(') !== FALSE) {
      $parts = preg_split('/(?=[\(])/', $name, 2);
    }
    elseif (
      // Find all uppercase letters with positions.
      preg_match_all('/[A-Z]/', $name, $matches, PREG_OFFSET_CAPTURE) >= 2
    ) {
      $second_uppercase_char_position = $matches[0][1][1];
      $parts = [
        substr($name, 0, $second_uppercase_char_position),
        substr($name, $second_uppercase_char_position)
      ];
    }
    else {
      // @TODO Check if there only 2 words.
      $parts = [$name];
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get genus+species.
    $pattern = '/^
      ([A-Z][a-z-]+)                         # 1. Genus
      \s+
      ([\×a-z-]+(?:\s*[\×a-z-.]+)?)          # 2. Species (including ×)
      /xu';
    if (preg_match($pattern, trim($parts[0]), $matches)) {

      $result['genus'] = $matches[1] ?? NULL;
      $result['species'] = $matches[2] ?? NULL;
    }

    if (empty($parts[1])) {
      return $result;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get infraspecific name.
    // Marker: first char is lowercase.
    $pattern = '/^
        ([\×a-z-]+(?:\s*[\×a-z-.]+)?)        # Infraspecific name (including ×)
        # Not to the end of string.
      /xu';
    preg_match($pattern, trim($parts[1]), $matches);
    if (!empty($matches[1])) {
      $result['infraspecific_name'] = $matches[1];
      $parts[1] = str_replace($result['infraspecific_name'], '', $parts[1]);
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get Year.
    $pattern = '/,*\s*(\d{4})$/';
    preg_match($pattern, trim($parts[1]), $matches);
    if (!empty($matches[1])) {
      $result['year'] = $matches[1];
      // Note: replace with leading comma and space.
      $parts[1] = trim(str_replace($matches[0], '', $parts[1]));
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Authority in parentheses (optional).
    $pattern = '/\(([^)]+)\)/';
    preg_match($pattern, trim($parts[1]), $matches);

    if (!empty($matches[1])) {
      $result['authority_in_parentheses'] = $matches[1];
      $parts[1] = trim(str_replace($matches[0], '', $parts[1]));
    }

    if (!empty($parts[1])) {
      // Parse cases like: Pinus malletii hort. ex Mottet
      // 0: Pinus malletii hort. ex;
      // 1: Mottet.
      $extra = trim(str_replace(
        $result['genus'] . ' ' . $result['species'], '', $parts[0])
      );
      $result['authority'] = (empty($extra) ? '' : $extra . ' ') . trim($parts[1]);
    }
    return $result;
  }

  /**
   * Get GBIF Taxonomy Id from the remote server.
   */
  public function requestGbifTaxonomyId() {
    $this->load();
    if (empty($this->name)) {
      $message = t("Can't get GBIF Taxonomy Id because there are no organism name.");
      watchdog('tpps', $message, [], WATCHDOG_ERROR);
      drupal_set_message($message);
      return;
    }

    try {
      $this->gbifTaxonomyId = GbifSpeciesApi::getId($this->name);
    }
    catch (Exception $e) {
      watchdog('tpps', "Can't get data from GBIF database: @message",
        ['@message' => $e->getMessage()],
        WATCHDOG_ERROR
      );
      return FALSE;
    }

    if (empty($this->gbifTaxonomyId)) {
      // We didn't get Taxonomy Id.
      return FALSE;
    }
    // Provide chaining only in case of success.
    return $this;
  }

  /**
   * Get NCBI Taxonomy Id from the remote server.
   */
  public function requestNcbiTaxonomyId() {
    $this->load();
    if (empty($this->name)) {
      $message = t("Can't get NCBI Taxonomy Id because there are no organism name.");
      watchdog('tpps', $message, [], WATCHDOG_ERROR);
      drupal_set_message($message);
      return;
    }

    try {
      $this->ncbiTaxonomyId = NcbiTaxonomy::getId($this->name);
    }
    catch (Exception $e) {
      watchdog('tpps', "Can't get data from NCBI database: @message",
        ['@message' => $e->getMessage()],
        WATCHDOG_ERROR
      );
      return FALSE;
    }

    if (empty($this->ncbiTaxonomyId)) {
      // We didn't get Taxonomy Id.
      return FALSE;
    }
    // Provide chaining only in case of success.
    return $this;
  }

  /**
   * Saves organism to db.
   */
  public function save() {
    $fields = [
      'organism_id' => $this->id ?? NULL,
      'abbreviation' => '',
      'genus' => $this->genus ?? '',
      'species' => $this->species ?? '',
      'common_name' => '',
      'infraspecific_name' => $this->infraspecificName ?? '',
      'type_id' => '',
      'comment' => '',
    ];
    if ($this->isRecordExists()) {
      // Update.
      unset($fields['organism_id']);
      db_update(self::TABLE_ORGANISM)
        ->fields($fields)
        ->condition('organism_id', $this->id)
        ->execute();

    }
    else {
      // Insert.
      db_insert(self::TABLE_ORGANISM)->fields($fields)->execute();
    }
  }

  /**
   * Checks if organism exists in DB table 'chado.organism'.
   *
   * Function db_select() is slower than db_query().
   * https://stackoverflow.com/a/21481960/1041470
   *
   * @return bool
   *   Returns TRUE if record was found in DB table and FALSE otherwise.
   *
   * @todo Minor. Allow to set key and it's value to use for many tables.
   * @TODO Add to Submission class the same code.
   */
  public function isRecordExists($table = '') {
    $organism_id = $this->id ?? NULL;
    if (empty($organism_id)) {
      return FALSE;
    }
    if (empty($table)) {
      $table = static::TABLE_ORGANISM;
    }
    $sql = 'select organism_id from {' . $table
      . '} where organism_id = :id limit 1';
    return (bool) (db_query($sql, [':id' => $organism_id])->fetchField());
  }

  /**
   * Checks if organism exists in database.
   *
   * @return bool
   *   Returns TRUE if record in database exists and FALSE otherwise.
   */
  public function doesExist() {
    $this->load();
    return (bool) (!empty($this->name) && !empty($this->id));
  }

  /**
   * Check if organism doesn't exist.
   *
   * @return bool
   *   Returns TRUE if record in database exists and FALSE otherwise.
   */
  public function doesNotExist() {
    return !($this->doesExist());
  }

  /**
   * Processes NCBI Taxonomy Id from NCBI and stores in correct table.
   *
   * Original idea: submit_all.php:875
   * $taxons = tpps_ncbi_get_taxon_id($raw_name, TRUE);
   *
   * DB tables which will be updated:
   * 'chado.dbxref',
   * 'chado.organism_dbxref'.
   *
   * @param mixed $submission
   * @param mixed $use_log
   *
   * @return void
   */
  public function processTaxonomyId($submission, $use_log = NULL) {
    $this->load();

    $use_log = $use_log ?? function_exists('tpps_log');
    $taxons = tpps_ncbi_get_taxon_id($this->name, TRUE);
    if ($use_log) {
      tpps_log("NCBI taxon data found: \n@taxons_dump",
        ['@taxons_dump' => print_r($taxons, 1)], TRIPAL_DEBUG
      );
    }
    $taxons = json_decode(json_encode($taxons))->Id;
    if (empty($taxons) || count($taxons) === 0) {
      if ($use_log) {
        tpps_log("This study contains a species in which we could not find a "
          . "matching record on NCBI: @name",
          ['@name' => $raw_name]
        );
      }
    }

    if (empty($taxons)) {
      if ($use_log) {
        tpps_log("NO NCBI ID could be found from NCBI for $genus $species $infra",
          [], TRIPAL_WARNING);
      }
    }
    else {
      // DB Id: 93.
      $ncbi_db_id = ChadoDb::getId('NCBI Taxonomy');

      $ncbi_id = $taxons;
      // Check to see if taxon exists in dbxref by getting the dbxref_id for it
      $results_dbxref = chado_query('SELECT * FROM chado.dbxref WHERE accession = :accession AND db_id = :db_id', [
        ':accession' => $ncbi_id,
        ':db_id' => $ncbi_db_id
      ]);
      $dbxref_id = NULL;
      foreach ($results_dbxref as $dbxref_row) {
        $dbxref_id = $dbxref_row->dbxref_id;
      }

      // If there is no NCBI ID in dbxref, add it
      if ($dbxref_id == NULL) {
        if ($use_log) {
          tpps_log('Did not find NCBI ID @ncbi_id, adding it to dbxref table.',
            ['@ncbi_id' => $ncbi_id],
            TRIPAL_INFO
          );
        }
        $dbxref_id = tpps_chado_insert_record('dbxref', [
          'db_id' => $ncbi_db_id,
          'accession' => $ncbi_id,
          'version' => '',
        ]);
      }
      else {
        if ($use_log) {
          tpps_log('NCBI ID @ncbi_id already exists in dbxref table.',
            ['@ncbi_id' => $ncbi_id], TRIPAL_INFO);
        }
      }

      // Now we still need to make sure there is no organism_dbxref record, search for one.
      $organism_dbxref_id = NULL;
      $results_organism_dbxref = chado_query('SELECT * FROM chado.organism_dbxref
        WHERE organism_id = :organism_id
        AND dbxref_id = :dbxref_id',
      [
        ':organism_id' => $this->id,
        ':dbxref_id' => $dbxref_id
      ]);
      foreach ($results_organism_dbxref as $organism_dbxref_row) {
        $organism_dbxref_id = $organism_dbxref_row->organism_dbxref_id;
      }

      // If there is no organism_dbxref_id, we need to add it to the organism_dbxref table.
      if ($organism_dbxref_id == NULL) {
        $organism_dbxref_id = tpps_chado_insert_record('organism_dbxref', [
          'organism_id' => $this->id,
          'dbxref_id' => $dbxref_id,
        ]);
        if ($use_log) {
          tpps_log("Did not find organism_dbxref_id, adding it to the organism_dbxref table.");
        }
      }
      else {
        if ($use_log) {
          tpps_log("organism_dbxref_id $organism_dbxref_id already exists in organism_dbxref table.");
        }
      }
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $shared_state = $submission->sharedState;
    $page1_values = $submission->sharedState['saved_values'][TPPS_PAGE_1];
    $i = $this->getIndex($submission);

    if (!empty(tpps_load_cvterm('Type'))) {
      tpps_chado_insert_record('organismprop', [
        'organism_id' => $this->id,
        'type_id' => tpps_load_cvterm('Type')->cvterm_id,
        'value' => $page1_values['organism'][$i]['is_tree'] ? 'Tree' : 'Non-tree',
      ]);
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $code_exists = tpps_chado_prop_exists(
      'organism',
      $shared_state['ids']['organism_ids'][$i],
      'organism 4 letter code'
    );

    if (!$code_exists) {
      foreach (tpps_get_species_codes($genus, $species) as $trial_code) {
        $new_code_query = chado_select_record('organismprop', ['value'], [
          'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id,
          'value' => $trial_code,
        ]);

        if (empty($new_code_query)) {
          break;
        }
      }

      tpps_chado_insert_record('organismprop', array(
        'organism_id' => $shared_state['ids']['organism_ids'][$i],
        'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id,
        'value' => $trial_code,
      ));
    }

    $ranks = array(
      'family',
      'order',
      'subkingdom',
    );

    foreach ($ranks as $rank) {
      $exists = tpps_chado_prop_exists(
        'organism', $shared_state['ids']['organism_ids'][$i], $rank
      );
      if (!$exists) {
        $taxon = tpps_get_taxon($page1_values['organism'][$i]['name'], $rank);
        if ($taxon) {
          tpps_chado_insert_record('organismprop', array(
            'organism_id' => $shared_state['ids']['organism_ids'][$i],
            'type_id' => tpps_load_cvterm($rank)->cvterm_id,
            'value' => $taxon,
          ));
        }
      }
    }

    tpps_chado_insert_record('project_organism', array(
      'organism_id' => $shared_state['ids']['organism_ids'][$i],
      'project_id' => $shared_state['ids']['project_id'],
    ));

    if ($use_log) {
      tpps_log("Adding " . $shared_state['ids']['organism_ids'][$i] . " to pub_organism");
    }


    // @TOdO Get from db.
    $publication_id = $shared_state['ids']['pub_id'];
    if (empty($publication_id)) {
      // @TODO Get from db.
      //$publication_id = tpps_chado_insert_record('pub', [
      //  'title' => $publication_title,
      //  'series_name' => $publication_journal,
      //  'type_id' => tpps_load_cvterm('article')->cvterm_id,
      //  'pyear' => $publication_year,
      //  'uniquename' => implode('; ', $authors)
      //    . " $publication_title. $publication_journal; $publication_year",
      //]);
    }

    tpps_chado_insert_record('pub_organism', array(
      'organism_id' => $shared_state['ids']['organism_ids'][$i],
      'pub_id' => $publication_id,
    ));

    // Not needed for already processed studies.
    //tpps_tripal_entity_publish('Organism', array(
    //  "$genus $species",
    //  $shared_state['ids']['organism_ids'][$i],
    //));
  }

  /**
   * Gets ordinal number of the organism on TPPS Form.
   *
   * @param object $submission
   *   TPPS Submission.
   *
   * @return mixed
   *   Returns integer index or FALSE in case of errors.
   */
  public function getIndex($submission) {
    if (empty($submission) && $submission->doesNotExist()) {
      return FALSE;
    }
    $this->load();
    $page1_values = $submission->sharedState['saved_values'][TPPS_PAGE_1];
    foreach ($page1_values['organism'] as $index => $item) {
      if (!is_numeric($index) || !is_array($item)) {
        continue;
      }
      if (trim($item['name']) == trim($this->name)) {
        return $index;
      }
    }
  }

  /**
   * Get initial state of the $parsedName property.
   *
   * Allows to reset property.
   *
   * @return array
   *   Returns initial state of the $parsedName property filled with NULL.
   */
  private static function getEmptyParsedName() {
    return [
      'genus' => NULL,
      'species' => NULL,
      'infraspecific_rank' => NULL,
      'infraspecific_name' => NULL,
      'authority_in_parentheses' => NULL,
      'authority' => NULL,
      'year' => NULL,
    ];
  }

}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Useful functions (not methods).

/**
 * FTest for organism class.
 */
function ftest_organism() {
  $organism = new Organism(2559);
  //dpm($organism);
  dpm($organism->name, 'name');
  dpm($organism->id, 'id');
  dpm($organism->genus, 'genus');

  $id = 2544;
  $o = tpps_organism($id);
  dpm($o->id, 'id');
  dpm($o->name, 'name');
  dpm($o->requestNcbiTaxonomyId()->ncbiTaxonomyId, 'taxon id');
}
