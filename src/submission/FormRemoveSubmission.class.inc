<?php

/**
 * @file
 * Contains 'FormRemoveSubmission' class.
 *
 * WARNING: form builder must be added in tpps_forms().
 */

/**
 * Manages TPPS Submission Removement Form.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class FormRemoveSubmission {

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
    return $form;
  }

  /**
   * Implements hook_form_validate().
   */
  public static function validate(array &$form, array &$form_state) {
      //form_set_error('name', $message);
  }

  /**
   * Implements hook_form_submit().
   */
  public static function submit($form, &$form_state) {
  }

}
