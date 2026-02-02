<?php

/**
 * @file
 * Contains 'Submission' class.
 *
 * Note:
 * 'data' element will be added by submit_all.php script to SharedState.
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
 * create() - fills object's properties and creates record in DB with
 *   Submission::STATUS_INCOMPLETE status.
 * save() - Sync object to DB.
 * load() - Sync DB to object.
 * delete() - remove from DB (old)
 * purge() - remove study and files in DB (newer version).
 *
 * ----------------------------------------------------------------------------
 * Submission's Status.
 *
 * To get: $submission->status;
 * To set: $submission->status = Submission::STATUS_APPROVED;
 * To check:
 *   $submission->isStatus($status_name);
 *   $submission->isIncompleteStatus();
 *
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
 *
 * ----------------------------------------------------------------------------
 * Execution time.
 *
 * There are 2 ways to call any method (which takes the same time):
 * 1. tpps_submission_load($a)->hasTag(4) // ~5.4 msec
 * 2. $submission = new Submission($a); // ~2.4 msec
 *    $submission->hasTag(4); // ~3 msec.
 * Performance tip:
 *   Using 'tpps_submission_load()' is good for a single method call
 *   but in other cases it's better to re-use instance $submission.
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

  // Submission Statuses.
  const STATUS_INCOMPLETE = 'Incomplete';
  const STATUS_PENDING_APPROVAL = 'Pending Approval';
  const STATUS_APPROVED = 'Approved';
  const STATUS_SUBMISSION_JOB_RUNNING = 'Submission Job Running';
  // Special statuses:
  const STATUS_APPROVED_DELAYED = 'Approved - Delayed Submission Release';

  // Name of the DB table where Submissions are stored.
  const TABLE = 'tpps_submission';


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
   * See __set()/__get().
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
   * Stored in public.tpps_submission.status as string.
   * Note: status also stored in $state and $sharedState but no need to update
   * them manually because method setStatus() does it.
   *
   * @var string
   */
  protected $status = '';

  /**
   * Flag which indicates that Submission was modified and must be saved to DB.
   *
   * When changed which require saving to database are done then set this flag
   * without manually calling save() method. When all changes are done or
   * hook_exit() was called then submission will be saved.
   *
   * @var bool
   *   When TRUE current submission has modifications which must be saved.
   *   When FALSE submission is the same as database record has.
   *
   * @TODO Implement hook_exit(). See
   * https://api.drupal.org/api/drupal/modules!system!system.api.php/function/hook_exit/7.x
   * @TODO Make $submission global variable so it will exist in hook_exit().
   */
  protected $isChanged = FALSE;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Class constants.

  // Name of the key in $state used to store Zenodo Deposit Id.
  const ZENODO_DEPOSIT_ID_KEY = 'zenodo_id';

  // Name of the key in $state used to store Zenodo Concept Record Id.
  // https://developers.zenodo.org/?shell#representation
  const ZENODO_CONCEPT_RECORD_ID_KEY = 'zenodo_conceptrecid';

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
    // @todo Minor. Validate $accession.
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
    $func = tpps_snake_to_camel_case('set_' . $name);
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
  // State's sub-elements.

  /**
   * Sets value of the sub-element of $state (and $sharedState if exists).
   *
   * WARNING:
   * Only non-modified during submit_all.php script processing elements in
   * $sharedState property could be updated using this method.
   *
   * @param array $parents
   *   Path to the element including element name.
   *   Non empty list of the parent elements on path to the element including
   *   element's name. If $parents are empty [] then use:
   *   Submision::setState or $submission->state = $state.
   * @param mixed $value
   *   Value to be set.
   * @param bool $update_shared_state
   *   Default is FALSE.
   *   When TRUE then sharedState will be updated.
   *   Note that value under $sharedState will always be updated when value
   *   was  found under $state.
   */
  public function set(array $parents, $value, $update_shared_state = FALSE) {
    TppsArray::set($this->state, $parents, $value);
    // Update sharedState only if this value was there before.
    $key_exists = FALSE;
    TppsArray::get($this->sharedState, $parents, $key_exists);
    // @todo Implement correct $parents for shared_state.
    // Shared State could have different $parents cause field was relocated.
    if ($key_exists || $update_shared_state) {
      TppsArray::set($this->sharedState, $parents, $value);
    }
    return $this;
  }

  /**
   * Unsets the sub-element of $state (and $sharedState).
   *
   * WARNING:
   * Only non-modified during submit_all.php script processing elements in
   * $sharedState property could be updated using this method.
   *
   * @param array $parents
   *   List of the parents including field name.
   *
   * @TODO Test.
   */
  public function unset(array $parents) {

    throw Exception('Not yet tested');

    // Last element is always a field name.
    $field_name = end($parents);
    array_pop($parents);
    // State.
    // Check if there is a list of parents.
    if (count($parents) >= 1) {
      $parent_element = &TppsArray::get($this->state, $parents);
    }
    if (!empty($parent_element) && !empty($field_name)) {
      unset($parent_element[$field_name]);
    }
    // sharedState.
    $key_exists = FALSE;
    TppsArray::get($this->sharedState, $parents, $key_exists);
    if ($key_exists) {
      if (count($parents) >= 1) {
        $parent_element = &TppsArray::get($this->sharedState, $parents);
      }
      if (!empty($parent_element) && !empty($field_name)) {
        unset($parent_element[$field_name]);
      }
    }
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
    $element = TppsArray::get($this->state, $parents, $key_exists);
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
  // $submission->state[$item_name] = $item_value;

  /**
   * Setter for $state.
   *
   * WARNING: This method will be called when:
   *   $submission->state = $state;
   *   but not:
   *   $submission->state[$element] = $value;
   */
  public function setState(array $state) {
    // See tpps_main_submit(). Be sure to update all calls and test.
    $this->state = $state;
    // Get submission's accession.
    if (!empty($state['accession'])) {
      $this->accession = $state['accession'];
    }
    // Status must be reset because when study is edited status became empty.
    // Status could be empty if Submission Object was created without $accession
    // and submission wasn't fully loaded.
    if (empty($this->status)) {
      $this->status = $state['status'] ?? Submission::STATUS_INCOMPLETE;
    }

    // @TODO Minor. Get also uid, dbxref_id from state and update them in save().
    return $this;
  }

  /**
   * Setter for $uid.
   *
   * @param int $uid
   *   The Drupal User Id of the Submission Creator.
   */
  public function setUid($uid) {
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
    $this->state['accession'] = $accession;
    $this->sharedState['accession'] = $accession;
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
      && $status != Submission::STATUS_APPROVED_DELAYED
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

    // @todo If status changed, then save and call hook_submission_status_changed()
    // @todo Implement this hook and reset cached lists of TPPS admin panel page.
    // This can't be done in save() method because not always status is changed.
    // @todo Probably it should be a drupal hook?
    if ($status == Submission::STATUS_APPROVED) {
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
      $date = $this->get(['saved_values', 'summarypage', 'release-date']);
      $release_time = ($date)
        ? strtotime("{$date['year']}-{$date['month']}-{$date['day']}")
        : NULL;
      if (
        $this->get(['saved_values', 'summarypage', 'release'])
        || (
          !$this->get(['saved_values', 'summarypage', 'release'])
          && time() > $release_time
        )
      ) {
        $jid = tripal_add_job(
          t('@type_label Record Submission - @accession',
            ['@type_label' => $type_label, '@accession' => $this->accession]
          ),
          'tpps', 'tpps_submit_all',
          $args, $this->get(['submitting_uid']), 10, $includes, TRUE
        );
        $this->set(['job_id'], $jid, TRUE);
      }
      else {
        $this->delayRelease();
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
      $this->status = Submission::STATUS_APPROVED_DELAYED;
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
    // Submission::STATUS_APPROVED_DELAYED
    // We don't use it because it's rarely used.
    return [
      Submission::STATUS_INCOMPLETE,
      Submission::STATUS_PENDING_APPROVAL,
      Submission::STATUS_APPROVED,
      Submission::STATUS_SUBMISSION_JOB_RUNNING,
    ];
  }

  /**
   * Get Submission's state.
   *
   * Will be called any time when $submission->state requested.
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
    // @todo Check case when $accession and $state both empty. What should
    // be returned?
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
    $stack = debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 3);
    if (empty($this->accession)) {
      $message = 'Study accession is empty.';
      // Function drupal_set_message escapes HTML tags.
      watchdog('tpps', $message . '<pre>@backtrace</pre>',
        ['@backtrace' => print_r($stack, 1)],
        WATCHDOG_ERROR
      );
      throw new Exception($message);
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Set status.
    // Note: setStatus() will be called in any case and all places where status
    // is stored will be update.
    $this->status = $status ?? $this->status;
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Update time.
    //
    // Let's check if submission in database wasn't updated after submission
    // in memory was created. Since 'udpated' stored in serialized array and
    // there is no special db table field for this. Let's use SubmissionCache
    // class which updates every time submission is stored.
    $memory_updated = $this->get(['updated']);
    $cached_submission = SubmissionCache::get($this->accession, $reset = FALSE);
    $db_record_updated = $cached_submission['date']['updated'];

    if ($db_record_updated > $memory_updated) {
      // @todo Merge with submission from database somehow.
      $message = 'Submission in database is newer than in memory.';
      if (!tpps_is_cli()) {
        // When in CLI-mode then watchdog-messages are shown in terminal and
        // submit_all.php script uses transactions which change status of the
        // submission and in case of fail rolls back with new status which
        // cause this message be shown. We can't only change field in DB because
        // status is stored (and probably used) also in State and SharedState.
        watchdog('tpps',
          $message . '<br />Updated date in DB: @db.'
          . '<br />Updated date in memory: @memory.'
          . '<pre>@backtrace</pre>',
          [
            '@db' => $db_record_updated,
            '@memory' => $memory_updated,
            '@backtrace' => print_r($stack, 1),
          ],
          WATCHDOG_ERROR
        );
      }
    }

    // Note: REQUEST_TIME couldn't be used because called from CLI script and
    // processing could take long time from script call.
    $this->set(['updated'], time());

    if (function_exists('drupal_alter')) {
      // tpps_submission_save_alter(),
      drupal_alter('submission_save', $submission);
    }

    $fields = [
      'status' => $this->status ?? Submission::STATUS_INCOMPLETE,
      'submission_state' => serialize($this->state ?? []),
      'shared_state' => serialize($this->sharedState ?? []),
    ];
    if (!empty($this->uid)) {
      $fields['uid'] = $this->uid;
    }
    db_update(static::TABLE)
      ->fields($fields)
      ->condition('accession', $this->accession)
      ->execute();

    // tpps_submission_saved().
    if (function_exists('module_invoke_all')) {
      module_invoke_all('submission_saved', $this);
    }
    return $this;
  }

  /**
   * Checks if 'tpps_submission' table has record for this submission.
   *
   * @return bool
   *   Returns TRUE record in 'public.tpps_submission' table found.
   *   Returns FALSE otherwise.
   */
  public function hasDbRecord() {
    return !empty(db_select(static::TABLE, 'ts')
      ->fields('ts', ['tpps_submission_id'])
      ->condition('accession', $this->accession)
      ->range(0, 1)
      ->execute()
      ->fetchField()
    );
  }

  /**
   * Gets status of the files of the submission.
   *
   * @return bool
   *   Returns TRUE if submission has files and FALSE otherwise.
   */
  public function hasFiles() {
    $cached_submission = SubmissionCache::get($this->accession);
    if (empty($cached_submission)) {
      $cached_submission = SubmissionCache::get($this->accession, $reset = TRUE);
    }
    if (!empty($cached_submission)) {
      return !empty(count($cached_submission['file']['id_list']));
    }
    else {
      $message = t(
        "Can't get list of files for submission @accession",
        ['@accession' => $this->accession]
      );
      //throw new Exception($message);
    }
  }

  /**
   * Gets list of all local VCF files.
   *
   * @return array
   *   Returns associated array where key is organism index and value is a
   *   path to the local VCF file stored on the server.
   */
  public function getLocalVcfList() {
    $sharedState = new FormState($this->sharedState);
    $list = [];
    for ($i = 1; $i <= $this->getOrganismNumber(); $i++) {
      $parents = $this->getLocalVcfParents($i);
      $local_vcf = $sharedState->get($parents);
      if (empty($local_vcf)) {
        continue;
      }
      $list[$i] = strip_tags($local_vcf);
    }
    return $list;
  }

  /**
   * Removes submission's files.
   *
   * Local VCF files will be removed only on live site and after confirmation.
   * By default local VCF files are renamed (not removed).
   *
   * Drupal managed files will be removed if other studies not use them but
   * in 'force' mode those files will be removed anyway.
   *
   * @param bool $force
   *   Indicates that 'force mode' should be used.
   *   In this mode:
   *   1. Local VCF files will be removed on live (not on dev which
   *      re-uses files from live-site).
   *   2. Managed files will be removed even if other studies are using them.
   */
  public function removeFiles($force = FALSE) {
    $debug_mode = TRUE;

    if (empty($this->sharedState)) {
      return;
    }
    $sharedState = new FormState($this->sharedState);
    // Flag which is TRUE when at least one file was removed.
    $any_file_was_removed = FALSE;

    // Remove local VCF files.
    if ($force && is_live_site()) {
      // Local VCF files must be removed only on live site because dev-site
      // re-uses this files and often has clonned studies which shares the
      // same local VCF files.
      for ($i = 1; $i <= $this->getOrganismNumber(); $i++) {
        $local_vcf_usage = $this->getLocalVcfUsage($i);
        if ($local_vcf_usage == 1) {
          // File exists and in use by only one study.
          $path = $this->getLocalVcf($organism_index);
          $parents = $this->getLocalVcfParents($i);

          $file_path = strip_tags($local_vcf);
          // Safer way is to rename file to allow rollback.
          if (variable_get('tpps_submission_remove_local_vcf', FALSE)) {
            $removement_result = unlink($file_path);
          }
          else {
            // Do not remove but create a backup copy.
            $removement_result = rename($file_path, $file_path . '.removed');
          }
          if ($removement_result) {
            //$this->sharedState = $sharedState->unset($parents);
            TppsArray::set($this->sharedState, $parents, NULL);
            $any_file_was_removed = TRUE;
          }
          else {
            $message = "File '@file_path' wasn't removed. Check permissions or file path.";
            $params = ['@file_path' => $file_path];
            drupal_set_message(t($message, $params), 'error');
            watchdog('tpps', $message, $params, WATCHDOG_ERROR);
          }
        }
      }
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Remove Drupal managed files.
    $parent_list = $this->getFileParents();
    foreach ($parent_list as $step => $step_list) {
      foreach ($step_list as $parents) {
        $new_parents = array_merge([$step], $parents);
        $fid = (int) TppsArray::get($this->sharedState['saved_values'], $new_parents);
        if (empty($fid)) {
          continue;
        }
        if ($file = tpps_file_load($fid)) {
          $count = (int) !$force;
          $usage = TppsFile::getUsage($file);
          // @TODO Minor. Use getAccessionNumber().
          $key = str_replace('TGDR', '', $this->accession);
          if (
            count($usage) > 1
            || (!empty($usage[$key]) && $usage[$key] >= 1)
            || (count($usage) == 1 && empty($usage[$key]))
          ) {
            // File used by other studies.
            if ($force) {
              // We still remove this file because of the 'force mode'.
            }
            else {
              // Do NOT remove this file and do NOT reduce it's usage counter.
              $message = "File '@fid' wasn't removed because stil in use: !usage";
              $params = [
                '@fid' => $fid,
                '!usage' => TppsArray::print($usage),
              ];
              drupal_set_message(t($message, $params), 'error');
              continue;
            }
          }
          // Remove file usage record (not file itself).
          TppsFile::deleteUsage($file, $this->accession, $count);
          // 0 - remove all ussage; 1 - (default) remove single usage.
          // Remove file from disk.
          $result = file_delete($file, $force);

          if ($result === TRUE) {
            // Sucess.
            // Remove from submission.
            // Note: Submission will be saved when all files will be processed.
            $field_parents = array_merge(['saved_values'], $new_parents);
            $this->set(
              $field_parents,
              $value = NULL,
              $update_shared_state = TRUE
            );
            // Unset also sub-fields.
            $field_name = $field_parents[count($field_parents) - 1];
            $subfield_list = [
              'columns',
              'no-header',
              'empty',
              'groups',
            ];
            foreach ($subfield_list as $sub_name) {
              $field_parents[count($field_parents) - 1]
                = implode('-', [$field_name, $sub_name]);
              $this->set(
                $field_parents,
                $value = NULL,
                $update_shared_state = TRUE
              );
            }
            // If at least one file was removed all data loaded to DB must be
            // removed.
            $any_file_was_removed = TRUE;
          }
          elseif ($result === FALSE) {
            // Error.
            $message = "File #@fid wasn't removed.";
            $params = ['@fid' => $fid];
            drupal_set_message(t($message, $params), 'error');
            watchdog('tpps', $message, $params, WATCHDOG_ERROR);
          }
          elseif (is_array($result)) {
            // Used by other modules/studies.
            $message = "File #@fid wasn't removed because it's used by other studies: !usage";
            $params = [
              '@fid' => $fid,
              '!usage' => TppsArray::print($usage),
            ];
            drupal_set_message(t($message, $params), 'error');
            watchdog('tpps', $message, $params, WATCHDOG_ERROR);
          }
        }
      }
    }
    if ($any_file_was_removed) {
      // Clear submission data loaded into DB.
      $this->clearDataBase();
      // No need to manually update SubmissionCache because it will be updated
      // automatically on submission's save.
      $this->save();
    }
  }

  /**
   * Removes record from 'chado.dbxref' table.
   *
   * Note:
   * It's enough to remove record in 'chado.dbxref' by study accession.
   */
  public function clearDbxrefTable() {
    db_delete('chado.dbxref')
      ->condition('accession', $this->accession)
      ->execute();
  }

  /**
   * Clears submission's data loaded into DB.
   *
   * This function is particularly useful when submitting a study to the database
   * multiple times. Note that Tripal Entities should be manually un-published
   * after this process has completed.
   *
   * @return bool
   *   Returns TRUE if submission successfully cleared and FALSE otherwise.
   */
  public function clearDataBase() {
    $debug_mode = FALSE;
    if ($debug_mode ?? 0) {
      return;
    }

    $this->sharedState['file_rank'] = 0;
    $project_id = $this->sharedState['ids']['project_id'] ?? NULL;
    if (empty($project_id)) {
      return FALSE;
    }
    chado_query("UPDATE chado.project_dbxref SET is_current = FALSE WHERE project_id = $project_id");

    foreach ($this->getTableListToClearDataBase() as $table_name) {
      dpm($table_name);
      //$this->removeLoadedData($table_name);
    }
    $this->sharedState['file_rank'] = 0;
    $this->save();
    return TRUE;
  }

  /**
   * Gets list of table names to be cleared.
   *
   * See clearDataBase() method.
   *
   * @return array
   *   Returns list of table names with leading schema.
   *
   * @TODO Minor. Merge with getTableListToRemoveProjectData().
   */
  public function getTableListToClearDataBase() {
    return [
      'chado.phenotype_cvterm',
      'chado.phenotype_to_synonym',
      'chado.phenotype',
      'chado.genotype_reads_per_plant',
      'chado.plants_per_marker',
      'chado.stock_genotype',
      'chado.stock_phenotype',
      'chado.stock',
      'chado.phenotype',
      'chado.plants_per_marker',
    ];
  }

  /**
   * Removes study data loaded into given table.
   *
   * From function tpps_submission_clear_db().
   *
   * @param string $table
   *   Table name with leading schema.
   *
   * @return mixed
   *   Returns FALSE if 'project_id' not found in Shared State.
   *   Returns TRUE in case of success sql query execution.
   */
  public function removeLoadedData($table) {
    if (function_exists('tpps_log')) {
      tpps_log('DELETE ' . $table . ' values...');
    }
    $project_id = $this->sharedState['ids']['project_id'] ?? NULL;
    if (empty($project_id)) {
      return FALSE;
    }
    // @TODO Execute $stocks_sql and use plain list of ids instead of query.
    $stocks_sql = "SELECT stock_id FROM chado.project_stock WHERE project_id = $project_id";
    // @TODO Minor. Move this check to class constructor method.
    if (strpos($this->accession, 'TGDR') !== FALSE && strlen($this->accession) >= 7) {
      $accession = strtoupper($this->accession);
    }
    else {
      return FALSE;
    }

    $sql = [
      // Stocks_sql.
      'chado.phenotype_cvterm' => 'DELETE FROM ' . $table . ' WHERE
        phenotype_id IN (SELECT phenotype_id from chado.stock_phenotype WHERE
        stock_id in (' . $stocks_sql . '))',
      'chado.phenotype_to_synonym' => 'DELETE FROM ' . $table
        . ' WHERE phenotype_id IN (SELECT phenotype_id FROM chado.stock_phenotype
        WHERE stock_id IN (' . $stocks_sql . '))',
      'chado.phenotype' => 'DELETE FROM ' . $table . ' WHERE phenotype_id IN
        (SELECT phenotype_id FROM chado.stock_phenotype WHERE stock_id IN
        (' . $stocks_sql . '))',
      'chado.stock_genotype' => "DELETE FROM $table WHERE stock_id IN ( $stocks_sql)",
      'chado.stock_phenotype' => "DELETE FROM $table WHERE stock_id IN ($stocks_sql)",
      'chado.stock' => "DELETE FROM $table WHERE stock_id IN ($stocks_sql)"
        // Force delete stock just in case by study accession name.
        . " OR uniquename LIKE $accession-%",

      // Accession.
      'chado.genotype_reads_per_plant' => "DELETE FROM $table WHERE study_accession = '$accession'",
      'chado.plants_per_marker' => "DELETE FROM $table WHERE study_accession = '$accession'",
      'chado.plants_per_marker' => "DELETE FROM $table WHERE study_accession = '$accession'",
      // Accession with suffix.
      'chado.phenotype' => "DELETE FROM $table WHERE uniquename LIKE '$accession-%'",
    ];
    if (empty($sql[$table])) {
      $message = t("Can't find sql-query for table @table.", ['@table' => $table]);
      throw new Exception($message);
    }
    else {
      if (db_table_exists($table)) {
        chado_query($sql[$table]);
      }
    }
  }

  /**
   * Removs SubmissionCache data.
   */
  public function removeSubmissionCache() {
    SubmissionCache::remove($this->accession);
  }

  /**
   * Checks if submission has cached version.
   *
   * See 'SubmissionCache' class.
   *
   * @return bool
   *   Returns TRUE if submission was cached and FALSE otherwise.
   */
  public function isCached() {
    return !empty(SubmissionCache::get($this->accession, $reset = TRUE));
  }

  /**
   * Removes parts of the  Submission.
   *
   * To remove only record from tpps_submission' table see Submission::remove().
   */
  public function remove(array $options) {






// @TODO Implement. See remove* methods.






  }

  /**
   * Totally removes the Submission.
   *
   * To remove only record from tpps_submission' table see Submission::remove().
   */
  public function purge() {
    $this->remove();

    // Remove record from 'public.tpps_submission' table.
    db_delete(static::TABLE)
      ->condition('accession', $this->accession)
      ->execute();

    if (function_exists('module_invoke_all')) {
      // 'tpps_submission_purged'.
      module_invoke_all('submission_purged', $this->accession);
    }
    // Unset current instance (in memory).
    $this->accession = NULL;
    $this->state = NULL;
    $this->sharedState = NULL;
    // Do not allow chaining methods because object destroyed.
    return NULL;
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
    $this->accession = $state['accession'];
    $fields = [
      'uid' => $uid ?? $user->uid,
      'status' => $state['status'] ?? Submission::STATUS_INCOMPLETE,
      'accession' => $state['accession'],
      'dbxref_id' => $state['dbxref_id'],
      'submission_state' => serialize($state),
      'shared_state' => serialize([]),
    ];
    db_insert(static::TABLE)->fields($fields)->execute();
    // Add default tag.
    $tag = $this->isTppsc() ? 'TPPSc' : 'TPPS';
    $this->addTag($tag);
    return $this;
  }

  /**
   * Loads Submission's data.
   *
   * Note: all the submission data will be loaded but with single SQL-request
   * instead of 3 requests if data loaded separately.
   *
   * To avoid reload the same study use global $submission object.
   *
   * @param mixed $generate_shared_state
   *   When TRUE and 'sharedState' property is empty after loading from DB then
   *   it will be generated.
   *   This could create ethernal loop when record in DB has not 'sharedState'.
   *   Default is TRUE.
   */
  public function load($generate_shared_state = FALSE) {
    global $user;
    if (empty($this->accession)) {
      throw new Exception(t('Study accession is empty.'));
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get data from DB.
    $result = db_select(static::TABLE, 'ts')
      ->fields('ts')
      ->condition('accession', $this->accession)
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();
    // Backup option.
    if (!$result) {
      // Search for alternative accessions.
      $query = db_select(static::TABLE, 'ts');
      $query->join(
        'tpps_submission_dbxref',
        's_dbx',
        'ts.tpps_submission_id = s_dbx.tpps_submission_id');
      $query->join(
        'chado.dbxref',
        'dbx',
        'dbx.dbxref_id = s_dbx.dbxref_id'
      );
      $result = $query->fields('ts')
        ->condition('dbx.accession', $this->accession)
        ->range(0, 1)
        ->execute()
        ->fetchAssoc();
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Load object's properties.
    if ($result) {
      // Note:
      // When 'submission_state' became "broken" when edited (and saved)
      // directly in phpPgAdmin it produces:
      // Notice: unserialize(): Error at offset 6 of 13667 bytes in
      // tpps_submission_get_accession_list() (line 1098 of
      // /var/www/Drupal/sites/all/modules/TGDR/includes/submissions.inc).
      // So to avoid this we do simple check:
      $this->state = [];
      if (strpos($result['submission_state'], '&quot;accession&quot;') === FALSE) {
        $this->state = unserialize($result['submission_state'] ?? '') ?? [];
      }
      $this->sharedState = [];
      if (strpos($result['shared_state'], '&quot;accession&quot;') === FALSE) {
        $this->sharedState = unserialize($result['shared_state'] ?? '') ?? [];
      }
      // Note: no need to set $this->accession.
      $this->id = $result['tpps_submission_id'] ?? 0;
      $this->uid = $result['uid'] ?? $user->uid;
      $this->dbxrefId = $result['dbxref_id'] ?? 0;
      $this->status = $result['status']
        ?? $this->state['status']
        ?? $this->sharedState['status']
        ?? Submission::STATUS_INCOMPLETE;

      // Generate sharedState if it's really empty.
      if ($generate_shared_state) {
        if (empty($this->sharedState) || empty($this->sharedState['accession'])) {
          $this->generateSharedState();
        }
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
   * Check if submission is empty.
   *
   * Note: Submission could exist in DB (use doesExists() to check this).
   * This method checks only state in memory.
   * $submission = new Submission();
   * $submission->isEmpty(); // Gives TRUE.
   *
   * @return bool
   *   Returns TRUE if submission is empty (no data added yet).
   */
  public function isEmpty() {
    return empty($this->accession);
  }

  /**
   * Gets Project Id.
   *
   * @return int
   *   Returns project id.
   *   Rerurns NULL if submission doesn't exist.
   */
  public function getProjectId() {
    if ($this->doesNotExist()) {
      return NULL;
    }
    return ($this->sharedState['ids']['project_id'] ?? NULL);
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
      ?? $this->state['stats']['species_count']
      ?? NULL
    );
  }

  /**
   * Checks if form type is 'tppsc'.
   *
   * Note: this is static method to be used in form building/validation.
   * See dynamic method: isTppsc().
   * See also tpps_is_admin_or_curation() which checks user role/permissions.
   *
   * @return bool
   *   Returns TRUE if form Id is 'tppsc_main' and FALSE otherwise.
   */
  public static function isCurationForm(array $submission_state) {
    $form_id = $submission_state['saved_values']['frontpage']['form_id']
      ?? $submission_state['build_info']['form_id']
      ?? 'tpps_main';
    return $form_id == 'tppsc_main';
  }

  /**
   * Checks if form type is 'tppsc'.
   *
   * WARNING: $submission->isTppsc() checks form_id which is not always set.
   * See: tpps_is_admin_or_curation().
   *
   * @return bool
   *   Returns TRUE if form Id is 'tppsc_main' and FALSE otherwise.
   *
   * @todo Check 'state/saved_values/frontpage/form_type'.
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
   * See TppsForm::getId();
   *
   * @return string
   *   Returns 'tpps_main' or 'tppsc_main'.
   */
  public function getFormId() {
    // $is_tppsc = (($form_state['tpps_type'] ?? NULL) == 'tppsc');
    return $this->state['saved_values']['frontpage']['form_id']
      ?? $this->state['build_info']['form_id'] ?? 'tpps_main';
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Info.

  /**
   * Checks if submission was loaded into memory from database.
   *
   *
   * WARNING:
   * Do not use $this->load() because when methods doesExist() / doesNotExist()
   * are called in the middle of the process then unsaved changes will be lost.
   *
   * @return bool
   *   Returns TRUE if record in database exists.
   *   Returns FALSE if there is no record in database.
   */
  public function doesExist() {
    return (bool) (!empty($this->accession) && !empty($this->state));
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
   * Checks if submission's state has Import Metadata.
   *
   * @return bool
   *   Returns TRUE if submission was imported and FALSE otherwise.
   */
  public function hasImportMeta() {
    return !empty($this->get(['saved_values', 'tpps_submission_import']));
  }

  /**
   * Removes Import Meta-data from State and SharedState.
   */
  public function removeImportMeta() {
    // @TODO Minor. Test and use $this->unset().
    $this->set(['saved_values', 'tpps_submission_import'], NULL, TRUE);
    // Note: save() will update SubmissionCache.
    $this->save();
  }

  /**
   * Checks if submission was imported.
   *
   * @return bool
   *   Returns TRUE if submission was imported and FALSE otherwise.
   */
  public function wasImported() {
    return !empty(ImportedSubmissionList::get()[$this->accession]);
  }

  /**
   * Removes record from the Imported submission list.
   *
   * This list is stored in variable because this is developer's tool and
   * using separate table for this is overrill. Cache table also couldn't be
   * used because this list must be stored for a long time.
   */
  public function removeImportRecord() {
    // Check if study was imported to avoid extra moves.
    if ($this->wasImported()) {
      ImportedSubmissionList::remove($this->accession);
    }
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

    // @TODO ? Create Submission class's property $importMeta.
    $import_meta = TppsArray::get($this->state,
      ['saved_values', 'tpps_submission_import']
    );
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
    $import_meta['current_version'] = $this->getFormVersion();
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
   * Get Submission's Shared State.
   *
   * @param bool $reset
   *   Flag is cache must be resetted. Default if FALSE.
   *   RESET_CACHE could be used.
   *
   * @return array
   *   Returns an reference to the Submission's Shared State.
   */
  public function &getSharedState(bool $reset = FALSE) {
    if ($reset || empty($this->sharedState)) {
      if (!empty($this->state)) {
        $this->generateSharedState();
      }
      elseif (!empty($this->accession)) {
        // WARNING:
        // To generate Shared State data from DB will be used and all changes
        // made in memory will be overwritten.
        // But $this->state is empty so it will be probably 'status' and 'uid'.
        $this->load($generate_shared_state = TRUE);
      }
    }
    return $this->sharedState;
  }

  /**
   * Get Submission's status.
   *
   * Note: Will be used status stored in separate column of the
   * 'tpps_submission' table. There are also status value stored in 'state'
   * and 'sharedState'.
   *
   * @return string
   *   Returns Submission's status
   */
  public function &getStatus(bool $reset = FALSE) {
    if (empty($this->status) && !empty($this->accession)) {
      $this->load();
    }
    return $this->status;
  }

  /**
   * Checks if current status of submission matches given name of the status.
   *
   * @param string $status
   *   Submission's status. See constants in 'Submission' class.
   *
   * @return bool
   *   Returns TRUE if given status name and current status of the submission
   *   are the same. Returns FALSE otherwise.
   */
  public function isStatus($status) {
    return $this->status == $status;
  }

  /**
   * Checks if current status of submission is Submission::STATUS_INCOMPLETE.
   *
   * @return bool
   *   Returns TRUE if given status is Submission::STATUS_INCOMPLETE.
   *   Returns FALSE otherwise.
   */
  public function isIncompleteStatus() {
    return $this->status == Submission::STATUS_INCOMPLETE;
  }

  /**
   * Checks if current status is Submission::STATUS_PENDING_APPROVAL.
   *
   * @return bool
   *   Returns TRUE if given status is Submission::STATUS_PENDING_APPROVAL.
   *   Returns FALSE otherwise.
   */
  public function isPendingApprovalStatus() {
    return $this->status == Submission::STATUS_PENDING_APPROVAL;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Removes Submission's Shared State.
   */
  public function removeSharedState() {
    db_update(static::TABLE)
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
   */
  public function generateSharedState($force_presave = FALSE) {
    // If submission has NULL in 'tpps_submission.shared_state' column it cause
    // ethernal loop. To avoid this non-empty value of the 'shared_state' field
    // must be created. See also tpps_update_7130().
    // To fix submission run:
    // $a = 'TGDRxxxx';
    // $s = new Submission();
    // $s->accession = $a;
    // $s->generateSharedState(TRUE)->save();
    if ($force_presave) {
      db_update(static::TABLE)
        ->fields(['shared_state' => serialize(['accession' => $this->accession])])
        ->condition('accession', $this->accession)
        ->execute();
    }

    // WARNING:
    // This extra step is required to break link between sharedState and state
    // properties of current Submission object.
    // Problem: when 'summarypage' step submitted 'state' downgraded
    // to version 1.
    // Here we create new (independent) submission which will never be saved
    // to DB and it used only to create SharedState.
    $temp_submission = new Submission();
    $temp_submission->accession = $this->accession;
    // We can't use __construct() because it will attempt to generate missing
    // sharedState and will create ethernal loop.
    $temp_submission->accession = $this->accession;
    $temp_submission->load($generate_shared_state = FALSE);
    $temp_state = $temp_submission->state;
    unset($temp_submission);

    $this->sharedState = SubmissionVersion::downgrade($temp_state);
    $this->purifySharedState();
    return $this;
  }

  /**
   * Reduce size of the Submission Interface array.
   *
   * 3S items which doesn't used by pipeline processing script will be removed.
   */
  public function purifySharedState() {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Remove useless elements to reduce size.
    $change_list = [
      // We unset not used items instead of copy required items from
      // $form_state because if new elements will be found they will be
      // left for sure. It's safer.
      'unset' => [
        [
          'field_name' => 'form_build_id',
          'parents' => ['saved_values', 'form_build_id'],
        ],
      ],
    ];
    // List of 1st level items to be removed from the 'sharedState' array.
    $field_list = [
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
      'tpps_submission_export_files',
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // List of the fields which must be left.
      // Required by submit_all() and tpps_table_display() and sub-functions.
      // 'saved_values',
      // 'dbxref_id',
      // 'created',
      // 'accession',
      // 'status',
      // 'stats', // Belongs to Submission State.
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
      // Stores tree id and coordinates to show google map on study details page.
      // 'tree_info'.
      //
      //  Probably could be unset:
      // 'stage',
    ];
    foreach ($field_list as $field) {
      $change_list['unset'][] = [
        'field_name' => $field,
        'parents' => [$field],
      ];
    }

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
        $change_list['unset'][] = [
          'field_name' => $field,
          'parents' => ['saved_values', $page, $field],
        ];
      }
    }

    $this->sharedState = TppsArray::applyChanges($change_list, $this->sharedState);
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

      dpm(print_r(debug_backtrace($flags, 3), TRUE));
      // Value.
      if (is_array($this->$name) && !empty($parents)) {
        $title = $name . '[' . implode('][', $parents) . ']';
        $value = TppsArray::get($this->$name, $parents);
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
    return (bool) ($this->doesExist() && ($this->uid == $uid));
  }

  /**
   * Get name of current form's stage.
   *
   * Note: Current stage could differ from last completed stage.
   * To get latest stage passed validation use getLastCompletedStage() method.
   * Stage is a step of the multistep form.
   * Stages: 'frontpage', 1, 2, 3, 4, 'summarypage'.
   *
   * @return mixed
   *   Returns current stage.
   */
  public function getCurrentStage() {
    return $this->get(['stage']);
  }

  /**
   * Gets current stage name.
   *
   * @return string
   *   Returns not localized stage name.
   */
  public function getCurrentStageName() {
    $stage = $this->getCurrentStage();
    $names = $this->getStageNames();
    return $names[$stage] ?? 'Unknown';
  }

  /**
   * Gets list of stage names.
   *
   * @return array
   *   Returns list of stages and their human readable names.
   */
  public function getStageNames() {
    return [
      TPPS_PAGE_1 => "Author and Species Information",
      TPPS_PAGE_2 => "Experimental Conditions",
      TPPS_PAGE_3 => "Plant Accession",
      TPPS_PAGE_4 => "Submit Data",
      'summarypage' => "Review Data and Submit",
    ];
  }

  /**
   * Gets name of last completed form's stage.
   *
   * Completed stage is a step of multistep form which passed validation
   * and was sucessfully submitted.
   * To get current stage use getLastCompletedStage() method.
   *
   * Stage is a step of the multistep form.
   * Stages: 'frontpage', 1, 2, 3, 4, 'summarypage'.
   *
   * @return mixed
   *   Returns name of the last completed stage.
   */
  public function getLastCompletedStage() {
    $last_completed_stage = NULL;
    $stage_list = ['frontpage', 1, 2, 3, 4, 'summarypage'];
    foreach ($stage_list as $stage) {
      if ($this->get(['saved_values', $stage, '#cleared'])) {
        $last_completed_stage = $stage;
      }
    }
    return $last_completed_stage;
  }

  /**
   * Checks if study has phenote data.
   *
   * Checks fields on Page 4.
   * See also isPhenotypeStudy() which checks value of the selectbox on Page 2.
   *
   * @return bool
   *   Returns TRUE when study has phenotype data and FALSE otherwise.
   */
  public function hasPhenotypeData() {
    $this->getState();
    for ($i = 1; $i <= $this->getOrganismNumber(); $i++) {
      $organism_key = 'organism-' . $i;
      if (
        // Phenotype medatadata file could be missing when data added manually
        // but data file is required.
        !empty($this->state['saved_values'][TPPS_PAGE_4]
        [$organism_key]['phenotype']['file'])
      ) {
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Checks if study has Mass Environmental data.
   *
   * Checks fields on Page 4.
   * See also isEnvironmentStudy() which checks value of the selectbox on Page 2.
   *
   * @return bool
   *   Returns TRUE when study has phenotype data and FALSE otherwise.
   */
  public function hasEnvironmentalData() {
    $this->getState();
    for ($i = 1; $i <= $this->getOrganismNumber(); $i++) {
      $organism_key = 'organism-' . $i;
      if (
        !empty($this->state['saved_values'][TPPS_PAGE_4]
        [$organism_key]['environment'])
      ) {
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Checks if study has Mass Spectrometry data.
   *
   * @return bool
   *   Returns TRUE when study has phenotype data and FALSE otherwise.
   */
  public function hasMassSpectrometryData() {
    $this->getState();
    for ($i = 1; $i <= $this->getOrganismNumber(); $i++) {
      $organism_key = 'organism-' . $i;
      if (
        !empty($this->state['saved_values'][TPPS_PAGE_4]
        [$organism_key]['phenotype']['iso-check'])
      ) {
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Checks if study has genotype data.
   *
   * Checks fields on Page 4.
   * See also isGenotypeStudy() which checks value of the selectbox on Page 2.
   *
   * @return bool
   *   Returns TRUE when study has phenotype data and FALSE otherwise.
   */
  public function hasGenotypeData() {
    $this->getState();
    $page4 = $this->state['saved_values'][TPPS_PAGE_4] ?? NULL;
    for ($i = 1; $i <= $this->getOrganismNumber(); $i++) {
      $organism_key = 'organism-' . $i;
      if (!empty($page4)
        && !empty($genotype = $page4[$organism_key]['genotype'] ?? NULL)
        && (
          ($genotype['does_study_include_snp_data'] ?? NULL) == "yes"
          || ($genotype['does_study_include_ssr_cpssr_data'] ?? NULL) == "yes"
          || ($genotype['does_study_include_other_genotypic_data'] ?? NULL) == "yes"
        )
      ) {
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Checks if study has local (located on server) VCF file.
   *
   * @param int $organism_index
   *   Ordinal number of the organism at page.
   *
   * @return bool
   *   Returns TRUE if study has local VCF file and FALSE otherwise.
   */
  public function hasLocalVcf($organism_index = 1) {
    $this->getSharedState();
    $organism_key = 'organism-' . $organism_index;
    $vcf_file_location_parents = [
      'saved_values',
      TPPS_PAGE_4,
      $organism_key,
      'genotype',
      'files',
      'vcf_file-location',
    ];
    if (
        TppsArray::get($this->sharedState, $vcf_file_location_parents) == 'remote'
        && !empty(TppsArray::get(
          $this->sharedState,
          $this->getLocalVcfParents($organism_index)
        ))
    ) {
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Checks if study has local (located on server) VCF file.
   *
   * @param int $organism_index
   *   Ordinal number of the organism at page.
   *
   * @return mixed
   *   Returns path to the local VCF file.
   *   Returns NULL if study has no local VCF file.
   */
  public function getLocalVcfParents($organism_index = 1) {
    $organism_key = 'organism-' . $organism_index;
    return [
      'saved_values',
      TPPS_PAGE_4,
      $organism_key,
      'genotype',
      'files',
      'local_vcf',
    ];
  }

  /**
   * Checks if study has local (located on server) VCF file.
   *
   * @param int $organism_index
   *   Ordinal number of the organism at page.
   *
   * @return mixed
   *   Returns path to the local VCF file.
   *   Returns NULL if study has no local VCF file.
   */
  public function getLocalVcf($organism_index = 1) {
    $organism_key = 'organism-' . $organism_index;
    if (!$this->hasLocalVcf($organism_index)) {
      return NULL;
    }
    return TppsArray::get(
      $this->sharedState,
      $this->getLocalVcfParents($organism_index)
    );
  }

  /**
   * Checks if study has local (located on server) VCF file.
   *
   * @param int $organism_index
   *   Ordinal number of the organism at page.
   *
   * @return mixed
   *   Returns number of studies which uses this file.
   *   Returns NULL if organism in this study has no local VCF file.
   */
  public function getLocalVcfUsage($organism_index = 1) {
    $path = $this->getLocalVcf($organism_index);
    if (is_null($path)) {
      return NULL;
    }
    return substr_count(
      json_encode(SubmissionCache::get()),
      json_encode($path)
    );
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Gets name of the organism specified on Page 1.
   *
   * @param int $organism_index
   *   Ordinal number of the organism at page.
   *
   * @return mixed
   *   Returns name of the organism or NULL.
   */
  public function getOrganismName($organism_index = 1) {
    $this->getSharedState();
    return $this->sharedState['saved_values'][TPPS_PAGE_1]['organism']
      [$organism_index]['name'] ?? NULL;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Load time ('loaded') is when study was loaded to database (processed).

  /**
   * Sets time of loading to database.
   *
   * @param int $time
   *   Timestamp of the moment when study added to database. See time().
   */
  public function setLoadTime($time = NULL) {
    $this->state['loaded'] = $this->sharedState['loaded'] = $time ?? time();
  }

  /**
   * Gets time of the loading to database.
   *
   * @return int
   *   Timestamp of the moment when study added to database. See time().
   */
  public function getLoadTime() {
    return $this->state['loaded'] ?? $this->sharedState['loaded'] ?? NULL;
  }

  /**
   * Checks if study has data of loading to database.
   *
   * It means study was processed and loaded to database.
   *
   * @return bool
   *   Returns TRUE when time of loading to database was set. It means data
   *   was loaded to database.
   *   Returns FALSE otherwise.
   */
  public function isLoaded() {
    return !empty($this->getLoadTime());
  }

  /**
   * Fixes missing family, order and other data from NCBI service.
   *
   * @return bool
   *   Returns TRUE if data was updated and FALSE in case of fail.
   */
  public function fixMissingNcbiData() {
    if ($this->doesNotExist()) {
      return FALSE;
    }
    $organism_id_list = $this->state['ids']['organism_ids'] ?? NULL;
    if (empty($organism_id_list)) {
      // This means that study wasn't yet processed. Run tripal job first.
      return FALSE;
    }
    foreach ($organism_id_list as $organism_id) {
      $organism = new Organism($organism_id);
      if ($organism->isLoaded()) {
        $organism->processTaxonomyId($this);
      }
    }
    return TRUE;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Zenodo Deposition.
  //
  // Note: Special methods used to encapsulate Zenodo Data in TPPS Submission
  // and have ability to change/rename it in single place.

  /**
   * Sets Zenodo Deposit Id.
   *
   * To clear stored id: $submission->setZenodoDepositId();
   *
   * @param int $deposit_id
   *   Zenodo Deposition Id.
   * @param bool $save_on_change
   *   Default is TRUE which means TPPS Submission object will be saved to
   *   database immidietly if Deposit Id changes.
   *   Set to FALSE if you need to make other changes to TPPS Submission and
   *   save it later manually.
   */
  public function setZenodoDepositId($deposit_id = NULL, $save_on_change = TRUE) {
    if ($this->get([self::ZENODO_DEPOSIT_ID_KEY]) != $deposit_id) {
      // Deposit Id was changed.
      $this->set([self::ZENODO_DEPOSIT_ID_KEY], $deposit_id);
      if ($save_on_change) {
        $this->save();
      }
    }
  }

  /**
   * Gets Zenodo Deposit Id.
   *
   * @return mixed
   *   Returns Zenodo Deposit Id or NULL if it wasn't set.
   */
  public function getZenodoDepositId() {
    return $this->state[self::ZENODO_DEPOSIT_ID_KEY] ?? NULL;
  }

  /**
   * Checks if Submission has Zenodo Deposit Id.
   *
   * @return bool
   *   Returns TRUE if Submission has Zenodo Deposit Id and FALSE otherwise.
   */
  public function hasZenodoDepositId() {
    return !empty($this->getZenodoDepositId());
  }

  /**
   * Removes Zenodo Deposit and files.
   */
  public function removeZenodoDeposit() {
    if (empty($deposit_id = $this->getZenodoDepositId())) {
      return;
    }

    try {
      $deposit = ZenodoDeposit::retrieve([
        'deposit_id' => $deposit_id,
      ]);
      if ($deposit) {
        if (empty($deposit['submitted'])) {
          // Remove deposit.
          // Note: When draft deposition is removing then all associated files
          // will be removed automatically.
          ZenodoDeposit::delete(['deposit_id' => $deposit_id]);
        }
        else {
          $message = "Zenodo Deposit was published and can't be removed.";
          throw new Exception($message);
        }
      }
    }
    catch (Exception $e) {
      // @TODO Check Exception message. Possible responses when not found:
      // Forbidden
      // Access denied.
      $message = t("Zenodo Deposit wasn't found on remote server.");
      drupal_set_message($message);
    }

    // Unset store 'Zenodo Id' and 'Zenodo Record Id'.
    $this->set([self::ZENODO_CONCEPT_RECORD_ID_KEY], NULL);
    $this->set([self::ZENODO_DEPOSIT_ID_KEY], NULL);
    $this->save();
    $message = t("Removed 'Zenodo Deposit Id' and 'Zenodo Concept Id' from Submission.");
    drupal_set_message($message);
    // @TODO Minor. Implement hook?
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Concept Record Id.
  //

  /**
   * Sets Zenodo Concept Record Id.
   *
   * Note: Zenodo Deposit Id changes more often then Zenodo Concept Record Id.
   *
   * To clear stored id: $submission->setZenodoConceptRecordId(NULL);
   *
   * @param int $conceptrecid
   *   Zenodo Concept Record Id which is parent of the deposition versions.
   * @param bool $save_on_change
   *   Default is TRUE which means TPPS Submission object will be saved to
   *   database immidietly if Deposit Id changes.
   *   Set to FALSE if you need to make other changes to TPPS Submission and
   *   save it later manually.
   */
  public function setZenodoConceptRecordId($conceptrecid = NULL, $save_on_change = TRUE) {
    if ($this->get([self::ZENODO_CONCEPT_RECORD_ID_KEY]) != $conceptrecid) {
      // Differs.
      $this->set([self::ZENODO_CONCEPT_RECORD_ID_KEY], $conceptrecid);
      if ($save_on_change) {
        $this->save();
      }
    }
  }

  /**
   * Gets Zenodo Deposit Id.
   *
   * @return mixed
   *   Returns Zenodo Concept Record Id  or NULL if it wasn't set.
   */
  public function getZenodoConceptRecordId() {
    return $this->state[self::ZENODO_CONCEPT_RECORD_ID_KEY] ?? NULL;
  }

  /**
   * Checks if Submission has Zenodo Deposit Id.
   *
   * @return bool
   *   Returns TRUE if Submission has Zenodo Concept Record Id.
   *   Returns FALSE otherwise.
   */
  public function hasZenodoConceptRecordId() {
    return !empty($this->getZenodoConceptRecordId());
  }

  /**
   * Generates Zenodo Deposition Metadata.
   *
   * See required fields: https://developers.zenodo.org/#representation.
   *
   * @return array
   *   Returns Zenodo Deposit Metadata.
   */
  public function buildZenodoDepositMetadata() {
    $page1_values = $this->get(['saved_values', TPPS_PAGE_1]);
    $page2_values = $this->get(['saved_values', TPPS_PAGE_2]);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Authors.
    // WARNING: Using objects is required.
    $primary_author = $this->sharedState['saved_values'][TPPS_PAGE_1]['primaryAuthor'] ?? NULL;
    if (empty($primary_author)) {
      throw new Exception('Primary Author field is empty');
    }
    else {
      $creator_list[] = (object) [
        'name' => ZenodoDeposit::formatAuthorName($primary_author),
      ];
    }
    $secondary_authors = $page1_values['publication']['secondaryAuthors'];
    for ($i = 1; $i <= $secondary_authors['number']; $i++) {
      $creator_list[] = (object) [
        'name' => ZenodoDeposit::formatAuthorName($secondary_authors[$i]),
      ];
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Keywords.
    $keyword_list = [];
    for ($i = 1; $i <= $page1_values['organism']['number']; $i++) {
      $keyword_list[] = $page1_values['organism'][$i]['name'];
    }
    $keyword_list[] = TppsForm::getStudyType($page2_values['study_type']);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Metadata.
    $metadata = (object) [
      'publication_date' => date('Y-m-d'),
      'title' => $page1_values['publication']['title'],
      'upload_type' => 'dataset',
      'description' => $page1_values['publication']['abstract'],
      'creators' => $creator_list,
      'keywords' => $keyword_list,
      'notes' => $this->get(['saved_values', 'summarypage', 'comments']),
      'related_identifiers' => [
        (object) [
          'relation' => 'isAlternateIdentifier',
          'identifier' => url(
            '/tpps/details/' . $this->accession, ['absolute' => TRUE]
          ),
        ],
      ],
      'access_right' => 'open',
      'license' => 'cc-zero',
    ];
    return $metadata;
  }

  /**
   * Gets list of Ids of the files attached to submission.
   *
   * Only prevously saved file parents will be used (hardcoded).
   *
   * @return array
   *   Returns list of submission's files where key is File Id and value is the
   *   parents of the file field in Submission's Shared State.
   *   Returns an empty array when nothing found.
   *
   * @todo Minor. Cache list of file ids and reset cache of save() method call.
   * @todo Minor. Use $this->state['input'] to get just submitteddd form data.
   * Search in 'values' and 'input' require extra work. There is no $step
   * parent element and structure of the submitted data is different.
   */
  public function getFileIdList() {
    $this->getSharedState();
    $saved_values = $this->sharedState['saved_values'] ?? NULL;
    // Note: Shared State doesn't have 'values' element but State could have
    // newer version of form and other paths.
    $fid_list = [];
    if (empty($saved_values)) {
      return $fid_list;
    }
    $parent_list = $this->getFileParents();
    foreach ($parent_list as $step => $step_list) {
      foreach ($step_list as $parents) {
        $new_parents = array_merge([$step], $parents);
        $fid = (int) TppsArray::get($saved_values, $new_parents);
        if (empty($fid)) {
          continue;
        }
        $fid_list[$fid] = $new_parents;
      }
    }
    return $fid_list;
  }

  /**
   * Sets status of list of files or all files.
   *
   * Method chaining: yes.
   * If file has no usage yet it will be added.
   *
   * @param int $status
   *   File status.
   *   '1' = permanent. See constant FILE_STATUS_PERMANENT.
   *     Permanent files has protection and won't be removed in 6h by cron.
   *   '0' = temporary. No constant for this case - use integer.
   *     Will be removed by cron in 6h.
   * @param array $fid_list
   *   Array with list of files which status must be updated.
   *   When empty then all files in current submission will be updated.
   */
  public function setFileStatus($status = FILE_STATUS_PERMANENT, array $fid_list = []) {
    if (empty($fid_list)) {
      $fid_list = array_keys($this->getFileIdList());
    }
    if (!empty($fid_list)) {
      foreach ($fid_list as $fid) {
        $file = tpps_file_load($fid);
        if ($file && $file->status != $status) {
          $file->status = $status;
          file_save($file);
          if (TppsFile::isEmptyUsage($file, $this->accession)) {
            TppsFile::addUsage($file, $this->accession);
          }
        }
      }
    }
    else {
      // @todo Minor. Silently report about empty list. Use Drupal hooks.
    }
    // Allow method chaining.
    return $this;
  }

  /**
   * Gets hardcoded list of file field parents.
   *
   * WARNING: For Shared State only.
   *
   * @return array
   *   Returns empty array if number of organisms is zero.
   *   Returns list of parents
   *   (from  $form_state['values'] or/and $form_state['saved_values']).
   *
   * @todo Make it private or protected.
   * @TODO Add environmental files?
   */
  public function getFileParents() {
    $result = [];
    $result[TPPS_PAGE_3] = [];
    $result[TPPS_PAGE_4] = [];
    for ($i = 1; $i <= $this->getOrganismNumber(); $i++) {
      $organism_key = 'organism-' . $i;
      $species_id = 'species-' . $i;
      // Page 3. Accession file.
      $result[TPPS_PAGE_3] = array_merge(
        $result[TPPS_PAGE_3],
        [
          ['tree-accession', $species_id, 'file'],
        ]
      );
      // Page 4. Phenotype and Genotype files.
      $result[TPPS_PAGE_4] = array_merge(
        $result[TPPS_PAGE_4],
        [
          // Phenotype.
          [$organism_key, 'phenotype', 'iso'],
          [$organism_key, 'phenotype', 'file'],
          [$organism_key, 'phenotype', 'metadata'],
          // Genotype.
          [$organism_key, 'genotype', 'files', 'snps-assay'], // Shared State.
          [$organism_key, 'genotype', 'files', 'assay-design'],
          [$organism_key, 'genotype', 'files', 'snps-association'],
          [$organism_key, 'genotype', 'files', 'snps-pop-struct'],
          [$organism_key, 'genotype', 'files', 'snps-kinship'],
          // SSR.
          [$organism_key, 'genotype', 'files', 'ssrs'],
          [$organism_key, 'genotype', 'files', 'ssrs_extra'],
          [$organism_key, 'genotype', 'files', 'indels'],
          [$organism_key, 'genotype', 'files', 'vcf'],
          // Other.
          [$organism_key, 'genotype', 'files', 'other'],
          // Fasta.
          [$organism_key, 'genotype', 'tripal_fasta', 'file', 'file_upload'],
          // @TODO Check this field:
          // [$organism_key, 'genotype', 'tripal_fasta', 'file_upload_existing'],
        ]
      );
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Summary page.
    $result['summarypage'] = [
      // Analysis.
      ['analysis', 'diversity_file'],
      ['analysis', 'population_structure_file'],
      ['analysis', 'association_genetics_file'],
      ['analysis', 'landscape_genomics_file'],
      ['analysis', 'phenotype_environment_file'],
    ];
    // It seems we have unlimited number of extra files which could be
    // added to study but dump has 10 items.
    for ($i = 1; $i <= 10; $i++) {
      $result['summarypage'][] = ['files', $i];
    }
    return $result;
  }

  /**
   * Uploads all files to the Zenodo Deposit.
   *
   * @throws Exception
   *   When study wasn't found.
   *
   * @return object
   *   Returns current object to all method chaining.
   */
  public function actionAllFilesZenodo($action) {
    if (!in_array($action, ['create', 'delete', 'update'])) {
      throw new Exception(t("Unsupported Zenodo File's action: @action.",
        ['@action' => $action]
      ));
    }
    if (empty($deposit_id = $this->getZenodoDepositId())) {
      throw new Exception(t('Submission "@accession" has empty Zenodo Deposit Id.',
        ['@accession' => $accession]
      ));
    }
    if (empty($fid_list = array_keys($this->getFileIdList()))) {
      // Study could have no files. It's not an error.
      return $this;
    }
    // @todo Minor. Check if file already was uploaded.
    if ($action == 'create') {
      foreach ($fid_list as $fid) {
        if ($file = tpps_file_load($fid)) {
          $data = ['drupal_file' => $file, 'deposit_id' => $deposit_id];
          ZenodoFile::$action($data);
        }
      }
    }
    elseif (in_array($action, ['delete', 'update'])) {
      $data = ['deposit_id' => $deposit_id];
      if (variable_get('tpps_zenodo_host_greedy_search', FALSE)) {
        ZenodoHost::search($data);
      }
      try {
        $deposition = ZenodoDeposit::retrieve($data);
      }
      catch (Exception $e) {
        // Can't get deposition.
        throw new Exception(t('Deposition "@deposit_id" not found.',
          ['@deposit_id' => $deposit_id]
        ));
      }

      switch ($action) {
        case 'delete':
          foreach ($deposition['files'] as $file) {
            ZenodoFile::delete([
              'deposit_id' => $deposit_id,
              'file_id' => $file['id'],
            ]);
          }
          break;

        case 'update':
          foreach ($fid_list as $fid) {
            if ($file = tpps_file_load($fid)) {
              ZenodoFile::update([
                'deposit_id' => $deposit_id,
                'drupal_file' => $file,
              ]);
            }
          }
          break;
      }
    }
    return $this;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // File Usage.
  // @TODO Minor. Create new File class.


  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Publication DOI field on Form Step 1.

  /**
   * Gets value of the Publication DOI field on From Step 1.
   *
   * @return string
   *   Returns value of the field or NULL.
   *   Example: "10.5281/zenodo.199793".
   */
  public function getPublicationDOI() {
    switch ($this->getFormVersion()) {
      case 2:
        return $this->get(['saved_values', TPPS_PAGE_1, 'publication_doi']);

      case 1:
      default:
      return $this->get(['saved_values', TPPS_PAGE_1, 'doi']);
    }
    // Backup: Shared State.
    return $this->sharedState['saved_values'][TPPS_PAGE_1]['doi'] ?? '';
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Dataset DOI field on Form Step 1.

  /**
   * Gets value of the Dataset DOI field on From Step 1.
   *
   * @return string
   *   Returns value of the field or NULL.
   *   Example: "10.5281/zenodo.199793".
   */
  public function getDatasetDOI() {
    switch ($this->getFormVersion()) {
      case 2:
        $parents = ['saved_values', TPPS_PAGE_1, 'publication', 'dataset_doi'];

      case 1:
      default:
        $parents = ['saved_values', TPPS_PAGE_1, 'dataset_doi'];
    }
    return TppsArray::get($this->state, $parents);
  }

  /**
   * Sets value of the Dataset DOI field on From Step 1.
   *
   * @param string $value
   *   Dataset DOI. E.g., '10.5281/zenodo.200001'.
   * @param bool $save_on_change
   *   Default is TRUE which means TPPS Submission object will be saved to
   *   database immidietly if Deposit Id changes.
   *   Set to FALSE if you need to make other changes to TPPS Submission and
   *   save it later manually.
   *
   * @return object
   *   Returns curren object to allow chaining methods.
   */
  public function setDatasetDOI($value, $save_on_change = TRUE) {
    $was_updated = FALSE;
    switch ($this->getFormVersion()) {
      case 2:
        $parents = ['saved_values', TPPS_PAGE_1, 'publication', 'dataset_doi'];

      case 1:
      default:
        $parents = ['saved_values', TPPS_PAGE_1, 'dataset_doi'];
    }
    // State.
    $state_value = TppsArray::get($this->state, $parents);
    if ($state_value != $value) {
      $was_updated = TRUE;
      TppsArray::set($this->state, $parents, $value);
    }

    // Shared State.
    $shared_state_parents = ['saved_values', TPPS_PAGE_1, 'dataset_doi'];
    $shared_state_value = TppsArray::get(
      $this->sharedState,
      $shared_state_parents
    );
    if ($shared_state_value != $value) {
      $was_updated = TRUE;
      TppsArray::set($this->sharedState, $shared_state_parents, $value);
    }
    // Save changes.
    if ($save_on_change && $was_updated) {
      $this->save();
    }
    return $this;
  }

  /**
   * Updates field "Dataset DOI".
   *
   * Note: Shared State and State has different parents for this field.
   * Note: Used variable by reference instead of methods
   * getDatasetDOI()/setDatasetDOI() to speed-up processing.
   *
   * @param string $dataset_doi
   *   New value of the Dataset DOI field.
   */
  public function updateDatasetDOI($dataset_doi = NULL) {
    $update_result = [];

    // DOI in State.
    $state_dataset_doi = $this->getDatasetDOI();
    if (
      // New one.
      (empty($state_dataset_doi) && !is_null($dataset_doi))
      // When overwriting of existing value is allowed.
      || variable_get('tpps_zenodo_overwrite_dataset_doi')
      // Unset existing.
      || (!empty($state_dataset_doi) && is_null($dataset_doi))
    ) {
      $update_result['State'] = NULL;
      $this->setDatasetDOI($dataset_doi);
    }

    if (function_exists('module_invoke_all')) {
      $context = [
        'new' => $dataset_doi,
        'old' => $state_dataset_doi,
      ];
      // tpps_submission_dataset_doi_updated().
      module_invoke_all('submission_dataset_doi_updated', $context);
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submitter.

  /**
   * Gets id of the submitter's Drupal Account.
   *
   * @return int
   *   Return submitter user id.
   */
  public function getSubmitter() {
    if ($this->isEmpty()) {
      return NULL;
    }
    return $this->get(['submitting_uid']) ?? $this->uid ?? NULL;
  }

  /**
   * Gets submitter's email address.
   *
   * @param bool $reset
   *   Flag to reset cache.
   *
   * @return string
   *   Returns email address or empty string.
   */
  public function getSubmitterMail($reset = FALSE) {
    $static = &drupal_static(__FUNCTION__);

    $uid = $this->getSubmitter();
    if ($reset || !isset($static[$uid])) {
      $static[$uid] = (string) db_select('users', 'u')
        ->fields('u', ['mail'])
        ->condition('uid', $uid)
        ->range(0, 1)
        ->execute()
        ->fetchField();
    }
    return $static[$uid];
  }

  /**
   * Gets Chado-name of the submitter.
   *
   * Note: Drupal name could differ.
   *
   * @param bool $reset
   *   Flag to reset cache.
   *
   * @return string
   *   Returns name or empty string.
   */
  public function getSubmitterChadoName($reset = FALSE) {
    $static = &drupal_static(__FUNCTION__);

    $uid = $this->getSubmitter();
    if ($reset || !isset($static[$uid])) {
      $query = db_select('chado.contact', 'c')
        ->fields('c', ['name'])->range(0, 1);
      $query->join('chado.contactprop', 'cp', 'cp.contact_id = c.contact_id');
      $query->condition('cp.value', $this->getSubmitterMail())
        ->condition('cp.type_id', tpps_load_cvterm('email')->cvterm_id);
      $static[$uid] = (string) $query->execute()->fetchField();
    }
    return $static[$uid];
  }

  /**
   * Generates list of links to Submission related pages.
   *
   * @param array $variables
   *   Keys are: 'accession', 'destination', 'use_abr'.
   *
   * @return array
   *   Returns list of links.
   *
   * @todo Separate list generation from theming.
   * @todo Cache this list.
   * @todo Implement adding accession at client-side (in browser).
   */
  public static function getPageList(array $variables) {
    $accession = $variables['accession'] ?? NULL;
    if (empty($accession)) {
      return '';
    }
    $destination = $variables['destination'] ?? NULL;
    $use_abr = $variables['use_abr'] ?? TRUE;
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $action_list = [
      // New tpps/submission pages.
      'view' => [
        'title' => t("Submission's view page"),
        'name' => t('View'),
        'abr' => t('V'),
        'path' => 'tpps/submission/' . $accession . '/view',
      ],
      'edit' => [
        'title' => t("Open submission using TPPSc form"),
        'name' => t('Edit'),
        'abr' => t('Ed'),
        'path' => 'tppsc/' . $accession,
      ],
      'export' => [
        'title' => t("Export Submission"),
        'name' => t('Export'),
        'abr' => t('Ex'),
        'path' => 'tpps/submission/' . $accession . '/export',
      ],
      'compare' => [
        'title' => t("Compare submission with other one"),
        'name' => t('Compare'),
        'abr' => t('Com'),
        'path' => 'tpps/submission/' . $accession . '/compare',
      ],
      'purge' => [
        'title' => t("Remove submission and it's files"),
        'name' => t('Purge'),
        'abr' => t('Pu'),
        'path' => 'tpps/submission/' . $accession . '/purge',
      ],
      // Old pages:
      'admin_panel' => [
        'name' => t('Panel'),
        'title' => t('TPPS Admin Panel'),
        'abr' => t('Pa'),
        'path' => 'tpps-admin-panel/' . $accession,
      ],
      'details' => [
        'name' => t('Details'),
        'title' => t('Study Details page'),
        'abr' => t('D'),
        'path' => 'tpps/details/' . $accession,
      ],
      'completed_submission' => [
        'name' => t('Confirmation page'),
        'title' => t('Confirmation page shown when submission completely submitted.'),
        'abr' => t('Con'),
        'path' => 'completed-submission/' . $accession,
      ],
      'files' => [
        'name' => t('Files'),
        'title' => t('List of the study files'),
        'abr' => t('F'),
        'path' => 'tpps-admin-panel/file-diagnostics/' . $accession,
      ],
      // API Endpoints.
      'state' => [
        'name' => t('JSON State'),
        'title' => t("Submission's State in JSON format"),
        'abr' => t('jS'),
        'path' => 'api/submission/' . $accession . '/view/state',
      ],
      'shared_state' => [
        'name' => t('JSON Shared State'),
        'title' => t("Submission's Shared State in JSON format"),
        'abr' => t('jSS'),
        'path' => 'api/submission/' . $accession . '/view/shared_state',
      ],
      'json_vcf' => [
        'name' => t('JSON VCF'),
        'title' => t("Submission's VCF-files in JSON format"),
        'abr' => t('jVCF'),
        'path' => 'api/submission/' . $accession . '/vcf',
      ],
      'cache' => [
        'name' => t('JSON Cached Submission'),
        'title' => t("Cached submission's data in JSON format"),
        'abr' => t('jCS'),
        'path' => 'api/submission/' . $accession . '/cache',
      ],
    ];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Output.
    $params = ['html' => TRUE];
    if ($destination) {
      $params['query']['destination'] = $destination;
    }
    $output = [];
    foreach ($action_list as $name => $action) {
      $params['attributes']['title'] = $action['title'] ?? '';
      if ($use_abr) {
        $name = $action['abr'];
      }
      else {
        // $name = theme('fcbold', ['string' => $name]);
        $name = $action['name'];
      }
      $output[] = l($name, $action['path'], $params);
    }
    return $output;

  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Form version.

  /**
   * Gets Submission Form Version.
   *
   * See: SubmissionVersion::get().
   *
   * @return int
   *   Returns Submission's Form version.
   *   Returns 1 (default value) if version wasn't set.
   */
  public function getFormVersion() {
    // @TODO Should lastest version be the default?
    // @todo Could SubmissionVersion::get() be used here?
    return $this->state['saved_values'][SubmissionVersion::FORM_KEY]
      ?? SubmissionVersion::V1;
  }

  /**
   * Sets only Submission Form Version number.
   *
   * WARNING: No upgrade/downgrade.
   * See SubmissionVersion::set() to upgrade/downgrade.
   *
   * @return bool
   *   Returns TRUE if version was changed and FALSE otherwise.
   */
  public function setFormVersion($version, $save_on_change = FALSE) {
    $current_version = $this->getFormVersion();
    if ($current_version != $version) {
      $this->state['saved_values'][SubmissionVersion::FORM_KEY] = $version;
      if ($save_on_change) {
        $this->save();
      }
      // Hook 'tpps_submission_form_version_set''.
      // See also 'hook_submission_version_upgraded' and
      // 'hook_submission_version_downgraded'.
      if (function_exists('module_invoke_all')) {
        module_invoke_all('submission_form_version_set', [
          'previous_version' => $current_version,
          'new_version' => $version,
        ]);
      }
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Get list of the submission's phenotype id.
   *
   * Note: Only for processed submissions (loaded into DB).
   * Unprocessed phenotypes has no ids yet.
   *
   * @return mixed
   *   Returns list of phenotype ids.
   *   Returns NULL when submission has no phenotype data or not yet loaded
   *   into DB (not yet processed by submit_all.php script).
   */
  public function getPhenotypeIdList() {
    if (!$this->hasPhenotypeData() || empty($this->getLoadTime())) {
      // Submission has no phenotype data or not yet loaded into DB.
      return NULL;
    }
    $query = db_select('chado.dbxref', 'dxr')
      ->condition('accession', $this->accession);
    $query->join('chado.project_dbxref', 'pdxr', 'pdxr.dbxref_id = dxr.dbxref_id');
    $query->join('chado.project_stock', 'ps', 'ps.project_id = pdxr.project_id');
    $query->join('chado.stock_phenotype', 'sph', 'sph.stock_id = ps.stock_id');
    $query->fields('sph', ['phenotype_id']);
    return $query->execute()->fetchCol('phenotype_id');
  }

  /**
   * Gets list of the unique phenotype names.
   *
   * Note: getPhenotypeIdList() method returns unique ids which have the same
   * phenotype names so list returned by this method could be smaller.
   *
   * @return mixed
   *   Returns list of the unique phenotype's names.
   */
  public function getPhenotypeNameList() {
    if (!$this->hasPhenotypeData()) {
      // Submission has no phenotype data.
      return NULL;
    }

    if (empty($this->getLoadTime())) {
      // Not yet loaded into DB.
      //
    }
    else {
      $list = $this->getPhenotypeIdList();
      $field = 'name';
      return db_select('chado.phenotype', 'p')
        ->distinct()
        ->condition('phenotype_id', $list, 'IN')
        ->fields('p', [$field])
        ->execute()
        ->fetchCol($field);
    }
  }

  /**
   * Checks if at least one phenotype is missing synonym.
   *
   * @return bool
   *   Returns TRUE if at least one phenotype is missing synonym.
   *   Returns FALSE otherwise.
   */
  public function isAnyPhenotypeSynonymMissing() {
    $missing = FALSE;
    $list = $this->getPhenotypeNameList();
    $synonym_list = PhenotypeSynonym::getAll();

    $debug_mode = FALSE;
    if ($debug_mode ?? FALSE) {
      dpm(print_r($list, 1), 'Phenotype Names');
      dpm(print_r($synonym_list, 1), 'Phenotype Synonyms');
    }

    foreach ($list as $item) {
      if (!in_array($item, $synonym_list)) {
        $missing = TRUE;
        if ($debug_mode ?? FALSE) {
          dpm($item, 'Missing');
        }
      }
      else {
        if ($debug_mode ?? FALSE) {
          dpm($item, 'Found');
        }
      }
    }
    return $missing;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Study Data Type.

  /**
   * Gets raw value of the Page 2 / Data Type field.
   *
   * Value example: "Genotype x Phenotype"
   *
   * @return mixed
   *   Returns value of the 'Data Type' field on Page 2.
   *   Returns NULL if value wasn't found (probably not yet set).
   *
   * @todo Move to 'TppsForm' class.
   */
  public function getStudyDataType() {
    return TppsArray::get($this->state,
      ['saved_values', TPPS_PAGE_2, 'data_type']
    );
  }

  /**
   * Checks if submission has Phenotypic data.
   *
   * Checks value of the selectbox on page 2.
   * See also method hasPhenotypeData() which checks fields on Page 4.
   *
   * @return bool
   *   Returns TRUE when study has phenotypic data and FALSE otherwise.
   */
  public function isPhenotypeStudy() {
    return (strpos($this->getStudyDataType(), 'Phenotype') !== FALSE);
  }

  /**
   * Checks if submission has Genotypic data.
   *
   * Checks value of the selectbox on page 2.
   * See also method hasGenotypeData() which checks fields on Page 4.
   *
   * @return bool
   *   Returns TRUE when study has phenotypic data and FALSE otherwise.
   */
  public function isGenotypeStudy() {
    return (strpos($this->getStudyDataType(), 'Genotype') !== FALSE);
  }

  /**
   * Checks if submission has Environmental data.
   *
   * Checks value of the selectbox on page 2.
   * See also method hasEnvironmentalData() which checks fields on Page 4.
   *
   * @return bool
   *   Returns TRUE when study has phenotypic data and FALSE otherwise.
   */
  public function isEnvironmentStudy() {
    return (strpos($this->getStudyDataType(), 'Environment') !== FALSE);
  }

  /**
   * Gets list of organisms which has manually added phenotypes.
   *
   * @return mixed
   *   Returns list of organisms and status. Key is organism index (ordinal
   *   number at page) and value is status. Where TRUE means organism has
   *   manually added phenotype and FALSE means phenotype metadata file was
   *   uploaded.
   *   Returns NULL if study has no phenotype data.
   *
   * @todo Should this data be added to SubmissionCache?
   */
  public function getPhenotypeSynonymList() {
    if (!$this->isPhenotypeStudy()) {
      return NULL;
    }
    $list = [];
    $formState = new FormState($this->state);
    for ($i = 1; $i <= $this->getOrganismNumber(); $i++) {
      $organism_index = $i;
      $organism_key = 'organism-' . $organism_index;

      $list[$organism_index]['name'] = $formState->getOrganismName($organism_index);
      // '0' means manually added phenotype.
      // '1' - metadata file was uploaded.
      $is_metadata_in_file = $formState->get(
        ['saved_values', TPPS_PAGE_4, $organism_key, 'phenotype', 'check']
      );
      if ($is_metadata_in_file) {
        // Metadata file.
        // See PhenotypeMeta::validate().
        $attribute_list = PhenotypeMeta::getColumnValues(
          $organism_index,
          $this->state,
          PhenotypeMeta::DATA_TYPE_ATTRIBUTE
        );
        $phenotype_list = PhenotypeMeta::getColumnValues(
          $organism_index,
          $this->state,
          PhenotypeMeta::DATA_TYPE_IDENTIFIER
        );
        $synonym_list = PhenotypeSynonym::getAll();
        foreach ($attribute_list as $key => $value) {
          // Note: $item is an attribute name.
          $synonym_id = (int) array_search($value, $synonym_list);
          $list[$organism_index][$key + 1]['synonym_id'] = $synonym_id;
          $list[$organism_index][$key + 1]['name'] = $phenotype_list[$key];
        }
      }
      else {
        // Manually added phenotypes.
        $phenotype_number = $formState->get(
          [
            'saved_values', TPPS_PAGE_4, $organism_key, 'phenotype',
            'phenotypes-meta', 'number'
          ]
        );
        for ($j = 1; $j <= $phenotype_number; $j++) {
          // Note; value '0' means no synonym was found.
          $synonym_id = $formState->get(
            [
              'saved_values', TPPS_PAGE_4, $organism_key, 'phenotype',
              'phenotypes-meta', $j, 'synonym_id',
            ]
          );
          $name = $formState::get(
            [
              'saved_values', TPPS_PAGE_4, $organism_key, 'phenotype',
              'phenotypes-meta', $j, 'name',
            ]
          );
          $list[$organism_index][$j]['name'] = $name;
          $list[$organism_index][$j]['synonym_id'] = $synonym_id;
        }
      }
    }
    return $list;
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission Tags.

  /**
   * Checks if submission has tag.
   *
   * @param mixed $tag_key
   *   Tag id or name.
   *
   * @return bool
   *   Returns TRUE if submission has given tag and FALSE otherwise.
   */
  public function hasTag($tag_key) {
    return SubmissionTag::isLinked($tag_key, $this->accession);
  }

  /**
   * Checks if submission has any tags.
   *
   * @return bool
   *   Returns TRUE if submission has any tag and FALSE otherwise.
   */
  public function hasTags() {
    return !empty($this->getTagIdList());
  }

  /**
   * Adds the tag to a the submission.
   *
   * @param mixed $tag_key
   *   The name or id of the tag to be added.
   */
  public function addTag($tag_key) {
    SubmissionTag::link($tag_key, $this->accession);
  }

  /**
   * Removes the tag from the submission.
   *
   * @param mixed $tag_key
   *   The name or id of the tag to be removed.
   */
  public function removeTag($tag_key) {
    SubmissionTag::unlink($tag_key, $this->accession);
  }

  /**
   * Removes all tags associated with the submission.
   *
   * We can't use SubmissionTag::unlink() because it leaves 'system' tags
   * (with Ids: 1 and 2), does extra check and call's hooks.
   */
  public function removeAllTags() {
    // Remove all tags linked to submission.
    db_delete(SubmissionTag::TABLE_SUBMISSION_TAG)
      ->condition('tpps_submission_id', $this->id)
      ->execute();

    // Update SubmissionCache.
    $cached = SubmissionCache::get();
    $cached[$this->accession]['tags'] = [];
    SubmissionCache::set($cached);
  }

  /**
   * Gets list of tag ids.
   *
   * To get list of tag objects use 'getTagList()'.
   * Execution time:
   * first call (empty static cache) takes 3 msec, then 0.005 msec.
   *
   * @return array
   *   Returns list of tag ids.
   */
  public function getTagIdList() {
    return SubmissionCache::get($this->accession)['tags'];
  }

  /**
   * Returns array of tags associated with a submission.
   *
   * To get only tag ids see 'getTagIdLIst()' which is faster.
   * Execution time:
   * first call: 5 msec, then 0.6 msec.
   *
   * @return array
   *   Returns array of tag objects associated with the submission.
   *   Returns empty array if submission has no tags.
   */
  public function getTagList() {
    $query = db_select('tpps_submission_tag', 'st')->distinct();
    $query->join('tpps_submission', 's', 's.tpps_submission_id = st.tpps_submission_id');
    $query->join('tpps_tag', 't', 't.tpps_tag_id = st.tpps_tag_id');
    $query->fields('t');
    $query->condition('s.accession', $this->accession);
    return $query->execute()->fetchAllAssoc('tpps_tag_id', PDO::FETCH_ASSOC);
  }

  /**
   * Removes project's data in the database.
   */
  public function removeProjectData() {
    $project_id = $this->sharedState['ids']['project_id'] ?? NULL;
    if (empty($project_id)) {
      return FALSE;
    }
    $verbose = (bool) function_exists('tpps_log');
    $table_list = $this->getTableListToRemoveProjectData();
    $message = 'Cleared "@table_name" for project @project_id';
    foreach ($table_list as $table_name) {
      $num_deleted = db_delete($table_name)
        ->condition('project_id', $project_id)
        ->execute();
      if ($verbose) {
        $params = ['@table_name' => $table_name, '@project_id' => $project_id];
        if (!is_null($num_deleted)) {
          $params['@num_deleted'] = $num_deleted;
          $message .= '. Deleted: @num_deleted rows';
        }
        tpps_log($message, $params);
      }
    }
  }

  /**
   * Gets list of the db tables to remove project's data.
   *
   * @TODO Minor. Merge with getTableListToClearDataBase().
   */
  public function getTableListToRemoveProjectData() {
    return [
      'chado.project_organism',
      'chado.project_pub',
      'chado.projectprop',
      'chado.genotype_call',
      'public.tpps_project_file_managed',
    ];
  }

}
