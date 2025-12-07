<?php

/**
 * @file
 * Contains 'SubmissionTag' class.
 *
 * Theming:
 *   To show all tags: theme('submission_tag_list', ['tags' => $tag_id_list]);
 *
 * URL to manage tags:
 *   /tpps/tag - list
 *   /tpps/tag/create
 *   /tpps/tag/<id> - view (not implemented)
 *   /tpps/tag/<id>/edit
 *   /tpps/tag/<id>/remove
 *   /tpps/tag/<id>/list - show list of the submissions which has this tag.
 */

/**
 * Manages TPPS Submission Tags.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class SubmissionTag {

  // Name of the DB table where tags are stored.
  const TABLE = 'tpps_tag';

  // URL of the main page.
  const BASE_URL = 'tpps/tag';

  /**
   * Form to create a new submission tag.
   *
   * Form callback.
   *
   * @param array $form
   *   The form object to be populated.
   * @param array $form_state
   *   The state of the form object to be populated.
   *
   * @return array
   *   The populated form object.
   */
  public static function buildForm(array $form, array &$form_state) {
    SubmissionTag::buildDefaultForm($form, $form_state);
    $form['#submit'][] = 'SubmissionTag::submitForm';
    $form['#validate'][] = 'SubmissionTag::validateForm';
    return $form;
  }

  /**
   * Default form to edit/create submission tag.
   *
   * @param array $form
   *   The form object to be populated.
   * @param array $form_state
   *   The state of the form object to be populated.
   *
   * @return array
   *   The populated form object.
   */
  public static function buildDefaultForm(array &$form, array &$form_state) {
    // This will add the color picker to the TPPS submission tags edit-color
    // field id.
    // @todo Minor. Use local copy of this js-file.
    drupal_add_js(
      'https://cdnjs.cloudflare.com/ajax/libs/tinyColorPicker/1.1.1/jqColorPicker.min.js',
      'external'
    );
    // @TODO [VS] This file must be stored under /sites/all/libraries to allow
    // other modules to reuse it and get updates independently from this module.
    drupal_add_js(TPPS_MODULE_PATH . '/js/tag_colorpicker.js');
    $form['name'] = [
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#description' => t('The name of the new tag'),
      '#required' => TRUE,
      '#prefix' => l(t('Back to TPPS Submission Tags page'), static::BASE_URL),
    ];
    $form['color'] = [
      '#type' => 'textfield',
      '#title' => t('Color'),
      '#description' => t('The color of the new tag. Can be the name of a '
        . 'color, like "red" or "blue", or can be a hexidecimal color like '
        . '"#FFFFFF" or "#A2C32F". Hexidecimal colors must start with the "#" '
        . 'character.'),
      '#required' => TRUE,
      '#suffix' => '<div id="color-picker"><a>Toggle Color Picker</a></div>',
    ];
    $form['submit'] = ['#type' => 'submit', '#value' => t('Submit')];
  }

  /**
   * Implements hook_form_validate().
   */
  public static function validateForm(array &$form, array &$form_state) {
    $result = db_select(static::TABLE, 't')
      ->fields('t')
      ->condition('name', $form_state['values']['name'], 'ILIKE')
      ->range(0, 1)
      ->execute()->fetchObject();
    if (!empty($result)) {
      $message = t("A tag already exists with this name. Please select a "
        . "different name or use the existing tag.");
      form_set_error('name', $message);
    }
  }

  /**
   * Implements hook_form_submit().
   */
  public static function submitForm(array $form, array &$form_state) {
    db_insert(static::TABLE)->fields([
      'name' => $form_state['values']['name'],
      'color' => $form_state['values']['color'],
    ])->execute();
    drupal_goto(static::BASE_URL);
  }

  /**
   * Returns the id of a tag based on the name.
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
      dpm('reset');
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
   *   Returns array of objects with all the submission tags when $id is empty.
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
   * Edit submission tags.
   *
   * @param string $type
   *   Either 'add' or 'remove'.
   * @param string $accession
   *   The accession of the desired submission.
   * @param mixed $tag
   *   The name or id of the tag to be added or removed.
   */
  public static function update($type, $accession, $tag) {
    $result = db_select(static::TABLE, 't')
      ->fields('t')
      ->condition('tpps_tag_id', $tag)
      ->range(0, 1)
      ->execute()->fetchObject();
    if ($result->static) {
      drupal_goto(static::BASE_URL);
    }
    $submission = new Submission($accession);
    switch ($type) {
      case 'add':
        $submission->addTag($tag);
        break;

      case 'remove':
        $submission->removeTag($tag);
        break;
    }
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
   * Form to edit a submission tag.
   *
   * @param array $form
   *   The form object to be populated.
   * @param array $form_state
   *   The state of the form object to be populated.
   * @param mixed $tag_id
   *   The id of the tag being edited.
   *
   * @return array
   *   The populated form object.
   */
  public static function buildEditForm(array $form, array &$form_state, $tag_id = NULL) {
    if (empty($tag_id)) {
      drupal_goto(static::BASE_URL);
    }

    $tag = db_select(static::TABLE, 't')
      ->fields('t')
      ->condition('tpps_tag_id', $tag_id)
      ->range(0, 1)
      ->execute()->fetchObject();

    if (empty($tag) or $tag->static) {
      drupal_goto(static::BASE_URL);
    }

    SubmissionTag::buildDefaultForm($form, $form_state);
    $form['name']['#default_value'] = $tag->name;
    $form['color']['#default_value'] = $tag->color;
    $form['tag_id'] = ['#type' => 'hidden', '#value' => $tag_id];
    $form['#submit'][] = 'SubmissionTag::submitEditForm';
    $form['#validate'][] = 'SubmissionTag::validateEditForm';
    return $form;
  }

  /**
   * Implements hook_form_validate().
   */
  public static function validateEditForm(array &$form, array &$form_state) {
    $tag_id = FormState::get($form_state, ['values', 'tag_id']);
    $tag_name = FormState::get($form_state, ['values', 'name']);
    $result = db_select('tpps_tag', 't')
      ->fields('t', ['name'])
      ->condition('tpps_tag_id', $tag_id, '!=')
      ->condition('name', $tag_name, 'ILIKE')
      ->range(0, 1)
      ->execute()->fetchField();

    if (!empty($result)) {
      $message = t("A tag already exists with this name. Please select a "
        . "different name or use the existing tag.");
      form_set_error('name', $message);
    }
  }

  /**
   * Implements hook_form_submit().
   */
  public static function submitEditForm($form, &$form_state) {
    $tag_id = FormState::get($form_state, ['values', 'tag_id']);
    $tag_name = FormState::get($form_state, ['values', 'name']);
    $tag_color = FormState::get($form_state, ['values', 'color']);
    db_update(static::TABLE)
      ->fields([
        'name' => $tag_name,
        'color' => $tag_color,
      ])
      ->condition('tpps_tag_id', $tag_id)
      ->execute();
    $message = t('Tag #@tag_id was updated.', ['@tag_id' => $tag_id]);
    drupal_set_message($message);
    drupal_goto(static::BASE_URL);
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
      $actions = [];
      if (empty($tag->static)) {
        $actions = [
          l(t('Edit'), $url . '/edit'),
          l(t('Remove'), $url . '/remove'),
        ];
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
   * Menu-callback. Shows form to remove tag.
   */
  public static function buildRemoveForm(array $form, array &$form_state, $tag_id = NULL) {
    $cancel_redirect = static::BASE_URL . '/' . $tag_id;
    $tag = static::load($tag_id);
    if (empty($tag)) {
      drupal_goto($cancel_redirect);
    }
    $message = t('Are you sure, you want to remove submission tag #@tag_id '
      . '"<strong>@tag_name</strong>"?',
      [
        '@tag_name' => $tag->name,
        '@tag_id' => $tag->tpps_tag_id,
      ]
    );
    $cancel_redirect = static::BASE_URL . '/' . $tag_id;
    $description = $message . '<br />' . t('The action cannot be undone.');
    $delete_button = t('Delete');
    $cancel_button = t('Cancel');

    $form = confirm_form(
      $form,
      $message,
      $cancel_redirect,
      $description,
      $delete_button,
      $cancel_button
    );
    $form['tag_id'] = ['#type' => 'hidden', '#value' => $tag_id];
    $form['#submit'][] = 'SubmissionTag::submitRemoveForm';
    return $form;
  }

  /**
   * Form submitter. Removes Submission tag.
   *
   * @param array $form
   *   Drupal Form Array.
   * @param array $form_state
   *   Drupal Form State Array.
   */
  public static function submitRemoveForm(array $form, array &$form_state) {
    if (FormState::get($form_state, ['values', 'confirm'])) {
      $tag_id = FormState::get($form_state, ['values', 'tag_id']);
      if ($tag_id) {
        SubmissionTag::remove($tag_id);
        $message = t('Tag #@tag_id was removed.', ['@tag_id' => $tag_id]);
        drupal_set_message($message);
      }
    }
    drupal_goto(static::BASE_URL);
  }

  /**
   * Removes tag.
   *
   * @param int $tag_id
   *   Submission Tag Id.
   */
  public static function remove($tag_id) {
    db_delete(Submission::TABLE_SUBMISSION_TAG)
      ->condition('tpps_tag_id', $tag_id)
      ->execute();
    db_delete(static::TABLE)->condition('tpps_tag_id', $tag_id)->execute();
  }

  /**
   * Gets list of submissions which has given tag.
   *
   * @param mixed $tag_key
   *   The name or id of the tag.
   *
   * @return array
   *   Returns list of submission ids (not accessions!).
   */
  public static function getSubmissionList($tag_key) {
    return db_select('tpps_submission_tag', 'st')
      ->fields('st', ['tpps_submission_id'])
      ->condition('st.tpps_tag_id', SubmissionTag::getId($tag_key))
      ->orderBy('st.tpps_submission_id')
      ->execute()
      ->fetchCol('tpps_submission_id');
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
      && !empty($tag_list = SubmissionCache::get($accession)['tags'])
      && in_array($tag_id, $tag_list)
    );
  }

}
