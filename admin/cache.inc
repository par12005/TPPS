<?php

/**
 * @file
 * Manage TPPS Cache.
 */

define('TPPS_CLEAR_CACHE_BUTTON_NAME', 'tpps_clear_cache');
define('TPPS_SUBMISSION_CLEAR_CACHE_BUTTON_NAME', 'tpps_submission_clear_cache');

/**
 * TPPS Cache Settings Form.
 */
function tpps_cache_settings_form(array $form, array &$form_state) {

  // Button to clear cache.
  $form['note'] = [
    '#markup' => t('Module TPPS provides 2 cache bins and each could be '
    . 'cleared independently.'
    . theme('item_list', [
      'items' => [
        "'TPPS' Cache Bin is for common things and full rebuild usually "
          . "doesn't take much time.",
        "'TPPS Submission' Cache Bin holds most required data for all "
        . "submissions and allows single submission update. "
        . "<br />Full rebuild could take more then 60 seconds. "
        . "<strong>Use with caution!</strong>"
      ]
    ])),
  ];
  $form[TPPS_CLEAR_CACHE_BUTTON_NAME] = [
    '#type' => 'submit',
    '#name' => TPPS_CLEAR_CACHE_BUTTON_NAME,
    '#value' => t('Clear TPPS Cache'),
  ];
  $form[TPPS_SUBMISSION_CLEAR_CACHE_BUTTON_NAME] = [
    '#type' => 'submit',
    '#name' => TPPS_SUBMISSION_CLEAR_CACHE_BUTTON_NAME,
    '#value' => t('Clear TPPS Submission Cache'),
  ];
  return $form;
}

/**
 * TPPS Cache Settings Form.
 */
function tpps_cache_settings_form_submit(array $form, array &$form_state) {
  // Process 'Clear TPPS Cache' button.
  $button_name = $form_state['triggering_element']['#name'];
  if ($button_name == TPPS_CLEAR_CACHE_BUTTON_NAME) {
    tpps_cache_clear_all();
    $bin = TPPS_CACHE_BIN;
    drupal_set_message(t('Cache bin "@bin" was cleared.', ['@bin' => $bin]));
  }
  if ($button_name == TPPS_SUBMISSION_CLEAR_CACHE_BUTTON_NAME) {
    SubmissionCache::clear();
    $bin = SubmissionCache::CACHE_BIN;
    drupal_set_message(t('Cache bin "@bin" was cleared.', ['@bin' => $bin]));
  }
}
