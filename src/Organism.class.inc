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
 * To get NCBI Taxonomy Id:
 *   tpps_organism($id)->requestNcbiTaxonomyId()->ncbiTaxonomyId;
 * To load organism from DB:
 *   tpps_organism($id);
 *   tpps_organism($name);
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
   * Name of the table where organism data is stored.
   */
  const TABLE_ORGANISM = 'chado.organism';
  /**
   * Name of the table which stores NCBI Taxonomy Ids per organism.
   */
  const TABLE_NCBI_TAXONOMY = 'tpps_ncbi_taxonomy';

  /**
   * NCBI Taxonomy Id.
   *
   * Stored in tpps_ncbi_taxonomy_id table.
   * See tpps_ncbi_get_taxon_id().
   *
   * @var int
   */
  protected $ncbiTaxonomyId;

  /**
   * Organism Name.
   *
   * @var string
   */
  protected $name;

  /**
   * Organism Id.
   *
   * @var int
   */
  protected $id;

  /**
   * Organism Genus. Required 1st part of the full name.
   *
   * @var string
   */
  protected $genus;

  /**
   * Organism Species. Required 2nd part of the full name.
   *
   * @var string
   */
  protected $species;

  /**
   * Organism infraspecific name. Optional 3rd part of the full name.
   *
   * @var string
   */
  protected $infraspecificName;

  /**
   * Class constructor.
   *
   * @param mixed $organism_key
   *   Organism Name or Id.
   */
  public function __construct($organism_key = NULL) {
    if (!empty($organism_key)) {
      if (is_numeric($organism_key)) {
        $this->id = $organism_key;
      }
      else {
        // Key is a full organism name.
        // @todo Minor. Validate organism name.
        $this->name = $organism_key;
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
   */
  public function __set($name, $value) {
    // Allow to overwrite magic with custom setter.
    $func = 'set' . ucfirst($name);
    if (method_exists($this, $func)) {
      $this->$func($value);
    }
    else {
      if (property_exists($this, $name)) {
        $this->$name = $value;
      }
      else {
        throw new InexistentPropertyException(
          t('Inexistent property: @name', ['@name' => $name])
        );
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
   */
  public function &__get($name) {
    // Allow to overwrite magic with custom getter.
    $func = 'get' . ucfirst($name);
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
    if (!empty($this->id) && !empty($this->name)) {
      return;
    }
    $query = db_select('chado.organism', 'co')
      ->fields('co')
      ->range(0, 1);
    // Loadby name.
    if (!empty($this->id)) {
      // Load by id.
      $query->condition('organism_id', $this->id);
    }
    else {
      // Load by name.
      if (!empty($this->name)) {
        $this->parseName();
        if (empty($this->genus) || empty($this->species)) {
          watchodg('tpps', 'Empty organism name.');
          return;
        }
        $query->condition('genus', '%' . db_like($this->genus) . '%', 'LIKE');
        $query->condition('species', '%' . db_like($this->species) . '%', 'LIKE');
        if (empty($this->infraspecificName)) {
          $query->condition(
            'infraspecificName',
            '%' . db_like($this->infraspecificName) . '%',
            'LIKE'
          );
        }
      }
      else {
        $message = t("Can't load organism because there are no organism id and name.");
        watchdog('tpps', $message, [], WATCHDOG_ERROR);
        return;
      }
    }
    if (!empty($query)) {
      $organism = $query->execute()->fetchAssoc();
      // Set properties.
      $this->genus = $organism['genus'] ?? NULL;
      $this->species = $organism['species'] ?? NULL;
      $this->infraspecificName = $organism['infraspecificName'] ?? NULL;
      // There is also columns:
      // abbreviation,
      // common_name,
      // type_id,
      // comment.
      $this->generateName();
    }
    return $this;
  }

  /**
   * Builds full organism name using name parts from DB table.
   */
  public function generateName() {
    $name_parts = [$this->genus, $this->species, $this->infraspecificName];
    $name = implode(' ', $name_parts);
    // Check if $this->name and new name differs.
    if (!empty($this->name) && $this->name != $name) {
      $message = t('Existing organism name and generated name differs');
      watchdog('tpps', $message, [], WATCHDOG_WARNING);
    }
    $this->name = $name;
  }

  /**
   * Extracts genus, species and extra parts from the organism name.
   */
  public function parseName() {
    if (empty($this->name)) {
      return;
    }
    $name = tpps_parse_organism_name($this->name);
    $this->genus = $name['genus'] ?? NULL;
    $this->species = $name['species'] ?? NULL;
    $this->infraspecificName = $name['extra'] ?? NULL;
  }

  /**
   * Get NCBI Taxonomy Id from the remote server.
   */
  public function requestNcbiTaxonomyId() {
    $this->load();
    if (empty($this->name)) {
      $message = t("Can't get NCBI Taxonomy Id because there are no organism name.");
      watchdog('tpps', $message, [], WATCHDOG_ERROR);
      dpm($message);
      return;
    }
    $xml = tpps_ncbi_get_taxon_id($this->name, FALSE);
    $response = json_decode(json_encode($xml), TRUE);
    $this->ncbiTaxonomyId = check_plain($response[0]) ?? FALSE;

// @TODO Save to DB table 'tpps_ncbi_taxonomy'.
    $fields = [
      'organism_id' => $this->id ?? NULL,
      'organism_name' => $this->name ?? NULL,
      'taxonomy_id' => $this->ncbiTaxonomyId ?? NULL,
    ];
    db_insert(self::TABLE_NCBI_TAXONOMY)->fields($fields)->execute();

    return $this;
  }

  /**
   * Saves organism to db.
   */
  function save() {
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
   * db_select() is way slower than db_query().
   * https://stackoverflow.com/a/21481960/1041470
   *
   * @return bool
   *   Returns TRUE if record was found in DB table and FALSE otherwise.
   *
   * @TODO Add to Submission class the same code.
   */
  public function isRecordExists($organism_id = NULL) {
    $organism_id = $organism_id ?? $this->id ?? NULL;
    if (empty($organism_id)) {
      return FALSE;
    }
    $sql = 'select organism_id from {' . self::TABLE_ORGANISM
      . '} where organism_id = :id limit 1';
    return (bool) (db_query($sql, [':id' => $organism_id])->fetchField());
  }

  /**
   * Loads NCBI Taxonomy Id from DB.
   */
  public function &getNcbiTaxonomyId() {
    if (empty($this->ncbiTaxonomyId)) {
      // Get from DB.
      // @TODO Use constant to store table names.
      $this->load();
      if (empty($this->id)) {
        $message = t('No organism Id');
        watchdog('tpps', $message, [], WATCHDOG_ERROR);
        return;
      }
      $this->ncbiTaxonomyId = db_select(self::TABLE_NCBI_TAXONOMY, 't')
        ->fields('t')
        ->condition('organism_id', $this->id)
        ->range(0, 1)
        ->execute()
        ->fetchField();
      if (empty($this->ncbiTaxonomyId)) {
        $this->requestNcbiTaxonomyId();
      }
    }
    return $this->ncbiTaxonomyId;
  }

  /**
   * Checks if organism exists in database.
   *
   * @return bool
   *   Returns TRUE if record in database exists and FALSE otherwise.
   */
  public function doesExist() {
    $this->load();
    return (bool) (
      !empty($this->name)
      && !empty($this->id)
    );
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

}

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
