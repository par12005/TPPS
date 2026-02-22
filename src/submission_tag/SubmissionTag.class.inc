<?php

/**
 * @file
 * Contains 'SubmissionTag' class.
 *
 * Theming:
 *   To show all tags:
 *   theme('submission_tag_list', ['tags' => $submission->getTagList()]);
 *
 * URL to manage tags:
 *   /tpps/tag - list
 *   /tpps/tag/create
 *   /tpps/tag/<id> - view (not implemented)
 *   /tpps/tag/<id>/edit
 *   /tpps/tag/<id>/remove
 *   /tpps/tag/<id>/list - show list of the submissions which has this tag.
 *
 *
 * Usage
 * To get tag name:
 *   SubmissionTag::load($tag_id)->name;
 *
 * Related classes:
 *   'SubmissionTag',
 *   'FormCreateSubmissionTag',
 *   'FormEditSubmissionTag',
 *   'FormRemoveSubmissionTag'.
 */

// @todo Review:
// tpps_show_tags, $items['tpps/tag/%/%/%']
// function tpps_details_tags_sort().
// Merge with SubmissionTag::getAll().
// search: && !array_key_exists($tag_id, SubmissionTag::getAll($shared_state['accession'])
// Search ‘tag’:
//   pages/admin/study.inc - a lot about tags.
//   pages/details/top.inc
// Search $submission->getTagList();
// use tag id and SubmissionTag::load() which now uses caching./

/**
 * Manages TPPS Submission Tags.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class SubmissionTag {

  // Name of the DB table where tags are stored.
  const TABLE = 'tpps_tag';

  // Name of the DB table where link between tag and submission is stored.
  const TABLE_SUBMISSION_TAG = 'tpps_submission_tag';

  // URL of the main page.
  const BASE_URL = 'tpps/tag';

  /**
   * Returns the id of a tag based on the name.
   *
   * Uses static caching.
   *
   * @param mixed $tag_key
   *   The name or id of the tag.
   * @param bool $reset
   *   TRUE to reset static cache. Default is FALSE.
   *
   * @return int
   *   The id of the tag.
   */
  public static function getId($tag_key, $reset = FALSE) {
    if (is_numeric($tag_key)) {
      return $tag_key;
    }
    $static_cache = &drupal_static(__METHOD__);
    $key = md5(json_encode(array_slice(func_get_args(), 0, -1)));
    if ($reset || !isset($static_cache[$key])) {
      $value = db_select(static::TABLE, 't')
        ->fields('t', ['tpps_tag_id'])
        ->condition('name', $key)
        ->range(0, 1)
        ->execute()
        ->fetchField();
      $static_cache[$key] = $value;
    }
    return $static_cache[$key];
  }

  /**
   * Loads single submisison tag or all tags.
   *
   * Uses static caching.
   * First call: 1.3 msec,
   * Next calls: 0.02 msec.
   *
   * @param int $tag_id
   *   Tag Id.
   * @param bool $reset
   *   Flag to reset cache.
   *
   * @return mixed
   *   Returns object with single submission tag when $id is not empty.
   *   Returns array of arrays with all the submission tags when $id is empty.
   */
  public static function load($tag_id = NULL, $reset = FALSE) {
    $list = &drupal_static(__METHOD__);
    if ($reset || empty($list)) {
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      $list = db_select(static::TABLE, 't')
        ->fields('t')
        ->orderBy('static', 'DESC')
        ->orderBy('tpps_tag_id')
        ->execute()
        ->fetchAllAssoc('tpps_tag_id');
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    }
    if (empty($tag_id)) {
      return $list;
    }
    else {
      return $list[$tag_id] ?? NULL;
    }
  }

  /**
   * Gets tag name.
   *
   * Note: ::load() uses static caching.
   *
   * @param int $tag_id
   *   Tag Id.
   *
   * @return mixed
   *   Returns name of the tag.
   *   Returns NULL when $tag_id is empty or tag wan't found.
   */
  public static function getName($tag_id) {
    if (!empty($tag_id)) {
      if (!empty($tag = static::load($tag_id))) {
        return $tag->name;
      }
    }
    return NULL;
  }

  /**
   * Removes default tags associated with a submission.
   *
   * @param string $accession
   *   The accession of the desired submission.
   */
  public static function clear($accession) {
    $tags = array(
      'Genotype',
      'Phenotype',
      'Environment',
    );
    foreach ($tags as $tag) {
      tpps_submission_load($accession)->removeTag($tag);
    }
  }

  /**
   * Menu callback. Shows all submission tags as a table.
   *
   * Shouldn't be converted into theme-function because used as a menu-callback.
   *
   * @return string
   *   String of renderable HTML.
   */
  public static function showTable() {
    $rows = [];
    $result = db_select('tpps_tag', 't')->fields('t')->execute()->fetchAll();
    foreach ($result as $tag) {
      $color = !empty($tag->color) ? $tag->color : 'white';
      $url = static::BASE_URL . '/' . check_plain($tag->tpps_tag_id);
      $actions = [l(t('Info'), $url . '/info')];
      if (empty($tag->static)) {
        $actions[] = l(t('Edit'), $url . '/edit');
        $actions[] = l(t('Remove'), $url . '/remove');
      }
      $rows[$tag->tpps_tag_id] = [
        $tag->tpps_tag_id,
        '<span class="tag" style="background-color:' . $color . '">'
        . '<span title="Tag Id: ' . $tag->tpps_tag_id . '" class="tag-text">'
        . $tag->name . '</span></span>',
        $tag->name,
        $color,
        implode(' | ', $actions),
      ];
    }
    ksort($rows);
    $output = theme('table', [
      'header' => [t('Id'), t('Tag'), t('Name'), t('Color'), t('Actions')],
      'rows' => $rows,
      'attributes' => ['class' => ['view'], 'id' => 'tpps_table_display'],
      'caption' => '',
      'colgroups' => NULL,
      'sticky' => FALSE,
      'empty' => '',
    ]);
    tpps_add_css_js('main');
    return $output;
  }

  /**
   * Removes tag.
   *
   * See also static::unlink().
   *
   * @param mixed $tag_key
   *   Tag id or name.
   */
  public static function remove($tag_key) {
    if (empty($tag_id = SubmissionTag::getId($tag_key))) {
      return;
    }
    // Update SubmissionCache.
    $cached_submission_list = SubmissionCache::get();
    foreach ($cached_submission_list as $accession => $submission) {
      if (
        !empty($submission['tags'])
        && !empty($key = array_search($tag_id, $submission['tags']))
      ) {
        unset($cached_submission_list[$accession]['tags'][$key]);
      }
    }
    SubmissionCache::set($cached_submission_list);
    // Remove link between submission and tag.
    db_delete(static::TABLE_SUBMISSION_TAG)
      ->condition('tpps_tag_id', $tag_id)
      ->execute();
    // Remove tag.
    db_delete(static::TABLE)
      ->condition('tpps_tag_id', $tag_id)
      ->execute();
  }

  /**
   * Gets list of submission's ids which has given tag.
   *
   * @param mixed $tag_key
   *   The name or id of the tag.
   *
   * @return array
   *   Returns list of submission ids (not accessions!).
   */
  public static function getSubmissionIdList($tag_key) {
    return db_select('tpps_submission_tag', 'st')
      ->fields('st', ['tpps_submission_id'])
      ->condition('st.tpps_tag_id', SubmissionTag::getId($tag_key))
      ->orderBy('st.tpps_submission_id')
      ->execute()
      ->fetchCol('tpps_submission_id');
  }

  /**
   * Gets list of submission's accession which has given tag.
   *
   * @param mixed $tag_key
   *   The name or id of the tag.
   *
   * @return array
   *   Returns list of submission accessions (e.g., 'TGDR1234');
   */
  public static function getSubmissionAccessionList($tag_key) {
    $query = db_select('tpps_submission', 's')->fields('s', ['accession']);
    $query->join('tpps_submission_tag', 'st', 's.tpps_submission_id = st.tpps_submission_id');
    $query->condition('st.tpps_tag_id', SubmissionTag::getId($tag_key));
    return $query->execute()->fetchCol('accession');
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Link to the Submission.

  /**
   * Creates link between tag and submission.
   *
   * @param mixed $tag_key
   *   Tag id or name.
   * @param mixed $accession
   *   Submission accession. E.g., 'TGDR1234'.
   */
  public static function link($tag_key, $accession) {
    if (
      empty($tag_key) || empty($accession)
      || empty($tag_id = SubmissionTag::getId($tag_key))
      || SubmissionTag::isLinked($tag_id, $accession)
      || empty($cached_submission = SubmissionCache::get($accession))
      || empty($submission_id = $cached_submission['id'])
    ) {
      return;
    }
    // Remove actual link.
    db_insert(static::TABLE_SUBMISSION_TAG)
      ->fields([
        'tpps_submission_id' => $submission_id,
        'tpps_tag_id' => $tag_id,
      ])
      ->execute();

    // Update SubmissionCache.
    $cached_submission['tags'][] = (int) $tag_id;
    SubmissionCache::setSingle($cached_submission);

    // 'tpps_submission_tag_linked';
    if (function_exists('module_invoke_all')) {
      module_invoke_all('submission_tag_linked', $accession, $tag_id);
    }
  }

  /**
   * Removes link between tag and submission.
   *
   * @param mixed $tag_key
   *   Tag id or name.
   * @param mixed $accession
   *   Submission accession. E.g., 'TGDR1234'.
   */
  public static function unlink($tag_key, $accession) {
    if (
      empty($tag_key) || empty($accession)
      || empty($tag_id = SubmissionTag::getId($tag_key))
      || !SubmissionTag::isLinked($tag_id, $accession)
      || empty($cached_submission = SubmissionCache::get($accession))
      || empty($submission_id = $cached_submission['id'])
      || empty($cached_submission['tags'])
    ) {
      return;
    }

    if ($tag_id == 1 || $tag_id == 2) {
      // 1st and 2nd tags are 'Tpps' and 'Tppsc' which can't be unlinked.
      // Note: This is not related to 'static' property of the tag.
      $result = 'protected';
    }
    else {
      // Remove actual link.
      db_delete(static::TABLE_SUBMISSION_TAG)
        ->condition('tpps_submission_id', $submission_id)
        ->condition('tpps_tag_id', $tag_id)
        ->execute();

      // Update SubmissionCache.
      $key = array_search($tag_id, $cached_submission['tags']);
      unset($cached_submission['tags'][$key]);
      SubmissionCache::setSingle($cached_submission);
      $result = 'success';
    }

    // 'tpps_submission_tag_unlinked';
    if (function_exists('module_invoke_all')) {
      module_invoke_all('submission_tag_unlinked', $accession, $tag_id, $result);
    }
  }

  /**
   * Checks if tag and submission are linked.
   *
   * @param mixed $tag_key
   *   Tag id or name.
   * @param mixed $accession
   *   Submission accession. E.g., 'TGDR1234'.
   *
   * @return bool
   *   Returns TRUE if submission has given tag and FALSE otherwise.
   */
  public static function isLinked($tag_key, $accession) {
    return (bool) (
      !empty($tag_id = SubmissionTag::getId($tag_key))
      && !empty($tag_list = SubmissionCache::getTags($accession))
      && in_array($tag_id, $tag_list)
    );
  }

  /**
   * Menu callback. Shows tag info page.
   *
   * URL: 'tpps/tag/%/info'.
   *
   * @param mixed $tag_key
   *   Tag id or name.
   *
   * @return string
   *   Returns rendered page content which shows list of submission accessions
   *   linked to given tag.
   *
   * @todo Minor. ?Move to separate class
   */
  public static function buildInfoPage($tag_key) {
    if (empty($tag_id = SubmissionTag::getId($tag_key))) {
      $message = t('Wrong tag key: @tag_key', ['@tag_key' => $tag_key]);
      drupal_set_message($message);
      drupal_goto('tpps/tag');
    }
    foreach (static::getSubmissionAccessionList($tag_id) as $accession) {
      $output[] = theme('submission_accession_link', [
        'accession' => $accession,
        'path' => 'tpps-admin-panel/' . $accession,
      ]);
    }
    if (empty($output)) {
      return t('No studies found.');
    }
    else {
      return theme('item_list', ['items' => $output]);
    }
  }

}
