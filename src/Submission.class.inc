<?php

/**
 * @file
 * Contains 'Submission' class.
 *
 * @todo Add submission's tag code from include/submission.inc
 *
 * Questions:
 * 1. Should class create watchdog messages? Probably no. Create exceptions.
 * 2. Should methods return data or just update class's properties?
 *    Returning is better because require less code to work.
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
  protected $state = [];

  /**
   * Submission State for pipeline processing and other pages.
   *
   * This state is a 1st version of Submission Form State but without some
   * useless (but huge) for processing items/
   *
   * @var array
   */
  protected $sharedState = [];

  /**
   * Submission's Information.
   *
   * Metadate store in table 'tpps_submission'.
   *
   * @var mixed
   */
  protected $info = [];

  /**
   * Should data be cached in 'tpps' cache bin?
   *
   * @var bool
   */
  protected $useCache = FALSE;

  /**
   * Submission status.
   *
   * Note: There are 2 places which store status:
   *   1. public.tpps_submision.status
   *   2. public.tpps_submision.submission_state subitem 'status'.
   *
   * @var string
   */
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
      $this->load(RESET_CACHE);
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
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // State.
  //
  // Properties:
  // - $state array - Submission's State.
  //
  // Method:
  // - saveState() - Sync memory to Db.
  //
  // @TODO Get rid of this methods.
  // - getState() - From memory. Loads from DB if missing.
  //   @TODO Replace with $submission->state.
  // - setState($state) - In memory.
  //   @TODO Replace with $submission->state.
  // - updateState($state) = setState() + saveState().
  //   @TODO Replace with $submission->state =...  and saveState().
  //
  // To set/get/update an item:
  // $submission = new Submission($accession);
  // $submission-state[$item_name] = $item_value;

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
  public function &getState(bool $reset = FALSE) {
    if ($reset || (empty($this->state) && !empty($this->accession))) {
      $this->load();
    }
    return $this->state;
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
  // create() - object and DB
  // save() - Sync object to DB
  // load() - Sync DB to object.
  // delete() - remove from DB
  // purge() - remove study and files in DB.

  /**
   * Saves Submission.
   */
  public function save() {


    // @TODO Improve to store only non-empty data.
    // Store also status.
    $this->state['updated'] = time();
    $this->sharedState['updated'] = time();
    // @TODO Set status (if any).
    $this->saveState();
    $this->saveSharedState();

    return;
    // Shared State.
    if (empty($interface) || empty($interface['accession'])) {
      return NULL;
    }

    $accession = $interface['accession'];
    // Note: REQUEST_TIME couldn't be used because called from CLI script and
    // processing could take long time from script call.
    $interface['updated'] = time();

    // Prepare data for query.
    $fields = [];
    if ($status) {
      $fields['status'] = $interface['status'] = $status;
    }
    $fields['submission_interface'] = serialize($interface);

    db_update('tpps_submission')
      ->fields($fields)
      ->condition('accession', $accession)
      ->execute();
  }

  /**
   * Delete submission from database.
   */
  public function delete() {
    if (empty($this->state['saved_values']['frontpage']['use_old_tgdr'])) {
      $dbxref_id = $this->info['dbxref_id'] ?? NULL;
      if ($dbxref_id) {
        db_delete('chado.dbxref')
          ->condition('dbxref_id', $dbxref_id)
          ->execute();
      }
    }
    db_delete('tpps_submission')
      ->condition('accession', $this->accession)
      ->execute();
    // If this study was imported we need to remove it from the list.
    // @TODO Check if study was imported.
    module_load_include('inc', 'tpps', 'includes/imported_studies_list');
    tpps_imported_studies_list_remove_item($this->accession);
    $this->accession = $this->state = $this->sharedState = NULL;
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
    $this->accession = $this->state = $this->sharedState = NULL;
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
   * Loads Submission's data.
   *
   * Note: all the submission data will be loaded but with single SQL-request
   * instead of 3 requests if data loaded separately.
   *
   * To avoid reload the same study use global $submission object.
   *
   * @return bool
   *   Returns TRUE in case of success load (even if there is no data).
   *   Returns FALSE if Submission's load failed.
   */
  public function load() {
    if (empty($this->accession)) {
      return FALSE;
    }

    $result = db_select('tpps_submission', 's')
      ->fields('s')
      ->condition('accession', $this->accession)
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();

    if (!$result) {
      // Search for alternative accessions.
      $query = db_select('tpps_submission', 's');
      $query->join(
        'tpps_submission_dbxref',
        's_dbx',
        's.tpps_submission_id = s_dbx.tpps_submission_id');
      $query->join(
        'chado.dbxref',
        'dbx',
        'dbx.dbxref_id = s_dbx.dbxref_id'
      );
      $result = $query->fields('s')
        ->condition('dbx.accession', $this->accession)
        ->range(0, 1)
        ->execute()
        ->fetchAssoc();
    }
    if ($result) {
      $this->info = $result;
      unset($this->info['submission_state']);
      unset($this->info['submission_interface']);
      $this->status = $this->info['status'] ?? NULL;
      $this->state = unserialize($result['submission_state'] ?? '') ?? [];
      $this->sharedState = unserialize($result['submission_interface'] ?? '') ?? [];
      if (empty($this->sharedState)) {
        $this->generateSharedState();
        $this->saveSharedState();
      }
      return TRUE;
    }
    return FALSE;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Helpers

  /**
   * Get only number (without 'TGDR' prefix) of Submission accession.
   *
   * @return int
   *   Return accession number.
   */
  public function &getAccessionNumber() {
    return str_replace('TGDR', '', $this->accession);
  }

  /**
   * Checks if form type is 'tppsc'.
   *
   * @return bool
   *   Returns TRUE if form Id is 'tppsc_main' and FALSE otherwise.
   */
  public function isTppsc() {
    return ($this->getFormId() == 'tppsc_main');
  }

  /**
   * Checks if form type is 'tpps' (but not 'tppsc').
   *
   * @return bool
   *   Returns TRUE if form Id is 'tpps_main' and FALSE otherwise.
   */
  public function isTpps() {
    return !$this->isTppsc();
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
   * Checks if submission exists in database.
   *
   * Only study accession is required to find record in database.
   *
   * @return bool
   *   Returns TRUE if record in database exists and FALSE otherwise.
   */
  public function doesExist() {
    $this->load();
    return (bool) (
      !empty($this->accession) && !empty($this->info) && !empty($this->state)
    );
  }

  /**
   * Check if submission doesn't exist.
   *
   * @return bool
   *   Returns TRUE if record in database exists and FALSE otherwise.
   */
  public function doesNotExist() {
    return !($this->doesExist());
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
   *
   *   @TODO Remove.
   */
  public function &getInfo($reset = FALSE) {
    if (empty($this->accession)) {
      return NULL;
    }
    $key = $this->accession;
    $static_cache = &drupal_static(__METHOD__);
    $cache_submissions = variable_get('tpps_cache_submissions', FALSE);
    if (!$cache_submissions || $reset || !isset($static_cache[$key])) {
      $cid = __METHOD__;
      $cache_bin = TPPS_CACHE_BIN ?? 'cache';
      $cache = cache_get($cid, $cache_bin);
      if (!$cache_submissions || $reset || empty($cache) || !isset($cache->data[$key])) {
        $this->load();
        $cache->data[$key] = $this->info ?? NULL;
        cache_set($cid, $cache->data, $cache_bin);
      }
      $static_cache[$key] = $cache->data[$key] ?? NULL;
    }
    return $this->info;
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
    $this->getState();
    if ($this->doesNotExist()) {
      return $output;
    }

    // @TODO Create property $importMeta?
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

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission's Shared State (3S, former Submission Interface).

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
  public function &getSharedState(bool $reset = FALSE) {
    if ($reset || (empty($this->sharedState) && !empty($this->accession))) {
      $this->load();
    }
    return $this->sharedState;
  }

  /**
   * Removes Submission's Shared State.
   */
  public function removeSharedState() {
    db_update('tpps_submission')
      ->fields(['submission_interface' => 'a:0:{}'])
      ->condition('accession', $this->accession)
      ->execute();
  }

  /**
   * Generates Submission Shared State.
   *
   * WARNING:
   * This method do not update record in database.
   * Use saveSharedState() to store in DB.
   *
   * @param array $state
   *   Drupal Form State array.
   *
   * @return mixed
   *   Returns array with datastructure prepared for TPPS pipeline
   *   (submit_all.php script).
   *   Returns empty array if $form_state is empty too.
   */
  public function generateSharedState(array $state = []) {
    $this->state = (!empty($state) ? $state : $this->getState(RESET_CACHE));

    // @TODO Avoid this hard linking.
    module_load_include('inc', 'tpps', 'includes/submission_form_version');
    $this->sharedState = $this->state;

// @TODO Important: Rename variable.

    $this->sharedState['is_submission_interface'] = TRUE;

    // We need to apply changes from 1st to $current version to have
    // incremental changes.
    tpps_submission_form_version_downgrade_to_latest($this->state, $this->sharedState);
    $this->purifySharedState();

    return $this->sharedState;
  }

  /**
   * Store Submission Shared State to database.
   *
   * To update submission's status:
   *   tpps_update_submission_info($accession, $data);
   * To update submission's Form State:
   *   tpps_update_submission_state($form_state);
   *
   * To remove SharedState:
   *   $this->removeSharedState();
   * WARNING: do not use 'phppgadmin' because it can't edit serialized data.
   *
   * @param string $status
   *   Optional. Study processing status.
   *   This value will be added to the Submission Interface Array and to
   *   public.tpps_submission.status column.
   */
  public function saveSharedState($status = NULL) {
    if (empty($this->sharedState) || empty($this->accession)) {
      return;
    }
    $this->sharedState['updated'] = time();
    $fields = [];
    if ($status) {
      $fields['status'] = $this->sharedState['status'] = $status;
    }
    $fields['submission_interface'] = serialize($this->sharedState);

    db_update('tpps_submission')
      ->fields($fields)
      ->condition('accession', $this->accession)
      ->execute();
  }

  /**
   * Reduce size of the Submission Interface array.
   *
   * 3S items which doesn't used by pipeline processing script will be removed.
   */
  public function purifySharedState() {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Remove useless elemLnts to reduce size.
    $change_list = [
      // We unset not used items instead of copy required items from
      // $form_state because if new elements will be found they will be
      // left for sure. It's safer.
      'unset' => [
        // List of path to the items in $interface at the top (1st) level.
        'build_info',
        'temporary',
        'programmed',
        'programmed_bypass_access_check',
        'cache',
        'rebuild',
        'rebuild_info',
        'redirect',
        'submitted',
        'executed',
        'method',
        'groups',
        'buttons',
        'input',
        'values',
        'complete form',
        'process_input',
        'invalid_token',
        'triggering_element',
        'clicked_button',
        ['saved_values', 'form_build_id'],
        'tpps_submission_export_files',
        // @TODO Submission Form Version is useless for pipeline processing
        // but could be useful for debugging.
        //['saved_values', TPPS_SUBMISSION_FORM_VERSION_KEY],

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // List of the fields which must be left.
        // Required by submit_all() and tpps_table_display() and sub-functions.
        // 'saved_values',
        // 'dbxref_id',
        // 'created',
        // 'accession',
        // 'status',
        // 'stats',
        // 'ids',
        // 'tpps_type',
        // 'file_info',
        // 'updated',
        // 'approved',
        // 'completed',
        // 'loaded',
        // 'submitting_uid',
        // 'job_id',
        // 'revised_files',
        // 'admin_comments',
        // 'alternative_accessions',
        // 'data',
        // 'tree_info'.
        //
        //  Probably could be unset:
        // 'stage',
      ],
    ];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Dynamic (repeated) changes.
    //
    // Remove Drupal's internal fields used for form building.
    // Note: Those fields are stored in column tpps_submission.submission_state.
    $page_list = [
      'frontpage', TPPS_PAGE_1, TPPS_PAGE_2, TPPS_PAGE_3, TPPS_PAGE_4, 'summarypage',
    ];
    $drupal_internal_fields = [
      // Note: Page 1 has not 'Back' button.
      'Back',
      'Save',
      'Next',
      'form_build_id',
      'form_token',
      'form_id',
      'op',
      '#cleared',
      // Only for Page 4.
      'submit',
      'diagnostics-curation',
    ];
    foreach ($page_list as $page) {
      foreach ($drupal_internal_fields as $field) {
        $change_list['unset'][] = ['saved_values', $page, $field];
      }
    }

    module_load_include('inc', 'tpps', 'includes/array');
    tpps_array_apply_changes($change_list, $this->sharedState);
  }

  public function isSharedState($state) {

  }

}
