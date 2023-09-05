<?php

/**
 * @file
 * Defines the data integrity checks for the second page of the form.
 */

/**
 * Defines the data integrity checks for the second page of the form.
 *
 * @param array $form
 *   The form that is being validated.
 * @param array $form_state
 *   The state of the form that is being validated.
 */
function tpps_page_2_validate_form(array &$form, array &$form_state) {
  $is_tppsc = (($form_state['build_info']['form_id'] ?? 'tpps_main') == 'tppsc_main');

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Curation form.
  if ($is_tppsc) {
    if ($form_state['submitted'] == '1') {
      if (!$form_state['values']['data_type']) {
        form_set_error('data_type', 'Data Type: field is required.');
      }
      if (!$form_state['values']['study_type']) {
        form_set_error('study_type', 'Study Type: field is required.');
      }
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Regular form.
  else {
    if ($form_state['submitted'] == '1') {
      if (!$form_state['values']['StartingDate']['year']) {
        form_set_error('StartingDate][year', t('Year: field is required.'));
      }
      elseif (!$form_state['values']['StartingDate']['month']) {
        form_set_error('StartingDate][month', t('Month: field is required.'));
      }

      if (!$form_state['values']['EndingDate']['year']) {
        form_set_error('EndingDate][year', t('Year: field is required.'));
      }
      elseif (!$form_state['values']['EndingDate']['month']) {
        form_set_error('EndingDate][month', t('Month: field is required.'));
      }

      if (!$form_state['values']['data_type']) {
        form_set_error('data_type', t('Data Type: field is required.'));
      }

      if (!$form_state['values']['study_type']) {
        form_set_error('study_type', t('Study Type: field is required.'));
      }

      if (!empty($form_state['values']['study_info']['season'])) {
        foreach ($form_state['values']['study_info']['season'] as $key => $val) {
          if (!$val) {
            unset($form_state['values']['study_info']['season'][$key]);
          }
        }
        if (
          array_key_exists('season', $form_state['values']['study_info'])
          and empty($form_state['values']['study_info']['season'])
        ) {
          form_set_error('study_info][season', t('Seasons: field is required.'));
        }
      }

      if (
        isset($form_state['values']['study_info']['assessions'])
        and !$form_state['values']['study_info']['assessions']
      ) {
        form_set_error('study_info][assessions',
          t('Number of times the populations were assessed: field is required.')
        );
      }

      if (!empty($form_state['values']['study_info']['temp'])) {
        if (empty($form_state['values']['study_info']['temp']['high'])) {
          form_set_error('study_info][temp][high', t('Average High Temperature: field is required.'));
        }
        if (empty($form_state['values']['study_info']['temp']['low'])) {
          form_set_error('study_info][temp][low', t('Average Low Temperature: field is required.'));
        }
      }

      $types = array(
        'co2' => 'CO2',
        'humidity' => 'Air Humidity',
        'light' => 'Light Intensity',
        'salinity' => 'Salinity',
      );

      foreach ($types as $type => $label) {
        if (!empty($form_state['values']['study_info'][$type])) {
          $set = $form_state['values']['study_info'][$type];
          if (!$set['option']) {
            form_set_error("study_info][$type][option",
              "$label controlled or uncontrolled: field is required.");
          }
          elseif (
            $set['option'] == '1'
            and !$set['controlled']
          ) {
            form_set_error("study_info][$type][controlled",
              "Controlled $label Value: field is required.");
          }
          elseif (
            $set['option'] == '2'
            and array_key_exists('uncontrolled', $set)
            and !$set['uncontrolled']
          ) {
            form_set_error("study_info][$type][uncontrolled", "Average $label Value: field is required.");
          }
        }
      }

      if (!empty($form_state['values']['study_info']['rooting'])) {
        $root = $form_state['values']['study_info']['rooting'];
        if (!$root['option']) {
          form_set_error("study_info][rooting][option", t("Rooting Type: field is required."));
        }
        elseif ($root['option'] == 'Soil') {
          if (!$root['soil']['type']) {
            form_set_error('study_info][rooting][soil][type', t('Soil Type: field is required.'));
          }
          elseif ($root['soil']['type'] == 'Other' and !$root['soil']['other']) {
            form_set_error('study_info][rooting][soil][other', t('Custom Soil Type: field is required.'));
          }

          if (!$root['soil']['container']) {
            form_set_error('study_info][rooting][soil][type', t('Soil Container Type: field is required.'));
          }
        }

        $ph = $root['ph'];
        if (!$ph['option']) {
          form_set_error("study_info][rooting][ph][option",
            t("pH controlled or uncontrolled: field is required.")
          );
        }
        elseif (
          $ph['option'] == '1'
          and !$ph['controlled']
        ) {
          form_set_error("study_info][rooting][ph][controlled",
            t("Controlled pH Value: field is required.")
          );
        }
        elseif (
          $ph['option'] == '2'
          and array_key_exists('uncontrolled', $ph)
          and !$ph['uncontrolled']
        ) {
          form_set_error("study_info][rooting][ph][uncontrolled",
            t("Average pH Value: field is required.")
          );
        }

        $selected = FALSE;
        $description = FALSE;

        foreach ($root['treatment'] as $field => $value) {
          if (!$description) {
            $description = TRUE;
            $selected = $value;
            continue;
          }
          elseif ($selected and !$value) {
            form_set_error("study_info][rooting][treatment][$field",
              t("@field: field is required.", array('@field' => $field))
            );
          }
          $description = FALSE;
        }
      }

      if (!empty($form_state['values']['study_info']['irrigation'])) {
        $irrigation = $form_state['values']['study_info']['irrigation'];
        if (!$irrigation['option']) {
          form_set_error('study_info][irrigation][option',
            t('Irrigation Type: field is required.')
          );
        }
        elseif ($irrigation['option'] == 'Other' and !$irrigation['other']) {
          form_set_error('study_info][irrigation][other',
            t('Custom Irrigation Type: field is required.')
          );
        }
      }

      if (
        !empty($form_state['values']['study_info']['biotic_env'])
        and preg_match('/^0+$/', implode('', $form_state['values']['study_info']['biotic_env']['option']))
      ) {
        form_set_error('study_info][biotic_env',
          t('Biotic Environment: field is required.'));
      }
      elseif (
        !empty($form_state['values']['study_info']['biotic_env']['option']['Other'])
        and !$form_state['values']['study_info']['biotic_env']['other']
      ) {
        form_set_error('study_info][biotic_env][other',
          t('Custom Biotic Environment: field is required.'));
      }

      if (!empty($form_state['values']['study_info']['treatment'])
        and $form_state['values']['study_info']['treatment']['check']
      ) {
        $treatment = $form_state['values']['study_info']['treatment'];
        $selected = FALSE;
        $description = FALSE;
        $treatment_empty = TRUE;

        foreach ($treatment as $field => $value) {
          if ($field != 'check') {
            if (!$description) {
              $description = TRUE;
              $selected = $value;
              if ($value) {
                $treatment_empty = FALSE;
              }
              continue;
            }
            elseif ($selected and !$value) {
              form_set_error("study_info][treatment][$field",
                t("@field: field is required.", array('@field' => $field))
              );
            }
            $description = FALSE;
          }
        }

        if ($treatment_empty) {
          form_set_error("study_info][treatment",
            t('Treatment: field is required.')
          );
        }
      }
    }

  }



}
