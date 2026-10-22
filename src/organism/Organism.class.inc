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
   */
  const TABLE = 'chado.organism';

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
   * Organism name abbreviation.
   *
   * See method generateAbbreviation().
   * Stored in 'chado.organism.abbreviation'.
   *
   * @var string
   */
  protected $abbreviation;

  /**
   * Class constructor.
   *
   * @param mixed $organism_key
   *   Organism Name or Organism Id.
   *
   * @throws Exception
   *   If $organism_key contains name (not id) which can't be parsed.
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
        $this->generateAbbreviation();
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
  public function __get($name) {
    // Allow to overwrite magic with custom getter.
    if (property_exists($this, $name)) {
      return $this->$name;
    }
    else {
      throw new Exception("Inexistent property: $name");
    }
  }

  /**
   * Magic isset.
   *
   * @param string $name
   *   Class property name.
   *
   * @return bool
   *   TRUE if the magic property exists (and is not null).
   */
  public function __isset($name) {
    return property_exists($this, $name) && isset($this->$name);
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Custom setter for $name property.
   *
   * To fill $id property: call findId() method.
   *
   * @param string $name
   *   Organism name.
   *
   * @return object
   *   Returns $this to allow method's chaining.
   */
  public function setName($name) {
    if (!empty($name)) {
      $this->name = trim(preg_replace('/\s+/', ' ', $name));
      $this->parsedName = Organism::parseName($this->name);
      $this->genus = $this->parsedName['genus'] ?? NULL;
      $this->species = $this->parsedName['species'] ?? NULL;
      $this->infraspecificRank = $this->parsedName['infraspecific_rank'] ?? NULL;
      $this->infraspecificName = $this->parsedName['infraspecific_name'] ?? NULL;
    }
    return $this;
  }

  /**
   * Dumps instance (object) properties.
   *
   * @return array
   *   Returns associative array where keys are object property name and
   *   value is property's value.
   */
  public function getDump() {
    $caller = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 2)[1];
    if (empty($caller['class'])) {
      $caller_name = $caller['function'];
    }
    else {
      $caller_name = $caller['class'] . $caller['type'] . $caller['function'];
    }
    return array_merge(
      ['CALLER' => $caller_name],
      get_object_vars($this)
    );
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
    if (empty($this->id) && empty($this->genus) && empty($this->species)) {
      $message = t("Can't load organism because there is no organism id and name.");
      watchdog('tpps', $message, [], WATCHDOG_ERROR);
      return;
    }

    $query = db_select('chado.organism', 'co')
      ->fields('co')
      ->range(0, 1);
    // Add conditions.
    if (!empty($this->id)) {
      // Id.
      $query->condition('organism_id', $this->id);
    }
    else {
      // Name.
      $query->condition('genus', $this->genus)
        ->condition('species', $this->species);
      $this->buildSelectQuery($query);
    }
    $organism = $query->execute()->fetchAssoc();

    // Set properties.
    $this->id = $this->id ?? $organism['organism_id'] ?? NULL;
    $this->genus = $organism['genus'] ?? NULL;
    $this->species = $organism['species'] ?? NULL;
    $this->infraspecificName = $organism['infraspecific_name'] ?? NULL;
    $this->abbreviation = $organism['abbreviation'];

    // Get infraspecific rank from the abbreviation (if any).
    $abbreviation_rank = NULL;
    if (!empty($this->abbreviation)) {
      $pattern = '/
        \s(                                         # Required leading space.
          (?:' . InfraspecificRank::getRegex() . ') # Infraspecific Rank.
        )\s                                         # Required trailing space.
        /x';
      preg_match($pattern, $this->abbreviation, $matches);
      $abbreviation_rank = $matches[1] ?? NULL;

      if (!empty($abbreviation_rank)) {
        $abbr_rank_id = InfraspecificRank::getCvtermId($abbreviation_rank);
        if (
          !empty($orgnaism['type_id'])
          && $orgnaism['type_id'] != $abbr_rank_id
        ) {
          // @todo compare with type_id.
          watchdog('tpps',
            'Infraspecific rank in the "chado.orgnaism.abbreviation" and '
            . 'in "chado.organism.type_id" differs for organism: #@id (@name)',
            [
              '@id' => $this->id,
              '@name' => $this->name,
            ],
            WATCHDOG_WARNING
          );
        }
      }
    }

    // Infraspecific Rank.
    if (
      !empty($this->infraspecificName)
      && !empty($organism['type_id'])
    ) {
      // Remove infraspecific rank from infraspecific name
      // if 'type_id' is not empty.
      $this->infraspecificRank = InfraspecificRank::getName($organism['type_id']);
      $pattern = '/'
        . InfraspecificRank::getRegex() . ' # Infraspecific Rank.
        \s                                  # Required trailing space.
        /x';
      $this->infraspecificName = trim(
        preg_replace($pattern, '', $this->infraspecificName)
      );
    }

    $this->generateAbbreviation();
    // @todo Minor. There is also columns:
    // common_name,
    // comment.
    $this->generateName();

    // Check if abbreviation uses the same rank.
    // @todo they could use different forms and it's better to compare ids.
    if (!empty($abbreviation_rank)
       && $this->infraspecificRank != $abbreviation_rank
    ) {
      watchdog('tpps',
        'Infraspecific rank after name generation (@generated) and rank '
        . 'from "abbreviation" field (@abbr_rank) are different for '
        . 'organism: #@id (@name)',
        [
          '@id' => $this->id,
          '@name' => $this->name,
          '@generated' => $this->infraspecificRank,
          '@abbr_rank' => $abbreviation_rank,
        ],
        WATCHDOG_WARNING
      );
    }
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
   * Generates organism name abbreviation.
   *
   * @return object
   *   Returns $this to allow method's chaining.
   */
  public function generateAbbreviation() {
    if (
      empty($this->abbreviation)
      && !empty($this->species)
      && !empty($this->genus)
    ) {
      $this->abbreviation = trim(implode(' ', [
        substr($this->genus, 0, 1) . ".",
        $this->species,
        $this->infraspecificRank,
        $this->infraspecificName,
      ]));
    }
    return $this;
  }

  /**
   * Builds full organism name using name parts stored in DB table.
   *
   * Result will be stored in $name property.
   */
  public function generateName() {
    if (!empty($this->genus) && !empty($this->species)) {
      // @todo Minor. Use $this->parsedName if not empty.
      $name_parts = [
        $this->genus,
        $this->species,
      ];
      if (!empty($this->infraspecificName)) {
        if (!empty($this->infraspecificRank)) {
          $name_parts[] = $this->infraspecificRank;
        }
        $name_parts[] = $this->infraspecificName;
      }
      $name_parts = array_filter($name_parts);
      $name = trim(implode(' ', $name_parts));

      // Check if $this->name and new name differs.
      if (empty($this->id)) {
        $this->name = $name;
      }
      else {
        if (empty($this->name)) {
          $this->name = $name;
        }
        else {
          // $id and $name are not empty.
          if ($this->name != $name) {
            // If $name is different it should be another organism with
            // different $id.
            watchdog('tpps', 'Existing organism name and generated name differs:'
              . '<br />Existing name: "@existing_name" (@existing_name_len).'
              . '<br />Generated name: "@existing_name" (@new_name_len).',
              [
                '@existing_name' => json_encode($this->name),
                '@new_name' => json_encode($name),
                '@existing_name_len' => strlen($this->name),
                '@new_name_len' => strlen($name),
              ],
              WATCHDOG_ERROR
            );
          }
        }
      }
    }
    return $this;
  }

  /**
   * Extracts genus, species and extra parts from the organism name.
   *
   * Note: this method do not validate parsed data.
   *
   * @param string $name
   *   Organism name.
   *
   * @return array
   *   Returns an accosiative array with result of the parsing.
   *
   * @throws Exception
   *   When organism name is empty.
   */
  public static function parseName($name) {

    // Allow to parse crossed ('Larix x marschlinsii') as:
    // 'genus': Larix,
    // 'species': 'x marschlinsii'.
    $allow_crosses = TRUE;

    $result = Organism::getEmptyParsedName();
    $name = trim($name);
    // Check if name is valid.

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // First genus letter is '?' or in lowercase.
    if (
      preg_match('/^([a-z\?])/', $name, $matches)
      // Reject 'x ' as first token. E.g., 'x Something'.
      || preg_match('/^x\s+/', $name, $matches)
    ) {
      return $result;
    }
    // Crosses which has ' x ' substring.
    if (!$allow_crosses && preg_match('/(\s+x\s+)/', $name, $matches)) {
      return $result;
    }

    // Handle cases where there's no species (just genus).
    if (preg_match('/^([A-Z][a-z]+)$/', $name, $matches)) {
      $result['genus'] = $matches[1];
      return $result;
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Handle "Genus sp." / "Genus sp" / "Genus spp." / "Genus spp"
    // (species unknown/unspecified).
    if (preg_match('/^([A-Z][a-z]+)\s+((?:sp|spp)\.?)$/u', $name, $matches)) {
      $result['genus'] = $matches[1];
      $result['species'] = $matches[2];
      return $result;
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // 'Genus species' or
    // 'Genus species 2nd-word'.
    $pattern = '/^
      ([A-Z][a-z]+)                  # 1. Genus
      \s+
      ([\×a-z-]+(?:\s*[\×a-z-.]+)?)  # 2. Species (1 or 2 words, incl. ×)
      $/xu';
    if (preg_match($pattern, $name, $matches)) {
      $result['genus'] = $matches[1] ?? NULL;
      $result['species'] = $matches[2] ?? NULL;
      return $result;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Allow optional dot after rank
    $pattern = '/
      \s(                                         # Required leading space.
        (?:' . InfraspecificRank::getRegex() . ') # Infraspecific Rank.
      )\s                                         # Required trailing space.
      /x';
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
   *
   * If record exists it will be updated.
   * If organism_id doesn't exist new record will be created.
   *
   * @throws Exception
   *   If species or genus is empty.
   */
  public function save() {

//dpm($this->getDump());

    if ($this->isRecordExists()) {
      return;
    }
    $watchdog_message = "Object's properties: <pre>!object_dump</pre>
      Stack:<pre>!stack</pre>";
    $stack = debug_backtrace(
      !DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS,
      10
    );
    $params = [
      '!stack' => print_r($stack, 1),
      '!object_dump' => var_export(get_object_vars($this), 1),
    ];
    if (empty($this->genus)) {
      watchdog('tpps', $watchdog_message, $params);
      $message = t(
        "Organism '@name' with empty genus can't be saved.",
        ['@name' => $this->name]
      );
      throw new Exception($message);
    }
    if (empty($this->species)) {
      watchdog('tpps', $watchdog_message, $params);
      $message = t(
        "Organism '@name' with empty species can't be saved.",
        ['@name' => $this->name]
      );
      throw new Exception($message);
    }
    // Get cvterm id for infraspecific rank.
    if (!empty($this->infraspecificRank)) {
      $rank_cvterm_id = InfraspecificRank::getCvtermId($this->infraspecificRank);
    }
    $this->generateAbbreviation();
    $fields = [
      'abbreviation' => $this->abbreviation,
      'genus' => $this->genus,
      'species' => $this->species,
      'common_name' => NULL,
      'infraspecific_name' => $this->infraspecificName,
      'type_id' => $rank_cvterm_id ?? NULL,
      'comment' => NULL,
    ];
    $this->id = db_insert(self::TABLE)->fields($fields)->execute();
  }

  /**
   * Checks if organism exists in DB table 'chado.organism'.
   *
   * WARNING: method load() won't called and organism won't be loaded.
   *
   * @return bool
   *   Returns TRUE if record was found in DB table and FALSE otherwise.
   */
  public function isRecordExists() {
    if (empty($this->id)) {
      $this->findId();
    }
    else {
      // Function db_select() is slower than db_query().
      // https://stackoverflow.com/a/21481960/1041470.
      $sql = 'SELECT organism_id
        FROM {' . static::TABLE . '}
        WHERE organism_id = :id
        LIMIT 1';
      $this->id = db_query($sql, [':id' => $this->id])->fetchField();
    }
    return (bool) $this->id;
  }

  /**
   * Checks if organism exists in DB table 'chado.organism'.
   *
   * Note: Makes db-query even if $id property is not empty.
   * Organism Id from DB will overwrite existing value of the $id property.
   *
   * @return object
   *   Returns $this to allow method chaining.
   *
   * @todo Minor. Add to Submission class the same code.
   */
  public function findId() {
    if (empty($this->genus) || empty($this->species)) {
      // No required genus+species. Organism couldn't be found.
      $this->id = NULL;
    }
    else {
      // Check by name.
      $query = db_select(static::TABLE, 'co')
        ->fields('co', ['organism_id'])
        ->condition('genus', $this->genus)
        ->condition('species', $this->species)
        ->range(0, 1);
      $this->buildSelectQuery($query);
      $organism_id = $query->execute()->fetchField();

      // Check if property $id equals to the found organism id.
      if (
        !empty($organism_id)
        && !empty($this->id)
        && $organism_id != $this->id
      ) {
        $message = t("Organism Id #@db_id found in database is different from "
          . "the value of the 'id' property #@property_id. Organism name: @name.");
        $params = [
          'db_id' => $organism_id,
          'property_id' => $this->id,
          '@name' => $this->name,
        ];
        watchdog('tpps', $message, [], WATCHDOG_WARNING);
      }
      $this->id = $organism_id;
    }
    return $this;
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

  /**
   * Adds infraspecific rank/name to db select-query.
   *
   * @param object $query
   *   DB select object. See db_select().
   */
  public function buildSelectQuery(&$query) {
    // @todo Major. Some names has only genus + authority.
    if (empty($this->infraspecificName)) {
      $query->isNull('infraspecific_name');
    }
    else {
      if (empty($this->infraspecificRank)) {
        $query->condition('infraspecific_name', $this->infraspecificName);
      }
      else {
        $query->condition(
          db_or()
            ->condition('infraspecific_name', $this->infraspecificName)
            ->condition('infraspecific_name', implode(' ', array_filter([
              $this->infraspecificRank,
              $this->infraspecificName,
            ])))
        );
        $rank_cvterm_id = InfraspecificRank::getCvtermId($this->infraspecificRank);
        // InfraspecificRank could be not part of the $infraspecificName but
        // stored in 'type_id' field and cvterm_id must be used.
        $query->condition(
          db_or()
            ->condition('type_id', $rank_cvterm_id)
            ->isNull('type_id')
        );
      }
    }
  }

  /**
   * Checks if organism name is invalid.
   *
   * @return bool
   *   Return TRUE if organism name is NOT valid and FALSE otherwise.
   */
  public function isNotValidName() {
    return !$this->isValidName();
  }

  /**
   * Checks if organism name valid.
   *
   * @return bool
   *   Return TRUE if organism name is valid and FALSE otherwise.
   *
   * @todo Validate also infraspecific rank/name.
   */
  public function isValidName() {
    if (empty($this->genus) || empty($this->species)) {
      return FALSE;
    }
    return TRUE;
  }

  /**
   * Gets short organism name.
   *
   * Short name contains only genus and species. No authority and year data.
   *
   * @return string|false
   *   Returns organism name.
   *   Returns fALSE if organism name is not valid: genus or species is empty.
   */
  public function getShortName() {
    if ($this->isValidName()) {
      return $this->genus . ' ' . strtolower($this->species);
    }
    else {
      return FALSE;
    }
  }

}
