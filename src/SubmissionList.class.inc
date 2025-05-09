<?php

/**
 * @file
 * Contains 'SubmissionList' class.
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
 *   - SubmissionList::update() (takes about 40-50 seconds)
 *   - cache_clear_all() - fast because just clears cache.
 *   - tpps_cache_clear_all(SubmissionList::CACHE_ID) - very fast.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Data structure.
 * Data structure defined in SubmissionList::buildSingle() and new items could be
 * added there. Each time Submission saved cached data for this single
 * submission will be udpate.
 *
 * ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
 * Usage.
 *  $data = SubmissionList::get($reset);
 */

/**
 * Manages List of TPPS Submissions.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class SubmissionList {

  // Cache Id.
  // Used to store most used submision's data for reports.
  // See TPPS_CACHE_BIN constant.
  const CACHE_ID = 'tpps_submission_list';

  // Cache bin name.
  const CACHE_BIN = TPPS_CACHE_BIN ?? 'cache';

  /**
   * Updates cached list of Submissions.
   */
  public static function update() {
    static::get($reset = TRUE);
  }

  /**
   * Clears both static and db caches for submission list.
   */
  public static function clear() {
    drupal_static_reset(static::CACHE_ID);
    cache_clear_all(static::CACHE_ID, static::CACHE_BIN);
  }

  /**
   * Gets list of submissions.
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
   *   Returns cached list of Submissions.
   *   Returns NULL if there is no submissions and nothing to cache.
   */
  public static function get($reset = FALSE) {
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
        krsort($submission_list, SORT_NATURAL);
        foreach ($submission_list as $accession => $submission) {
          $accession = $submission->accession;
          $list[$accession] = SubmissionList::buildSingle($submission);
        }
      }
      else {
        $list = $cache->data;
      }
      static::set($list);
    }
    return $list;
  }

  /**
   * Sets list of submissions.
   */
  public static function set(array $new_list) {
    $list = &drupal_static(static::CACHE_ID);
    $list = $new_list;
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
    if ($submission->doesNotExist()) {
      return $data;
    }
    $page1_values = $submission->get(['saved_values', TPPS_PAGE_1]);
    $data = [
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
      // @TODO Move tags to 'Submission' class and store only row data.
      'tags' => tpps_show_tags(tpps_submission_get_tags($submission->accession)),
      'status' => $submission->getStatus(),
      'stage' => $submission->getCurrentStage(),
      'publication' => [
        'status' => tpps_array_get_value($page1_values,
          ['publication', 'status']
        ),
        'title' => tpps_array_get_value($page1_values,
          ['publication', 'title']
        ),
      ],
      'file' => [
        'grouped_list' => [
          'tree-accession' => [],
          'phenotype' => [],
          'genotype' => [],
          // Files are missing in file system.
          // Files which doesn't belong to any group.
          'other' => [],
        ],
        'missing' => [],
        'id_list' => [],
      ],
      //'accession' => $submission->accession,
    ];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Files.
    $file_list = $submission->getFileIdList();
    $data['file']['id_list'] = array_keys($file_list);

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
            break;

          case TPPS_PAGE_4:
            $group = $parents[2];
            $organism_index = str_replace('organism-', '', $parents[1]);
            break;

          default:
            $group = 'other';
            $organism_index = NULL;
            break;
        }
        $organism_name = $submission->getOrganismName($organism_index);

        // Missing files goes to the 'other' group and will have no hyperlink
        // to 'File Info' API page.
        if (empty($file = file_load($fid))) {
          $data['file']['missing'][] = $fid;
        }
        $organism_label = check_plain($organism_index . '. ' . $organism_name);
        $field_name = check_plain(end($parents));
        $file_row[$group][$organism_label][$field_name] = $fid;
      }
      $data['file']['grouped_list'] = $file_row;
    }
    return $data;
  }

  /**
   * Updates single submission in the list of cached submissions.
   *
   * WARNING: Do not use in the loop. See buildSingle() method.
   */
  public static function updateSingle($submission) {
    if ($submission->doesNotExist()) {
      return;
    }
    $list = static::get();
    $list[$submission->accession] = static::buildSingle($submission);
    static::set($list);
  }

}
