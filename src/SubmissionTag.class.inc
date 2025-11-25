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
 * - Connection between tag and submission: static::SUBMISSION_TABLE.
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

  // Name of the DB table where link between tag and submission is stored.
  const SUBMISSION_TABLE = 'tpps_submission_tag';

  // URL of the main page.
  const MAIN_PAGE = 'tpps-tag';

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
      '#prefix' => l(t('Back to TPPS Submission Tags page'), static::MAIN_PAGE),
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
    drupal_goto(static::MAIN_PAGE);
  }

  /**
   * Removes a tag from a submission.
   *
   * @param string $accession
   *   The accession of the desired submission.
   * @param mixed $tag
   *   The name or id of the tag to be removed.
   */
  public static function remove($accession, $tag) {
    if (gettype($tag) == 'string') {
      $id = SubmissionTag::getId($tag);
      if (!empty($id)) {
        $tag = $id;
      }
    }
    $submission = new Submission($accession);
    db_delete(static::SUBMISSION_TABLE)
      ->condition('tpps_submission_id', $submission->id)
      ->condition('tpps_tag_id', $tag)
      ->execute();
  }

  /**
   * Adds a tag to a submission.
   *
   * @param string $accession
   *   The accession of the desired submission.
   * @param mixed $tag
   *   The name or id of the tag to be added.
   */
  public static function add($accession, $tag) {
    if (gettype($tag) == 'string') {
      $id = SubmissionTag::getId($tag);
      if (!empty($id)) {
        $tag = $id;
      }
    }
    $tags = SubmissionTag::getAll($accession) ?? [];
    if (!array_key_exists($tag, $tags)) {
      $submission = new Submission($accession);
      if ($submission->id) {
        db_insert(static::SUBMISSION_TABLE)
          ->fields([
            'tpps_submission_id' => $submission->id,
            'tpps_tag_id' => $tag,
          ])
          ->execute();
      }
    }
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
    $result = db_select('tpps_tag', 't')
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
      drupal_goto(static::MAIN_PAGE);
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
      drupal_goto(static::MAIN_PAGE);
    }

    $tag = db_select(static::TABLE, 't')
      ->fields('t')
      ->condition('tpps_tag_id', $tag_id)
      ->range(0, 1)
      ->execute()->fetchObject();

    if (empty($tag) or $tag->static) {
      drupal_goto(static::MAIN_PAGE);
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
    drupal_goto(static::MAIN_PAGE);
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
      $edit_link = NULL;
      if ($tag->static) {
        $url = static::MAIN_PAGE . '/edit/' . check_plain($tag->tpps_tag_id);
        $edit_link = l(t('edit'), $url);
      }
      $rows[$tag->tpps_tag_id] = [
        '<span class="tag" style="background-color:' . $color . '">'
          . '<span class="tag-text">' . $tag->name . '</span></span>',
        $tag->name,
        $color,
        $edit_link ?? '',
      ];
    }
    ksort($rows);
    $output = theme('table', [
      'header' => [t('Tag'), t('Name'), t('Color'), ''],
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

}
