<?php

/**
 * @file
 * Contains 'FormRemoveSubmissionTag' class.
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
 * Manages TPPS Submission Tag Remove Form.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class FormRemoveSubmissionTag {

  /**
   * Builds form. Form callback.
   */
  public static function build(array $form, array &$form_state, $tag_id = NULL) {
    $cancel_redirect = SubmissionTag::BASE_URL . '/' . $tag_id;
    $tag = SubmissionTag::load($tag_id);
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
    $cancel_redirect = SubmissionTag::BASE_URL . '/' . $tag_id;
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
    $form['#attributes']['class'][] = 'tpps';
    $form['tag_id'] = ['#type' => 'hidden', '#value' => $tag_id];
    $form['#submit'][] = 'FormRemoveSubmissionTag::submit';
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
  public static function submit(array $form, array &$form_state) {
    $formState = new FormState($form_state);
    if ($formState->values(['confirm'])) {
      $tag_id = $formState->values(['tag_id']);
      if ($tag_id) {
        SubmissionTag::remove($tag_id);
        $message = t('Tag #@tag_id was removed.', ['@tag_id' => $tag_id]);
        drupal_set_message($message);
      }
    }
    drupal_goto(SubmissionTag::BASE_URL);
  }

}
