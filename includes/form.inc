<?php

/**
 * @file
 * Helper functions use at any step of TPPS form.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/form');
 */

/**
 * Adds buttons Back/Save/Next to the forms.
 *
 * The same buttons will be added above and below main form.
 *
 * @param array $form
 *   Drupal Form API array.
 * @param string $page
 *   Page number with 'page_' prefix to improve readability.
 * @param array $meta
 *   Submission metadata.
 *   'organism_number' - int Organism Number;
 *   'data_type' - Contains:
 *      'G' for genotype,
 *      'P' for phenotype,
 *      'E' for environment.
 */
function tpps_add_buttons(array &$form, $page, array $meta = []) {
  $scope_list = [
    'header' => -100,
    'footer' => 100,
  ];

  foreach ($scope_list as $scope => $weight) {
    $form[$scope] = [
      '#type' => 'fieldset',
      '#tree' => FALSE,
      '#weight' => $weight,
    ];
    // @todo [VS] Use Drupal Form API 'required' attribute.
    $form[$scope]['input_description'] = [
      '#markup' => '<div class="input-description">* : Required Field</div>',
      '#weight' => -($weight),
    ];
    switch ($page) {
      case 'page_2':
      case 'page_3':
        $form[$scope]['Back'] = ['#type' => 'submit', '#value' => t('Back')];

      default:
      case 'page_1':
        $form[$scope]['Save'] = ['#type' => 'submit', '#value' => t('Save')];
        $form[$scope]['Next'] = [
          '#type' => 'submit',
          '#value' => t('Next'),
          '#attributes' => ['class' => ['next-button']],
        ];
        break;

      case 'page_4':
        $form[$scope]['Back'] = ['#type' => 'submit', '#value' => t('Back')];
        $form[$scope]['Save'] = ['#type' => 'submit', '#value' => t('Save')];
        $form[$scope]['submit'] = [
          '#type' => 'submit',
          '#value' => t('Review and Submit'),
          // We add class because we have buttons above and below the form.
          '#attributes' => ['class' => ['review-and-submit-button']],
        ];

        // Pre-validate VCF.
        if (!empty($meta['data_type']) && preg_match('/G/', $meta['data_type'])) {
          $pre_valid_states = array();
          for ($i = 1; $i <= $meta['organism_number']; $i++) {
            $pre_valid_states["input[name=\"organism-{$i}[genotype][files][file-type][VCF]\"]"]
              = ['checked' => FALSE];
          }
          $form[$scope]['pre_validate'] = [
            '#type' => 'button',
            '#value' => t('Pre-validate my VCF files'),
            '#states' => ['invisible' => $pre_valid_states],
            '#suffix' => '<div>Sometimes large VCF files can cause problems '
              . 'during the validation step. If your VCF file is very large, '
              . 'you can click the "Pre-validate my VCF files" button to '
              . 'validate your VCF file in the background, before clicking '
              . '"Review Information and Submit".</div>',
            '#attributes' => ['class' => ['vcf-pre-validate-button']],
          ];
          $form[$scope]['pre_validate']['#prefix'] = '<div class="pre-validate-message"></div>';
        }
        break;
    }
  }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Validation.

/**
 * Checks if required field is empty and shows error message.
 *
 * @param array $form_state
 *   Drupal Form API array with submitted values.
 * @param array $path
 *   Path to field form $form_state['complete form'] or $form.
 *   For example:
 *   ['organism-1', 'genotype', 'files', 'marker-type'].
 *
 * @return bool
 *   Returns TRUE if required field is empty and FALSE otherwise.
 */
function tpps_is_required_field_empty(array $form_state, array $path) {
  $field = drupal_array_get_nested_value($form_state['complete form'], $path);
  if (empty($field)) {
    watchdog('tpps', "Field wasn't found in 'complete form'. Path: @path",
      ['@path' => implode('/', $path)], WATCHDOG_ERROR
    );
    return FALSE;
  }
  $value = drupal_array_get_nested_value(
    $form_state['values'], tpps_get_field_path($field, $path)
  );
  if (empty($value)) {
    tpps_form_error_required($form_state, $path);
    return TRUE;
  }
  return FALSE;
}

/**
 * Shows form_validation error message that field is required.
 *
 * Field title will be detected automatically.
 * If field uses '#parents' then full path (not '#parents' must be specified).
 *
 * @param array $form_state
 *   Drupal Form API array with submitted values.
 * @param array $parents
 *   Path to field. For example:
 *   ['organism-1', 'genotype', 'files', 'file-type'].
 */
function tpps_form_error_required(array $form_state, array $parents) {
  tpps_form_error($form_state, $parents, 'field is required.');
}

/**
 * Shows validation error message for given form element.
 *
 * Field's value will not be checked. Use:
 *   tpps_is_required_field_empty() to check value.
 * Field title will be detected automatically.
 *
 * @param array $form_state
 *   Drupal Form API array with submitted values.
 * @param array $path
 *   Path to field from $form_state['complete form'].
 *   (The same will be from $form). Using fields configuration data we could
 *   get all necessary data.
 *   WARNING: Not path from $form_state['values'] or $form_state['input']
 *   which could be different because of using '#parents' and '#tree' options.
 *   For example:
 *   - ['organism-1', 'genotype', 'files', 'file-type'];
 *   - ['publication', 'doi_container', 'doi'];
 *   See https://www.drupal.org/docs/7/api/form-api/tree-and-parents
 * @param string $message
 *   Error message text.
 */
function tpps_form_error(array $form_state, array $path, $message) {
  $field = drupal_array_get_nested_value($form_state['complete form'], $path);
  if (empty($field['#title'])) {
    $message = t("Field didn't found at form. Path: @path",
      ['@path' => implode(' / ', $path)]
    );
    watchdog('tpps', $message, NULL, WATCHDOG_ERROR);
    // drupal_set_message($message, 'error');
    return;
  }
  // Remove colon ':' and required field label.
  $title = strtok($field['#title'], ':');
  $form_path = implode('][', tpps_get_field_path($field, $path));
  form_set_error($form_path,
    t('@title: @message', ['@title' => $title, '@message' => $message])
  );
}

/**
 * Gets field's path from $form.
 *
 * This path could be used to highlight form element when validation failed,
 * to get submitted value ($form_state['values'] or $form_state['input'])
 * but can't be used to get field's configuration from $form['completed form']
 * which has unmodified by '#tree' or '#parents' path.
 *
 * @param array $field
 *   Drupal Form API Field's data.
 * @param array $path
 *   Path to field form $form_state['complete form'] or $form.
 *   For example:
 *   ['organism-1', 'genotype', 'files', 'marker-type'].
 *
 * @return array
 *   Returns array with path to field.
 */
function tpps_get_field_path(array $field, array $path) {
  return $field['#parents'] ?? ($field['#tree'] ? $path : $field['#name']) ?? [];
}

/**
 * Adds the Status Bar to the top of the TPPSc form.
 *
 * The navigation bar will be shown inside form, allows to detect
 * current step and switch to already completed steps (move back).
 *
 * @param array $form
 *   The form being populated.
 * @param array $form_state
 *   The state of the form being populated.
 */
function tpps_form_navigation_bar(array &$form, array $form_state) {
  $current_page = $form_state['stage'] ?? 1;

  // @TODO Each stage has '#cleared' option which indicates that form step
  // passed validation when it's set and value is TRUE. Remove for TPPSc.
  $page_list = [
    '1' => t('Publication Information'),
    '2' => t('Experimental Design'),
    '3' => t('Data'),
    '4' => t('Review and Submit'),
  ];
  foreach ($page_list as $number => $label) {
    $options['query'] = ['page' => $number];
    if ($current_page == $number) {
      $options['attributes']['class'] = ['current_page'];
    }
    $markup = ($markup ?? '') . l($label, 'tppsc', $options);
  }
  $form['navigation_bar'] = [
    '#markup' => '<div class="navigation-bar">' . $markup . '</div>',
    '#weight' => -1000,
  ];
}

/**
 * Get number of organisms.
 *
 * Note: Could be used when TPPS Page 1 submitted.
 *
 * @param array $state
 *   Drupal Form API State array.
 *
 * @return mixed
 *   Returns integer number of organisms.
 *   Returns NULL if TPPS Page 1 wasn't yet submitted.
 */
function tpps_form_get_organism_number(array $state) {
  return ($state['saved_values'][TPPS_PAGE_1]['organism']['number'] ?? NULL);
}

/**
 * Searches for a field of given type in array.
 *
 * @param array $array
 *   Drupal Form API array.
 *   This could be a $form or $form_state['complete form']
 *   or any other form array.
 * @param string $field_type
 *   Type of Form API field to be found in given $array.
 *
 * @return array
 *   Returns list of parents to element of requested type.
 *   Returns empty array if no elements found.
 *   Each item of array is a list of parents which could be used by
 *   drupal_array_get_nested_value() and similar.
 *
 *   Example of returned array:
 *   Array
 *   (
 *        [0] => Array
 *            (
 *                [0] => organism-1
 *                [1] => genotype
 *                [2] => tripal_fasta
 *                [3] => file
 *                [4] => file_upload
 *            )
 *
 *         [1] => Array
 *             (
 *                [0] => organism-1
 *                [1] => genotype
 *                [2] => files
 *                [3] => snps-assay
 *           )
 *     ....
 *   );
 */
function tpps_form_search_field(array $array, $field_type = 'managed_file') {
  $result = [];
  if (empty($field_type)) {
    return $result;
  }
  foreach ($array as $key => $value) {
    $type = ($value['#type'] ?? NULL);
    if (substr($key, 0, 1) == '#' || empty($type)) {
      // Elements with leading '#' in name are for internal use Drupal Form API.
      continue;
    }
    if (in_array($type, ['fieldset', 'container'])) {
      foreach (call_user_func(__FUNCTION__, $value, $field_type) as $item) {
        $result[] = array_merge([$key], $item);
      }
    }
    // Check if element is 'file'.
    elseif ($type == $field_type) {
      $result[] = [$key];
    }
  }
  return $result;
}

/**
 * Adds form fieldset for reports.
 *
 * Used by TPPS Submission Forms.
 *
 * @param array $form
 *   Drupal Form API array.
 * @param array $meta
 *   Report's metadata. Format:
 *   <unique key> => [
 *    'title' => '',
 *    'data' => '',
 *   ].
 */
function tpps_form_add_report(array &$form, array $meta) {
  $form['report'] = [
    '#type' => 'fieldset',
    '#title' => t('Reports'),
    '#collapsed' => FALSE,
    '#collapsible' => FALSE,
    '#weight' => 100,
  ];
  foreach ($meta as $key => $report) {
    $form['report'][($key ?? md5(time))] = [
      '#title' => $report['title'],
      '#type' => 'fieldset',
      '#collapsed' => TRUE,
      '#collapsible' => TRUE,
      'list' => ['#markup' => $report['data']],
    ];
  }
}

/**
 * Sets focus on given field by name.
 *
 * WARNING: Be sure element has no 'autofocus' attribute because it won't work.
 * Timeout is required to have element at page.
 * NOTE: Multistep forms could call this function multiple times for each
 * step but if field not found in $form extra JS code will not be added to page.
 * See js/tpps_autofocus.js file.
 *
 * @param array $form
 *   Drupal Form API Array to be updated.
 * @param string $fieldname
 *   Field name. Use key from $form array. Example: 'tpps_submission_accession'.
 *   When $fieldname is empty we just do not add JS
 *   because it's not critical feature.
 * @param array $parents
 *   Optional and not critical. Allows to remove 'autofocus' attribute.
 *
 * @TODO Minor. Use CSS id intead of name attribute to set focus.
 */
function tpps_form_autofocus(array &$form = [], $fieldname = '', array $parents = []) {
  // Delay in msec.
  $timeout = 1000;
  if (empty($fieldname)) {
    return;
  }
  if ($parents) {
    $path = array_merge($parents, [$fieldname]);
    // We need not Drupal From API Field name but HTML fieldname.
    // Example: 'publication[status]'.
    $html_fieldname = str_replace(
      $parents[0] . ']', $parents[0], implode('][', $path)
    ) . ']';
    $field = drupal_array_get_nested_value($form, $path) ?? NULL;
  }
  else {
    $html_fieldname = $fieldname;
    $field = $form[$fieldname] ?? NULL;
  }
  if ($field && !empty($html_fieldname)) {
    // Do nothing if field not found at form.
    tpps_add_css_js('autofocus', $form);
    $form['#attached']['js'][] = [
      'type' => 'setting',
      'data' => [
        'tpps' => [
          'autoFocus' => [
            'FieldName' => $html_fieldname,
            'Timeout' => $timeout,
          ],
        ],
      ],
      'scope' => 'footer',
      'group' => JS_THEME,
      'weight' => 5,
    ];
    // @TODO Minor. Field could be not on 1st level.
    unset($field['#attributes']['autofocus']);
  }
}

/**
 * Checks if current form is TPPSc form.
 *
 * @param array $form_state
 *   Drupal Form State array.
 *
 * @return bool
 *   Returns TRUE if form Id is 'tppsc_main' and FALSE otherwise.
 */
function tpps_form_is_tppsc(array $form_state) {
  return (bool) (($form_state['build_info']['form_id'] ?? 'tpps_main') == 'tppsc_main');
}

/**
 * Get study type list or one item.
 *
 * Usage example:
 * module_load_include('inc', 'tpps', 'includes/form');
 * $study_type_options = tpps_form_get_study_type($key);
 *
 * @param int $key
 *   Key in the list of options. This value used in form submitters.
 *
 * @return mixed
 *   Returns:
 *   1. $key was specified and index $key exists: translated option label
 *   2. $key was specified but index $key not exists: NULL
 *   3. $key wasn't specified (NULL): whole list of study types/
 */
function tpps_form_get_study_type($key = NULL) {
  $options = [
    0 => t('- Select -'),
    1 => t('Natural Population (Landscape)'),
    2 => t('Growth Chamber'),
    3 => t('Greenhouse'),
    4 => t('Experimental/Common Garden'),
    5 => t('Plantation'),
  ];
  return is_null($key) ? $options : ($options[$key] ?? NULL);
}
