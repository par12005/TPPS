<?php

/**
 * @file
 * Contains 'SubmissionTag' class.
 *
 * Theming:
 *   To show all tags: theme('submission_tag_list', ['tags' => $tag_list]);
 *
 * DB tables:
 * - Table where tags are stored: static::TABLE.
 *
 * @TODO Store list of tag Ids in SubmsisionCache.
 *
 * URL to manage tags:
 *   /tpps/tag - list
 *   /tpps/tag/create
 *   /tpps/tag/<id> - view
 *   /tpps/tag/<id>/edit
 *   /tpps/tag/<id>/remove
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
    $form['#submit'][] = 'SubmissionTag::submitForm';
    $form['#validate'][] = 'SubmissionTag::validateForm';
    return $form;
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
   * @param string $name
   *   The name of the desired tag.
   *
   * @return int
   *   The id of the tag.
   */
  public static function getId($name) {

    if (gettype($tag) == 'string') {
      $id = SubmissionTag::getId($tag);
      if (!empty($id)) {
        $tag = $id;
      }
    }



    $result = db_select(static::TABLE, 't')
      ->fields('t', array('tpps_tag_id'))
      ->condition('name', $name)
      ->range(0, 1)
      ->execute()->fetchObject();

    if ($result) {
      return $result->tpps_tag_id;
    }
    return FALSE;
  }

  /**
   * Loads submisison tag.
   *
   * @param int $id
   *   Tag Id.
   *
   * @return object
   *   Returns Submission's metadata.
   */
  public static function load($id) {
    return db_select(static::TABLE, 't')
      ->fields('t')
      ->condition('tpps_tag_id', $id)
      ->range(0, 1)
      ->execute()
      ->fetchObject();
  }

  /**
   * Returns array of tags associated with a submission.
   *
   * @param string $accession
   *   The accession of the desired submission.
   *
   * @return array
   *   Array of tags associated with the submission.
   */
  public static function getAll($accession) {
    $query = db_select('tpps_submission_tag', 'st');
    $query->join('tpps_submission', 's', 's.tpps_submission_id = st.tpps_submission_id');
    $query->join('tpps_tag', 't', 't.tpps_tag_id = st.tpps_tag_id');
    $query->fields('t');
    $query->condition('s.accession', $accession);
    return $query->execute()->fetchAllAssoc('tpps_tag_id', PDO::FETCH_ASSOC);
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
    switch ($type) {
      case 'add':
        SubmissionTag::add($accession, $tag);
        break;

      case 'remove':
        SubmissionTag::remove($accession, $tag);
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
      SubmissionTag::remove($accession, $tag);
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

    $form = SubmissionTag::buildForm($form, $form_state);

    $form['name']['#default_value'] = $tag->name;
    $form['color']['#default_value'] = $tag->color;

    $form['id'] = ['#type' => 'hidden', '#value' => $tag_id];
    $form['#submit'][] = 'SubmissionTag::submitEditForm';
    $form['#validate'][] = 'SubmissionTag::validateEditForm';
    return $form;
  }

  /**
   * Implements hook_form_validate().
   */
  public static function validateEditForm(array &$form, array &$form_state) {
    $result = db_select('tpps_tag', 't')
      ->fields('t')
      ->condition('name', $form_state['values']['name'], 'ILIKE')
      ->condition('tpps_tag_id', $form_state['values']['id'], '!=')
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
  public static function submitEditForm($form, &$form_state) {
    db_update('tpps_tag')
      ->fields(array(
        'name' => $form_state['values']['name'],
        'color' => $form_state['values']['color'],
      ))
      ->condition('tpps_tag_id', $form_state['values']['id'])
      ->execute();
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
        '<span class="tag" style="background-color:' . $color . '">'
          . '<span class="tag-text">' . $tag->name . '</span></span>',
        $tag->name,
        $color,
        implode(' | ', $actions),
      ];
    }
    ksort($rows);
    $output = theme('table', [
      'header' => [t('Tag'), t('Name'), t('Color'), t('Actions')],
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
    dpm($tag_id);
    //$form = confirm_form(
    //  $form,
    //  t('Are you sure, you want to remove @tag_name?', ['@tag_name' => $tag_id]),
    //  'activity-list',
    //  'The action cannot be undone.',
    //  'Delete',
    //  'Cancel',
    //);
    return $form;
  }

  //function my_module_delete_activity_form_submit($form, &$form_state) {
  //  // Required actions here
  //}

}
