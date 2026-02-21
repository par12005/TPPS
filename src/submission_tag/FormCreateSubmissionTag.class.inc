<?php

/**
 * @file
 * Contains 'FormCreateSubmissionTag' class.
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
 * Manages TPPS Submission Tags Create Form.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class FormCreateSubmissionTag {

  /**
   * Builds form. Form callback.
   *
   * @param array $form
   *   The form object to be populated.
   * @param array $form_state
   *   The state of the form object to be populated.
   *
   * @return array
   *   The populated form object.
   */
  public static function build(array $form, array &$form_state) {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Common.
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
      '#prefix' => l(t('Back to TPPS Submission Tags page'), SubmissionTag::BASE_URL),
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
    $form['#attributes']['class'][] = 'tpps';

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Custom.
    $form['#validate'][] = 'FormCreateSubmissionTag::validate';
    $form['#submit'][] = 'FormCreateSubmissionTag::submit';
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
  public static function submit(array $form, array &$form_state) {
    db_insert(SubmissionTag::TABLE)->fields([
      'name' => $form_state['values']['name'],
      'color' => $form_state['values']['color'],
    ])->execute();
    drupal_goto(SubmissionTag::BASE_URL);
  }

}
