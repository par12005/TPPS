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
    // Note: existing callbacks must be overwritten.
    $form['#validate'] = ['FormEditSubmissionTag::validate'];
    $form['#submit'] = ['FormEditSubmissionTag::submit'];

    return $form;
  }

  /**
   * Implements hook_form_validate().
   */
  public static function validate(array &$form, array &$form_state) {
    $formState = new FormState($form_state);
    // Check id.
    $tag_id = $formState->get(['values', 'tag_id']);
    if (empty($tag_id)) {
      $message = t("Tag id is empty.");
      form_set_error('name', $message);
    }
    // Check name.
    $tag_name = $formState->get(['values', 'name']);
    if (empty($tag_name)) {
      $message = t("Tag name is empty.");
      form_set_error('name', $message);
    }
    // Check if color field is not empty.
    if (empty($formState->get(['values', 'color']))) {
      $message = t("Tag color is empty.");
      form_set_error('color', $message);
    }
    // Check if tag exists.
    $result = db_select('tpps_tag', 't')
      ->fields('t', ['name'])
      ->condition('tpps_tag_id', $tag_id)
      ->condition('name', $tag_name, 'ILIKE')
      ->range(0, 1)
      ->execute()->fetchField();
    if (empty($result)) {
      $message = t("A tag doesn't exist. Please create tag before edit it.");
      form_set_error('name', $message);
    }
  }

  /**
   * Implements hook_form_submit().
   */
  public static function submit($form, &$form_state) {
    $formState = new FormState($form_state);
    $tag_id = $formState->get(['values', 'tag_id']);
    $tag_name = $formState->get(['values', 'name']);
    $tag_color = $formState->get(['values', 'color']);
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
