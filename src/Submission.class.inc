<?php

/**
 * @file
 * Contains 'Submission' class.
 *
 * @todo Add submission's tag code from include/submission.inc
 * @todo Add Submission Interface and rename.
 */

/**
 * Manages TPPS Submission.
 *
 * Allows to set/get/update/save/load:
 * - Submission State Array ('state');
 * - Submission's Metadata ('info');
 * - Submission's tags (not yet implemented)
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class Submission {
  /**
   * Submission accession.
   *
   * Format: 'TGDRxxxx'.
   *
   * @var string
   */
  protected $accession;

  /**
   * Submission's State Array.
   *
   * @var array
   */
  protected $state;

  /**
   * Submission's Information.
   *
   * Metadate store in table 'tpps_submission'.
   *
   * @var mixed
   */
  protected $info;

  /**
   * Submission Interface.
   *
   * @var array
   */
  protected $pipeline;

  /**
   * Should data be cached in 'tpps' cache bin?
   *
   * @var bool
   */
  protected $useCache = FALSE;

  protected $status;


  /**
   * Class constructor.
   *
   * @param string $accession
   *   Study accession. Format is 'TGDRxxxx'.
   *   If $accession wasn't specified then it left unset and
   *   could be set using $submission->setState($state) which will
   *   populate $this->accession with value form $state array.
   */
  public function __construct(string $accession = NULL) {
    // @TODO Validate $accession.
    if (!empty($accession)) {
      $this->accession = $accession;
    }
    $this->useCache = variable_get('tpps_cache_submissions', FALSE);
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
        throw new InexistentPropertyException("Inexistent property: $name");
      }
    }
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
  public function __get($name) {
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
        throw new InexistentPropertyException("Inexistent property: $name");
      }
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // State

  /**
   * Loads a TPPS Submission Form State Array.
   *
   * @return array
   *   Returns Submission Form State array.
   */
  public function loadState() {
    if (empty($this->accession)) {
      // @todo Throw exception.
      watchdog('tpps', "Submission: Empty accession.", [], WATCHDOG_ERROR);
      return [];
    }
    $result = db_select('tpps_submission', 's')
      ->fields('s', ['submission_state'])
      ->condition('accession', $this->accession)
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();
    if (!$result) {
      // Search for alternative accessions.
      $query = db_select('tpps_submission', 's');
      $query->join('tpps_submission_dbxref', 's_dbx',
        's.tpps_submission_id = s_dbx.tpps_submission_id'
      );
      $query->join('chado.dbxref', 'dbx', 'dbx.dbxref_id = s_dbx.dbxref_id');
      $query->fields('s', ['submission_state']);
      $query->condition('dbx.accession', $this->accession);
      $query->condition('s.status', 'Approved');
      $query->range(0, 1);
      $query = $query->execute();
      // @todo Use fetchField().
      $result = $query->fetchAssoc();
    }
    if ($result) {
      // Return and set property.
      return $this->state = unserialize($result['submission_state']) ?? NULL;
    }
  }

  /**
   * Setter for $state.
   *
   * @param array $state
   *   Submission State Array.
   *
   * @todo Add caching.
   */
  public function setState(array $state) {
    // Accession.
    if (!empty($state['accession'])) {
      $this->accession = $state['accession'];
    }
    $this->state = $state;
  }

  /**
   * Get Submission's state.
   *
   * Caching:
   *   Disabled (or cold cache): 12 msec
   *   Drupal Cache Bin (warm cache): 4 msec.
   *   Static Cache: 0.004 msec.
   *
   * @param bool $reset
   *   Flag is cache must be resetted. Default if FALSE.
   *   RESET_CACHE could be used.
   *
   * @return array
   *   Returns array.
   */
  public function getState(bool $reset = FALSE) {
    if (!empty($this->state)) {
      return $this->state;
    }
    elseif (!empty($this->accession) || $reset) {
      return $this->state = $this->loadState();
    }
    return [];
  }

  /**
   * Saves Submission State to database.
   */
  public function saveState() {
    $this->state['updated'] = time();
    db_update('tpps_submission')
      ->fields(['submission_state' => serialize($this->state)])
      ->condition('accession', $this->accession)
      ->execute();
  }


  /**
   * Updates Submission state in object and in database.
   *
   * Update = Set + Save.
   *
   * @param array $state
   *   Submission State Array.
   *
   * @return array
   *   Returns new (updated) Submission State Array.
   */
  public function updateState(array $state) {
    $this->setState($state);
    $this->saveState();
    return $this->state;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submision Common methods.

  /**
   * Delete submission from database.
   */
  public function delete() {
    db_delete('tpps_submission')
      ->condition('accession', $this->accession)
      ->execute();
    if (empty($this->state['saved_values']['frontpage']['use_old_tgdr'])) {
      $submission_info = tpps_load_submission_info($this->accession);
      $dbxref_id = $submission_info['dbxref_id'];
      db_delete('chado.dbxref')
        ->condition('dbxref_id', $dbxref_id)
        ->execute();
    }
    // If this study was imported we need to remove it from the list.
    // @TODO Check if study was imported.
    module_load_include('inc', 'tpps', 'includes/imported_studies_list');
    tpps_imported_studies_list_remove_item($this->accession);
    $this->accession = $this->state = NULL;
  }

  /**
   * Removes TPPS Submission and it's files.
   *
   * @param bool $force
   *   Boolean indicating that the file should be deleted
   *   if the file not in use by the file_usage table.
   *   Removement of not used files are safe but I'm not sure that old studies
   *   created correct records about usage of files so do NOT delete files
   *   by default.
   *   But for testing and Study Import file removement should be allowed.
   */
  public function purge($force = FALSE) {
    // @todo Minor. Query DB table 'file_usage' to find if there is orphan
    // files related to this non-existing submission.
    if (!empty($this->state)) {
      // Remove usage of files first.
      $fid_list = tpps_submission_file_get_id_list($this->state);
      foreach ($fid_list as $fid) {
        if ($file = tpps_file_load($fid)) {
          // Remove file usage record (not file itself).
          file_usage_delete($file, 'tpps', 'tpps_project', $this->getAccessionNumber(), 1);
          if ($force) {
            // Remove file from disk.
            file_delete($file);
          }
        }
      }
    }
    // Remove submission state from DB.
    db_delete('tpps_submission')
      ->condition('accession', $this->accession)
      ->execute();
    db_delete('chado.dbxref')
      ->condition('accession', $this->accession)
      ->execute();
    // If this study was imported we need to remove it from the list.
    // @todo Check if study was imported to avoid extra moves.
    module_load_include('inc', 'tpps', 'includes/imported_studies_list');
    tpps_imported_studies_list_remove_item($this->accession);
  }

  /**
   * Get only number (without 'TGDR' prefix) of Submission accession.
   *
   * @return int
   *   Return accession number.
   */
  public function getAccessionNumber() {
    return str_replace('TGDR', '', $this->accession);
  }


  /**
   * Create new record in database.
   *
   * @param array $state
   *   Submission State array.
   * @param int $uid
   *   Drupal User Id.
   */
  public function create(array $state, $uid = NULL) {
    global $user;
    $this->setState($state, $force = TRUE);
    $values = [
      'uid' => $uid ?? $user->uid,
      'status' => 'Incomplete',
      'accession' => $this->state['accession'],
      'dbxref_id' => $this->state['dbxref_id'],
      'submission_state' => serialize($this->state),
      'submission_interface' => serialize([]),
    ];
    db_insert('tpps_submission')->fields($values)->execute();
    // Add default tag.
    $tag = $this->isTppsc() ? 'TPPSc' : 'TPPS';
    tpps_submission_add_tag($this->accession, $tag);
  }

  /**
   * Checks if current form is TPPSc form.
   *
   * @return bool
   *   Returns TRUE if form Id is 'tppsc_main' and FALSE otherwise.
   */
  public function isTppsc() {
    return ($this->getFormId() == 'tppsc_main');
  }

  /**
   * Get TPPS Form Id.
   *
   * @return string
   *   Returns 'tpps_main' or 'tppsc_main'.
   */
  public function getFormId() {
    return $this->state['saved_values']['front_page']['form_id']
      ?? $this->state['build_info']['form_id'] ?? 'tpps_main';
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Info.

  /**
   * Loads a TPPS Submission Info from database.
   */
  public function loadInfo() {
    if (empty($this->accession)) {
      return;
    }
    // Columns 'submission_state' and 'submission_interface' will be excluded
    // to reduce size of data and speed-up processing.
    $fields = array_diff(
      array_keys(drupal_get_schema('tpps_submission')['fields']),
      ['submission_interface', 'submission_state']
    );
    $result = db_select('tpps_submission', 's')
      ->fields('s', $fields)
      ->condition('accession', $this->accession)
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();

    if (!$result) {
      // Search for alternative accessions.
      $query = db_select('tpps_submission', 's');
      $query->join('tpps_submission_dbxref', 's_dbx',
        's.tpps_submission_id = s_dbx.tpps_submission_id');
      $query->join('chado.dbxref', 'dbx', 'dbx.dbxref_id = s_dbx.dbxref_id');
      $query->fields('s', $fields);
      $query->condition('dbx.accession', $this->accession);
      $query->condition('s.status', 'Approved');
      $query->range(0, 1);
      $query = $query->execute();
      $result = $query->fetchAssoc();
    }
    if ($result) {
      $this->info = $result;
    }
  }

  /**
   * Loads a TPPS Submission Info.
   *
   * Caching:
   * Disabled (or cold cache): 11 msec
   * Drupal Cache Bin (warm cache): 3 msec.
   * Static Cache: 0.003 msec.
   *
   * @param bool $reset
   *   Flag is cache must be resetted. Default if FALSE.
   *
   * @return mixed
   *   Returns an array with Submission's Info from DB table 'tpps_submission'.
   *   Returns NULL if study wasn't found.
   */
  public function getInfo($reset = FALSE) {
    if (empty($this->accession)) {
      return;
    }
    $key = $this->accession;
    $static_cache = &drupal_static(__METHOD__);
    $cache_submissions = variable_get('tpps_cache_submissions', FALSE);
    if (!$cache_submissions || $reset || !isset($static_cache[$key])) {
      $cid = __METHOD__;
      $cache_bin = TPPS_CACHE_BIN ?? 'cache';
      $cache = cache_get($cid, $cache_bin);
      if (!$cache_submissions || $reset || empty($cache) || !isset($cache->data[$key])) {
        $this->loadInfo();
        $cache->data[$key] = $this->info ?? NULL;
        cache_set($cid, $cache->data, $cache_bin);
      }
      $static_cache[$key] = $cache->data[$key] ?? NULL;
    }
    return $static_cache[$key] ?? NULL;
  }

  /**
   * Updates Submission Information.
   *
   * @param array $updated_fields
   *   Values for existing columns in DB table 'tpps_submission'.
   */
  public function updateInfo(array $updated_fields = []) {
    if (empty($this->accession)) {
      return;
    }
    db_update('tpps_submission')
      ->fields($updated_fields)
      ->condition('accession', $this->accession)
      ->execute();
    // Reset cached data.
    $this->getInfo($reset = TRUE);
  }

  /**
   * Sets State item value.
   *
   * @param array $parents
   *   Path to the element including element name.
   * @param mixed $value
   *   Value to be set.
   */
  public function set(array $parents, $value) {
    // @todo Update cache if it's in use.
    drupal_array_set_nested_value($this->state, ($parents ?? []), $value);
  }

  /**
   * Gets State item value.
   *
   * @param array $parents
   *   Path to the element including element name.
   */
  public function get(array $parents = []) {
    return drupal_array_get_nested_value($this->state, $parents);
  }

  /**
   * Gets Import Metadata.
   *
   * Imported Studies has some extra information about export/import process.
   *
   * @return array
   *   Returns Submission Import Meta with some useful extra items.
   */
  public function getImportMeta() {
    $output = [];
    if (empty($this->state) && empty($this->accession)) {
      watchdog('tpps', "Couldn't get Submission Import Metadata because Form "
        . "State Array and Study Accession are empty.", [], WATCHDOG_ERROR);
      return $output;
    }

    $import_meta = $this->state['saved_values']['tpps_submission_import'];
    if (empty($import_meta)) {
      return $output;
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Sanitize data.
    foreach ($import_meta as $key => $value) {
      if ($safe_key = check_plain(($key ?? '')) != $key) {
        watchdog('tpps', "Submission Import Metadata value had not allowed "
          . "symbols which was sanitized.", [], WATCHDOG_NOTICE);
        unset($import_meta[$key]);
      }
      if ($safe_value = check_plain(($value ?? '')) != $value) {
        watchdog('tpps', "Submission Import Metadata value had not allowed "
          . "symbols which was sanitized.", [], WATCHDOG_NOTICE);
        unset($import_meta[$key]);
        $import_meta[$safe_key] = $safe_value;
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Add some useful items and default values.
    $import_meta['site'] = $import_meta['site'] ?? 'https://treegenesdb.org';
    $import_meta['accession_link'] = ($import_meta['site'])
      ? l($import_meta['original_accession'], $import_meta['site']
        . '/tpps/submission/' . $import_meta['original_accession'] . '/view')
        : $import_meta['original_accession'];
    // Full data: Human readable and timestamp.
    $import_meta['full_date'] = $import_meta['imported_date'];
    if (variable_get('tpps_imported_study_report_show_timestamp', FALSE)) {
      $import_meta['full_date'] .= (empty($import_meta['imported']) ? ''
        : ' [' . $import_meta['imported'] . ']');
    }
    $import_meta['current_version'] = $this->state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY];
    return $import_meta;
  }

}
