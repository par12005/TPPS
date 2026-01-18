<?php

/**
 * @file
 * Contains 'FormEditSubmissionTag' class.
 *
 * Related classes:
 *   'SubmissionTag',
 *   'FormCreateSubmissionTag',
 *   'FormEditSubmissionTag',
 *   'FormRemoveSubmissionTag'.
 *
 * WARNING: form builder must be added in tpps_forms().
 */

/**
 * Manages TPPS Submission Tags Edit Form.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class FormEditSubmissionTag {

  /**
   * Builds form. Form callback.
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
  public static function build(array $form, array &$form_state, $tag_id = NULL) {
    if (empty($tag_id)) {
      drupal_goto(SubmissionTag::BASE_URL);
    }

    $tag = db_select(SubmissionTag::TABLE, 't')
      ->fields('t')
      ->condition('tpps_tag_id', $tag_id)
      ->range(0, 1)
      ->execute()->fetchObject();

    if (empty($tag) or $tag->static) {
      drupal_goto(SubmissionTag::BASE_URL);
    }

    $form = FormCreateSubmissionTag::build($form, $form_state);

    $form['name']['#default_value'] = $tag->name;
    $form['color']['#default_value'] = $tag->color;
    $form['tag_id'] = ['#type' => 'hidden', '#value' => $tag_id];
    $form['#validate'][] = 'FormEditSubmissionTag::validate';
    $form['#submit'][] = 'FormEditSubmissionTag::submit';
    return $form;
  }

  /**
   * Implements hook_form_validate().
   */
  public static function validate(array &$form, array &$form_state) {
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
  public static function submit($form, &$form_state) {
    $tag_id = FormState::get($form_state, ['values', 'tag_id']);
    $tag_name = FormState::get($form_state, ['values', 'name']);
    $tag_color = FormState::get($form_state, ['values', 'color']);
    db_update(SubmissionTag::TABLE)
      ->fields([
        'name' => $tag_name,
        'color' => $tag_color,
      ])
      ->condition('tpps_tag_id', $tag_id)
      ->execute();
    $message = t('Tag #@tag_id was updated.', ['@tag_id' => $tag_id]);
    drupal_set_message($message);
    drupal_goto(SubmissionTag::BASE_URL);
  }

}
