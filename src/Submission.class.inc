<?php

/**
 * @file
 * Contains 'Submission' class.
 *
 * @todo
 * 1. Add submission's tag code from include/submission.inc
 * @TODO Minor. Review function tpps_form_state_info() which could be used
 * to reduce size of the sharedState.
 *
 * Concepts:
 * 1. Do not use watchdog()/tpps_message()/dpm()/drupal_set_message(). Use exceptions.
 * 2. Update internal properties and don't return data from methods.
 * 3. Setting of the sub-element of property won't call getter-method.
 *
 * To set/get sub-elements: $submission->set() and ->get().
 *
 *
 * ----------------------------------------------------------------------------
 * Properties of th object Submission.
 *
 * Each column in DB table 'tpps_submission' represented with object's property
 * but names was shortened and camelCase used to meet PHP's Coding Standards.
 *
 * Mapping:
 * +----------------+--------------------+
 * | Object         | DB                 |
 * +----------------+--------------------+
 * | id             | tpps_submission_id |
 * | uid            | uid                |
 * | status         | status             |
 * | accession      | accession          |
 * | dbxrefId       | dbxref_id          |
 * | state          | submission_state   |
 * | shared_state   | shared_state       |
 * +----------------+--------------------+
 *
 * Methods of the object Submission.
 *
 * ----------------------------------------------------------------------------
 * Submision Common methods.
 *
 * create() - fills object's properties and creates record in DB. Status 'Incomplete'.
 * save() - Sync object to DB.
 * load() - Sync DB to object.
 * delete() - remove from DB (old)
 * purge() - remove study and files in DB (newer version).
 *
 * ----------------------------------------------------------------------------
 * Submission's Status.
 *
 * To get: $submission->status;
 * To set: $submission->status = 'Approved';
 * Note: status is also stored in $state, $sharedState and those properties
 * will also be updated and no need to update them manually.
 *
 * ----------------------------------------------------------------------------
 * State's sub-elements methods.
 *
 * set() - sets value of sub-element of $state.
 * get() - gets value of sub-element of $state.
 * unset() - Unset's sub-element in $state. To get this change in $sharedState
 *           just regenerate it.
 *
 *
 * ----------------------------------------------------------------------------
 * Usage Example.
 *
 * $a = 'TGDR906';
 * $list = Submission::getStatusList();
 * $status =array_rand(array_flip($list), 1);
 * dpm($status);
 * $s = tpps_submission($a)
 * ->dump('state', ['status'])
 * ->setStatus($status)->save()
 * ->dump('state', ['status']);
 *
 * $s2 = new Submission($a);
 * $s2->load();
 * dpm($s2->status, 's2 status');
 * dpm($s2->get(['status']), 's2 state');
 */

/**
 * Manages TPPS Submission.
 *
 * Allows to set/get/update/save/load:
 * - Submission State Array ('state');
 * - Submission's Metadata ('info');
 * - Submission's Shared State ('sharedState');
 *
 * Not implemented yet:
 * - Submission's tags
 * - Shared State versions.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class Submission {

  /**
   * Submission accession.
   *
   * Format: 'TGDRxxxx'.
   * Stored in tpps_submission.accession as string.
   *
   * @var string
   */
  protected $accession;

  /**
   * Submission's State Array.
   *
   * Stored in tpps_submission.submission_state as serialized string.
   *
   * @var array
   */
  protected $state = [];

  /**
   * Submission State for pipeline processing and other pages.
   *
   * This state is a 1st version of Submission Form State but without some
   * useless (but huge) for processing items/
   * Stored in tpps_submission.shared_state as serialized string.
   *
   * @var array
   */
  protected $sharedState = [];

  /**
   * Internal Submission Id.
   *
   * Seems not used so much but $accession is used a lot.
   * Stored in tpps_submission.tpps_submission_id.
   *
   * @var int
   */
  protected $id = 0;

  /**
   * Drupal User Id of the Submission creator.
   *
   * Stored in tpps_submission.uid.
   *
   * @var int
   */
  protected $uid = 0;

  /**
   * The dbxref_id from chado.dbxref.
   *
   * Stored in tpps_submission.dbxref_id.
   *
   * @var int
   */
  protected $dbxrefId = 0;

  /**
   * Submission status.
   *
   * Stored in public.tpps_submision.status as string.
   * Note: status also stored in $state and $sharedState but no need to update
   * them manually because method setStatus() does it.
   *
   * @var string
   */
  protected $status = '';

  /**
   * Class constructor.
   *
   * @param string $accession
   *   Study accession. Format is 'TGDRxxxx'.
   *   If $accession wasn't specified then it left unset and
   *   could be set using $submission->state = $state which will
   *   populate $this->accession with value form $state array.
   */
  public function __construct(string $accession = NULL) {
    // @TODO Minor. Validate $accession.
    if (!empty($accession)) {
      $this->accession = $accession;
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
  // State's sub-elements.

  /**
   * Sets value of the sub-element of $state (and $sharedState if exists).
   *
   * @param array $parents
   *   Path to the element including element name.
   *   Non empty list of the parent elements on path to the element including
   *   element's name. If $parents are empty [] then use:
   *   Submision::setState or $submission->state = $state.
   * @param mixed $value
   *   Value to be set.
   * @param bool $force
   *   When TRUE then sharedState will be updated even if sub-element not exists.
   *   Default is FALSE.
   */
  public function set(array $parents, $value, $force = FALSE) {
    drupal_array_set_nested_value($this->state, $parents, $value);
    // Update sharedState only if this value was there before.
    $key_exists = FALSE;
    drupal_array_get_nested_value($this->sharedState, $parents, $key_exists);
    if ($key_exists || $force) {
      drupal_array_set_nested_value($this->sharedState, $parents, $value);
    }
    return $this;
  }

  /**
   * Unsets the sub-element of $state (and $sharedState).
   */
  public function unset(array $parents) {

    // Not yet implemented.
    return;
    // @TODO See admin/panel.php:1565.
    if (empty(drupal_array_get_nested_value($array, $path))) {
      drupal_array_set_nested_value($array, $path, NULL);
    }
    // Last element is always a field name.
    $result['field_name'] = end($path);
    array_pop($path);
    return $this;
  }

  /**
   * Gets State item value.
   *
   * @param array $parents
   *   Path to the element including element name.
   *
   * @return mixed
   *   Returns reference to the sub-element of the $state.
   */
  public function &get(array $parents = []) {
    $key_exists = FALSE;
    $element = drupal_array_get_nested_value($this->state, $parents, $key_exists);
    if ($key_exists) {
      return $element;
    }
    else {
      $null = NULL;
      return $null;
    }
  }


  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // State.
  //
  // Properties:
  // - $state array - Submission's State.
  //
  // @TODO Get rid of this methods.
  // - getState() - From memory. Loads from DB if missing.
  //
  // To set/get/update an item:
  // $submission = new Submission($accession);
  // $submission-state[$item_name] = $item_value;

  /**
   * Setter for $state.
   *
   * WARNING: This method will be called when:
   * $submission->state = $state;
   * but not:
   * $submission->state[$element] = $value;
   *
   * @param array $state
   *   Submission State Array.
   */
  public function setState(array &$state) {
    $this->state = &$state;
    // Accession.
    if (!empty($this->state['accession'])) {
      $this->accession = $this->state['accession'];
    }
    // Status must be reset because when study opened for edit status became empty.
    $this->status = $this->status
      ?? $this->state['status']
      ?? TPPS_SUBMISSION_STATUS_INCOMPLETE;
    return $this;
  }

  /**
   * Setter for $uid.
   *
   * @param int $uid
   *   The Drupal User Id of the Submission Creator.
   */
  public function setUid($uid) {
    if ($this->uid == $uid) {
      return;
    }

    // @TODO validate $uid.
    $this->uid = $uid;
    $this->set(['submitting_uid'], $uid);
    return $this;
  }

  /**
   * Setter for $accession.
   *
   * Note:
   * If Submission::load() will be called later then this change
   * will be overwritten. To avoid this use:
   * $submission->load();
   * $submission->accession = $accession;
   * $submission->save();
   *
   * @param string $accession
   *   Submission Accession. E.g., 'TGDR12345'.
   */
  public function setAccession($accession) {
    $this->accession = $accession;
    $this->state['accession'] = $this->sharedState['accession'] = $accession;
    return $this;
  }

  /**
   * Setter for $status.
   *
   * @param string $status
   *   Submission's status. See Submission::getStatusList().
   */
  public function setStatus($status) {
    if (
      !in_array($status, $this->getStatusList())
      && $status != TPPS_SUBMISSION_STATUS_APPROVED_DELAYED
    ) {
      throw new Exception(
        t('Submission status @status is not allowed.', ['@status' => $status])
      );
    }
    if ($this->status != $status) {
      $this->status = $status;
      // Update $state and $sharedState.
      $this->set(['status'], $status);
      $this->set(['values', 'status'], $status);
      $this->set(['saved_values', 'status'], $status);
    }

    if ($status == TPPS_SUBMISSION_STATUS_APPROVED) {
      module_load_include('php', 'tpps', 'forms/submit/submit_all');
      global $user;
      $this->set(['submitting_uid'], $user->uid, TRUE);

      // Prepare data for Tripal Job.
      $includes = [
        module_load_include('php', 'tpps', 'forms/submit/submit_all'),
        module_load_include('inc', 'tpps', 'includes/file_parsing'),
      ];
      $args = [$this->accession];
      $type = $this->state['tpps_type'] ?? 'tpps';
      $type_label = ($type == 'tpps') ? t('TPPS') : t('TPPSC');

// @TODO Avoid code duplication.

      if ($this->state['saved_values']['summarypage']['release']) {

        $jid = tripal_add_job("$type_label Record Submission - $this->accession",
          'tpps', 'tpps_submit_all',
          $args, $this->state['submitting_uid'], 10, $includes, TRUE
        );
        $this->set(['job_id'], $jid, TRUE);
      }
      else {
        $date = $this->state['saved_values']['summarypage']['release-date'];
        $time = strtotime("{$date['year']}-{$date['month']}-{$date['day']}");
        if (time() > $time) {

          $jid = tripal_add_job("$type_label Record Submission - $this->accession",
            'tpps', 'tpps_submit_all',
            $args, $this->state['submitting_uid'], 10, $includes, TRUE
          );
          $this->set(['job_id'], $jid, TRUE);

        }
        else {
          $this->delayRelease();
        }
      }
    }
    return $this;
  }

  /**
   * Delays Submission's release and changes status.
   */
  public function delayRelease() {
    if (!empty($this->accession)) {
      $var_name = 'tpps_delayed_submissions';
      $list = variable_get($var_name, []);
      $list[$this->accession] = $this->accession;
      variable_set($var_name, $list);
      $this->status = TPPS_SUBMISSION_STATUS_APPROVED_DELAYED;
    }
    return $this;
  }

  /**
   * Gets list of Submission Statuses.
   *
   * To just get a list: Submission::getStatusList().
   *
   * @return array
   *   Returns machine names of Submission statuses.
   */
  public static function getStatusList() {
    // Special extra status:
    // TPPS_SUBMISSION_STATUS_APPROVED_DELAYED
    // ('Approved - Delayed Submission Release')
    // We don't use it because it's rarely used.
    return [
      TPPS_SUBMISSION_STATUS_INCOMPLETE,
      TPPS_SUBMISSION_STATUS_PENDING_APPROVAL,
      TPPS_SUBMISSION_STATUS_APPROVED,
      TPPS_SUBMISSION_STATUS_SUBMISSION_JOB_RUNNING,
    ];
  }

  /**
   * Get Submission's state.
   *
   * Will be called any type $submission->state requested.
   * See Submission:__get().
   *
   * @param bool $reset
   *   Flag is cache must be resetted. Default if FALSE.
   *   RESET_CACHE could be used.
   *
   * @return array
   *   Returns an reference to the Submission's State.
   */
  public function &getState(bool $reset = FALSE) {
    if ($reset || (empty($this->state) && !empty($this->accession))) {
      $this->load();
    }
    return $this->state;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submision Common methods.

  /**
   * Saves Submission.
   *
   * @param string $status
   *   Optional. Study processing status.
   *   This value will be added to the Submission Interface Array and to
   *   public.tpps_submission.status column.
   */
  public function save($status = NULL) {
    if (empty($this->accession)) {
      throw new Exception(t('Study accession is empty.'));
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Set status.
    // Note: setStatus() will be called in any case and all places where status
    // is stored will be update.
    $this->status = $status ?? $this->status;
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Update time.
    // Note: REQUEST_TIME couldn't be used because called from CLI script and
    // processing could take long time from script call.
    $this->state['updated'] = $this->sharedState['updated'] = time();

    db_update('tpps_submission')
      ->fields([
        'status' => $this->status ?? TPPS_SUBMISSION_STATUS_INCOMPLETE,
        'submission_state' => serialize($this->state ?? []),
        'shared_state' => serialize($this->sharedState ?? []),
      ])
      ->condition('accession', $this->accession)
      ->execute();
    return $this;
  }

  /**
   * Delete submission from database.
   */
  public function delete() {
    if (empty($this->state['saved_values']['frontpage']['use_old_tgdr'])) {
      if (!empty($this->dbxrefId)) {
        db_delete('chado.dbxref')
          ->condition('dbxref_id', $this->dbxrefId)
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
   * Will populate $state and $sharedState.
   *
   * @param array $state
   *   Submission State array.
   * @param int $uid
   *   Drupal User Id.
   */
  public function create(array $state, $uid = NULL) {
    global $user;

    $this->state = $state;
    $this->accession = $this->state['accession'];
    $fields = [
      'uid' => $uid ?? $user->uid,
      'status' => $this->state['status'] ?? TPPS_SUBMISSION_STATUS_INCOMPLETE,
      'accession' => $this->state['accession'],
      'dbxref_id' => $this->state['dbxref_id'],
      'submission_state' => serialize($this->state),
      'shared_state' => serialize([]),
    ];
    db_insert('tpps_submission')->fields($fields)->execute();
    // Add default tag.
    $tag = $this->isTppsc() ? 'TPPSc' : 'TPPS';
    tpps_submission_add_tag($this->accession, $tag);
    return $this;
  }

  /**
   * Loads Submission's data.
   *
   * Note: all the submission data will be loaded but with single SQL-request
   * instead of 3 requests if data loaded separately.
   *
   * To avoid reload the same study use global $submission object.
   */
  public function load() {
    global $user;
    if (empty($this->accession)) {
      throw new Exception(t('Study accession is empty.'));
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get data from DB.
    $result = db_select('tpps_submission', 's')
      ->fields('s')
      ->condition('accession', $this->accession)
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();
    // Backup option.
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

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Load object's properties.
    if ($result) {
      $this->state = unserialize($result['submission_state'] ?? '') ?? [];
      $this->sharedState = unserialize($result['shared_state'] ?? '') ?? [];
      // Note: no need to set $this->accession.
      $this->id = $result['tpps_submission_id'] ?? 0;
      $this->uid = $result['uid'] ?? $user->uid;
      $this->dbxrefId = $result['dbxref_id'] ?? 0;
      $this->status = $result['status']
        ?? $this->state['status']
        ?? $this->sharedState['status']
        ?? TPPS_SUBMISSION_STATUS_INCOMPLETE;

      // Generate sharedState if it's really empty.
      if (empty($this->sharedState) || empty($this->sharedState['accession'])) {
        $this->generateSharedState();
      }
    }
    return $this;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Helpers

  /**
   * Get only number (without 'TGDR' prefix) of Submission accession.
   *
   * @return int
   *   Return accession number.
   */
  public function getAccessionNumber() {
    return str_replace('TGDR', '', ($this->accession));
  }

  /**
   * Gets number of the organisms.
   *
   * Note: Could be used when TPPS Page 1 submitted.
   *
   * @return int
   *   Returns number of organisms in submission.
   *   Rerurns NULL if submission doesn't exist.
   */
  public function getOrganismNumber() {
    if ($this->doesNotExist()) {
      return NULL;
    }
    return (
      $this->state['saved_values'][TPPS_PAGE_1]['organism']['number']
      ?? $this->sharedState['saved_values'][TPPS_PAGE_1]['organism']['number']
      ?? NULL
    );
  }

  /**
   * Checks if form type is 'tppsc'.
   *
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
    // $is_tppsc = (($form_state['tpps_type'] ?? NULL) == 'tppsc');
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
      !empty($this->accession)
      && !empty($this->id)


// @TODO Uncomment before release.


      //&& !empty($this->status)
      && !empty($this->uid)
      && !empty($this->dbxrefId)
      && !empty($this->state)
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

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

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
    $import_meta = $this->state['saved_values']['tpps_submission_import'] ?? NULL;
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
    $import_meta['full_date'] = $import_meta['imported_date'] ?? '';
    if (variable_get('tpps_imported_study_report_show_timestamp', FALSE)) {
      $import_meta['full_date'] .= (empty($import_meta['imported']) ? ''
        : ' [' . $import_meta['imported'] . ']');
    }
    $import_meta['current_version'] = $this->state['saved_values'][TPPS_SUBMISSION_FORM_VERSION_KEY];
    return $import_meta;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission's Shared State (3S, former Submission Interface).
  //
  // Submission Shared State initially is a copy of the Submission Form State
  // modified for Submission Pipeline to always have the same structure and
  // data format (type). For this each Submission Form State has version
  // (default version is '1') and function Submission::generateSharedState()
  // will convert existing $form_state to the Submission Shared State (3S)
  // which will be stored in 'public.tpps_submissions' database table.

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
   *   Returns an reference to the Submission's SharedState.
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
      ->fields(['shared_state' => 'a:0:{}'])
      ->condition('accession', $this->accession)
      ->execute();
    return $this;
  }

  /**
   * Generates Submission Shared State.
   *
   * WARNING:
   * This method don't update record in database to have ability to get
   * Shared State for processing and for temporary use.
   * Use Submission::save() to store in DB.
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
    $this->state = !empty($state) ? $state : $this->state;

    $this->sharedState = $this->state;
    // We need to apply changes from 1st to $current version to have
    // incremental changes.
    // @TODO Avoid this hard linking.
    module_load_include('inc', 'tpps', 'includes/submission_form_version');
    tpps_submission_form_version_downgrade_to_latest($this->state, $this->sharedState);
    $this->purifySharedState();
    return $this->sharedState;
    // @TODO Change to allow chaining.
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
    return $this;
  }

  /**
   * Dumps object properties into browser.
   *
   * Usage example:
   * $a = 'TGDR906';
   * $s = tpps_submission($a)
   * ->dump('state', ['status'])
   * ->setStatus($status)
   * ->save()
   * ->dump('state', ['status']);
   *
   * @param string $name
   *   Name of the property.
   * @param array $parents
   *   List of parents of the sub-element.
   */
  public function dump($name, array $parents = []) {
    if (property_exists($this, $name)) {
      // Caller function.
      $flags = !DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS;
      $caller_function = debug_backtrace($flags, 2)[1]['function'];
      drupal_set_message('Caller function: ' . $caller_function);
      // Value.
      if (is_array($this->$name) && !empty($parents)) {
        $title = $name . '[' . implode('][', $parents) . ']';
        $value = drupal_array_get_nested_value($this->$name, $parents);
      }
      else {
        $title = $name;
        $value = $this->$name;
      }
      drupal_set_message($title . ': ' . print_r($value, TRUE));
    }
    else {
      drupal_set_message(t("Property @name doesn't exist.", ['@name' => $name]));
    }
    return $this;
  }

  /**
   * Returns a flattened and filtered Shared State.
   *
   * This is useful for any functions that display all of the
   * detailed contents of a TPPS Submission.
   *
   * @return array
   *   The flattened array.
   *
   * @TODO Store changes to $this->flat property and do not return result.
   * @TODO Clean up code.
   */
  public function flatten() {
    $state = $this->sharedState;

    $data = isset($state['saved_values']) ? $state['saved_values'] : NULL;

    if (!$data) {
      return [];
    }
    unset($data['frontpage']);
    unset($data['form_build_id']);

    $page_1 = &$data[TPPS_PAGE_1];
    $pub = &$page_1['publication'];
    $org = &$page_1['organism'];

    if (isset($page_1['#cleared']) and $page_1['#cleared']) {
      $data['#page_1_cleared'] = TRUE;
    }
    else {
      $data['#page_1_cleared'] = FALSE;
    }
    $data['Project ID'] = $state['ids']['project_id'] ?? NULL;
    $data['#study_photo'] = $page_1['photo'] ?? NULL;
    $data['Publication DOI'] = $page_1['doi'] ?? NULL;
    $data['Dataset DOI'] = $page_1['dataset_doi'] ?? NULL;

    $data['Publication Title'] = isset($pub['title']) ? $pub['title'] : NULL;
    $data['Primary Author'] = isset($page_1['primaryAuthor']) ? $page_1['primaryAuthor'] : NULL;
    $data['Publication Status'] = isset($pub['status']) ? $pub['status'] : NULL;
    $data['Publication Year'] = isset($pub['year']) ? $pub['year'] : NULL;
    $data['Publication Abstract'] = isset($pub['abstract']) ? $pub['abstract'] : NULL;
    $data['Publication Journal'] = isset($pub['journal']) ? $pub['journal'] : NULL;

    if (isset($pub['secondaryAuthors']['number'])) {
      $data['#number_secondary_authors'] = $pub['secondaryAuthors']['number'];
      for ($i = 1; $i <= $data['#number_secondary_authors']; $i++) {
        if (isset($pub['secondaryAuthors']["$i"]) and $pub['secondaryAuthors']["$i"] != '') {
          $data["Secondary Author $i"] = $pub['secondaryAuthors']["$i"];
        }
      }
    }

    if (isset($org['number'])) {
      $data['#number_organisms'] = $org['number'];
      for ($i = 1; $i <= $data['#number_organisms']; $i++) {
        if (isset($org["$i"]['name']) and $org["$i"]['name'] != '') {
          $data["Species $i"] = $org["$i"]['name'];
        }
      }
    }
    unset($data[TPPS_PAGE_1]);

    // Page 2.
    $page_2 = &$data[TPPS_PAGE_2];
    $data['#page_2_cleared'] = $page_2['#cleared'] ?? FALSE;
    $data['Experiment Starting Month'] = $page_2['StartingDate']['month'] ?? NULL;
    $data['Experiment Starting Year'] = $page_2['StartingDate']['year'] ?? NULL;
    $data['Experiment Ending Month'] = $page_2['EndingDate']['month'] ?? NULL;
    $data['Experiment Ending Year'] = $page_2['EndingDate']['year'] ?? NULL;
    if (!empty($page_2['study_location'])) {
      // Legacy support for old TPPS submissions.
      $legacy_loc = &$page_2['study_location'];
      $data['#location_type'] = $legacy_loc['type'] ?? NULL;

      if ($data['#location_type'] == '2' and isset($legacy_loc['custom'])) {
        $data['Study Location'] = $legacy_loc['custom'];
      }
      elseif ($data['#location_type'] and isset($legacy_loc['coordinates'])) {
        $data['Study Location'] = $legacy_loc['coordinates'];
      }
    }

    if (isset($page_2['data_type']) and $page_2['data_type']) {
      $data['Data Type'] = $page_2['data_type'];
    }

    if (!empty($page_2['study_type'])) {
      module_load_include('inc', 'tpps', 'includes/form');
      $data['Study Type'] = tpps_form_get_study_type($page_2['study_type']);

      if (!empty($page_2['study_info']['season'])) {
        $data['Seasons Assessed'] = implode($page_2['study_info']['season']);
      }

      if (!empty($page_2['study_info']['assessions'])) {
        $data['Average number of assessions'] = $page_2['study_info']['assessions'];
      }

      $data['Average High Temperature'] = $page_2['study_info']['temp']['high'] ?? NULL;
      $data['Average Low Temperature'] = $page_2['study_info']['temp']['low'] ?? NULL;

      $types = array(
        'co2' => 'CO2',
        'humidity' => 'Air Humidity',
        'light' => 'Light Intensity',
        'salinity' => 'Salinity',
      );

      foreach ($types as $type => $label) {
        if (!empty($page_2['study_info'][$type])) {
          $set = $page_2['study_info'][$type];
          if (!empty($set['option'])) {
            $data["$label Control"] = ($set['option'] == '1') ? 'Controlled' : 'Uncontrolled';
            if ($set['option'] == '1' and !empty($set['controlled'])) {
              $data["$label Value"] = $set['controlled'];
            }
            elseif (!empty($set['uncontrolled'])) {
              $data["$label Value"] = $set['uncontrolled'];
            }
          }
        }
      }

      if (!empty($page_2['study_info']['rooting']['option'])) {
        $data['Rooting Type'] = $page_2['study_info']['rooting']['option'];

        if (
          $data['Rooting Type'] == 'Soil'
          and !empty($page_2['study_info']['rooting']['soil']['type'])
        ) {
          $soil = &$page_2['study_info']['rooting']['soil'];
          $data['Soil Type'] = $soil['type'];

          if ($data['Soil Type'] == 'Other') {
            $data['Custom Soil Type'] = $soil['other'];
          }

          $data['Soil Container Type'] = $soil['container'] ?? NULL;
        }
      }

      if (!empty($page_2['study_info']['rooting']['ph']['option'])) {
        $set = $page_2['study_info']['rooting']['ph'];
        $data['pH Control'] = ($set['option'] == '1') ? 'Controlled' : 'Uncontrolled';

        if ($set['option'] == '1' and !empty($set['controlled'])) {
          $data["pH Value"] = $set['controlled'];
        }
        elseif (!empty($set['uncontrolled'])) {
          $data["pH Value"] = $set['uncontrolled'];
        }
      }

      if (!empty($page_2['study_info']['rooting']['treatment'])) {
        $treatment = $page_2['study_info']['rooting']['treatment'];
        $selected = FALSE;
        $description = FALSE;

        foreach ($treatment as $field => $value) {
          if (!$description) {
            $description = TRUE;
            $selected = ($value) ? $field : FALSE;
            continue;
          }
          elseif ($selected and !empty($value)) {
            $data["Treatment: $selected description"] = $value;
          }
          $description = FALSE;
        }
      }

      if (
        !empty($page_2['study_info']['irrigation'])
        && !empty($page_2['study_info']['irrigation']['option'])
      ) {
        $data['Irrigation Type'] = $page_2['study_info']['irrigation']['option'];
        if (
          $data['Irrigation Type'] == 'Other'
          && !empty($page_2['study_info']['irrigation']['other'])
        ) {
          $data['Custom Irrigation Type'] = $page_2['study_info']['irrigation']['other'];
        }
      }

      if (!empty($page_2['study_info']['biotic_env']['option'])) {
        $biotic_envs = [];
        foreach ($page_2['study_info']['biotic_env']['option'] as $key => $val) {
          if ($val) {
            $biotic_envs[] = $key;
          }
        }

        if (!empty($biotic_envs)) {
          $data['Biotic Environment'] = implode(', ', $biotic_envs);
          if (
            in_array('Other', $biotic_envs)
            && !empty($page_2['study_info']['biotic_env']['other'])
          ) {
            $data['Custom Biotic Environment'] = $page_2['study_info']['biotic_env']['other'];
          }
        }
      }

      if (!empty($page_2['study_info']['treatment']['check'])) {
        $treatment = &$page_2['study_info']['treatment'];
        $selected = FALSE;
        $description = FALSE;

        foreach ($treatment as $field => $value) {
          if ($field != 'check') {
            if (!$description) {
              $description = TRUE;
              $selected = ($value) ? $field : FALSE;
              continue;
            }
            elseif ($selected and !empty($value)) {
              $data["Treatment: $selected description"] = $value;
            }
            $description = FALSE;
          }
        }
      }
    }
    unset($data[TPPS_PAGE_2]);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $page_3 = &$data[TPPS_PAGE_3];

    if (isset($page_3['#cleared']) and $page_3['#cleared']) {
      $data['#page_3_cleared'] = TRUE;
    }
    else {
      $data['#page_3_cleared'] = FALSE;
    }

    $data['#skip_loc'] = FALSE;
    $loc = &$page_3['study_location'];
    $data['#location_type'] = $loc['type'] ?? NULL;

    if ($data['#location_type'] == '2' and isset($loc['custom'])) {
      $data['Study Location'] = $loc['custom'];
    }
    elseif ($data['#location_type'] and isset($loc['coordinates'])) {
      $data['Study Location'] = $loc['coordinates'];
    }
    $page_3 = &$data[TPPS_PAGE_3]['tree-accession'];

    if (!empty($data['#number_organisms'])) {
      for ($i = 1; $i <= $data['#number_organisms']; $i++) {
        if (!empty($page_3["species-$i"]['file'])) {
          $organism_name = $data["Species $i"] . ' Accession File';
          if ($data['#number_organisms'] > 1 and empty($page_3['check'])) {
            $organism_name = 'Plant Accession File';
          }

          $data[$organism_name] = $page_3["species-$i"]['file'];
          $name = 'Location (latitude/longitude or country/state or population group)';
          $loc_type = $page_3["species-$i"]['file-groups'][$name]['#type'] ?? NULL;
          if (
            !empty($loc_type)
            and ($loc_type == "gps" or (array_search('gps', $loc_type) !== FALSE))
          ) {
            $fid = $page_3["species-$i"]['file'];
            drupal_add_js(array(
              'tpps' => array(
                'accession_files' => array(
                  $fid => array(
                    'no_header' => $page_3["species-$i"]['file-no-header'] ?? '0',
                    'id_col' => $page_3["species-$i"]['file-groups']['Tree Id'][1],
                    'lat_col' => $page_3["species-$i"]['file-groups'][$name][4],
                    'long_col' => $page_3["species-$i"]['file-groups'][$name][5],
                  ),
                ),
              ),
            ), 'setting');
          }
        }

        if (empty($page_3['check'])) {
          break;
        }
      }
    }

    $data["Total number of plants"] = $state['stats']['tree_count'] ?? NULL;
    unset($data[TPPS_PAGE_3]);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $page_4 = &$data[TPPS_PAGE_4];

    if (isset($page_4['#cleared']) and $page_4['#cleared']) {
      $data['#page_4_cleared'] = TRUE;
    }
    else {
      $data['#page_4_cleared'] = FALSE;
    }
    if (!empty($data['#number_organisms'])) {
      for ($i = 1; $i <= $data['#number_organisms']; $i++) {
        if (isset($page_4["organism-$i"])) {
          $org = &$page_4["organism-$i"];
          $org_name = isset($data["Species $i"]) ? $data["Species $i"] : NULL;

          if (isset($org['phenotype'])) {
            $pheno = &$org['phenotype'];

            if (!empty($pheno['iso-check'])) {
              $data["$org_name Phenotype Isotope/Mass Spectrometry File"] = $pheno['iso'];
            }
            else {
              if (isset($pheno['phenotypes-meta']['number'])) {
                $data['#number_phenotypes'] = $pheno['phenotypes-meta']['number'];
                $pheno_meta = &$pheno['phenotypes-meta'];
                for ($j = 1; $j <= $data['#number_phenotypes']; $j++) {
                  $fields = array(
                    'name' => 'Name',
                    'attribute' => 'Attribute',
                    'description' => 'Description',
                    'unit' => 'Unit',
                  );
                  if (isset($pheno_meta["$j"]['struct-check']) and $pheno_meta["$j"]['struct-check']) {
                    $fields['structure'] = 'Structure';
                  }
                  // [VS] #8669rmrw5
                  //if (isset($pheno_meta["$j"]['val-check']) and $pheno_meta["$j"]['val-check']) {
                  //  $fields['min'] = 'Minimum Value';
                  //  $fields['max'] = 'Maximum Value';
                  //}
                  // [/VS] #8669rmrw5

                  foreach ($fields as $field => $field_string) {
                    if (isset($pheno_meta["$j"][$field]) and $pheno_meta["$j"][$field] != '') {
                      if ($field == 'unit') {
                        $value = $pheno_meta[$j][$field];
                        $value = tpps_unit_get_name($value);
                      }
                      else {
                        $value = $pheno_meta[$j][$field];
                      }
                      $data["$org_name Phenotype $j $field_string"] = $value;
                    }
                  }
                }
              }

              if (!empty($pheno['check']) && !empty($pheno['metadata'])) {
                $data["$org_name Phenotype Metadata File"] = $pheno['metadata'];
              }

              if (!empty($pheno['file'])) {
                $data["$org_name Phenotype File"] = $pheno['file'];
              }
              $data["$org_name Phenotypes Assessed"] = $pheno['phenotype_count'] ?? NULL;
            }

            // Rish comments: 21 March 2023
            // Based on the example of TGDR465, there is a single ISO file that can be uploaded
            // I *think* the headers should match each phenotype name that is added manually
            // from the previous part of page 4
            if (!empty($pheno['file'])) {
              $data["$org_name Phenotype File"] = $pheno['file'];
            }
          }

          if (isset($org['genotype'])) {
            $geno = &$org['genotype'];
            $snps_fieldset = $geno['SNPs'];

            $marker_types = [];
            // Note: $geno['marker-type'] is not set for phenotype studies.
            foreach (($geno['marker-type'] ?? []) as $key => $val) {
              if ($val) {
                $marker_types[] = $key;
              }
            }
            $data["$org_name Genotype Marker Types"]
              = implode(', ', ($marker_types ?? []));

            if (!empty($geno['marker-type']['SNPs'])) {
              if (!empty($snps_fieldset['genotyping-design'])) {
                $geno_design_options =[
                  1 => 'GBS',
                  2 => 'Targeted Capture',
                  3 => 'Whole Genome Resequencing',
                  4 => 'RNA-Seq',
                  5 => 'Genotyping Array',
                ];
                if (isset($geno_design_options[$snps_fieldset['genotyping-design']])) {
                  $data["$org_name Genotyping Design"]
                    = $geno_design_options[$snps_fieldset['genotyping-design']];
                }

                if (
                  ($data["$org_name Genotyping Design"] ?? NULL) == 'GBS'
                  && !empty($snps_fieldset['GBS'])
                ) {
                  $gbs_options = [
                    1 => 'RADSeq',
                    2 => 'ddRAD-Seq',
                    3 => 'NextRAD',
                    4 => 'RAPTURE',
                    5 => 'Other',
                  ];
                  if (isset($gbs_options[$snps_fieldset['GBS']])) {
                    $data["$org_name GBS Type"] = $gbs_options[$snps_fieldset['GBS']];
                  }

                  if (
                    ($data["$org_name GBS Type"] ?? NULL) == 'Other'
                    && !empty($snps_fieldset['GBS-other'])
                  ) {
                    $data["$org_name Custom GBS Type"] = $snps_fieldset['GBS-other'];
                  }
                }

                if (
                  ($data["$org_name Genotyping Design"] ?? NULL) == 'Targeted Capture'
                  && !empty($snps_fieldset['targeted-capture'])
                ) {
                  $targeted_capture_options = array(
                    1 => 'Exome Capture',
                    2 => 'Other',
                  );
                  if (isset($targeted_capture_options[$snps_fieldset['targeted-capture']])) {
                    $data["$org_name Targeted Capture Type"]
                      = $targeted_capture_options[$snps_fieldset['targeted-capture']];
                  }

                  if (
                    ($data["$org_name Targeted Capture Type"] ?? NULL) == 'Other'
                    && !empty($snps_fieldset['targeted-capture-other'])
                  ) {
                    $data["$org_name Custom Targeted Capture Type"]
                      = $snps_fieldset['targeted-capture-other'];
                  }
                }
              }
            }
            if (
              !empty($geno['marker-type']['SSRs/cpSSRs'])
              && !empty($geno['SSRs/cpSSRs'])
            ) {
              $data["$org_name SSRs/cpSSRs Type"] = $geno['SSRs/cpSSRs'];
            }
            if (
              !empty($geno['marker-type']['Other'])
              && !empty($geno['other-marker'])
            ) {
              $data["$org_name Other Marker Type"] = $geno['other-marker'];
            }

            if (
              isset($geno['ref-genome'])
              && $geno['ref-genome'] === 'bio'
              && isset($geno['BioProject-id'])
              && $geno['BioProject-id'] != ''
            ) {
              $data["$org_name BioProject Id"] = $geno['BioProject-id'];

              if (is_array($geno['assembly-auto'] ?? NULL)) {
                $count = 1;
                foreach ($geno['assembly-auto'] as $id => $check) {
                  if (isset($check) and $check) {
                    $data["$org_name Linked Accession $count"] = $id;
                    $count++;
                  }
                }
              }
            }
            elseif (
              isset($geno['ref-genome'])
              && in_array($geno['ref-genome'], ['manual', 'manual2'])
              && isset($geno['tripal_fasta']['file']['file_upload'])
              and isset($geno['tripal_fasta']['file']['file_upload_existing'])
              and (
                $geno['tripal_fasta']['file']['file_upload']
                || $geno['tripal_fasta']['file']['file_upload_existing']
              )
            ) {
              if ($geno['tripal_fasta']['file']['file_upload']) {
                $data["$org_name Uploaded Reference File"]
                  = $geno['tripal_fasta']['file']['file_upload'];
              }
              if ($geno['tripal_fasta']['file']['file_upload_existing']) {
                $data["$org_name Reference File"]
                  = $geno['tripal_fasta']['file']['file_upload_existing'];
              }
            }
            elseif (
              ($geno['ref-genome'] ?? NULL) == 'url'
              && !empty($geno['tripal_fasta']['file']['file_remote'])
            ) {
              $data["$org_name Website of reference file"] = $geno['tripal_fasta']['file']['file_remote'];
            }
            elseif (!empty($geno['ref-genome'])) {
              $data["$org_name Reference Genome"] = $geno['ref-genome'];
            }

            if (!empty($geno['SNPs']['snps-assay'])) {
              $data["$org_name SNP Genotype Assay File"] = $geno['SNPs']['snps-assay'];
            }

            if (!empty($geno['files']['assay-design'])) {
              $data["$org_name Assay Design File"] = $geno['files']['assay-design'];
            }

            if (!empty($geno['files']['ssrs'])) {
              $data["$org_name SSRs Genotype File"] = $geno['files']['ssrs'];
            }

            if (!empty($geno['files']['ssrs_extra'])) {
              $data["$org_name cpSSRs Additional Genotype File"] = $geno['files']['ssrs_extra'];
            }

            //if (!empty($geno['files']['indels'])) {
            //  $data["$org_name Indel Genotype File"] = $geno['files']['indels'];
            //}

            if (!empty($geno['files']['other'])) {
              $data["$org_name Other Marker Genotype File"] = $geno['files']['other'];
            }

            if (!empty($geno['files']['local_vcf'])) {
              $data["$org_name VCF File (cluster)"] = $geno['files']['local_vcf'];
            }

            if (($geno['files']['vcf'] ?? NULL) > 0) {
              if ($file = tpps_file_load($geno['files']['vcf'])) {
                $file_url = file_create_url($file->uri);
                $data["$org_name VCF File (uploaded)"] = l($file->filename,
                  $file_url, ['attributes' => ['target' => 'blank']]
                );
              }
              else {
                drupal_set_message(t('Uploaded VCF file is missing'), 'error');
                // Value must be not empty to fire error message.
                $data["$org_name VCF File (uploaded)"] = '&nbsp';
              }
            }
          }

          if (isset($org['environment'])) {
            $env = &$org['environment'];

            if (variable_get('tpps_cartogratree_env', FALSE)) {
              $data["CartograPlant Layers"] = array();
              foreach ($env['env_layers'] as $layer => $layer_id) {
                if (!empty($layer_id)) {
                  $data["CartograPlant Layers"][] = $layer;
                }
                if (!empty($layer_id) and !empty($env['env_params'][$layer])) {
                  $params = array();
                  foreach ($env['env_params'][$layer] as $param => $param_id) {
                    if (!empty($param_id)) {
                      $params[] = $param;
                    }
                  }
                  $data["CartograPlant Layer: $layer Parameters"] = implode(', ', $params);
                }
              }
              if (!empty($data["CartograPlant Layers"])) {
                $data["CartograPlant Layers"] = implode(', ', $data["CartograPlant Layers"]);
              }
              else {
                unset($data["CartograPlant Layers"]);
              }
            }
          }
        }
      }
    }
    unset($data[TPPS_PAGE_4]);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $sum_page = &$data['summarypage'];
    if (!empty($sum_page['comments'])) {
      $data["Additional Comments"] = $sum_page['comments'];
    }
    for ($i = 1; $i <= 10; $i++) {
      if (!empty($sum_page['files'][$i])) {
        $data["Supplemental File $i"] = $sum_page['files'][$i];
      }
    }
    if (!empty($sum_page['tree_pictures'])) {
      foreach ($sum_page['tree_pictures'] as $name => $fid) {
        if (
          substr($name, -4) == '_url'
          or substr($name, -12) == '_attribution' or substr($name, -8) == '_license'
        ) {
          continue;
        }
        $data["$name species photo"] = (!$fid) ? t('None provided') : $fid;
      }
    }
    unset($data['summarypage']);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // @TODO Use array_filter().
    $data['#dbxref_id'] = $state['dbxref_id'] ?? NULL;
    $data['#accession'] = $state['accession'] ?? NULL;
    $data['#status'] = $state['status'] ?? NULL;
    foreach ($data as $key => $val) {
      if (!isset($val)) {
        unset($data[$key]);
      }
    }
    return $data;
  }

  /**
   * Checks if user is a submission creator.
   *
   * Note: Creator information also stored in $state and $sharedState but
   * we do not use it here and only check value of the
   * public.tpps_submission.uid field.
   *
   * @param int $uid
   *   The Drupal User Id.
   *
   * @return bool
   *   Returns TRUE if user is the creator and FALSE otherwise.
   *   Non-existing submission will give FALSE.
   */
  public function isCreator($uid = NULL) {
    global $user;
    $uid = $uid ?? $user->uid;
    return (bool) ($this->doesExist() && ($submission->uid == $uid));
  }

}
