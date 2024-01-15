<?php

/**
 * @file
 * Validates 'Frontpage' Form.
 */

/**
 * Validates 'Frontpage' form.
 *
 * When 'frontpage' form submitted we already have accession number but
 * study is empty and we remove it from the list of studies at frontpage
 * so better to add new study when Page 1 saved or submitted rather than
 * when 'Frontpage' form passed validation. See tpps_main_submi().
 *
 * @param array $form
 *   The form that is being validated.
 * @param array $form_state
 *   The state of the form that is being validated.
 */
function tpps_front_page_validate_form(array &$form, array &$form_state) {
  if (
    !$form_state['submitted']
    || !tpps_access('administer tpps module')
    || empty($values['custom_accession_check'])
  ) {
    return;
  }
  $values = $form_state['values'] ?? NULL;
  // Check if accession number has more then 3 digits.
  if (!preg_match('/^TGDR\d{3,}$/', $values['custom_accession'])) {
    form_set_error('custom_accession',
      t('The accession number @accession is invalid. '
        . 'Must have more then 3 digits.'
        ['@accession' => $values['custom_accession']]
      )
    );
  }
  else {
    // Accession number has more then 3 digits.
    $result = chado_select_record('dbxref',
      ['accession'],
      ['accession' => $values['custom_accession']],
      ['limit' => 1]
    );
    if (!empty($result)) {
      form_set_error('custom_accession',
        t('The accession number @accession is already in use.',
          ['@accession' => $values['custom_accession']]
        )
      );
    }
  }
}
