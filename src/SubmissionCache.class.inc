<?php

/**
 * @file
 * Contains 'SubmissionCache' class.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Overview.
 * List of Submissions is not a list of fully loaded 'Submission' objects but
 * cached list of mostly used Submission's data such as status, title and etc.
 * But to get those data a fully loaded Submission object is required which
 * takes a lot of time so we cache them using this class.
 *
 * To get list of fully loaded Submission objects use:
 *   $submission_list = tpps_load_submission_multiple();
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Caching.
 * Cached data is stored in TPPS_CACHE_BIN which uses own table 'cache_tpps'.
 * Custom cache id is used to separate from other cached data and allow other
 * code to reuse this list.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Update cached data.
 *   Submission->save() updates cached data for thise single submission.
 *   To reset all the cached list use:
 *   - SubmissionCache::update() (takes about 40-50 seconds)
 *   - cache_clear_all() - fast because just clears cache.
 *   - tpps_cache_clear_all(SubmissionCache::CACHE_ID) - very fast.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Data structure.
 * Data structure defined in SubmissionCache::buildSingle() and new items could be
 * added there. Each time Submission saved cached data for this single
 * submission will be udpate.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Usage.
 *
 * Get all submissions:
 *   $data = SubmissionCache::getAll($reset);
 */

/**
 * Manages List of TPPS Submissions.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class SubmissionCache {

  // Cache Id.
  // Used to store most used submision's data for reports.
  // See TPPS_CACHE_BIN constant.
  const CACHE_ID = 'tpps_submission_list';

  // Cache bin name.
  // If custom cache bin wasn't yet created for some reasons we use common one.
  const CACHE_BIN = 'cache_tpps_submission' ?? 'cache';

  // The name of the key used to reset cache in GET-request.
  // VALUE will be CACHE_BIN.
  const RESET_KEY = 'reset';

  // Name of the container of the files data in single item.
  const FILES_CONTAINER = 'file';
  const FILE_GROUPED_LIST = 'grouped_list';

  /**
   * Updates cached list of Submissions.
   */
  public static function update() {
    static::get($accession = NULL, $reset = TRUE);
    // 'tpps_submission_cache_updated';
    if (function_exists('module_invoke_all')) {
      module_invoke_all('submission_cache_updated');
    }
  }

  /**
   * Clears both static and db caches for submission list.
   */
  public static function clear() {
    drupal_static_reset(static::CACHE_ID);
    cache_clear_all(static::CACHE_ID, static::CACHE_BIN);
  }

  /**
   * Gets list of the all cached submissions.
   *
   * Note: statically cached.
   *
   * Performance (550 studies):
   * - Full cache recreation: 39334 msec (40 seconds)
   * - DB cache: 40 msec
   * - Static cache: 0.005 msec
   *
   * @param bool $reset
   *   Flag is cache must be resetted. Default if FALSE.
   *
   * @return mixed
   *   Returns list of the cached submission's data.
   */
  public static function getAll($reset = FALSE) {
    return static::get(NULL, $reset);
  }

  /**
   * Gets single submission or list of the all submissions.
   *
   * Note: statically cached.
   *
   * Performance (550 studies):
   * - Full cache recreation: 39334 msec (40 seconds)
   * - DB cache: 40 msec
   * - Static cache: 0.005 msec
   *
   * @param string $accession
   *   Submission's accession. E.g., 'TGDRxxxx'.
   *   When not set (NULL) then list of the all submissions will be returned.
   * @param bool $reset
   *   Flag is cache must be resetted. Default if FALSE.
   *
   * @return mixed
   *   Returns list of the all cached submissions when $accession wasn't set.
   *   Returns single cached submission when $accession was set and cached
   *   version exists.
   *   Return NULL if there is no cached submission for given $accession.
   */
  public static function get($accession = NULL, $reset = FALSE) {
    // Static caching.
    // Using static cache allows to avoid extra db query to get stored in db
    // cache and allows to update single items faster.
    $list = &drupal_static(static::CACHE_ID);
    if ($reset || (empty($list) && !is_array($list))) {
      $cache = cache_get(static::CACHE_ID, static::CACHE_BIN);
      if ($reset || empty($cache) || (empty($cache->data) && !is_array($cache->data))) {
        // Cache will be empty when module just installed and no submission yet.
        $list = [];
        // Note: Do not load cache from db but use statically cached list to
        // speed-up processing.
        $submission_list = tpps_load_submission_multiple();
        // WARNIG: ksort do not respect keys and they will be lost but asort()
        // doesn't work with objects in array values...
        foreach ($submission_list as $accession => $submission) {
          $accession = $submission->accession;
          if (!empty($accession)) {
            $list[$accession] = static::buildSingle($submission);
          }
        }
        // List of studies which must must be resubmitted.
        // Those studies are present in DB but doesn't have a record in
        // 'tpps_submission' table.
        foreach (static::getResubmitList() as $accession) {
          if (!empty($accession)) {
            $list[$accession] = [];
          }
        }
        krsort($list, SORT_NATURAL);
      }
      else {
        $list = $cache->data;
      }
      static::set($list);
    }
    // Output.
    if (empty($accession)) {
      return $list;
    }
    else {
      if (isset($list[$accession])) {
        return $list[$accession];
      }
      else {
        return NULL;
      }
    }
  }

  /**
   * Gets list of old TGDR Submissions to be resubmitted.
   *
   * Example:
   * [
   *   [0] => TGDR001
   *   [1] => TGDR019
   *   [2] => TGDR203
   * ]
   *
   * @param bool $reset
   *   Flag to reset cache.
   *
   * @return array
   *   Returns list of study accessions.
   */
  public static function getResubmitList($reset = FALSE) {
    $list = &drupal_static(__METHOD__);
    if ($reset || (empty($list) && !is_array($list))) {
      $list = [];

      $subquery = db_select('tpps_submission', 's');
      $subquery->fields('s', ['accession']);

      $query = db_select('chado.dbxref', 'dbx');
      $query->join('chado.project_dbxref', 'pd', 'pd.dbxref_id = dbx.dbxref_id');
      $query->condition('dbx.accession', $subquery, 'NOT IN');
      $query->condition('dbx.accession', 'TGDR%', 'ILIKE');
      $query->fields('dbx', ['accession']);
      $query->orderBy('dbx.accession');
      $list = $query->execute()->fetchCol('accession');
      if (is_null($list) || !is_array($list)) {
        $list = [];
      }
    }
    return $list;
  }

  /**
   * Sets list of submissions.
   */
  public static function set(array $new_list) {
    $list = &drupal_static(static::CACHE_ID);
    $list = $new_list;
    krsort($list, SORT_NATURAL);
    cache_set(static::CACHE_ID, $new_list, static::CACHE_BIN);
  }

  /**
   * Builds single submission data for cached.
   *
   * Those data could be used in reports to avoid full load of submission.
   *
   * @param object $submission
   *   TPPS Submission.
   *
   * @return array
   *   Returns data which must be stored in cache.
   */
  public static function buildSingle($submission) {
    $data = [];
    if ($submission->isEmpty()) {
      return $data;
    }
    if (
      $submission->isIncompleteStatus()
      && ($submission->state['stage'] ?? NULL) == TPPS_PAGE_2
      && empty($submission->state['saved_values'][TPPS_PAGE_1])
      // @TODO Check if publication title is empty (not set).
    ) {
      // Note: There is no need to use Submission::purge() because it's probably
      // not submitted fully study left on Step 1.
      $submission->delete();
      // Study is treated as orphan (or empty) and will be removed when:
      // 1. It has no publication title.
      // 2. Page 1 wasn't submitted or saved yet.
      // 3. Removement is allowed (default is allow removement).
      // Note: Empty study will be created when user goes to Page 1 from
      // 'Welcome' page. Empty studies often created during developement
      // but users could also create empty studies. Not sure if accession
      // number of those studies could be reused but it's better to avoid
      // useless records in DB.
      watchdog('tpps', 'Removed empty submission @accession.',
        ['@accession' => $submission->accession]
      );
      return $data;
    }
    $page1_values = $submission->get(['saved_values', TPPS_PAGE_1]);
    $data = [
      //'accession' => $submission->accession,
      'status' => $submission->getStatus(),
      'stage' => $submission->getCurrentStage(),
      'project_id' => $submission->get(['ids', 'project_id']),
      SubmissionVersion::FORM_KEY => $submission->getFormVersion(),
      'publication' => [
        'status' => TppsArray::getValue($page1_values,
          ['publication', 'status']
        ),
        'title' => TppsArray::getValue($page1_values,
          ['publication', 'title']
        ),
        'publication_doi' => $submission->getPublicationDOI(),
        'dataset_doi' => $submission->getDatasetDOI(),
      ],
      'submitter' => [
        'uid' => $submission->getSubmitter(),
        'mail' => $submission->getSubmitterMail(),
        'name' => $submission->getSubmitterChadoName(),
      ],
      'date' => [
        'completed' => $submission->state['completed'] ?? NULL,
        'loaded' => $submission->getLoadTime(),
        'approved' => $submission->state['approved'] ?? NULL,
        'release' => $submission->state['saved_values']['summarypage']['release-date'] ?? NULL,
        'updated' => $submission->state['updated'] ?? NULL,
        'cached' => REQUEST_TIME,
      ],
      'tags' => tpps_submission_get_tags($submission->accession),
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Files.
      static::FILES_CONTAINER => [
         static::FILE_GROUPED_LIST => [
          Report::TREE_ACCESSION_FILES => [],
          Report::PHENOTYPE_FILES => [],
          Report::GENOTYPE_FILES => [],
          // Files are missing in file system.
          // Files which doesn't belong to any group.
          Report::OTHER_FILES => [],
        ],
        Report::MISSING_FILES => [],
        Report::FILE_ID_LIST => [],
      ],
    ];

    // List of 'Assay Design' files.
    $reset_assay_design_list = FALSE;
    for ($n = 1; $n <= $submission->getOrganismNumber(); $n++) {
      $organism_key = 'organism-' . $n;
      // @TODO Use AssayDesign::getValue() if possible.
      $assay_design_fid = TppsArray::getValue(
        $submission->sharedState,
        [
          'saved_values',
          TPPS_PAGE_4,
          $organism_key,
          'genotype',
          // Not static::FILES_CONTAINER but fieldset name.
          'files',
          'assay-design',
        ]
      );
      if (empty($assay_design_fid)) {
        continue;
      }
      if ($assay_design_fid != ($data[static::FILES_CONTAINER]['assay-design'][$n] ?? NULL)) {
        $data[static::FILES_CONTAINER]['assay-design'][$n] = $assay_design_fid;
        $reset_assay_design_list = TRUE;
      }
    }
    // Rebuild list of studies which has Assay Desigin Files.
    if ($reset_assay_design_list) {
      AssayDesign::clearSubmissionList();
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Zenodo.
    if ($deposit_id = $submission->getZenodoDepositId()) {
      $data['zenodo'] = [
        'deposit_id' => $deposit_id,
        'conceptrecid' => $submission->getZenodoConceptRecordId(),
      ];
      // Get Zenodo host.
      try {
        ZenodoHost::search(['deposit_id' => $data['zenodo']['deposit_id']]);
        $host = ZenodoHost::isLive() ? 'live' : 'sandbox';
      }
      catch (Exception $e) {
        $host = NULL;
      }
      $data['zenodo']['host'] = $host;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Files.
    $file_list = $submission->getFileIdList();
    $data['file'][Report::FILE_ID_LIST] = array_keys($file_list);

    if (!empty($file_list)) {
      // List of files per group.
      $file_row = [];
      // @todo Minor. Check 'vcf' and 'local vcf' in $file->uri.
      // Process local VCF files.
      for ($n = 1; $n <= $submission->getOrganismNumber(); $n++) {
        if ($submission->hasLocalVcf($n)) {
          $organism_name = $submission->getOrganismName($n);
          $organism_label = check_plain($n . '. ' . $organism_name);
          $file_row['genotype'][$organism_label]['local_vcf'] = $submission->getLocalVcf($n);
        }
      }

      foreach ($file_list as $fid => $parents) {
        $step = $parents[0];
        // 3: 'tree-accession';
        // 4: 'phenotype', 'genotype', '...'
        switch ($step) {
          case TPPS_PAGE_3:
            $group = $parents[1];
            $organism_index = str_replace('species-', '', $parents[2]);
            $organism_name = $submission->getOrganismName($organism_index);
            $organism_label = check_plain($organism_index . '. ' . $organism_name);
            break;

          case TPPS_PAGE_4:
            $group = $parents[2];
            $organism_index = str_replace('organism-', '', $parents[1]);
            $organism_name = $submission->getOrganismName($organism_index);
            $organism_label = check_plain($organism_index . '. ' . $organism_name);
            break;

          case 'frontpage':
          case 'summarypage':
            $group = Report::OTHER_FILES;
            $organism_index = NULL;
            $organism_label = $step;
            break;

          default:
            $group = Report::OTHER_FILES;
            $organism_index = NULL;
            $organism_label = 'Other';
            break;
        }

        // Missing files goes to the 'missing' group and will have NO hyperlink
        // to 'File Info' API page.
        if (empty($file = tpps_file_load($fid))) {
          $data['file'][Report::MISSING_FILES][] = $fid;
        }
        $field_name = check_plain(end($parents));
        $file_row[$group][$organism_label][$field_name] = $fid;
      }
      $data['file'][static::FILE_GROUPED_LIST] = $file_row;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Imported Study Metadata.
    $data['import_meta'] = $submission->getImportMeta();
    $tripal_job_id = $submission->sharedState['job_id'] ?? NULL;
    $data['tripal_job']['id'] = $tripal_job_id;
    // Get Tripal Job status.
    if (!empty($tripal_job_id)) {
      try {
        $job = new TripalJob();
        $job->load($tripal_job_id);
        $data['tripal_job']['status'] = $job->getJob()->status;
      }
      catch (Exception $e) {
        if ($e->getMessage() != "Cannot find a job with this ID provided.") {
          throw $e;
        }
      }
    }
    return $data;
  }

  /**
   * Updates single submission in the list of cached submissions.
   *
   * WARNING: Do not use in the loop. See buildSingle() method.
   *
   * @param object $submission
   *   TPPS Submission.
   */
  public static function updateSingle($submission) {
    // Note: Do not use Submission::doesExist() because it will try to load
    // submission from DB and will fail if it wasn't yet saved.
    // WARNING: Do not use PHP's empty() to check if protected property
    // 'accession' was set and not empty.
    if ($submission->isEmpty()) {
      return;
    }
    $list = static::get();
    $list[$submission->accession] = static::buildSingle($submission);
    static::set($list);
    // 'tpps_submission_cache_single_updated';
    if (function_exists('module_invoke_all')) {
      module_invoke_all('submission_cache_single_updated', $submission->accession);
    }
  }

  /**
   * Remove single item from the cached list.
   *
   * @param string $accession
   *   TPPS Submission accession. E.g., 'TGDRxxxx'.
   *
   * @return bool
   *   Returns TRUE if cached version was actually removed and FALSE if it
   *   wasn't found in cache.
   */
  public static function remove($accession) {
    $list = static::get();
    if (isset($list[$accession])) {
      unset($list[$accession]);
      static::set($list);
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Prints message about cached report and link to reset cache.
   *
   * Note: When message is shown using 'drupal_set_message()' it doesn't matter
   * where this function called but when theme_item_list() should be used then
   * it's important to get returned result and show in correct place at page.
   */
  public static function showClearCacheWidget() {
    if (($_GET[static::RESET_KEY] ?? FALSE) == static::CACHE_BIN) {
      $message = t('Cleared "@cid" cache bin.', ['@cid' => static::CACHE_BIN]);
      drupal_get_messages();
      drupal_set_message($message);
      static::update();
      drupal_goto(current_path());
    }
    $query = [static::RESET_KEY => static::CACHE_BIN];
    $update_url = url($_GET['q'], ['query' => $query]);
    $message = t('Report was cached. <a href="@update_url">Update</a> to get '
      . 'most recent changes.', ['@update_url' => $update_url]
    );
    if ($debug_mode ?? FALSE) {
      drupal_set_message($message, 'warning');
    }
    return theme('item_list', ['items' => [$message]]);
  }

  /**
   * Builds a list of accessions for form.
   *
   * Uses cache:
   *   No cache/Reset: 25-28 msec.
   *   Cache not empty: 2-3 msec.
   * Former: tpps_submission_get_accession_list().
   *
   * @param array $condition_list
   *   List of condtions for Drupal DB API contions.
   *   For example: [['uid'], $user->uid, '=']
   *     First element is an array of parents against item in
   *     'public.cache_tpps_submission' DB table.
   *     Second is value. Empty value is allowed.
   *     3rd is optional operator. Default is '='.
   *     Possible values: '!=', '<>', '='.
   *   Possible fields: 'uid', 'accession', 'status' and etc.
   *   See SubmissionCache::buildSingle().
   * @param bool $reset
   *   Flag is cache must be resetted. Default if FALSE.
   *
   * @return array
   *   Returns list of accession to be used in form.
   *   Key is an accession and value is a human readable title.
   */
  public static function getAccessionList(array $condition_list = [], $reset = FALSE) {
    $default_value = [1];
    $cid = __METHOD__;
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    $cache = cache_get($cid, $cache_bin);

    $key = md5(json_encode($condition_list));
    if ($reset || empty($cache) || empty($cache->data[$key])) {
      $list = $default_value;
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      $data = static::get();
      foreach ($data as $accession => $cached_submission) {
        $title = $cached_submission['publication']['title'] ?? t('No Title');
        $title = (strlen($title) > 96) ? substr($title, 0, 97) . '...' : $title;
        $item = $accession . ' - ' . $title;
        if (empty($condition_list)) {
          $list[$accession] = $item;
        }
        else {
          $match = [];
          foreach ($condition_list as $condition) {
            // @TODO Minor. To use list of parents switch $key and $value in
            // $conditions.
            $parents = $condition[0] ?? NULL;
            $value = $condition[1] ?? NULL;
            if (empty($parents)) {
              continue;
            }
            $operation = $condition[2] ?? '=';
            $existing_value = TppsArray::getValue($cached_submission, $parents);
            if ($operation == '=') {
              $match[] = ($existing_value == $value);
            }
            elseif ($operation == '!=' || $operation == '<>') {
              $match[] = ($existing_value != $value);
            }
            else {
              // dpm('Not supported operation.');
            }
          }
          // Add to list when ALL conditions matches.
          if (
            count($match) == count($condition_list)
            && count(array_filter($match)) == count($match)
          ) {
            $list[$accession] = $item;
          }
        }
      }
      krsort($list, SORT_NATURAL);
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      $cache->data[$key] = $list;
      cache_set($cid, $cache->data, $cache_bin);
    }
    return $cache->data[$key] ?? $default_value;
  }

}
